#ifndef EVEMU_BOTS_BOT_TYPES_H
#define EVEMU_BOTS_BOT_TYPES_H

#include <cstdint>
#include <string>
#include <vector>

namespace EvEmuBots {

enum class Career : uint8_t {
    Miner = 0,
    Ratter,
    Industrialist,
    Hauler,
    Trader,
    Explorer,
    Pvper,
    Count
};

enum class SpBand : uint8_t {
    Starter = 0,
    SmallGang,
    Cruiser,
    Battleship,
    Specialist,
    Count
};

enum class SecurityBand : uint8_t {
    High = 0,
    Low,
    Null,
    Count
};

enum class Activity : uint8_t {
    Docked = 0,
    Mining,
    Ratting,
    Industry,
    Hauling,
    Trading,
    Exploring,
    Pvp,
    Count
};

inline const char* CareerName(Career c) {
    switch (c) {
        case Career::Miner: return "miner";
        case Career::Ratter: return "ratter";
        case Career::Industrialist: return "industrialist";
        case Career::Hauler: return "hauler";
        case Career::Trader: return "trader";
        case Career::Explorer: return "explorer";
        case Career::Pvper: return "pvper";
        default: return "unknown";
    }
}

inline const char* SpBandName(SpBand b) {
    switch (b) {
        case SpBand::Starter: return "starter";
        case SpBand::SmallGang: return "small";
        case SpBand::Cruiser: return "cruiser";
        case SpBand::Battleship: return "battleship";
        case SpBand::Specialist: return "specialist";
        default: return "unknown";
    }
}

inline const char* SecurityName(SecurityBand s) {
    switch (s) {
        case SecurityBand::High: return "high";
        case SecurityBand::Low: return "low";
        case SecurityBand::Null: return "null";
        default: return "unknown";
    }
}

inline const char* ActivityName(Activity a) {
    switch (a) {
        case Activity::Docked: return "docked";
        case Activity::Mining: return "mining";
        case Activity::Ratting: return "ratting";
        case Activity::Industry: return "industry";
        case Activity::Hauling: return "hauling";
        case Activity::Trading: return "trading";
        case Activity::Exploring: return "exploring";
        case Activity::Pvp: return "pvp";
        default: return "unknown";
    }
}

inline Career CareerFromName(const std::string& n) {
    if (n == "miner") return Career::Miner;
    if (n == "ratter") return Career::Ratter;
    if (n == "industrialist") return Career::Industrialist;
    if (n == "hauler") return Career::Hauler;
    if (n == "trader") return Career::Trader;
    if (n == "explorer") return Career::Explorer;
    if (n == "pvper") return Career::Pvper;
    return Career::Miner;
}

inline SpBand SpBandFromName(const std::string& n) {
    if (n == "starter") return SpBand::Starter;
    if (n == "small") return SpBand::SmallGang;
    if (n == "cruiser") return SpBand::Cruiser;
    if (n == "battleship") return SpBand::Battleship;
    if (n == "specialist") return SpBand::Specialist;
    return SpBand::Starter;
}

inline SecurityBand SecurityFromName(const std::string& n) {
    if (n == "low") return SecurityBand::Low;
    if (n == "null") return SecurityBand::Null;
    return SecurityBand::High;
}

inline Activity ActivityFromName(const std::string& n) {
    if (n == "mining") return Activity::Mining;
    if (n == "ratting") return Activity::Ratting;
    if (n == "industry") return Activity::Industry;
    if (n == "hauling") return Activity::Hauling;
    if (n == "trading") return Activity::Trading;
    if (n == "exploring") return Activity::Exploring;
    if (n == "pvp") return Activity::Pvp;
    return Activity::Docked;
}

struct MixWeights {
    double miner = 0.22;
    double ratter = 0.18;
    double industrialist = 0.16;
    double hauler = 0.12;
    double trader = 0.12;
    double explorer = 0.08;
    double pvper = 0.12;

    double Get(Career c) const {
        switch (c) {
            case Career::Miner: return miner;
            case Career::Ratter: return ratter;
            case Career::Industrialist: return industrialist;
            case Career::Hauler: return hauler;
            case Career::Trader: return trader;
            case Career::Explorer: return explorer;
            case Career::Pvper: return pvper;
            default: return 0;
        }
    }

    void Set(Career c, double v) {
        switch (c) {
            case Career::Miner: miner = v; break;
            case Career::Ratter: ratter = v; break;
            case Career::Industrialist: industrialist = v; break;
            case Career::Hauler: hauler = v; break;
            case Career::Trader: trader = v; break;
            case Career::Explorer: explorer = v; break;
            case Career::Pvper: pvper = v; break;
            default: break;
        }
    }

    void Normalize() {
        double s = miner + ratter + industrialist + hauler + trader + explorer + pvper;
        if (s <= 0.0) {
            miner = 0.22; ratter = 0.18; industrialist = 0.16;
            hauler = 0.12; trader = 0.12; explorer = 0.08; pvper = 0.12;
            return;
        }
        miner /= s; ratter /= s; industrialist /= s;
        hauler /= s; trader /= s; explorer /= s; pvper /= s;
    }
};

struct BotProfile {
    Career career = Career::Miner;
    SpBand spBand = SpBand::Starter;
    SecurityBand security = SecurityBand::High;
};

struct BotSnapshot {
    uint32_t characterID = 0;
    uint32_t accountID = 0;
    uint32_t corpID = 0;
    int32_t allianceID = 0;
    uint32_t systemID = 0;
    uint32_t shipTypeID = 0;
    double walletIsk = 0;
    uint32_t skillPoints = 0;
    std::string name;
    std::string career;
    std::string spBand;
    std::string security;
    std::string activity;
    std::string shipName;
    bool online = false;
};

struct PopulationState {
    uint32_t targetCount = 0;
    uint32_t onlineCount = 0;
    MixWeights mix;
};

} // namespace EvEmuBots

#endif
