# C2 contract

C2 must **not** parse eve-server internals for playerbots. It is an HTTP client of this add-on. TraderJoe stays a local C2/XML view.

## Connection

| Setting | Value |
| --- | --- |
| Base URL | `http://server:8091` (docker network `evemu_crucible_default`) |
| Auth | `Authorization: Bearer <token>` on `/v1/*` |
| Metrics | `GET /metrics` — no auth (Prometheus scrape) |
| Env on C2 | `BOTS_API_URL`, `BOTS_API_TOKEN` |

Token must match `authToken` in `playerbots.conf`.

## Endpoints

- `GET /v1/health` — `{ ok, status, service }`
- `GET /v1/population` — `{ targetCount, onlineCount, mix, corps, alliances, wars }`
- `PUT /v1/population` — `{ targetCount, mix: { miner, ratter, ... } }` (queued to main thread)
- `GET /v1/bots` — `{ count, bots: [ { id, name, career, spBand, security, activity, systemID, corpID, allianceID, shipName, walletIsk, skillPoints, online } ] }`
- `GET /v1/bots/{id}` — one bot or 404
- `GET /metrics` — Prometheus text

Full schema: [openapi.yaml](../openapi.yaml)

## Failure

If the add-on is not compiled in, or 8091 is down, C2 shows playerbots as **missing** and keeps serving TraderJoe and the rest of the console.
