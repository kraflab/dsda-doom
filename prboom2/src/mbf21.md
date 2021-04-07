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
