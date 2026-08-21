#ifndef EVEMU_BOTS_IACTIVITY_H
#define EVEMU_BOTS_IACTIVITY_H

#include "BotTypes.h"

class Client;

namespace EvEmuBots {

class IActivity {
public:
    virtual ~IActivity() = default;
    virtual Activity Id() const = 0;
    virtual const char* Name() const = 0;
    // First delivery parks bots in station. Career ticks are no-ops until implemented.
    virtual void Tick(Client* client) = 0;
};

IActivity* ActivityForCareer(Career career);
void TickAllActivities(Client* client, Career career);

} // namespace EvEmuBots

#endif
