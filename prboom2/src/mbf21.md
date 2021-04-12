## MBF21 Current Status

These are changes / features that are currently implemented.

Tracked [here](https://trello.com/b/qyrnGsFs/mbf21).

**This is NOT a spec.**

This is proof-of-concept implemented in dsda-doom.

#### Demo format / header
- [PR](https://github.com/kraflab/dsda-doom/pull/16)
- MBF21 occupies complevel 21.
- Demo version is 221.
- longtics are enabled while recording.

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

#### Fix 3 key doors bug from mbf
- already fixed actually in post-mbf pr+ levels.

#### Fix blockmap issue seen in btsx e2 Map 20
- [commit](https://github.com/kraflab/dsda-doom/commit/c31040e0df9c2bc0c865d84bd496840f8123984a)

#### Block land monsters line flag
- [PR](https://github.com/kraflab/dsda-doom/pull/19)
- Uses bit 12 (4096).

#### Block players line flag
- [commit](https://github.com/kraflab/dsda-doom/commit/687237e3d236056730f58dca27efd45e1774d53e)
- Uses bit 13 (8192).

#### Fix generalized crusher walkover lines
- [commit](https://github.com/kraflab/dsda-doom/commit/76776f721b5d1d8a1a0ae95daab525cf8183ce44)

#### New comp flags
- comp_ledgeblock: [commit](https://github.com/kraflab/dsda-doom/commit/4423cbcf8580e4d3839ddf4403b1fb4a0f993507)

| Name            | Number | Default | Description                 |
|-----------------|--------|---------|-----------------------------|
| comp_ledgeblock | 28     | 1       | Ledges block ground enemies |

#### Option default changes
- comp_pursuit: 1 (was 0)

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
- Things with the same value of `S` will not deal splash damage to each other.
- Splash damage coming through a neutral thing (e.g., exploding a barrel) will still occur.

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

Thing 22 (Cyberdemon)
Splash group = 0
```

In this example:
- Imp projectiles now damage other Imps (and they will infight with their own kind).
- Barons and Arachnotrons are in the same projectile group: their projectiles will no longer damage each other.
- Barons and Hell Knights are not in the same projectile group: their projectiles will now damage each other, leading to infighting.
- Hell Knights and Arachnotrons are in the same infighting group: they will not infight with each other, despite taking damage from each other's projectiles.
- Imps and Cyberdemons are in the same splash group: cyberdemon rocket splash will no longer damage imps (but direct impacts do).
- Note that the group numbers are separate - being in infighting group 1 doesn't mean you are in projectile group 1.

#### New Thing Flags

Implementations match between DSDA-Doom and Eternity Engine,
except for the ripper projectile, which is still TODO.
The DEH specification is still TBD - this is just a list of implemented flags of note.

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
| MF2_NEUTRAL_SPLASH | ?                  | Splash damage from this thing is not affected by splash groups                                 |

#### New DEHACKED Codepointers
- [PR](https://github.com/kraflab/dsda-doom/pull/20)
- Actor pointers:
  - **A_SpawnFacing(type, height)** -- spawns an actor of `type` at `height` z units and sets its angle to the caller's angle.
  - **A_MonsterProjectile(type, angle)** -- generic monster projectile attack; always sets `tracer` field.
  - **A_MonsterBulletAttack(damage, spread)** -- generic monster bullet attack w/horizontal `spread`; if `spread` is negative, apply vertical spread equal to 2/3 of this value (approx. equal to SSG vert-spread)
  - **A_RadiusDamage(damage, radius)** -- generic A_Explode, w/customizable damage and radius (helll yeah)
- Weapon pointers:
  - **A_WeaponProjectile(type, angle)** -- generic weapon projectile attack; does not consume ammo
  - **A_WeaponBulletAttack(damage, spread)** -- generic weapon bullet attack; does not consume ammo; same `spread` behavior as A_MonsterBulletAttack
  - **A_WeaponSound(sound, fullvol)** -- same as A_PlaySound, but for weapons
  - **A_WeaponJump(state, chance)** -- same as A_RandomJump, but for  weapons
  - **A_ConsumeAmmo(amount)** -- subtract `amount` units of ammo. if `amount` is zero, use the weapon slot's `ammopershot`. will not reduce ammo below zero.
  - **A_CheckAmmo(state, amount)** -- jumps to `state` if ammo is below `amount`; if `amount` is zero, use the weapon slot's `ammopershot` value instead
  - **A_RefireTo(state, noammocheck)** -- jumps to `state` if trigger is still held down; will also check ammo unless `noammocheck` is set
  - **A_GunFlashTo(state, nothirdperson)** -- sets the weapon's flash state to `state`; also sets the player's 3rd-person sprite to the player actor's firing frame unless `nothirdperson` is set
