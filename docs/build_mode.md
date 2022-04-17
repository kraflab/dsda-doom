## Build Mode

*Build Mode is currently in an alpha state! There may be bugs! Back up your demos!*

Building is the process of editing a demo frame-by-frame, in order to make highly optimized TASes, or to deal with specific, difficult sections in segmented TASes.

### Input Explanation

- Toggle Build Mode: enter or exit build mode at any time.
- Advance Frame: moves forward one frame, using the pending command.
- Reverse Frame: go back one frame.
- Reset Command: sets the pending command empty (a wait tic). If your command source is an existing buffer, this will not change the command that gets executed.
- Toggle Source: by default, the command you edit in build mode is the one that gets sent to the game when you advance a frame. You can toggle the source to play back commands from the demo buffer instead. If you are past the end of any existing demo buffer, wait tics will be played.
- Fine Movement: adjusts movement commands by 1 unit.
- Turn values and the use action are reset every time the frame advances. Other values are kept.

### Time

- You can freely use the quick key frames and automatic key framing / rewind feature while in build mode.
- To jump to a specific tic, you can use the console command `jump.tic X`. If `X` is negative, it is relative to your current tic. Positive values are absolute from the start of the demo.
- All typical skip features are also available (skip to end of map, etc).

### Trackers

- You can display extra data about different aspects of the game state using trackers.
- `tracker.addline X` / `t.al X`: track the special value of line X.
- `tracker.addsector X` / `t.as X`: track the special value, active state, and floor height of sector X.
- `tracker.addmobj X` / `t.am X`: track the health of mobj X.
- `tracker.addplayer` / `t.ap`: track the last damage dealt by the player.
- You can remove trackers with the same commands, substituting `remove` / `r` for `add` / `a`.
