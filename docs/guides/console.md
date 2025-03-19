## In-Game Console

The in-game console offers extra tools for advanced users, including adjusting player equipment, reassigning config variables, entering cheats, and running scripts. Feedback from the console is currently limited, but command entry is fully supported.

### Commands

#### Player
- `player.set_health <health>`
- `player.set_armor <armor_points> [armor_type]`
  - `armor_type` defaults to the current type
- `player.give_weapon <weapon>`
- `player.give_ammo <ammo_type> [ammo_amount]`
  - `ammo_amount` defaults to the maximum
- `player.set_ammo <ammo_type> <ammo_amount>`
- `player.give_key <key>`
- `player.remove_key <key>`
- `player.give_power <power> <duration>`
- `player.remove_power <power>`
- `player.set_x <coordinate>`
- `player.set_y <coordinate>`
- `player.set_z <coordinate>`
- `player.round_x`
- `player.round_y`
- `player.round_xy`
- `player.set_angle <angle>`
- `player.round_angle`
- `player.set_vx <velocity>`
- `player.set_vy <velocity>`
- `player.set_vz <velocity>`
- `player.kill`

#### Miscellaneous
- `script.run <script>`
- `check <attribute>`
  - prints config attribute info to the terminal
- `assign <attribute> <value>`
  - temporarily assigns a config value (will not be saved to config file)
- `update <attribute> <value>`
  - permanently updates a config value (will be saved to config file)
- `toggle_assign <attribute>`
  - temporarily toggles a config value (will not be saved to config file)
- `toggle_update <attribute>`
  - permanently toggles a config value (will be saved to config file)
- `config.forget`
  - do not overwrite the config file on exit
- `config.remember`
  - do overwrite the config file on exit
- `wad_stats.forget`
  - do not update wad stats on exit
- `wad_stats.remember`
  - do update wad stats on exit
- `free_text.update <text>`
  - update free text component
- `free_text.clear`
  - clear free text component
- `music.restart`
  - restart the current music track
- `level.exit`
  - exit the current level (go to intermission screen)
- `level.secret_exit`
  - exit the current level via the secret exit (go to intermission screen)
- `game.quit`
  - quit the game immediately (no prompt)
- `game.describe`
  - show the level, skill, and monster params
- `exit`
- `quit`

#### Tracking
- `tracker.add_line / t.al <line_id>`
- `tracker.remove_line / t.rl <line_id>`
- `tracker.add_line_distance / t.ald <line_id>`
- `tracker.remove_line_distance / t.rld <line_id>`
- `tracker.add_sector / t.as <sector_id>`
- `tracker.remove_sector / t.rs <sector_id>`
- `tracker.add_mobj / t.am <mobj_id>`
- `tracker.remove_mobj / t.rm <mobj_id>`
- `tracker.add_player / t.ap`
- `tracker.remove_player / t.rp`
- `tracker.reset / t.r`

#### Thing Manipulation
- States
  - `target.spawn`
    - set the target thing to its spawn state
  - `target.see`
    - set the target thing to its see state
  - `target.pain`
    - set the target thing to its pain state
  - `target.melee`
    - set the target thing to its melee state
  - `target.missile`
    - set the target thing to its missile state
  - `target.death`
    - set the target thing to its death state
  - `target.xdeath`
    - set the target thing to its xdeath state
  - `target.raise`
    - set the target thing to its raise state
  - `target.set_state <state_num>`
    - set the target thing to the given state
- Flags
  - Use named format (e.g., `LOGRAV+SHOOTABLE`)
  - Some flag changes may have unforeseen side effects
  - `target.add_flags <flags>`
    - adds the given flags to the target
  - `target.remove_flags <flags>`
    - removes the given flags from the target
  - `target.set_flags <flags>`
    - sets the target's flags
- `target.set_health <health>`
  - set the target thing's health
- `target.move <x> <y>`
  - move the target to the given coordinates
- `target.set_target <mobj_index>`
  - changes target's target to the mobj with the given index
- `target.target_player`
  - changes target's target to the player
- `mobj.* <mobj_index> [other args]`
  - same as the above commands, but applied to a specific mobj index
- `spawn <x> <y> <z> <type>`
  - spawns a mobj at the given location, with the given type

#### Line Manipulation
- Line activation (use / cross / shoot)
  - `player.activate_line <line_id>`
  - `target.activate_line <line_id>`
  - `mobj.activate_line <mobj_index> <line_id>`
  - `boss.activate_line <mobj_index> <line_id>`
    - activates the line with the boss action flag set

#### State Manipulation
- `state.set_tics <state_id> <value>`
- `state.set_misc1 <state_id> <value>`
- `state.set_misc2 <state_id> <value>`
- `state.set_args1 <state_id> <value>`
- `state.set_args2 <state_id> <value>`
- `state.set_args3 <state_id> <value>`
- `state.set_args4 <state_id> <value>`
- `state.set_args5 <state_id> <value>`
- `state.set_args6 <state_id> <value>`
- `state.set_args7 <state_id> <value>`
- `state.set_args8 <state_id> <value>`

#### Mobj Info Manipulation
- Some changes to mobj info require restarting a map
- `mobjinfo.set_health <type> <value>`
- `mobjinfo.set_radius <type> <value>`
- `mobjinfo.set_height <type> <value>`
- `mobjinfo.set_mass <type> <value>`
- `mobjinfo.set_damage <type> <value>`
- `mobjinfo.set_speed <type> <value>`
- `mobjinfo.set_fast_speed <type> <value>`
- `mobjinfo.set_melee_range <type> <value>`
- `mobjinfo.set_reaction_time <type> <value>`
- `mobjinfo.set_pain_chance <type> <value>`
- `mobjinfo.set_infighting_group <type> <value>`
- `mobjinfo.set_projectile_group <type> <value>`
- `mobjinfo.set_splash_group <type> <value>`
- Flags
  - Use named format (e.g., `LOGRAV+SHOOTABLE`)
  - `mobjinfo.add_flags <type> <value>`
  - `mobjinfo.remove_flags <type> <value>`
  - `mobjinfo.set_flags <type> <value>`

#### Demo Tools
- `jump.to_tic <tic>`
- `jump.by_tic <tic_count>`
- `demo.export <name>`
- `demo.start <name>`
- `demo.stop`
- `demo.join`

#### Build Mode
See the [build mode guide](./build_mode.md) for more info.
- `brute_force.start / bf.start <depth> [<forwardmove_range> <sidemove_range> <angleturn_range>] <conditions>`
- `brute_force.frame / bf.frame <frame> <forwardmove_range> <sidemove_range> <angleturn_range> [<buttons> <weapon>]`
- `build.turbo / b.turbo`
- `mf <value>`
- `mb <value>`
- `sr <value>`
- `sl <value>`
- `tr <value>`
- `tl <value>`
- `fu <value>`
- `fd <value>`
- `fc`
- `lu <value>`
- `ld <value>`
- `lc`
- `ua <value>`

#### Cheats
- `idchoppers`
- `iddqd`
- `idkfa`
- `idfa`
- `idspispopd`
- `idclip`
- `idmypos`
- `idrate`
- `iddt`
- `iddst`
- `iddkt`
- `iddit`
- `idclev <map>`
- `idmus <map>`
- `idbeholdv`
- `idbeholds`
- `idbeholdi`
- `idbeholdr`
- `idbeholda`
- `idbeholdl`
- `skill`
- `tntcomp`
- `tntem`
- `tnthom`
- `tntka`
- `tntsmart`
- `tntpitch`
- `tntfast`
- `tntice`
- `tntpush`
- `notarget`
- `fly`
- `fullclip`
- `freeze`
- `nosleep`
- `allghosts`
- `quicken`
- `ponce`
- `kitty`
- `massacre`
- `rambo`
- `skel`
- `shazam`
- `ravmap`
- `cockadoodledoo`
- `gimme <artifact>`
- `engage <map>`
- `satan`
- `clubmed`
- `butcher`
- `nra`
- `indiana`
- `locksmith`
- `sherlock`
- `casper`
- `init`
- `mapsco`
- `deliverance`
- `shadowcaster <class>`
- `visit <map>`
- `puke <script>`
