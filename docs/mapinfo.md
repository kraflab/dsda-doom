## MAPINFO

This page tracks support for the MAPINFO lump, as seen in ZDoom. This is all a work in progress - the status of any feature (including whether or not it is planned) is subject to change.

### Current Status

You can enable MAPINFO parsing via the `-mapinfo` command line option. There is *very limited* support currently and crashing is possible.

### Legend

| Symbol             | Meaning                        |
| ------------------ | ------------------------------ |
| :heavy_check_mark: | Feature is supported           |
| :warning:          | Feature is partially supported |
| :telescope:        | Feature is planned             |
| :grey_question:    | Feature is under investigation |
| :x:                | Feature is not planned         |

### Top Level Keys

| Key           | Status          |
| ------------- | --------------- |
| map           | :telescope:     |
| defaultmap    | :telescope:     |
| adddefaultmap | :telescope:     |
| gamedefaults  | :x:             |

### Map Properties

| Key                                | Status          |
| ---------------------------------- | --------------- |
| LevelNum                           | :telescope:     |
| Next                               | :telescope:     |
| SecretNext                         | :telescope:     |
| Slideshow                          | :x:             |
| DeathSequence                      | :x:             |
| Redirect                           | :telescope:     |
| Cluster                            | :telescope:     |
| Sky1                               | :telescope:     |
| Sky2                               | :grey_question: |
| Skybox                             | :telescope:     |
| DoubleSky                          | :grey_question: |
| ForceNoSkyStretch                  | :grey_question: |
| SkyStretch                         | :grey_question: |
| Fade                               | :x:             |
| FadeTable                          | :grey_question: |
| OutsideFog                         | :x:             |
| TitlePatch                         | :telescope:     |
| Par                                | :telescope:     |
| SuckTime                           | :telescope:     |
| NoIntermission                     | :grey_question: |
| Intermission                       | :grey_question: |
| Music                              | :telescope:     |
| CDTrack                            | :x:             |
| CDId                               | :x:             |
| ExitPic                            | :telescope:     |
| EnterPic                           | :telescope:     |
| InterMusic                         | :telescope:     |
| BorderTexture                      | :telescope:     |
| Lightning                          | :grey_question: |
| EvenLighting                       | :telescope:     |
| SmoothLighting                     | :telescope:     |
| VertWallShade                      | :x:             |
| HorizWallShade                     | :x:             |
| TeamDamage                         | :x:             |
| Gravity                            | :telescope:     |
| AirControl                         | :telescope:     |
| AirSupply                          | :x:             |
| F1                                 | :x:             |
| MapBackground                      | :x:             |
| Translator                         | :x:             |
| AllowMonsterTelefrags              | :telescope:     |
| ActivateOwnDeathSpecials           | :telescope:     |
| KillerActivatesDeathSpecials       | :telescope:     |
| SpecialAction                      | :telescope:     |
| Map07Special                       | :x:             |
| BaronSpecial                       | :x:             |
| CyberdemonSpecial                  | :x:             |
| SpiderMastermindSpecial            | :x:             |
| IronlichSpecial                    | :x:             |
| MinotaurSpecial                    | :x:             |
| DSparilSpecial                     | :x:             |
| SpecialAction_ExitLevel            | :x:             |
| SpecialAction_OpenDoor             | :x:             |
| SpecialAction_LowerFloor           | :x:             |
| SpecialAction_KillMonsters         | :x:             |
| ClipMidTextures                    | :telescope:     |
| NoAutoSequences                    | :x:             |
| AutoSequences                      | :x:             |
| StrictMonsterActivation            | :telescope:     |
| LaxMonsterActivation               | :telescope:     |
| MissileShootersActivateImpactLines | :telescope:     |
| MissilesActivateImpactLines        | :telescope:     |
| FallingDamage                      | :x:             |
| OldFallingDamage                   | :x:             |
| ForceFallingDamage                 | :x:             |
| StrifeFallingDamage                | :x:             |
| NoFallingDamage                    | :x:             |
| MonsterFallingDamage               | :x:             |
| ProperMonsterFallingDamage         | :x:             |
| AvoidMelee                         | :telescope:     |
| FilterStarts                       | :telescope:     |
| AllowRespawn                       | :telescope:     |
| TeamPlayOn                         | :x:             |
| TeamPlayOff                        | :x:             |
| NoInventoryBar                     | :x:             |
| KeepFullInventory                  | :x:             |
| InfiniteFlightPowerup              | :x:             |
| NoJump                             | :telescope:     |
| AllowJump                          | :telescope:     |
| NoCrouch                           | :x:             |
| AllowCrouch                        | :x:             |
| NoFreelook                         | :telescope:     |
| AllowFreelook                      | :telescope:     |
| NoInfighting                       | :telescope:     |
| NormalInfighting                   | :telescope:     |
| TotalInfighting                    | :telescope:     |
| CheckSwitchRange                   | :telescope:     |
| NoCheckSwitchRange                 | :telescope:     |
| UnFreezeSinglePlayerConversations  | :x:             |
| NoAllies                           | :telescope:     |
| ResetHealth                        | :telescope:     |
| ResetInventory                     | :telescope:     |
| ResetItems                         | :x:             |
| Grinding_Polyobj                   | :grey_question: |
| No_Grinding_Polyobj                | :grey_question: |
| compat_*                           | :x:             |
| DefaultEnvironment                 | :x:             |
| NoAutosaveHint                     | :x:             |
| UsePlayerStartZ                    | :telescope:     |
| RandomPlayerStarts                 | :telescope:     |
| PrecacheSounds                     | :x:             |
| PrecacheTextures                   | :x:             |
| PrecacheClasses                    | :x:             |
| ForgetState                        | :telescope:     |
| RememberState                      | :telescope:     |
| SpawnWithWeaponRaised              | :telescope:     |
| ForceFakeContrast                  | :x:             |
| ForceWorldPanning                  | :x:             |
| HazardColor                        | :x:             |
| HazardFlash                        | :x:             |
| EventHandlers                      | :x:             |
| NeedClusterText                    | :x:             |
| NoClusterText                      | :x:             |
| Author                             | :telescope:     |
| EnableSkyboxAO                     | :x:             |
| DisableSkyboxAO                    | :x:             |
| EnableShadowmap                    | :x:             |
| DisableShadowmap                   | :x:             |
| AttenuateLights                    | :x:             |
| SndInfo                            | :x:             |
| SoundInfo                          | :x:             |
| SndSeq                             | :x:             |
| Intro                              | :x:             |
| Outro                              | :x:             |
| NoPassover                         | :telescope:     |
| Passover                           | :telescope:     |

#### Notes
- The `lookup` key for map name localization is not supported
- The `tracknum` field for music is not supported
- The `EnterPic` and `ExitPic` keys do not support intermission scripts
- The `Passover` and `NoPassover` flags are new (the gzdoom equivalent is `compat_nopassover = x`)
