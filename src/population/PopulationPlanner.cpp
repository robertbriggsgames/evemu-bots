#include "population/PopulationPlanner.h"
#include "population/BotConfig.h"

#include <random>

namespace EvEmuBots {

static thread_local std::mt19937 rng{std::random_device{}()};

static double NextUnit()
{
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng);
}

static Career PickCareer(const MixWeights& mix)
{
    double r = NextUnit();
    double acc = 0;
    for (int i = 0; i < static_cast<int>(Career::Count); ++i) {
        auto c = static_cast<Career>(i);
        acc += mix.Get(c);
        if (r <= acc)
            return c;
    }
    return Career::Miner;
}

static SpBand PickSp()
{
    const auto& cfg = BotConfig::Instance();
    double total = cfg.spStarter + cfg.spSmall + cfg.spCruiser + cfg.spBattleship + cfg.spSpecialist;
    if (total <= 0)
        return SpBand::Starter;
    double r = NextUnit() * total;
    if ((r -= cfg.spStarter) <= 0) return SpBand::Starter;
    if ((r -= cfg.spSmall) <= 0) return SpBand::SmallGang;
    if ((r -= cfg.spCruiser) <= 0) return SpBand::Cruiser;
    if ((r -= cfg.spBattleship) <= 0) return SpBand::Battleship;
    return SpBand::Specialist;
}

static SecurityBand PickSec(Career career)
{
    const auto& cfg = BotConfig::Instance();
    // Industry lives mostly in highsec; PvP and null ratting lean out.
    double high = cfg.secHigh, low = cfg.secLow, nul = cfg.secNull;
    if (career == Career::Miner || career == Career::Industrialist || career == Career::Hauler || career == Career::Trader) {
        high += 0.15; nul *= 0.4;
    } else if (career == Career::Pvper) {
        high *= 0.4; low += 0.15; nul += 0.15;
    } else if (career == Career::Ratter) {
        high *= 0.7; nul += 0.1;
    }
    double total = high + low + nul;
    double r = NextUnit() * total;
    if ((r -= high) <= 0) return SecurityBand::High;
    if ((r -= low) <= 0) return SecurityBand::Low;
    return SecurityBand::Null;
}

BotProfile PopulationPlanner::Roll(const MixWeights& mix)
{
    MixWeights m = mix;
    m.Normalize();
    BotProfile p;
    p.career = PickCareer(m);
    p.spBand = PickSp();
    p.security = PickSec(p.career);
    // Specialist Titans in highsec industry do not make sense.
    if (p.spBand == SpBand::Specialist && (p.career == Career::Miner || p.career == Career::Hauler))
        p.spBand = SpBand::Battleship;
    if (p.security == SecurityBand::Null && p.spBand == SpBand::Starter)
        p.spBand = SpBand::SmallGang;
    return p;
}

Activity PopulationPlanner::DefaultActivity(Career career)
{
    switch (career) {
        case Career::Miner: return Activity::Mining;
        case Career::Ratter: return Activity::Ratting;
        case Career::Industrialist: return Activity::Industry;
        case Career::Hauler: return Activity::Hauling;
        case Career::Trader: return Activity::Trading;
        case Career::Explorer: return Activity::Exploring;
        case Career::Pvper: return Activity::Pvp;
        default: return Activity::Docked;
    }
}

} // namespace EvEmuBots
