# Add or remove

## Runtime (no rebuild)

| Action | How |
| --- | --- |
| Park all bots | `PUT /v1/population` with `{"targetCount":0}` from C2 |
| Change mix | `PUT /v1/population` with `mix` weights |
| Disable API + spawning | `enabled = false` in `playerbots.conf` and restart `server` |

Characters remain in MariaDB (`bot_meta`) so a later target increase **resumes** them instead of creating duplicates.

## Compile-out (vanilla server)

```bash
rm -rf /opt/evemu/source/evemu_Crucible/modules/evemu-bots
cd /opt/evemu/source/evemu_Crucible
docker compose up -d --build
```

With an empty `modules/` directory the core still builds. `ModuleHooks` is a no-op.

Do **not** delete `bot_meta` rows unless you also want the bot accounts/characters gone. The add-on never drops human accounts.
