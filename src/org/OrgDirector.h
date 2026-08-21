#ifndef EVEMU_BOTS_ORG_DIRECTOR_H
#define EVEMU_BOTS_ORG_DIRECTOR_H

#include <cstdint>

namespace EvEmuBots {

class OrgDirector {
public:
    static OrgDirector& Instance();

    void Tick();

    uint32_t CorpCount() const { return m_corps; }
    uint32_t AllianceCount() const { return m_alliances; }
    uint32_t WarCount() const { return m_wars; }

private:
    OrgDirector() = default;
    uint32_t m_corps = 0;
    uint32_t m_alliances = 0;
    uint32_t m_wars = 0;
};

} // namespace EvEmuBots

#endif
