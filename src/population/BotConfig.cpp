#include "eve-server.h"
#include "population/BotConfig.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>

namespace EvEmuBots {

static std::string Trim(const std::string& s)
{
    size_t a = 0;
    while (a < s.size() && isspace(static_cast<unsigned char>(s[a])))
        ++a;
    size_t b = s.size();
    while (b > a && isspace(static_cast<unsigned char>(s[b - 1])))
        --b;
    return s.substr(a, b - a);
}

BotConfig& BotConfig::Instance()
{
    static BotConfig cfg;
    return cfg;
}

bool BotConfig::Load()
{
    const char* env = getenv("EVEMU_BOTS_CONF");
    std::vector<std::string> candidates;
    if (env && *env)
        candidates.push_back(env);
    candidates.push_back("../etc/playerbots.conf");
    candidates.push_back("/app/etc/playerbots.conf");
    candidates.push_back("playerbots.conf");

    for (const auto& path : candidates) {
        std::ifstream in(path.c_str());
        if (!in.good())
            continue;
        in.close();
        if (ParseFile(path)) {
            confPath = path;
            mix.Normalize();
            sLog.Green("evemu-bots", "Loaded config %s (target=%u port=%u)",
                       path.c_str(), targetCount, apiPort);
            return true;
        }
    }

    sLog.Warning("evemu-bots", "No playerbots.conf found; using built-in defaults (target=0).");
    mix.Normalize();
    return true;
}

bool BotConfig::ParseFile(const std::string& path)
{
    std::ifstream in(path.c_str());
    if (!in)
        return false;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        std::string t = Trim(line);
        if (t.empty() || t[0] == '#' || t[0] == ';')
            continue;
        auto eq = t.find('=');
        if (eq == std::string::npos)
            continue;
        std::string key = Trim(t.substr(0, eq));
        std::string val = Trim(t.substr(eq + 1));
        if (key == "enabled")
            enabled = (val == "1" || val == "true" || val == "yes");
        else if (key == "targetCount")
            targetCount = static_cast<uint32_t>(strtoul(val.c_str(), nullptr, 10));
        else if (key == "spawnPerTick")
            spawnPerTick = std::max(1u, static_cast<uint32_t>(strtoul(val.c_str(), nullptr, 10)));
        else if (key == "apiPort")
            apiPort = static_cast<uint16_t>(strtoul(val.c_str(), nullptr, 10));
        else if (key == "apiBind")
            apiBind = val;
        else if (key == "authToken")
            authToken = val;
        else if (key == "mix.miner") mix.miner = strtod(val.c_str(), nullptr);
        else if (key == "mix.ratter") mix.ratter = strtod(val.c_str(), nullptr);
        else if (key == "mix.industrialist") mix.industrialist = strtod(val.c_str(), nullptr);
        else if (key == "mix.hauler") mix.hauler = strtod(val.c_str(), nullptr);
        else if (key == "mix.trader") mix.trader = strtod(val.c_str(), nullptr);
        else if (key == "mix.explorer") mix.explorer = strtod(val.c_str(), nullptr);
        else if (key == "mix.pvper") mix.pvper = strtod(val.c_str(), nullptr);
        else if (key == "sp.starter") spStarter = strtod(val.c_str(), nullptr);
        else if (key == "sp.small") spSmall = strtod(val.c_str(), nullptr);
        else if (key == "sp.cruiser") spCruiser = strtod(val.c_str(), nullptr);
        else if (key == "sp.battleship") spBattleship = strtod(val.c_str(), nullptr);
        else if (key == "sp.specialist") spSpecialist = strtod(val.c_str(), nullptr);
        else if (key == "sec.high") secHigh = strtod(val.c_str(), nullptr);
        else if (key == "sec.low") secLow = strtod(val.c_str(), nullptr);
        else if (key == "sec.null") secNull = strtod(val.c_str(), nullptr);
    }
    return true;
}

} // namespace EvEmuBots
