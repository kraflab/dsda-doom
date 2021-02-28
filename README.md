# dsda-doom v0.14.0
This is a fork of prboom+ with extra tooling for demo recording and playback, with a focus on speedrunning.

### Heretic Support (beta)
- DSDA-Doom includes demo-compatible support for heretic (all the demos stored on dsda are in sync).
- Not everything is a 1 to 1 match with vanilla heretic (similar to doom in prboom+) - some of this will be fixed and refined. The game should be identical _mechanically_.
- Heretic game logic should be set automatically if you use `HERETIC.WAD` as the iwad. If it doesn't work, please use the `-heretic` commandline option. This flips a switch in the engine that determines all the core game data.
- You do not need to (and can't) set the complevel when playing heretic. It is implicitly set to `0`, since heretic is based on doom v1.2.
- Setting the "Status Bar and Menu Appearance" option to "not adjusted" will have no effect for heretic (it will default instead to "Doom format").
- The "Apply multisampling" automap option is disabled for heretic.
- Arbitrary music file replacement is currently disabled (even for doom). This will be fixed.
- The automap is partially supported. It's functional but does not have, for example, the parchment background. That being said, many of the extra features added in prboom+ also apply to the heretic automap. Automap colors are not configurable for heretic.
- Some of the more advanced features are not implemented for heretic yet, and using them may cause crashes or other odd behaviour. This includes ghost players, key frames (working but not always in sync), and the analysis code.
- Heretic cheats aren't implemented yet, but you should be able to use the corresponding doom ones.
- Dehacked support for heretic isn't implemented / investigated yet. It's possible some things will work by chance.
- At the end of E2, one of the images requires palette swapping. This is not implemented yet, so the image will look wrong.
- There isn't a separate heretic configuration file - if you'd like to keep different key configurations for each game, you'll have to keep two config files (for now).
- Save files should work but are not compatible with the original.
- Advanced hud is not implemented for heretic.
- Extended hud colors are different due to heretic's different palette and the existing color translation tables. This can be improved eventually.
- The heretic font does not have all the characters that the doom font does. As a result, the prboom+ hud font has been extended to the full character set and is used in various menus.
- Some menus extend over the hud. This will be cleaned up later.
- Any number of things may be broken...please let me know on dw or discord if you run into a problem.

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
- Through the use of automatic and manual key frames, you can now rewind the game.
- The time with tics is now displayed on the intermission screen (top left corner).
- The extended hud provides a hybrid of the classic and advanced huds.
- Use the "restart current map" key while demo recording to perform an in-game restart. Known issue: solo net is not preserved properly.
- You can configure multiple input profiles.
- You can configure multiple keys to one action.

### Changes
- Smart Totals renamed to Max Totals and fixed to show kill constraint for max.
- If there is a demo name clash and overwriting is off: use incrementing name `demo-12345.lmp`.
- Removed the "continue from save slots set in demos" feature. The implementation was confusing and bugged.

### New Settings
- Strict Mode: disable TAS options while recording, unless using `-tas`.
- Cycle Ghost Colors: cycle through the 4 player colors when spawning ghosts.
- Automatic Key Frame Interval (s): time between automatic key frames.
- Automatic Key Frame Depth: how many key frames to store (max rewind length).
- Extended Hud: add extra info on top of the default hud.

### Key Frames
Key frames capture the game state at a given moment, similar to save files. By automatically recording key frames at fixed intervals, it is possible to "rewind" the game. This can be used during normal play, while recording (tas) demos, and during demo playback. You can also set a manual "quick key frame" at a specific point and rewind to that moment at any later time. When storing a key frame while recording, a backup file is created (`backup-ttt.kf`). You can continue a demo _from a key frame_ like so: `-record x.lmp -from_key_frame backup-1234.kf -complevel x`. While recording from a key frame, the demo restart key will return you to the original key frame, even if you have made other key frames later on.

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

### Credits
- The DSDA-Doom icon was designed by Mal (129thVisplane). Thanks!
