# Install

The add-on compiles **into** `eve-server`. There is no extra container.

## Prerequisites

- A Crucible tree with the generic modules loader (`modules/` glob in the root CMakeLists, `ModuleHooks` in `eve-server`)
- Docker Compose build of EVEmu as used on this host

## Steps

1. Checkout the loader branch (or merge it) on the Crucible fork.
2. Clone this repo:

   ```bash
   cd /opt/evemu/source/evemu_Crucible
   mkdir -p modules
   git clone https://github.com/robertbriggsgames/evemu-bots.git modules/evemu-bots
   ```

   On the development VM you can symlink the Cursor workspace instead:

   ```bash
   ln -s /home/eveadmin/evemu /opt/evemu/source/evemu_Crucible/modules/evemu-bots
   ```

3. Copy config into the bind-mounted `config/` directory (this overlays `/app/etc` in the container):

   ```bash
   cp modules/evemu-bots/conf/playerbots.conf.dist config/playerbots.conf
   ```

   Set `authToken` to the same value as C2 `BOTS_API_TOKEN`. Leave `targetCount = 0` until you raise it from C2.

4. Rebuild the game image:

   ```bash
   docker compose up -d --build
   ```

5. Confirm the module:

   ```bash
   sudo docker logs server 2>&1 | grep evemu-bots
   ```

   You should see `Ready` and `Control API listening on 0.0.0.0:8091`.

6. Point C2 and Prometheus at `http://server:8091` (see `docs/c2-contract.md` and `deploy/`).

Schema tables `bot_meta` and `bot_org` are created automatically on first start.
