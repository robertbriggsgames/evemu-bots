#include "api/ControlApi.h"
#include "api/Json.h"
#include "api/Metrics.h"
#include "host/BotMgr.h"
#include "org/OrgDirector.h"

#include <cstdlib>
#include <sstream>

namespace EvEmuBots {

static std::string MixJson(const MixWeights& m)
{
    std::ostringstream o;
    o << "{\"miner\":" << m.miner
      << ",\"ratter\":" << m.ratter
      << ",\"industrialist\":" << m.industrialist
      << ",\"hauler\":" << m.hauler
      << ",\"trader\":" << m.trader
      << ",\"explorer\":" << m.explorer
      << ",\"pvper\":" << m.pvper << "}";
    return o.str();
}

static std::string SnapshotJson(const BotSnapshot& s)
{
    std::ostringstream o;
    o << "{\"id\":" << s.characterID
      << ",\"accountID\":" << s.accountID
      << ",\"name\":" << JsonString(s.name)
      << ",\"career\":" << JsonString(s.career)
      << ",\"spBand\":" << JsonString(s.spBand)
      << ",\"security\":" << JsonString(s.security)
      << ",\"activity\":" << JsonString(s.activity)
      << ",\"systemID\":" << s.systemID
      << ",\"corpID\":" << s.corpID
      << ",\"allianceID\":" << s.allianceID
      << ",\"shipTypeID\":" << s.shipTypeID
      << ",\"shipName\":" << JsonString(s.shipName)
      << ",\"walletIsk\":" << s.walletIsk
      << ",\"skillPoints\":" << s.skillPoints
      << ",\"online\":" << (s.online ? "true" : "false")
      << "}";
    return o.str();
}

static HttpResponse JsonOk(const std::string& body)
{
    HttpResponse r;
    r.body = body;
    return r;
}

static HttpResponse JsonErr(int status, const std::string& msg)
{
    HttpResponse r;
    r.status = status;
    r.body = std::string("{\"error\":") + JsonString(msg) + "}";
    return r;
}

HttpResponse HandleApi(const std::string& method, const std::string& path, const std::string& body)
{
    auto& mgr = BotMgr::Instance();

    if (method == "GET" && path == "/metrics") {
        HttpResponse r;
        r.contentType = "text/plain; version=0.0.4; charset=utf-8";
        r.body = RenderPrometheus();
        return r;
    }

    if (method == "GET" && (path == "/v1/health" || path == "/health")) {
        std::ostringstream o;
        o << "{\"ok\":" << (mgr.Ready() ? "true" : "false")
          << ",\"status\":" << JsonString(mgr.Status())
          << ",\"service\":\"evemu-bots\"}";
        return JsonOk(o.str());
    }

    if (method == "GET" && path == "/v1/population") {
        auto pop = mgr.GetPopulation();
        auto& org = OrgDirector::Instance();
        std::ostringstream o;
        o << "{\"targetCount\":" << pop.targetCount
          << ",\"onlineCount\":" << pop.onlineCount
          << ",\"mix\":" << MixJson(pop.mix)
          << ",\"corps\":" << org.CorpCount()
          << ",\"alliances\":" << org.AllianceCount()
          << ",\"wars\":" << org.WarCount()
          << "}";
        return JsonOk(o.str());
    }

    if (method == "PUT" && path == "/v1/population") {
        MixWeights mix = mgr.GetPopulation().mix;
        uint32_t target = mgr.GetPopulation().targetCount;
        bool hasTarget = JsonExtractUint(body, "targetCount", target);
        double v;
        if (JsonExtractDouble(body, "miner", v)) mix.miner = v;
        if (JsonExtractDouble(body, "ratter", v)) mix.ratter = v;
        if (JsonExtractDouble(body, "industrialist", v)) mix.industrialist = v;
        if (JsonExtractDouble(body, "hauler", v)) mix.hauler = v;
        if (JsonExtractDouble(body, "trader", v)) mix.trader = v;
        if (JsonExtractDouble(body, "explorer", v)) mix.explorer = v;
        if (JsonExtractDouble(body, "pvper", v)) mix.pvper = v;
        if (!hasTarget && body.empty())
            return JsonErr(400, "JSON body required");
        mgr.Queue().Enqueue([target, mix]() {
            BotMgr::Instance().SetTargetCount(target);
            BotMgr::Instance().SetMix(mix);
        });
        std::ostringstream o;
        o << "{\"ok\":true,\"queued\":true,\"targetCount\":" << target
          << ",\"mix\":" << MixJson(mix) << "}";
        return JsonOk(o.str());
    }

    if (method == "GET" && path == "/v1/bots") {
        auto roster = mgr.GetRoster();
        std::ostringstream o;
        o << "{\"count\":" << roster.size() << ",\"bots\":[";
        for (size_t i = 0; i < roster.size(); ++i) {
            if (i) o << ",";
            o << SnapshotJson(roster[i]);
        }
        o << "]}";
        return JsonOk(o.str());
    }

    if (method == "GET" && path.compare(0, 9, "/v1/bots/") == 0) {
        uint32_t id = static_cast<uint32_t>(strtoul(path.c_str() + 9, nullptr, 10));
        BotSnapshot snap;
        if (!mgr.GetBot(id, snap))
            return JsonErr(404, "Bot not found");
        return JsonOk(SnapshotJson(snap));
    }

    return JsonErr(404, "Not found");
}

} // namespace EvEmuBots
