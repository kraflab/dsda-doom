# MBF21 Current Status

These are changes / features that are currently implemented.

Tracked [here](https://trello.com/b/qyrnGsFs/mbf21).

**This is NOT a spec.**

This is proof-of-concept implemented in dsda-doom.

## Sectors

#### Instant death sector special
- [Implementation](https://github.com/kraflab/dsda-doom/blob/07639e2f1834c6d6ae5a37c720e01d52c2c95d4d/prboom2/src/p_spec.c#L2437-L2463)
- Bit 12 (4096) turns on "alternate damage meaning" for bit 5 & 6:

| Dec | Bit 6-5 | Description                                                   |
|-----|---------|---------------------------------------------------------------|
| 0   | 00      | Kills a player unless they have a rad suit or invulnerability |
| 32  | 01      | Kills a player                                                |
| 64  | 10      | Kills all players and exits the map (normal exit)             |
| 96  | 11      | Kills all players and exits the map (secret exit)             |

#### Kill monsters sector special
- [PR](https://github.com/kraflab/dsda-doom/pull/18)
- Bit 13 turns on "kill monsters" flag for sectors - kills grounded monsters.

## Lines

#### Block land monsters line flag
- [PR](https://github.com/kraflab/dsda-doom/pull/19)
- Uses bit 12 (4096).

#### Block players line flag
- [commit](https://github.com/kraflab/dsda-doom/commit/687237e3d236056730f58dca27efd45e1774d53e)
- Uses bit 13 (8192).

#### Line scroll special variants
- [PR](https://github.com/kraflab/dsda-doom/pull/29)
- Scroll like special 255, but the special line determines the speed / direction with which all tagged lines scroll.
- 1024 is without control sector / acceleration.
- 1025 uses control sector.
- 1026 uses control sector + acceleration.

## Things

#### Dehacked Thing Groups
- [PR](https://github.com/kraflab/dsda-doom/pull/22), [PR](https://github.com/kraflab/dsda-doom/pull/23)

##### Infighting
- Add `Infighting group = N` in Thing definition.
- `N` is a nonnegative integer.
- Things with the same value of `N` will not target each other after taking damage.

##### Projectile
- Add `Projectile group = M` in Thing definition.
- `M` is an integer.
- Things with the same value of `M` will not deal projectile damage to each other.
- A negative value of `M` means that species has no projectile immunity, even to other things in the same species.

##### Splash
- Add `Splash group = S` in Thing definition.
- `S` is a nonnegative integer.
- If an explosion (or other splash damage source) has the same splash group as the target, it will not deal damage.

##### Examples
```
Thing 12 (Imp)
Projectile group = -1
Splash group = 0

Thing 16 (Baron of Hell)
Projectile group = 2

Thing 18 (Hell Knight)
Infighting group = 1
Projectile group = 1

Thing 21 (Arachnotron)
Infighting group = 1
Projectile group = 2

Thing 31 (Barrel)
Splash group = 0
```

In this example:
- Imp projectiles now damage other Imps (and they will infight with their own kind).
- Barons and Arachnotrons are in the same projectile group: their projectiles will no longer damage each other.
- Barons and Hell Knights are not in the same projectile group: their projectiles will now damage each other, leading to infighting.
- Hell Knights and Arachnotrons are in the same infighting group: they will not infight with each other, despite taking damage from each other's projectiles.
- Imps and Barrels are in the same splash group: barrels no longer deal splash damage to imps.
- Note that the group numbers are separate - being in infighting group 1 doesn't mean you are in projectile group 1.

#### New Thing Flags
- [commit](https://github.com/kraflab/dsda-doom/commit/10907e5d37dc2337c93f6dd59573fd42c5a8aaf6)
- Add `MBF21 Bits = X` in the Thing definition.
- The format is the same as the existing `Bits` field.
- Example: `MBF21 Bits = LOGRAV+DMGIGNORED+MAP07BOSS1`.
- Implementations match between DSDA-Doom and Eternity Engine for labeled flags.

| DSDA-Doom          | Eternity Engine    | Description                                                                                    |
|--------------------|--------------------|------------------------------------------------------------------------------------------------|
| MF2_LOGRAV         | MF2_LOGRAV         | Lower gravity (1/8)                                                                            |
| MF2_SHORTMRANGE    | MF2_SHORTMRANGE    | Short missile range (archvile)                                                                 |
| MF2_DMGIGNORED     | MF3_DMGIGNORED     | Other things ignore its attacks (archvile)                                                     |
| MF2_NORADIUSDMG    | MF4_NORADIUSDMG    | Doesn't take splash damage (cyberdemon, mastermind)                                            |
| MF2_FORCERADIUSDMG | MF4_FORCERADIUSDMG | Thing causes splash damage even if the target shouldn't                                        |
| MF2_HIGHERMPROB    | MF2_HIGHERMPROB    | Higher missile attack probability (cyberdemon)                                                 |
| MF2_RANGEHALF      | MF2_RANGEHALF      | Use half distance for missile attack probability (cyberdemon, mastermind, revenant, lost soul) |
| MF2_NOTHRESHOLD    | MF3_NOTHRESHOLD    | Has no targeting threshold (archvile)                                                          |
| MF2_LONGMELEE      | MF2_LONGMELEE      | Has long melee range (revenant)                                                                |
| MF2_BOSS           | MF2_BOSS           | Full volume see / death sound & splash immunity (cyberdemon, mastermind)                       |
| MF2_MAP07BOSS1     | MF2_MAP07BOSS1     | Tag 666 "boss" on doom 2 map 7 (mancubus)                                                      |
| MF2_MAP07BOSS2     | MF2_MAP07BOSS2     | Tag 667 "boss" on doom 2 map 7 (arachnotron)                                                   |
| MF2_E1M8BOSS       | MF2_E1M8BOSS       | E1M8 boss (baron)                                                                              |
| MF2_E2M8BOSS       | MF2_E2M8BOSS       | E2M8 boss (cyberdemon)                                                                         |
| MF2_E3M8BOSS       | MF2_E3M8BOSS       | E3M8 boss (mastermind)                                                                         |
| MF2_E4M6BOSS       | MF2_E4M6BOSS       | E4M6 boss (cyberdemon)                                                                         |
| MF2_E4M8BOSS       | MF2_E4M8BOSS       | E4M8 boss (mastermind)                                                                         |
| MF2_RIP            | MF3_RIP            | Ripper projectile (does not disappear on impact)                                               |

#### Rip sound
- [commit](https://github.com/kraflab/dsda-doom/commit/3d9fc1cccc7b85c527331e74802dd25d94a80b10)
- When set, this is the sound that plays for ripper projectiles when they rip through something.
- Add `Rip sound = X` in the Thing definition.
- `X` is the sound index, as seen in other sound fields.

#### Fast speed
- [PR](https://github.com/kraflab/dsda-doom/pull/37), [commit](https://github.com/kraflab/dsda-doom/commit/ad8304b7df2a6fde2c26f6241eb40e00e954cb58)
- Sets the thing speed for skill 5 / -fast.
- Add `Fast speed = X` in the Thing definition.
- `X` has the same units as the normal `Speed` field.

#### Melee range
- [PR](https://github.com/kraflab/dsda-doom/pull/46)
- Sets the range at which a monster will initiate a melee attack.
- Also affects the range for vanilla melee attack codepointers, e.g. A_SargAttack, A_TroopAttack, etc.
  - Similarly, adjusting the player mobj's melee range will adjust the range of A_Punch and A_Saw
- Add `Melee range = X` in the Thing definition.
- `X` is in fixed point, like the Radius and Height fields

## Weapons

#### Weapon Flags
- [PR](https://github.com/kraflab/dsda-doom/pull/27)
- Add `MBF21 Bits = X` in the Weapon definition.
- The format is the same as the existing thing `Bits` field.
- Example: `MBF21 Bits = SILENT+NOAUTOFIRE`.

| Name           | Description                                      |
|----------------|--------------------------------------------------|
| NOTHRUST       | Doesn't thrust things                            |
| SILENT         | Weapon is silent                                 |
| NOAUTOFIRE     | Weapon won't autofire when swapped to            |
| FLEEMELEE      | Monsters consider it a melee weapon              |
| AUTOSWITCHFROM | Can be switched away from when ammo is picked up |
| NOAUTOSWITCHTO | Cannot be switched to when ammo is picked up     |

MBF21 defaults:

| Weapon          | Flags                                   |
|-----------------|-----------------------------------------|
| Fist            | FLEEMELEE+AUTOSWITCHFROM+NOAUTOSWITCHTO |
| Pistol          | AUTOSWITCHFROM                          |
| Shotgun         |                                         |
| Chaingun        |                                         |
| Rocket Launcher | NOAUTOFIRE                              |
| Plasma Rifle    |                                         |
| BFG             | NOAUTOFIRE                              |
| Chainsaw        | NOTHRUST+FLEEMELEE+NOAUTOSWITCHTO       |
| Super Shotgun   |                                         |

#### Ammo pickup weapon autoswitch changes
- [PR](https://github.com/kraflab/dsda-doom/pull/26)
- Weapon autoswitch on ammo pickup now accounts for the ammo per shot of a weapon, as well as the `NOAUTOSWITCHTO` and `AUTOSWITCHFROM` weapon flags, allowing more accuracy and customization of this behaviour.
- If the current weapon is enabled for `AUTOSWITCHFROM` and the player picks up ammo for a different weapon, autoswitch will occur for the highest ranking weapon (by index) matching these conditions:
  - player has the weapon
  - weapon is not flagged with `NOAUTOSWITCHTO`
  - weapon uses the ammo that was picked up
  - player did not have enough ammo to fire the weapon before
  - player now has enough ammo to fire the weapon

#### New DEHACKED "Ammo per shot" Weapon field
- [PR](https://github.com/kraflab/dsda-doom/pull/24)
- Add `Ammo per shot = X` in the Weapon definition.
- Value must be a nonnegative integer.
- Tools should assume this value is undefined for all vanilla weapons (i.e. always write it to the patch if the user specifies any valid value)
- Weapons WITH this field set will use the ammo-per-shot value when:
  - Checking if there is enough ammo before firing
  - Determining if the weapon has ammo during weapon auto-switch
  - Deciding how much ammo to subtract in native Doom weapon attack pointers
    - Exceptions: A_Saw and A_Punch will never attempt to subtract ammo.
  - The `amount` param is zero for certain new MBF21 DEHACKED codepointers (see below).
- Weapons WITHOUT this field set will use vanilla Doom semantics for all above behaviors.
- For backwards-compatibility, setting the `BFG cells/shot` misc field will also set the BFG weapon's `Ammo per shot` field (but not vice-versa).

## Frames

#### Frame Flags
- [PR](https://github.com/kraflab/dsda-doom/pull/37)
- Add `MBF21 Bits = X` in the Frame definition.
- The format is the same as the existing thing `Bits` field.
- Example: `MBF21 Bits = SKILL5FAST`.

| Name       | Description                           |
|------------|---------------------------------------|
| SKILL5FAST | Tics halve on nightmare skill (demon) |

#### New "Args" fields for DEHACKED states
- [PR](https://github.com/kraflab/dsda-doom/pull/30)
- Defines 8 new integer fields in the state table for use as codepointer arguments
- Args are defined in dehacked by adding `Args1 = X`, `Args2 = X`... up to `Args8 = X` in the State definition.
- Default value for every arg is 0
- For future-proofing, if more nonzero args are defined on a state than its action pointer expects (e.g. defining Args3 on a state that uses A_WeaponSound), an error will be thrown on startup.

#### New DEHACKED Codepointers
- [PR](https://github.com/kraflab/dsda-doom/pull/20), [PR](https://github.com/kraflab/dsda-doom/pull/38), [PR](https://github.com/kraflab/dsda-doom/pull/40), [PR](https://github.com/kraflab/dsda-doom/pull/41), [PR](https://github.com/kraflab/dsda-doom/pull/45)
- All new MBF21 pointers use the new "Args" fields for params, rather than misc1/misc2 fields
- Arg fields are listed in order in the docs below, e.g. for `A_SpawnObject`, `type` is Args1, `angle` is Args2, etc.
- Although all args are integers internally, there are effectively the following types of args:
  - `int`: An integer. 'Nuff said.
  - `uint`: An unsigned integer. Must be greater than or equal to zero.
  - `fixed` A [fixed-point](https://doomwiki.org/wiki/Fixed_point) number.
    - For user-friendliness, a suggested convention for MBF21-supporting DEHACKED tools is to convert numbers with decimal points to their fixed-point representation when saving the patch (e.g. "1.0" gets saved as "65536"), so this doesn't get super-ugly on the user side of life.

##### Actor pointers

- **A_SpawnObject(type, angle, x_ofs, y_ofs, z_ofs, x_vel, y_vel, z_vel)**
  - Generic actor spawn function.
  - Args:
    - `type (uint)`: Type (dehnum) of actor to spawn
    - `angle (fixed)`: Angle (degrees), relative to calling actor's angle
    - `x_ofs (fixed)`: X (forward/back) spawn position offset
    - `y_ofs (fixed)`: Y (left/right) spawn position offset
    - `z_ofs (fixed)`: Z (up/down) spawn position offset
    - `x_vel (fixed)`: X (forward/back) velocity
    - `y_vel (fixed)`: Y (left/right) velocity
    - `z_vel (fixed)`: Z (up/down) velocity
  - Notes:
    - If the spawnee is a missile, the `tracer` and `target` pointers are set as follows:
      - If spawner is also a missile, `tracer` and `target` are copied to spawnee
      - Otherwise, spawnee's `target` is set to the spawner and `tracer` is set to the spawner's `target`.

- **A_MonsterProjectile(type, angle, pitch, hoffset, voffset)**
  - Generic monster projectile attack.
  - Args:
    - `type (uint)`: Type (dehnum) of actor to spawn
    - `angle (fixed)`: Angle (degrees), relative to calling actor's angle
    - `pitch (fixed)`: Pitch (degrees), relative to calling actor's pitch
    - `hoffset (fixed)`: Horizontal spawn offset, relative to calling actor's angle
    - `voffset (fixed)`: Vertical spawn offset, relative to actor's default projectile fire height
  - Notes:
    - The `pitch` arg uses the same approximated pitch calculation that Doom's monster aim / autoaim uses. Refer to the implementation for specifics.
    - The spawned projectile's `tracer` pointer is always set to the spawner's `target`, for generic seeker missile support.

- **A_MonsterBulletAttack(hspread, vspread, numbullets, damagebase, damagedice)**
  - Generic monster bullet attack.
  - Args:
    - `hspread (fixed)`: Horizontal spread (degrees, in fixed point)
    - `vspread (fixed)`: Vertical spread (degrees, in fixed point)
    - `numbullets (int)`: Number of bullets to fire; if not set, defaults to 1
    - `damagebase (int)`: Base damage of attack; if not set, defaults to 3
    - `damagedice (int)`: Attack damage random multiplier; if not set, defaults to 5
  - Notes:
    - Damage formula is: `damage = (damagebase * random(1, damagedice))`
    - Damage arg defaults are identical to Doom's usual monster bullet attack values.
    - Entering a negative value for `numbullets`, `damagebase`, and `damagedice` is undefined behavior (for now).

- **A_MonsterMeleeAttack(damagebase, damagedice, sound, range)**
  - Generic monster melee attack.
  - Args:
    - `damagebase (int)`: Base damage of attack; if not set, defaults to 3
    - `damagedice (int)`: Attack damage random multiplier; if not set, defaults to 8
    - `sound (uint)`: Sound to play if attack hits
    - `range (fixed)`: Attack range; if not set, defaults to calling actor's melee range property
  - Notes:
    - Damage formula is: `damage = (damagebase * random(1, damagedice))`

- **A_RadiusDamage(damage, radius)**
  - Generic A_Explode (hell yeah).
  - Args:
    - `damage (int)`: Max explosion damge
    - `radius (int)`: Explosion radius

- **A_NoiseAlert**
  - Alerts monsters within sound-travel distance of the calling actor's target.
  - No Args.

- **A_HealChase(state, sound)**
  - Generic A_VileChase. Chases the player and resurrects any corpses with a Raise state when bumping into them.
  - Args:
    - `state (uint)`: State to jump to on the calling actor when resurrecting a corpse
    - `sound (uint)`: Sound to play when resurrecting a corpse

- **A_SeekTracer(threshold, maxturnangle)**
  - Generic seeker missile function.
  - Args:
    - `threshold (fixed)`: If angle to target is lower than this, missile will 'snap' directly to face the target
    - `maxturnangle (fixed)`: Maximum angle a missile will turn towards the target if angle is above the threshold
  - Notes:
    - When using this function, keep in mind that seek 'strength' also depends on how often this function is called -- e.g. calling `A_SeekTracer` every tic will result in a much more aggressive seek than calling it every 4 tics, even if the args are the same
    - This function uses Heretic's seeker missile logic (P_SeekerMissile), rather than Doom's, with two notable changes:
      - The actor's `tracer` pointer is used as the seek target, rather than Heretic's `special1` field
      - On the z-axis, the missile will seek towards the vertical centerpoint of the seek target, rather than the bottom (resulting in much friendly behavior when seeking toward enemies on high ledges). Refer to the implementation for details.

- **A_FindTracer(fov, distance)**
  - Searches for a valid tracer (seek target), if the calling actor doesn't already have one. Particularly useful for player missiles.
  - Args:
    - `fov (fixed)`: Field-of-view, relative to calling actor's angle, to search for targets in. If zero, the search will occur in all directions.
    - `distance (int)`: Distance to search, in map blocks (128 units); if not set, defaults to 10. Setting this value too high may hurt performance, so don't get overzealous.
  - Notes:
    - This function is intended for use on player-fired missiles; it may not produce particularly useful results if used on a monster projectile.
    - This function will no-op if the calling actor already has a tracer. To forcibly re-acquire a new tracer, call A_ClearTracer first.
    - This function uses a variant of Hexen's blockmap search algorithm (P_RoughMonsterSearch); refer to the implementation for specifics, but it's identical to Hexen's except for the rules for picking a valid target (in order of evaluation):
      - Actors without the SHOOTABLE flag are skipped
      - The projectile's owner (target) is skipped
      - Actors on the same "team" are skipped (e.g. MF_FRIEND actors will not pick players or fellow MF_FRIENDs), with a few exceptions:
        - If actors are infighting (i.e. candidate actor is projectile owner's target), actor will not be skipped
        - Players will not be skipped in deathmatch
          - This rule is to work around a weird quirk in MBF (namely, that players have MF_FRIEND set even in DM); ports with a robust "team" implementation may be able to do a smarter check here in general.
      - If the `fov` arg is nonzero, actors outside of an `fov`-degree cone, relative to the missile's angle, are skipped
      - Actors not in line-of-sight of the missile are skipped

- **A_ClearTracer**
  - Clears the calling actor's tracer (seek target) field.
  - No Args.

- **A_JumpIfHealthBelow(state, health)**
  - Jumps to a state if caller's health is below the specified threshold.
  - Args:
    - `state (uint)`: State to jump to
    - `health (int)`: Health to check
  - Notes:
    - The jump will only occur if health is BELOW the given value -- e.g. `A_JumpIfHealthBelow(420, 69)` will jump to state 420 if health is 68 or lower.

- **A_JumpIfTargetInSight(state)**
  - Jumps to a state if caller's target is in line-of-sight.
  - Args:
    - `state (uint)`: State to jump to
  - Notes:
    - This function only considers line-of-sight, i.e. independent of calling actor's facing direction.

- **A_JumpIfTargetCloser(state, distance)**
  - Jumps to a state if caller's target is closer than the specified distance.
  - Args:
    - `state (uint)`: State to jump to
    - `distance (int)`: Distance threshold, in map units
  - Notes:
    - This function uses the same approximate distance check as other functions in the game (P_AproxDistance).
    - The jump will only occur if distance is BELOW the given value -- e.g. `A_JumpIfTargetCloser(420, 69)` will jump to state 420 if distance is 68 or lower.

- **A_JumpIfTracerInSight(state)**
  - Jumps to a state if caller's tracer (seek target) is in line-of-sight.
  - Args:
    - `state (uint)`: State to jump to
  - Notes:
    - This function only considers line-of-sight, i.e. independent of calling actor's facing direction.

- **A_JumpIfTracerCloser(state, distance)**
  - Jumps to a state if caller's tracer (seek target) is closer than the specified distance.
  - Args:
    - `state (uint)`: State to jump to
    - `distance (fixed)`: Distance threshold, in map units
  - Notes:
    - This function uses the same approximate distance check as other functions in the game (P_AproxDistance).
    - The jump will only occur if distance is BELOW the given value -- e.g. `A_JumpIfTracerCloser(420, 69)` will jump to state 420 if distance is 68 or lower.

- **A_JumpIfFlagsSet(state, flags, flags2)**
  - Jumps to a state if caller has the specified thing flags set.
  - Args:
    - `state (uint)`: State to jump to.
    - `flags (int)`: Standard actor flag(s) to check
    - `flags2 (int)`: MBF21 actor flag(s) to check
  - Notes:
    - If multiple flags are specified in a field, jump will only occur if all the flags are set (e.g. AND comparison, not OR)

- **A_AddFlags(flags, flags2)**
  - Adds the specified thing flags to the caller.
  - Args:
    - `flags (int)`: Standard actor flag(s) to add
    - `flags2 (int)`: MBF21 actor flag(s) to add

- **A_RemoveFlags(flags, flags2)**
  - Removes the specified thing flags from the caller.
  - Args:
    - `flags (int)`: Standard actor flag(s) to remove
    - `flags2 (int)`: MBF21 actor flag(s) to remove

##### Weapon pointers

- **A_WeaponProjectile(type, angle, pitch, hoffset, voffset)**
  - Generic weapon projectile attack.
  - Args:
    - `type (uint)`: Type (dehnum) of actor to spawn
    - `angle (fixed)`: Angle (degrees), relative to player's angle
    - `pitch (fixed)`: Pitch (degrees), relative to player's pitch
    - `hoffset (fixed)`: Horizontal spawn offset, relative to player's angle
    - `voffset (fixed)`: Vertical spawn offset, relative to player's default projectile fire height
  - Notes:
    - Unlike native Doom attack codepointers, this function will not consume ammo, trigger the Flash state, or play a sound.
    - The `pitch` arg uses the same approximated pitch calculation that Doom's monster aim / autoaim uses. Refer to the implementation for specifics.
    - The spawned projectile's `tracer` pointer is set to the player's autoaim target, if available.

- **A_WeaponBulletAttack(hspread, vspread, numbullets, damagebase, damagedice)**
  - Generic weapon bullet attack.
  - Args:
    - `hspread (fixed)`: Horizontal spread (degrees, in fixed point)
    - `vspread (fixed)`: Vertical spread (degrees, in fixed point)
    - `numbullets (int)`: Number of bullets to fire; if not set, defaults to 1
    - `damagebase (int)`: Base damage of attack; if not set, defaults to 5
    - `damagedice (int)`: Attack damage random multiplier; if not set, defaults to 3
  - Notes:
    - Unlike native Doom attack codepointers, this function will not consume ammo, trigger the Flash state, or play a sound.
    - Damage formula is: `damage = (damagebase * random(1, damagedice))`
    - Damage arg defaults are identical to Doom's usual monster bullet attack values.
      - Note that these defaults are different for weapons and monsters (5d3 vs 3d5) -- yup, Doom did it this way. :P

- **A_WeaponMeleeAttack(damagebase, damagedice, zerkfactor, sound, range)**
  - Generic weapon melee attack.
  - Args:
    - `damagebase (int)`: Base damage of attack; if not set, defaults to 2
    - `damagedice (int)`: Attack damage random multiplier; if not set, defaults to 10
    - `zerkfactor (fixed)`: Berserk damage multiplier; if not set, defaults to 1.0
    - `sound (uint)`: Sound to play if attack hits
    - `range (fixed)`: Attack range; if not set, defaults to player mobj's melee range property
  - Notes:
    - Damage formula is: `damage = (damagebase * random(1, damagedice))`; this is then multiplied by `zerkfactor` if the player has Berserk.

- **A_WeaponSound(sound, fullvol)**
  - Generic playsound for weapons.
  - Args:
    - `sound (uint)`: DEH Index of sound to play.
    - `fullvol (int)`: If nonzero, play this sound at full volume across the entire map.

- **A_WeaponJump(state, chance)**
  - Random state jump for weapons.
  - Args:
    - `state (uint)`: State index to jump to.
    - `chance (int)`: Chance out of 256 to perform the jump. 0 never jumps, 256 always jumps.

- **A_ConsumeAmmo(amount)**
  - Subtracts ammo from the currently-selected weapon's ammo pool.
  - Args:
    - `amount (int)`: Amount of ammo to subtract. If zero, will default to the current weapon's `ammopershot` value.
  - Notes:
    - This function will not reduce ammo below zero.
    - This function will no-op if the current weapon uses the `am_noammo` ("None"/"Infinite") ammotype.

- **A_CheckAmmo(state, amount)**
  - Jumps to `state` if ammo is below `amount`.
  - Args:
    - `state (uint)`: State index to jump to.
    - `amount (int)`: Amount of ammo to check. If zero, will default to the current weapon's `ammopershot` value.
  - Notes:
    - The jump will only occur if ammo is BELOW the given value -- e.g. `A_CheckAmmo(420, 695)` will jump to state 420 if ammo is 68 or lower.
    - This function will no-op if the current weapon uses the `am_noammo` ("None"/"Infinite") ammotype.

- **A_RefireTo(state, noammocheck)**
  - Jumps to `state` if the fire button is currently being pressed and the weapon has enough ammo to fire.
  - Args:
    - `state (uint)`: State index to jump to.
    - `noammocheck (int)`: If nonzero, skip the ammo check.

- **A_GunFlashTo(state, nothirdperson)**
  - Generic weapon muzzle flash.
  - Args:
    - `state (uint)`: State index to set the flash psprite to.
    - `nothirdperson (int)`: If nonzero, do not change the 3rd-person player sprite to the player muzzleflash state.

- **A_WeaponAlert**
  - Alerts monsters within sound-travel distance of the player's presence. Handy when combined with the WPF_SILENT flag.
  - No Args.

## Miscellaneous

#### New comp flags
- comp_ledgeblock: [commit](https://github.com/kraflab/dsda-doom/commit/4423cbcf8580e4d3839ddf4403b1fb4a0f993507)
  - Ledges block ground enemies
  - Exception: movement due to scrolling / pushers / pullers disables comp_ledgeblock for the next xy movement: [commit](https://github.com/kraflab/dsda-doom/commit/db8c3d606ed23dfb6b2408c4ddbf0af91d33f3de)
- comp_friendlyspawn: [PR](https://github.com/kraflab/dsda-doom/pull/34)
  - When on: A_Spawn new thing inherits friend flag from source thing.
  - When off: A_Spawn new thing keeps its default friend flag.

Summary of comp flags since mbf in pr+ and changes:

| Name               | Index | Default | Description                                   |
|--------------------|-------|---------|-----------------------------------------------|
| comp_moveblock-    | 19    | 0       | Large negative displacements are mishandled   |
| comp_respawn*      | 20    | 0       | Creatures with no spawnpoint respawn at (0,0) |
| comp_sound-        | 21    | 0       | Assorted sound errors                         |
| comp_666-          | 22    | 0       | Buggy pre-udoom boss checks                   |
| comp_soul*         | 23    | 0       | Lost souls do not bounce                      |
| comp_maskedanim-   | 24    | 0       | Two-sided midtextures don't animate           |
| comp_ouchface-     | 25    | 0       | Buggy vanilla ouchface code                   |
| comp_maxhealth-    | 26    | 0       | Max health in deh only applies to potions     |
| comp_translucency- | 27    | 0       | Disable some predefined translucency          |
| comp_ledgeblock    | 28    | 1       | Ledges block ground enemies                   |
| comp_friendlyspawn | 29    | 1       | A_Spawn new thing inherits friendliness       |

- Comp options marked with a `-` have been deoptionalized in mbf21 (forced to `0`). Many of these have nothing to do with demo compatibility - others are simple bug fixes.
- Comp options marked with a `*` are already implemented in EE.

#### Option default changes
- comp_pursuit: 1 (was 0)

#### Demo format / header
- MBF21 occupies complevel 21.
- Demo version is 221.
- longtics are enabled while recording.
- Old option entries removed:
  - variable_friction (always 1)
  - allow_pushers (always 1)
  - demo_insurance (always 0)
  - obsolete / empty bytes
- New option entries:
  - New comp options from above
  - Size of comp option list
    - This could be used to add comp options later without adding a complevel
    - When reading a demo that has a size larger than what you expect, abort
- Header options block:

| Key                   | Bytes |
|-----------------------|--------------|
| monsters_remember     | 1     |
| weapon_recoil         | 1     |
| player_bobbing        | 1     |
| respawnparm           | 1     |
| fastparm              | 1     |
| nomonsters            | 1     |
| rngseed               | 4     |
| monster_infighting    | 1     |
| dogs                  | 1     |
| distfriend            | 2     |
| monster_backing       | 1     |
| monster_avoid_hazards | 1     |
| monster_friction      | 1     |
| help_friends          | 1     |
| dog_jumping           | 1     |
| monkeys               | 1     |
| comp list size (23)   | 1     |
| comp_telefrag         | 1     |
| comp_dropoff          | 1     |
| comp_vile             | 1     |
| comp_pain             | 1     |
| comp_skull            | 1     |
| comp_blazing          | 1     |
| comp_doorlight        | 1     |
| comp_model            | 1     |
| comp_god              | 1     |
| comp_falloff          | 1     |
| comp_floors           | 1     |
| comp_skymap           | 1     |
| comp_pursuit          | 1     |
| comp_doorstuck        | 1     |
| comp_staylift         | 1     |
| comp_zombie           | 1     |
| comp_stairs           | 1     |
| comp_infcheat         | 1     |
| comp_zerotags         | 1     |
| comp_respawn          | 1     |
| comp_soul             | 1     |
| comp_ledgeblock       | 1     |
| comp_friendlyspawn    | 1     |

#### Fixes / adjustments since mbf
- Fix 3 key door bug
  - Already fixed in pr+ / EE.
  - [code](https://github.com/kraflab/dsda-doom/blob/61eac73ea246b48b17a30bc5a678a46b80d48fa1/prboom2/src/p_spec.c#L1086-L1090)
- Fix T_VerticalDoor mistake
  - Already fixed in pr+ / EE.
  - [code](https://github.com/kraflab/dsda-doom/blob/cd2ce9f532a80b871f0fdef2ae3ce6331b6e47b4/prboom2/src/p_doors.c#L588-L589)
- Fix buggy comp_stairs implementation
  - Already fixed in pr+ / EE.
  - [code](https://github.com/kraflab/dsda-doom/blob/6006aa42d3fba0ad2822ea35b144a921678821bf/prboom2/src/p_floor.c#L894-L895) and [code](https://github.com/kraflab/dsda-doom/blob/cd2ce9f532a80b871f0fdef2ae3ce6331b6e47b4/prboom2/src/p_floor.c#L934-L942)
- P_CreateSecNodeList global tmthing fix
  - Already fixed in pr+ / EE.
  - [code](https://github.com/kraflab/dsda-doom/blob/cd2ce9f532a80b871f0fdef2ae3ce6331b6e47b4/prboom2/src/p_map.c#L2601-L2603)
- A_CheckReload downstate
  - Already fixed in pr+ / EE.
  - [code](https://github.com/kraflab/dsda-doom/blob/cd2ce9f532a80b871f0fdef2ae3ce6331b6e47b4/prboom2/src/p_pspr.c#L636-L643)
- Fix P_DivlineSide bug
  - Already fixed in pr+ / EE.
  - [code](https://github.com/kraflab/dsda-doom/blob/cd2ce9f532a80b871f0fdef2ae3ce6331b6e47b4/prboom2/src/p_sight.c#L408)
- P_InterceptVector precision / overflow fix
  - Already fixed in pr+ / EE.
  - [code](https://github.com/kraflab/dsda-doom/blob/cd2ce9f532a80b871f0fdef2ae3ce6331b6e47b4/prboom2/src/p_maputl.c#L161-L166)
- Fix generalized crusher walkover lines
  - Already fixed in EE, but not in pr+.
  - [commit](https://github.com/kraflab/dsda-doom/commit/76776f721b5d1d8a1a0ae95daab525cf8183ce44)
- Fix blockmap issue seen in btsx e2 Map 20
  - Already fixed in EE, but not in pr+.
  - [commit](https://github.com/kraflab/dsda-doom/commit/c31040e0df9c2bc0c865d84bd496840f8123984a)
- Fix missing dropoff condition
  - Already fixed in pr+, but not in EE.
  - [code](https://github.com/kraflab/dsda-doom/blob/cd2ce9f532a80b871f0fdef2ae3ce6331b6e47b4/prboom2/src/p_map.c#L1037)
  - [EE](https://github.com/team-eternity/eternity/blob/0fc2a38da688d9f5001fef723b40ef92c5db0956/source/p_map.cpp#L1342)
- P_KillMobj thinker updates
  - Changed in pr+, reverted for mbf21.
  - [commit](https://github.com/kraflab/dsda-doom/commit/c5d99305ef2aa79983f5e95ac6cdc13ce415b54c)
- A_Mushroom changes
  - Changed in pr+, reverted for mbf21.
  - [commit](https://github.com/kraflab/dsda-doom/commit/a330db45dee7f255510f6b2c06006e97dc04d578)
- Fix negative ammo counts
  - [PR](https://github.com/kraflab/dsda-doom/pull/24)
- Fix weapon autoswitch not taking DEHACKED ammotype changes into account
  - [PR](https://github.com/kraflab/dsda-doom/pull/24)

#### Important Notes
- The default ammopershot value for fist / chainsaw is 1 (matters for backwards compatibility).
- Depending on how your port has modified spawning code, you may have a lingering bug related to the validcount variable. Basically, in vanilla doom, this variable is incremented in the wrong position in P_CheckPosition. The explanation for why this is a problem is complicated, but ripper projectiles (and possibly other cases) will expose this bug and cause desyncs. If your port strives for demo compatibility, I recommend adding an extra validcount increment in P_CheckPosition before running the P_BlockLinesIterator, as was done in dsda-doom (wrap it in a compatibility check for mbf21 if necessary): [code](https://github.com/kraflab/dsda-doom/blob/5ebd1c7860fae536dc46e5a310acb98ca59a3165/prboom2/src/p_map.c#L916-L920). Without that fix, vanilla heretic demos go out of sync, and the same will be true for mbf21 demos with certain dehacked features in play. A port like crispy doom wouldn't be affected because it hasn't modified the spawn code in a way that introduces the bugged validcount. Ports like pr+ and ee do have this problem, for example.
