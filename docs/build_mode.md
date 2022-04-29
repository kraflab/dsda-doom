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

### Brute Force

Brute force is a technique in built tases where you automatically apply different sequences of commands until you reach a desired outcome. A common use case is for performing glides. You can activate brute force from the console.

- `bruteforce.start / bf.start depth forward_range strafe_range turn_range conditions`
  - `depth` is the number of tics you want to brute force (limit 5)
  - `forward_range` is the range of values for forwardmove. Format: `40,50`, `-50,-40`, etc.
  - `strafe_range` is the same as `forward_range`, but for strafe values.
  - `turn_range` is the same as `forward_range`, but for turn values.
  - `conditions` are comma separated. Format: `attribute operator value`.
    - `attribute` has the following options:
      - `x` (player x position)
      - `y` (player y position)
      - `z` (player z position)
      - `vx` (player x velocity)
      - `vy` (player y velocity)
      - `spd` (player speed)
      - `dmg` (single frame player damage dealt)
      - `rng` (rng index)
    - `operator` has the following options:
      - `lt` (less than)
      - `lteq` (less than or equal to)
      - `gt` (greater than)
      - `gteq` (greater than or equal to)
      - `eq` (equal)
      - `neq` (not equal)
      - These operators look for a best result (by checking all sequences):
        - `acap` (as close as possible)
        - `max` (maximum)
        - `min` (minimum)
    - `value` is the number to compare against (currently limited to integer).
      - `max` and `min` do not require a value
    - Examples:
      - `x gt 4, y lt -34, vx gteq 10` means the player x position must be greater than 4, and the player y position must be less than -34, and the player x velocity must be greater than or equal to 10, all at the same time.
      - `y eq 0, vy acap 0` means the player y position must be equal to 0. All sequences will be checked, and the one that leads to a player y velocity closest to 0 will be used.
  - Full example: `bf.start 2 40,50 40,50 -2,2 x lt 1056, vx gt 5` is a depth 2 brute force, with possible forward and strafe values ranging from 40 to 50, possible turn values ranging from -2 to 2, and with the condition that x is less than 1056 and x velocity is greater than 5.
- Brute force metadata gets printed to the console (conditions, progress, etc).
