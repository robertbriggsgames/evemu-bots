#include "org/OrgDirector.h"

namespace EvEmuBots {

OrgDirector& OrgDirector::Instance()
{
    static OrgDirector d;
    return d;
}

void OrgDirector::Tick()
{
    // Later: form corps from same-region career clusters, alliances from
    // capable corps, wars over resources, sov only in null with military mass.
}

} // namespace EvEmuBots
