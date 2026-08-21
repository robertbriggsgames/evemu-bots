#include "eve-server.h"
#include "ai/IActivity.h"

namespace EvEmuBots {

namespace {

class DockedStub : public IActivity {
public:
    Activity Id() const override { return Activity::Docked; }
    const char* Name() const override { return "docked"; }
    void Tick(Client*) override {}
};

template <Activity A>
class CareerStub : public IActivity {
public:
    Activity Id() const override { return A; }
    const char* Name() const override { return ActivityName(A); }
    void Tick(Client*) override {
        // Later: undock, warp, cycle modules, market, corp, sov.
    }
};

DockedStub g_docked;
CareerStub<Activity::Mining> g_mining;
CareerStub<Activity::Ratting> g_ratting;
CareerStub<Activity::Industry> g_industry;
CareerStub<Activity::Hauling> g_hauling;
CareerStub<Activity::Trading> g_trading;
CareerStub<Activity::Exploring> g_exploring;
CareerStub<Activity::Pvp> g_pvp;

} // namespace

IActivity* ActivityForCareer(Career career)
{
    switch (career) {
        case Career::Miner: return &g_mining;
        case Career::Ratter: return &g_ratting;
        case Career::Industrialist: return &g_industry;
        case Career::Hauler: return &g_hauling;
        case Career::Trader: return &g_trading;
        case Career::Explorer: return &g_exploring;
        case Career::Pvper: return &g_pvp;
        default: return &g_docked;
    }
}

void TickAllActivities(Client* client, Career career)
{
    IActivity* act = ActivityForCareer(career);
    if (act)
        act->Tick(client);
}

} // namespace EvEmuBots
