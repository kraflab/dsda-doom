# dsda-doom v0.3.0
This is a fork of prboom+ with extra tooling for dsda.
This is based on the unstable branch of PRBoom+, so there could be bugs - please keep this in mind. :^)

### New Stuff
- Use `-analysis` to write an analysis.txt file with run details.
- Use `-track_pacifist` to enable a "Not pacifist!" message to appear when breaking the category rules.

### Analysis File
This file contains summary data about a run in key-value pairs.
Current contents:

- `pacifist`, `1` or `0`.
- `reality`, `1` or `0` (no damage taken).
- `almost_reality`, `1` or `0` (only nukage damage taken).
- `missed_monsters`, monsters left alive (not including icon spawns).
- `missed_secrets`, secrets left uncollected.

### PRBoom+ Stuff (since 2.5.1.5 - heavily abridged)
- Fix boom autoswitch behaviour (in some cases running out of ammo forced a specific weapon swap)
- Add mouse code option (classic prboom+ vs chocolate doom)
- Forbid 180 while strafe is on (previously could produce sr50 on turns)
- Add configurable quick start window (simulates different hardware speed)
- Include secret exit format in levelstat (E1M3s instead of E1M3)
- Use `-stroller` to prevent strafing and limit speed (-turbo 50)
- Fix boom rng seed (previously this was hardware dependent and not random)
- Add mouse strafe divisor setting - limit horizontal mouse strafe.
