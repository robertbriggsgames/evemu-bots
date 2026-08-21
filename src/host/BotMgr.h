#ifndef EVEMU_BOTS_BOT_MGR_H
#define EVEMU_BOTS_BOT_MGR_H

#include "BotTypes.h"
#include "host/BotDB.h"
#include "host/CommandQueue.h"

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

class Client;
class EVEServiceManager;

namespace EvEmuBots {

struct BotBrain {
    Client* client = nullptr;
    BotRecord rec;
    BotProfile profile;
    Activity activity = Activity::Docked;
    uint32_t skillPoints = 0;
};

class BotMgr {
public:
    static BotMgr& Instance();

    void OnServerReady(EVEServiceManager* svc);
    void OnTick();
    void OnShutdown();

    CommandQueue& Queue() { return m_queue; }

    void SetTargetCount(uint32_t n);
    void SetMix(const MixWeights& mix);
    PopulationState GetPopulation() const;
    std::vector<BotSnapshot> GetRoster() const;
    bool GetBot(uint32_t characterID, BotSnapshot& out) const;

    bool Ready() const { return m_ready; }
    const char* Status() const { return m_status.c_str(); }

    uint32_t CountByCareer(Career c) const;
    uint32_t CountByActivity(Activity a) const;
    uint32_t CountBySecurity(SecurityBand s) const;

    double IskVelocity() const { return m_iskVelocity; }
    double OreM3() const { return m_oreM3; }
    uint64_t PvpKills() const { return m_pvpKills; }

private:
    BotMgr() = default;
    void ReconcilePopulation();
    bool SpawnOne();
    void DespawnOne();
    BotSnapshot SnapshotLocked(const BotBrain& brain) const;
    void RefreshLiveStats(BotBrain& brain);

    EVEServiceManager* m_svc = nullptr;
    CommandQueue m_queue;
    mutable std::mutex m_mutex;
    std::unordered_map<uint32_t, std::unique_ptr<BotBrain>> m_online;
    std::vector<BotRecord> m_offline;
    MixWeights m_mix;
    uint32_t m_target = 0;
    uint32_t m_spawnBudget = 0;
    bool m_ready = false;
    bool m_enabled = false;
    std::string m_status = "starting";
    double m_iskVelocity = 0;
    double m_oreM3 = 0;
    uint64_t m_pvpKills = 0;
};

} // namespace EvEmuBots

#endif
