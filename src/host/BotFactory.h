#ifndef EVEMU_BOTS_BOT_FACTORY_H
#define EVEMU_BOTS_BOT_FACTORY_H

#include "BotTypes.h"
#include "host/BotDB.h"

class Client;
class EVEServiceManager;

namespace EvEmuBots {

class BotFactory {
public:
    static Client* CreateOnlineClient(EVEServiceManager& svc);
    static bool CreateNewBot(EVEServiceManager& svc, const BotProfile& profile, BotRecord& rec, Client*& client);
    static bool ResumeBot(EVEServiceManager& svc, const BotRecord& rec, Client*& client);
    static void ConfigureSession(Client* client, uint32_t accountID);
};

} // namespace EvEmuBots

#endif
