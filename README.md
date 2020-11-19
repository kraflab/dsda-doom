# dsda-doom v0.3.0
This is a fork of prboom+ with extra tooling for dsda.
This is based on the unstable branch of PRBoom+, so there could be bugs - please keep this in mind. :^)

### New Stuff
- Use `-analysis` to write an analysis.txt file with run details.
- Use `-track_pacifist` to enable a "Not pacifist!" message to appear when breaking the category rules.
- Use `-track_100k` to enable a "100K achieved!" message to appear when reaching 100% kills on a map.

### Analysis File
This file contains summary data about a run in key-value pairs.
Current contents:

- `skill`, `1` to `5`.
- `nomonsters`, `1` or `0` (-nomonsters parameter).
- `respawn`, `1` or `0` (-respawn parameter).
- `fast`, `1` or `0` (-fast parameter).
- `pacifist`, `1` or `0`.
- `reality`, `1` or `0` (no damage taken).
- `almost_reality`, `1` or `0` (only nukage damage taken).
- `100k`, `1` or `0` (100% kills as seen on intermission).
- `100s`, `1` or `0` (100% secrets as seen on intermission).
- `missed_monsters`, monsters left alive (not including icon spawns).
- `missed_secrets`, secrets left uncollected.
- `tyson_weapons`, `1` or `0` (only pistol, chainsaw, and fist used).

### PRBoom+ Stuff (since 2.5.1.5 - heavily abridged)
- Fix boom autoswitch behaviour (in some cases running out of ammo forced a specific weapon swap)
- Add mouse code option (classic prboom+ vs chocolate doom)
- Forbid 180 while strafe is on (previously could produce sr50 on turns)
- Add configurable quick start window (simulates different hardware speed)
- Include secret exit format in levelstat (E1M3s instead of E1M3)
- Use `-stroller` to prevent strafing and limit speed (-turbo 50)
- Fix boom rng seed (previously this was hardware dependent and not random)
- Add mouse strafe divisor setting - limit horizontal mouse strafe.
