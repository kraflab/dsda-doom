# dsda-doom v0.8.1 (experimental)
This is a fork of prboom+ with extra tooling for dsda.
This is based on the unstable branch of PRBoom+, so there could be bugs - please keep this in mind. :^)

### Experimental Stuff
- Use the "restart current map" key while demo recording to perform an in-game restart.

### New Stuff
- Use `-analysis` to write an analysis.txt file with run details.
- Use `-track_pacifist` to enable a "Not pacifist!" message to appear when breaking the category rules.
- Use `-track_100k` to enable a "100K achieved!" message to appear when reaching 100% kills on a map.
- Use `-time_keys` to show a split when you pick up a key.
- Use `-time_use` to show a split when you press the use key.
- Use `-time_secrets` to show a split when you find a secret.
- Use `-time_all` to enable all the split options.
- Use `-export_ghost ghost` to write a ghost file (`.gst`).
- Use `-import_ghost ghost_a ghost_b ...` to import ghost files (`.gst`).
- Use `-tas` to disable strict mode.

### New Settings
- Strict Mode: disable TAS options while recording, unless using `-tas`.

### Ghosts
A ghost follows the life of the player recorded in the ghost file. This can be useful to compare demos, or to compete against demos while you play. For movies, ghosts that enter the next map ahead of the player will pause until that map is reached. Ghosts that are left behind will fast-forward to the current map.

### Analysis File
This file contains summary data about a run in key-value pairs.
Current contents:

- `skill`, `1` to `5`.
- `nomonsters`, `1` or `0` (-nomonsters parameter).
- `respawn`, `1` or `0` (-respawn parameter).
- `fast`, `1` or `0` (-fast parameter).
- `pacifist`, `1` or `0`.
- `stroller`, `1` or `0`.
- `reality`, `1` or `0` (no damage taken).
- `almost_reality`, `1` or `0` (only nukage damage taken).
- `100k`, `1` or `0` (100% kills as seen on intermission).
- `100s`, `1` or `0` (100% secrets as seen on intermission).
- `missed_monsters`, monsters left alive (not including icon spawns).
- `missed_secrets`, secrets left uncollected.
- `weapon_collector`, `1` or `0` (`0` if no weapons).
- `tyson_weapons`, `1` or `0` (only pistol, chainsaw, and fist used).
- `turbo`, `1` or `0`.
- `category`, `UV Max`, `NoMo`, etc.

### Category Detection
If multiple categories are detected, the first match in this list is chosen:
UV Max, UV Tyson, Stroller, Pacifist, UV Speed

Example: if you complete UV Tyson on a map and collect all the secrets, the analysis will show UV Max.

Use the extra flags (`pacifist`, `stroller`, `tyson_weapons`) to check the details.

Irrelevant categories for a run are ignored. E.g., you won't see `NM 100S` if a map has no secrets and you won't see `Pacifist` if a map has no monsters.

### PRBoom+ Stuff (since 2.5.1.5 - heavily abridged)
- Fix boom autoswitch behaviour (in some cases running out of ammo forced a specific weapon swap)
- Add mouse code option (classic prboom+ vs chocolate doom)
- Forbid 180 while strafe is on (previously could produce sr50 on turns)
- Add configurable quick start window (simulates different hardware speed)
- Include secret exit format in levelstat (E1M3s instead of E1M3)
- Use `-stroller` to prevent strafing and limit speed (-turbo 50)
- Fix boom rng seed (previously this was hardware dependent and not random)
- Add mouse strafe divisor setting - limit horizontal mouse strafe.
