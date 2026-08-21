#include "eve-server.h"

#include "Client.h"
#include "EntityList.h"

#include <algorithm>

#include "ai/IActivity.h"
#include "host/BotFactory.h"
#include "host/BotMgr.h"
#include "org/OrgDirector.h"
#include "population/BotConfig.h"
#include "population/PopulationPlanner.h"

namespace EvEmuBots {

BotMgr& BotMgr::Instance()
{
    static BotMgr mgr;
    return mgr;
}

void BotMgr::OnServerReady(EVEServiceManager* svc)
{
    m_svc = svc;
    auto& cfg = BotConfig::Instance();
    cfg.Load();
    m_enabled = cfg.enabled;
    m_target = cfg.targetCount;
    m_mix = cfg.mix;
    m_mix.Normalize();

    if (!BotDB::EnsureSchema()) {
        m_status = "schema_failed";
        sLog.Error("evemu-bots", "Schema init failed; module idle.");
        return;
    }

    std::vector<BotRecord> all;
    BotDB::LoadAll(all);
    for (auto& rec : all)
        m_offline.push_back(rec);

    m_ready = true;
    m_status = m_enabled ? "ready" : "disabled";
    sLog.Green("evemu-bots", "Ready. target=%u stored=%zu enabled=%s",
               m_target, m_offline.size(), m_enabled ? "yes" : "no");
}

void BotMgr::SetTargetCount(uint32_t n)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_target = n;
    BotConfig::Instance().targetCount = n;
}

void BotMgr::SetMix(const MixWeights& mix)
{
    MixWeights m = mix;
    m.Normalize();
    std::lock_guard<std::mutex> lock(m_mutex);
    m_mix = m;
    BotConfig::Instance().mix = m;
}

PopulationState BotMgr::GetPopulation() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    PopulationState st;
    st.targetCount = m_target;
    st.onlineCount = static_cast<uint32_t>(m_online.size());
    st.mix = m_mix;
    return st;
}

BotSnapshot BotMgr::SnapshotLocked(const BotBrain& brain) const
{
    BotSnapshot s;
    s.characterID = brain.rec.characterID;
    s.accountID = brain.rec.accountID;
    s.name = brain.rec.career;
    s.career = brain.rec.career;
    s.spBand = brain.rec.spBand;
    s.security = brain.rec.securityBand;
    s.activity = ActivityName(brain.activity);
    s.skillPoints = brain.skillPoints;
    s.online = brain.client != nullptr;
    if (brain.client != nullptr) {
        s.name = brain.client->GetName();
        s.corpID = static_cast<uint32_t>(brain.client->GetCorporationID());
        s.allianceID = brain.client->GetAllianceID();
        s.systemID = brain.client->GetSystemID();
        s.walletIsk = brain.client->GetBalance();
        if (brain.client->GetShip().get() != nullptr) {
            s.shipTypeID = brain.client->GetShip()->typeID();
            s.shipName = brain.client->GetShip()->itemName();
        }
    }
    return s;
}

void BotMgr::RefreshLiveStats(BotBrain& brain)
{
    if (brain.client == nullptr || brain.client->GetChar().get() == nullptr)
        return;
    DBQueryResult res;
    if (sDatabase.RunQuery(res, "SELECT skillPoints FROM chrCharacters WHERE characterID=%u", brain.rec.characterID)) {
        DBResultRow row;
        if (res.GetRow(row))
            brain.skillPoints = row.GetUInt(0);
    }
}

std::vector<BotSnapshot> BotMgr::GetRoster() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<BotSnapshot> out;
    out.reserve(m_online.size());
    for (const auto& kv : m_online)
        out.push_back(SnapshotLocked(*kv.second));
    return out;
}

bool BotMgr::GetBot(uint32_t characterID, BotSnapshot& out) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_online.find(characterID);
    if (it == m_online.end())
        return false;
    out = SnapshotLocked(*it->second);
    return true;
}

uint32_t BotMgr::CountByCareer(Career c) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    uint32_t n = 0;
    const char* name = CareerName(c);
    for (const auto& kv : m_online)
        if (kv.second->rec.career == name)
            ++n;
    return n;
}

uint32_t BotMgr::CountByActivity(Activity a) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    uint32_t n = 0;
    for (const auto& kv : m_online)
        if (kv.second->activity == a)
            ++n;
    return n;
}

uint32_t BotMgr::CountBySecurity(SecurityBand s) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    uint32_t n = 0;
    const char* name = SecurityName(s);
    for (const auto& kv : m_online)
        if (kv.second->rec.securityBand == name)
            ++n;
    return n;
}

bool BotMgr::SpawnOne()
{
    if (m_svc == nullptr)
        return false;

    BotRecord rec;
    Client* client = nullptr;

    if (!m_offline.empty()) {
        rec = m_offline.back();
        if (sEntityList.IsOnline(rec.characterID)) {
            m_offline.pop_back();
            return false;
        }
        if (!BotFactory::ResumeBot(*m_svc, rec, client) || client == nullptr)
            return false;
        m_offline.pop_back();
    } else {
        BotProfile profile;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            profile = PopulationPlanner::Roll(m_mix);
        }
        if (!BotFactory::CreateNewBot(*m_svc, profile, rec, client) || client == nullptr)
            return false;
    }

    auto brain = std::make_unique<BotBrain>();
    brain->client = client;
    brain->rec = rec;
    brain->profile.career = CareerFromName(rec.career);
    brain->profile.spBand = SpBandFromName(rec.spBand);
    brain->profile.security = SecurityFromName(rec.securityBand);
    brain->activity = Activity::Docked;
    RefreshLiveStats(*brain);

    std::lock_guard<std::mutex> lock(m_mutex);
    m_online[rec.characterID] = std::move(brain);
    return true;
}

void BotMgr::DespawnOne()
{
    uint32_t charID = 0;
    Client* client = nullptr;
    BotRecord rec;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_online.empty())
            return;
        auto it = m_online.begin();
        charID = it->first;
        client = it->second->client;
        rec = it->second->rec;
        m_online.erase(it);
    }
    rec.activity = "docked";
    m_offline.push_back(rec);
    BotDB::UpdateActivity(charID, "docked");
    if (client != nullptr)
        client->CloseClientConnection();
}

void BotMgr::ReconcilePopulation()
{
    if (!m_enabled || m_svc == nullptr)
        return;

    const uint32_t budget = BotConfig::Instance().spawnPerTick;
    uint32_t online = 0;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        online = static_cast<uint32_t>(m_online.size());
    }

    if (online < m_target) {
        uint32_t n = std::min(budget, m_target - online);
        for (uint32_t i = 0; i < n; ++i) {
            if (!SpawnOne())
                break;
        }
    } else if (online > m_target) {
        uint32_t n = std::min(budget, online - m_target);
        for (uint32_t i = 0; i < n; ++i)
            DespawnOne();
    }
}

void BotMgr::OnTick()
{
    m_queue.DrainOnMainThread();
    if (!m_ready)
        return;

    ReconcilePopulation();
    OrgDirector::Instance().Tick();

    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& kv : m_online) {
        BotBrain& brain = *kv.second;
        if (brain.client == nullptr)
            continue;
        TickAllActivities(brain.client, brain.profile.career);
    }
}

void BotMgr::OnShutdown()
{
    std::vector<Client*> toClose;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& kv : m_online) {
            if (kv.second->client)
                toClose.push_back(kv.second->client);
            m_offline.push_back(kv.second->rec);
        }
        m_online.clear();
        m_ready = false;
        m_status = "stopped";
    }
    for (Client* c : toClose)
        c->CloseClientConnection();
}

} // namespace EvEmuBots
