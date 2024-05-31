## Build Mode

*Build Mode is currently in an alpha state! There may be bugs! Back up your demos!*

Building is the process of editing a demo frame-by-frame, in order to make highly optimized TASes, or to deal with specific, difficult sections in segmented TASes.

### Input Explanation

- Toggle Build Mode: enter or exit build mode at any time.
- Advance Frame: moves forward one frame, using the pending command.
- Reverse Frame: go back one frame.
- Reset Command: sets the pending command empty (a wait tic). If your command source is an existing buffer, this will not change the command that gets executed.
- Toggle Source: switches between editing the command and replaying the existing command.
- Fine Movement: adjusts movement commands by 1 unit.
- Turn values and the use action are reset every time the frame advances. Other values are kept.
- Use `build.turbo` / `b.turbo` in the console to toggle turbo on and off (or launch with `-turbo`).
- You can also set values with `mf X`, `mb X`, `sr X`, `sl X`, `tr X`, and `tl X` in the console.
- The raven games also support `fu X` / `fd X` / `fc` (fly up / down / center), `lu X` / `ld X` / `lc` (look up / down / center), and `ua X` (use artifact).

### Time

- You can freely use the quick key frames and automatic key framing / rewind feature while in build mode.
- To jump to a specific tic, you can use the console command `jump.tic X`. If `X` is negative, it is relative to your current tic. Positive values are absolute from the start of the demo.
- All typical skip features are also available (skip to end of map, etc).

### Trackers

- You can display extra data about different aspects of the game state using trackers.
  - `tracker.add_line X` / `t.al X`: track the special value of line X.
  - `tracker.add_line_distance X` / `t.ald X`: track the distance from line X.
  - `tracker.add_sector X` / `t.as X`: track the special value, active state, and floor height of sector X.
  - `tracker.add_mobj X` / `t.am X`: track the health of mobj X.
  - `tracker.add_player` / `t.ap`: track the last damage dealt by the player.
  - `tracker.reset` / `t.r`: removes all trackers
- You can remove trackers with the same commands, substituting `remove` / `r` for `add` / `a`.
- You can also populate these through command line arguments:
  - `-track_line a [b c ...]`
  - `-track_line_distance a [b c ...]`
  - `-track_sector a [b c ...]`
  - `-track_mobj a [b c ...]`
  - `-track_player`

### Brute Force

Brute force is a technique in built tases where you automatically apply different sequences of commands until you reach a desired outcome. A common use case is for performing glides. You can activate brute force from the console.

- `brute_force.frame / bf.frame frame forward_range strafe_range turn_range [buttons weapon]`
  - `frame` is the frame number from the start of brute force (0, 1, 2...)
  - `forward_range` is the range of values for forwardmove. Format: `40:50`, `-50:-40`, etc.
  - `strafe_range` is the same as `forward_range`, but for strafe values.
  - `turn_range` is the same as `forward_range`, but for turn values.
  - `buttons` is any combination of `a`, `u`, and `c` (attack, use, change weapon).
  - `weapon` is the number for the weapon to change to if `c` is given as a button.
- `brute_force.keep / bf.keep frame`
  - Keeps the existing command on the given frame
- `brute_force.nomonsters / bf.nomo`
  - Performs a faster brute force by ignoring monster activity (may desync)
  - Use `brute_force.monsters` / `bf.mo` to reset to the regular brute force mode
- `brute_force.start / bf.start depth [forward_range strafe_range turn_range] conditions`
  - Ranges are optional and will override frame-specific instructions
  - `depth` is the number of tics you want to brute force (limit 35)
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
      - `arm` (player armor)
      - `hp` (player health)
      - `am0` (player bullets / crystals / blue mana)
      - `am1` (player shells / bolts / green mana)
      - `am2` (player cells / claw orbs)
      - `am3` (player rockets / runes)
      - `am4` (player flame orbs)
      - `am5` (player spheres)
      - `bmw` (blockmap width)
    - `operator` has the following options:
      - `<` (less than)
      - `<=` (less than or equal to)
      - `>` (greater than)
      - `>=` (greater than or equal to)
      - `==` (equal)
      - `!=` (not equal)
      - These operators look for a best result (by checking all sequences):
        - `acap` (as close as possible)
        - `max` (maximum)
        - `min` (minimum)
    - `value` is the number to compare against (currently limited to integer).
      - `max` and `min` do not require a value
    - Examples:
      - `x gt 4, y < -34, vx >= 10` means the player x position must be greater than 4, and the player y position must be less than -34, and the player x velocity must be greater than or equal to 10, all at the same time.
      - `y == 0, vy acap 0` means the player y position must be equal to 0. All sequences will be checked, and the one that leads to a player y velocity closest to 0 will be used.
  - There are additional conditions that do not follow the above pattern:
    - `skip X` (skip line X)
    - `act X` (activate line X)
    - `have X` (have item X)
      - `rkc`, `bkc`, `ykc`, `rsk`, `bsk`, `ysk` (keys)
      - `sg`, `cg`, `rl`, `pg`, `bfg`, `cs`, `ssg` (weapons)
    - `lack X` (do not have item X)
      - `rkc`, `bkc`, `ykc`, `rsk`, `bsk`, `ysk` (keys)
      - `sg`, `cg`, `rl`, `pg`, `bfg`, `cs`, `ssg` (weapons)
  - Basic example: `bf.start 2 40:50 40:50 -2:2 x < 1056, vx > 5`
    - This is a depth 2 brute force, with possible forward and strafe values ranging from 40 to 50, possible turn values ranging from -2 to 2, and with the condition that x is less than 1056 and x velocity is greater than 5.
  - Frame-specific example (different ranges on different frames):
    ```c
    bf.frame 0 40:50 40:50 -2:2
    bf.frame 1 50:50 50:50 -10:10
    bf.frame 2 40:50 40:50 -2:2
    bf.start 3 x < 1056, vx > 5
    ```
- Brute force metadata gets printed to the console (conditions, progress, etc).
