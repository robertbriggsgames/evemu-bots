#ifndef EVEMU_BOTS_POPULATION_PLANNER_H
#define EVEMU_BOTS_POPULATION_PLANNER_H

#include "BotTypes.h"

namespace EvEmuBots {

class PopulationPlanner {
public:
    static BotProfile Roll(const MixWeights& mix);
    static Activity DefaultActivity(Career career);
};

} // namespace EvEmuBots

#endif
