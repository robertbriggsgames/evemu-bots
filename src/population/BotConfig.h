#ifndef EVEMU_BOTS_BOT_CONFIG_H
#define EVEMU_BOTS_BOT_CONFIG_H

#include "BotTypes.h"

#include <cstdint>
#include <string>

namespace EvEmuBots {

class BotConfig {
public:
    static BotConfig& Instance();

    bool Load();

    bool enabled = true;
    uint32_t targetCount = 0;
    uint32_t spawnPerTick = 1;
    uint16_t apiPort = 8091;
    std::string apiBind = "0.0.0.0";
    std::string authToken = "changeme";
    std::string confPath;
    MixWeights mix;

    // SP band weights (must sum ~1 after normalize)
    double spStarter = 0.35;
    double spSmall = 0.30;
    double spCruiser = 0.20;
    double spBattleship = 0.12;
    double spSpecialist = 0.03;

    // Security placement
    double secHigh = 0.70;
    double secLow = 0.20;
    double secNull = 0.10;

private:
    BotConfig() = default;
    bool ParseFile(const std::string& path);
};

} // namespace EvEmuBots

#endif
