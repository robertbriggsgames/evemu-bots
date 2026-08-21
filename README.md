# evemu-bots

In-process **playerbots add-on** for [EVEmu Crucible](https://github.com/EvEmu-Project/evemu_Crucible). It fills the cluster with server-managed characters that are real `Client` sessions — the same idea as AzerothCore `mod-playerbots`.

This repository is the add-on. It is not a fork of EVEmu. Clone it into `modules/evemu-bots` on a core that has the generic modules loader, rebuild, and the bots compile into `eve-server`. Delete the directory and rebuild to go back to vanilla.

## What it does today (v0.1)

- Spawns and parks bots **in station** with careers, SP bands, and security placement that make sense
- Runtime population via HTTP (`PUT /v1/population`) for the C2 console
- Roster (`GET /v1/bots`) for C2
- Prometheus `/metrics` for Grafana
- Activity, corp, alliance, war, and sov planners as **interfaces** (ticks are stubs)

TraderJoe, the in-tree market bot, is unchanged.

## Add

Requires the `modules-loader` hook on your Crucible fork (generic `modules/` CMake + `OnServerReady` / `OnTick` / `OnShutdown`).

```bash
cd /opt/evemu/source/evemu_Crucible
mkdir -p modules
git clone https://github.com/robertbriggsgames/evemu-bots.git modules/evemu-bots
cp modules/evemu-bots/conf/playerbots.conf.dist config/playerbots.conf
# set authToken to match C2 BOTS_API_TOKEN
docker compose up -d --build
```

The control API listens on **8091 inside the `server` container**. Do not publish it to the LAN. C2 and Prometheus reach it on the `evemu_crucible_default` Docker network.

## Remove

```bash
# runtime: drain population
curl -H "Authorization: Bearer changeme" \
  -X PUT http://127.0.0.1:8091/v1/population \
  -d '{"targetCount":0}'   # only works if you temporarily publish 8091; prefer C2

# fully remove
rm -rf /opt/evemu/source/evemu_Crucible/modules/evemu-bots
cd /opt/evemu/source/evemu_Crucible
docker compose up -d --build
```

Setting `enabled = false` in `playerbots.conf` (or `targetCount = 0`) parks everyone without a rebuild.

## Layout

- `src/host/` — dummy TCP, spawn/resume, main-thread command queue
- `src/api/` — HTTP control plane and Prometheus
- `src/population/` — career / SP / security mix
- `src/ai/` — `IActivity` plugins (stubs)
- `src/org/` — corp / alliance / war / sov director (stub)
- `docs/` — install, C2 contract, careers
- `deploy/` — Prometheus scrape snippet and Grafana dashboard

LGPL-3.0-or-later, matching EVEmu.
