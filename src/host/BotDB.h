#ifndef EVEMU_BOTS_BOT_DB_H
#define EVEMU_BOTS_BOT_DB_H

#include "BotTypes.h"

#include <string>
#include <vector>

namespace EvEmuBots {

struct BotRecord {
    uint32_t characterID = 0;
    uint32_t accountID = 0;
    std::string career;
    std::string spBand;
    std::string securityBand;
    std::string activity;
};

class BotDB {
public:
    static bool EnsureSchema();
    static bool Insert(const BotRecord& rec);
    static bool UpdateActivity(uint32_t characterID, const std::string& activity);
    static bool LoadAll(std::vector<BotRecord>& out);
    static bool NameTaken(const std::string& name);
    static bool AccountExists(const std::string& accountName);
};

} // namespace EvEmuBots

#endif
