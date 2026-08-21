#include "eve-server.h"

#include "api/HttpServer.h"
#include "host/BotMgr.h"
#include "population/BotConfig.h"
#include "services/ServiceManager.h"

extern "C" void EvEmu_Module_OnServerReady(EVEServiceManager* svc)
{
    auto& cfg = BotConfig::Instance();
    BotMgr::Instance().OnServerReady(svc);
    if (cfg.enabled) {
        if (!HttpServer::Instance().Start(cfg.apiBind, cfg.apiPort, cfg.authToken))
            sLog.Error("evemu-bots", "Failed to bind control API on %s:%u",
                       cfg.apiBind.c_str(), cfg.apiPort);
    }
}

extern "C" void EvEmu_Module_OnTick()
{
    BotMgr::Instance().OnTick();
}

extern "C" void EvEmu_Module_OnShutdown()
{
    HttpServer::Instance().Stop();
    BotMgr::Instance().OnShutdown();
}
