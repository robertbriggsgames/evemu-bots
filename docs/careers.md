# Careers and “making sense”

Bots are rolled from config weights, not cloned as identical Titans.

## Careers

| Career | Default activity stub | Typical space |
| --- | --- | --- |
| miner | mining | highsec |
| ratter | ratting | high / low / some null |
| industrialist | industry | highsec |
| hauler | hauling | highsec |
| trader | trading | highsec |
| explorer | exploring | mixed |
| pvper | pvp | low / null, fewer highsec |

v0.1 parks everyone **docked**. `IActivity::Tick` is the hook for later undock/warp/module cycles.

## SP bands

starter / small / cruiser / battleship / specialist (tiny tail). Extra skill levels are applied on top of racial starting skills (capped at 5). Specialist miners/haulers are flattened to battleship so industry alts are not all capital pilots.

## Orgs (planned)

`OrgDirector` will form corps when enough same-region bots share a career, alliances when corps have numbers and a doctrine, wars over resources, and sovereignty only in null with military mass. Counts are exported as `evemu_bots_corps` / `_alliances` / `_wars` (zero until that tick exists).

## Economy (planned)

Miners sell ore, industrialists buy and produce, traders move goods, ratters inject loot ISK, PvPers consume ships. `evemu_bots_isk_velocity` and `evemu_bots_ore_m3` are gauges reserved for that work.
