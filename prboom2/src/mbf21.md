## MBF21 Current Status / Proposal

These are changes / features that are currently implemented.

Tracked [here](https://trello.com/b/qyrnGsFs/mbf21).

**This is NOT a spec. Anything could change.**

#### Demo format / header
- MBF21 occupies complevel 21.
- Demo version is 221.
- longtics are enabled while recording.
- [PR](https://github.com/kraflab/dsda-doom/pull/16)

#### Instant death sector special
- [PR](https://github.com/kraflab/dsda-doom/pull/17)
- Bit 12 (4096) turns on "alternate damage meaning" for bit 5 & 6:

| Dec | Bit 6-5 | Description                                                        |
|-----|---------|--------------------------------------------------------------------|
| 0   | 00      | Kills a player if they have neither a rad suit nor invulnerability |
| 32  | 01      | Kills a player unless they have invulnerability                    |
| 64  | 10      | Kills a player unless they have a radsuit                          |
| 96  | 11      | Kills a player                                                     |

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
