## MAPINFO

This page tracks support for the MAPINFO lump, as seen in ZDoom. This is all a work in progress - the status of any feature (including whether or not it is planned) is subject to change. Only the "new" format of mapinfo is valid in dsda-doom (the curly braces).

### Current Status

You can enable MAPINFO parsing via the `-debug_mapinfo` command line option. Support is currently experimental and various errors are present.

### Legend

| Symbol             | Meaning                        |
| ------------------ | ------------------------------ |
| :heavy_check_mark: | Feature is supported           |
| :telescope:        | Feature is planned             |

### Top Level Keys

| Key           | Status             |
| ------------- | ------------------ |
| cluster       | :telescope:        |
| clearepisodes | :telescope:        |
| episode       | :telescope:        |
| map           | :heavy_check_mark: |
| defaultmap    | :heavy_check_mark: |
| adddefaultmap | :heavy_check_mark: |
| clearskills   | :heavy_check_mark: |
| skill         | :heavy_check_mark: |

### Map Properties

| Key                                | Status             |
| ---------------------------------- | ------------------ |
| LevelNum                           | :heavy_check_mark: |
| Next                               | :heavy_check_mark: |
| SecretNext                         | :heavy_check_mark: |
| Cluster                            | :heavy_check_mark: |
| Sky1                               | :heavy_check_mark: |
| TitlePatch                         | :heavy_check_mark: |
| Par                                | :heavy_check_mark: |
| Music                              | :heavy_check_mark: |
| ExitPic                            | :heavy_check_mark: |
| EnterPic                           | :heavy_check_mark: |
| InterMusic                         | :heavy_check_mark: |
| BorderTexture                      | :heavy_check_mark: |
| Gravity                            | :heavy_check_mark: |
| AirControl                         | :heavy_check_mark: |
| Author                             | :heavy_check_mark: |
| SpecialAction                      | :heavy_check_mark: |
| Intermission                       | :heavy_check_mark: |
| NoIntermission                     | :heavy_check_mark: |
| EvenLighting                       | :heavy_check_mark: |
| SmoothLighting                     | :heavy_check_mark: |
| AllowMonsterTelefrags              | :heavy_check_mark: |
| ActivateOwnDeathSpecials           | :heavy_check_mark: |
| KillerActivatesDeathSpecials       | :heavy_check_mark: |
| StrictMonsterActivation            | :heavy_check_mark: |
| LaxMonsterActivation               | :heavy_check_mark: |
| MissileShootersActivateImpactLines | :heavy_check_mark: |
| MissilesActivateImpactLines        | :heavy_check_mark: |
| FilterStarts                       | :heavy_check_mark: |
| AllowRespawn                       | :heavy_check_mark: |
| NoJump                             | :heavy_check_mark: |
| AllowJump                          | :heavy_check_mark: |
| CheckSwitchRange                   | :heavy_check_mark: |
| NoCheckSwitchRange                 | :heavy_check_mark: |
| ResetHealth                        | :heavy_check_mark: |
| ResetInventory                     | :heavy_check_mark: |
| UsePlayerStartZ                    | :heavy_check_mark: |
| NoPassover                         | :heavy_check_mark: |
| Passover                           | :heavy_check_mark: |

#### Notes

The `Passover` and `NoPassover` fields are new. ZDoom has different defaults and expectations, where this behavior can be adjusted using zdoom's specific compatibility flags when _necessary_. In dsda-doom this is considered a feature rather than a compatibility and must be turned on explicitly by the author.

### Skill Properties

| Key             | Status             |
| --------------- | ------------------ |
| AmmoFactor      | :heavy_check_mark: |
| DamageFactor    | :heavy_check_mark: |
| ArmorFactor     | :heavy_check_mark: |
| HealthFactor    | :heavy_check_mark: |
| MonsterHealth   | :heavy_check_mark: |
| FriendlyHealth  | :heavy_check_mark: |
| RespawnTime     | :heavy_check_mark: |
| SpawnFilter     | :heavy_check_mark: |
| Key             | :heavy_check_mark: |
| MustConfirm     | :heavy_check_mark: |
| Name            | :heavy_check_mark: |
| PicName         | :heavy_check_mark: |
| TextColor       | :heavy_check_mark: |
| SpawnMulti      | :heavy_check_mark: |
| FastMonsters    | :heavy_check_mark: |
| InstantReaction | :heavy_check_mark: |
| NoPain          | :heavy_check_mark: |
| DefaultSkill    | :heavy_check_mark: |
| PlayerRespawn   | :heavy_check_mark: |
| EasyBossBrain   | :heavy_check_mark: |

#### Notes

The `SpawnFilter` field only takes numbers, not names.
