# C2 wiring

Add to `/opt/evemu/c2/docker-compose.yml` environment:

```yaml
BOTS_API_URL: ${BOTS_API_URL:-http://server:8091}
BOTS_API_TOKEN: ${BOTS_API_TOKEN:-changeme}
```

And the matching keys in `/opt/evemu/c2/.env`. Token must equal `authToken` in `playerbots.conf`.

C2 pages:

- `GET /bots` — TraderJoe plus playerbots roster and population controls
- `GET /api/bots` — aggregated JSON
- `PUT /api/bots/population` — proxy to the add-on
