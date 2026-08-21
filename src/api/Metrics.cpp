#include "api/Metrics.h"
#include "host/BotMgr.h"
#include "org/OrgDirector.h"

#include <sstream>

namespace EvEmuBots {

static void Gauge(std::ostringstream& o, const char* name, const char* help, double v)
{
    o << "# HELP " << name << " " << help << "\n";
    o << "# TYPE " << name << " gauge\n";
    o << name << " " << v << "\n";
}

std::string RenderPrometheus()
{
    auto& mgr = BotMgr::Instance();
    auto pop = mgr.GetPopulation();
    auto& org = OrgDirector::Instance();

    std::ostringstream o;
    o << "# OpenTelemetry resource: service.name=evemu-bots\n";
    Gauge(o, "evemu_bots_target", "Configured bot population target", pop.targetCount);
    Gauge(o, "evemu_bots_online", "Online playerbots", pop.onlineCount);

    o << "# HELP evemu_bots_by_career Online bots by career\n";
    o << "# TYPE evemu_bots_by_career gauge\n";
    for (int i = 0; i < static_cast<int>(Career::Count); ++i) {
        auto c = static_cast<Career>(i);
        o << "evemu_bots_by_career{career=\"" << CareerName(c) << "\"} "
          << mgr.CountByCareer(c) << "\n";
    }

    o << "# HELP evemu_bots_by_activity Online bots by activity\n";
    o << "# TYPE evemu_bots_by_activity gauge\n";
    for (int i = 0; i < static_cast<int>(Activity::Count); ++i) {
        auto a = static_cast<Activity>(i);
        o << "evemu_bots_by_activity{activity=\"" << ActivityName(a) << "\"} "
          << mgr.CountByActivity(a) << "\n";
    }

    o << "# HELP evemu_bots_by_security Online bots by space security band\n";
    o << "# TYPE evemu_bots_by_security gauge\n";
    for (int i = 0; i < static_cast<int>(SecurityBand::Count); ++i) {
        auto s = static_cast<SecurityBand>(i);
        o << "evemu_bots_by_security{security=\"" << SecurityName(s) << "\"} "
          << mgr.CountBySecurity(s) << "\n";
    }

    Gauge(o, "evemu_bots_corps", "Bot-managed corporations", org.CorpCount());
    Gauge(o, "evemu_bots_alliances", "Bot-managed alliances", org.AllianceCount());
    Gauge(o, "evemu_bots_wars", "Active bot wars", org.WarCount());
    Gauge(o, "evemu_bots_isk_velocity", "ISK moved by bots (stub until economy tick)", mgr.IskVelocity());
    Gauge(o, "evemu_bots_ore_m3", "Ore m3 mined by bots (stub until mining tick)", mgr.OreM3());
    Gauge(o, "evemu_bots_pvp_kills", "Bot PvP kills (stub until PvP tick)", static_cast<double>(mgr.PvpKills()));
    Gauge(o, "evemu_bots_up", "1 if the playerbots module is ready", mgr.Ready() ? 1 : 0);
    return o.str();
}

} // namespace EvEmuBots
