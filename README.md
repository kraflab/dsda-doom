# dsda-doom v0.19.2
This is a fork of prboom+ with extra tooling for demo recording and playback, with a focus on speedrunning.

### Patch Notes
- [v0.19](./patch_notes/v0.19.md)

### Heretic Support (beta)
- DSDA-Doom includes demo-compatible support for heretic (all the demos stored on dsda are in sync).
- Heretic game logic should be set automatically if you use `HERETIC.WAD` as the iwad. If it doesn't work, please use the `-heretic` commandline option. This flips a switch in the engine that determines all the core game data.
- The prboom+ launcher doesn't work properly with heretic.
- You do not need to (and can't) set the complevel when playing heretic. It is implicitly set to `0`, since heretic is based on doom v1.2.
- Setting the "Status Bar and Menu Appearance" option to "not adjusted" will have no effect for heretic (it will default instead to "Doom format").
- The "Apply multisampling" automap option is disabled for heretic.
- Automap colors are not configurable for heretic.
- Some of the more advanced features are not implemented for heretic yet, and using them may cause crashes or other odd behaviour. This includes ghost players, key frames (working but not always in sync), and the analysis code.
- Dehacked support for heretic isn't implemented yet.
- Advanced hud is not implemented for heretic.
- Some menus extend over the hud. This will be cleaned up later.

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
- Use `-first_input f s t` to "build" the first command of a tas, where `f = forwardmove`, `s = sidemove`, and `t = angleturn`.
  - Example: `-first_input 50 50 32` will do a quarter turn left and sr50 forward right.
- Through the use of automatic and manual key frames, you can now rewind the game.
- The time with tics is now displayed on the intermission screen (top left corner).
- The extended hud provides a hybrid of the classic and advanced huds.
- Use the "restart current map" key while demo recording to perform an in-game restart. Known issue: solo net is not preserved properly.
- You can configure multiple input profiles.
- You can configure multiple keys to one action.
- You can bind (many) cheats.
- You can cycle through alternate palettes (for testing purposes).
  The cycle starts with `PLAYPAL`, followed by `PLAYPAL1` to `PLAYPAL9`.
- Also see patch notes.

### Changes
- Smart Totals renamed to Max Totals and fixed to show kill constraint for max.
- If there is a demo name clash and overwriting is off: use incrementing name `demo-12345.lmp`.
- Removed the "continue from save slots set in demos" feature. The implementation was confusing and bugged.
- Mouse sensitivity: multiply your pr+ value by 1.6 in order to get the same result. This change was done to create a 1:1 relationship between mouse input and the turning value created in the game. See "Fine Sensitivity" in the settings below to handle fractions.
- Vertical mouse movement and mouse strafing carry fractional values, so you eventually move even at a very slow input speed.
- Mouse scales show integer values and have no limits.
- Also see patch notes.

### New Settings
- Strict Mode: disable TAS options while recording, unless using `-tas`.
- Cycle Ghost Colors: cycle through the 4 player colors when spawning ghosts.
- Automatic Key Frame Interval (s): time between automatic key frames.
- Automatic Key Frame Depth: how many key frames to store (max rewind length).
- Extended Hud: add extra info on top of the default hud.
- Wipe At Full Speed: always display the wipe animation at normal speed.
- Track Demo Attempts: show the attempt count (session / total) when starting a demo.
- Fine Sensitivity: hundredths of a point adjustment to horizontal mouse sensitivity.
- Also see patch notes.

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

### Credits
- The DSDA-Doom icon was designed by Mal (129thVisplane). Thanks!
