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
| map           | :grey_question: |
| defaultmap    | :grey_question: |
| adddefaultmap | :grey_question: |
| gamedefaults  | :grey_question: |

### Map Properties

| Key                                | Status          |
| ---------------------------------- | --------------- |
| LevelNum                           | :grey_question: |
| Next                               | :grey_question: |
| SecretNext                         | :grey_question: |
| Slideshow                          | :x:             |
| DeathSequence                      | :x:             |
| Redirect                           | :grey_question: |
| Cluster                            | :grey_question: |
| Sky1                               | :grey_question: |
| Sky2                               | :grey_question: |
| Skybox                             | :grey_question: |
| DoubleSky                          | :grey_question: |
| ForceNoSkyStretch                  | :grey_question: |
| SkyStretch                         | :grey_question: |
| Fade                               | :x:             |
| FadeTable                          | :grey_question: |
| OutsideFog                         | :x:             |
| TitlePatch                         | :grey_question: |
| Par                                | :grey_question: |
| SuckTime                           | :grey_question: |
| NoIntermission                     | :grey_question: |
| Intermission                       | :grey_question: |
| Music                              | :grey_question: |
| CDTrack                            | :x:             |
| CDId                               | :x:             |
| ExitPic                            | :grey_question: |
| EnterPic                           | :grey_question: |
| InterMusic                         | :grey_question: |
| BorderTexture                      | :grey_question: |
| Lightning                          | :grey_question: |
| EvenLighting                       | :grey_question: |
| SmoothLighting                     | :grey_question: |
| VertWallShade                      | :x:             |
| HorizWallShade                     | :x:             |
| TeamDamage                         | :x:             |
| Gravity                            | :grey_question: |
| AirControl                         | :grey_question: |
| AirSupply                          | :x:             |
| F1                                 | :x:             |
| MapBackground                      | :x:             |
| Translator                         | :x:             |
| AllowMonsterTelefrags              | :grey_question: |
| ActivateOwnDeathSpecials           | :grey_question: |
| SpecialAction                      | :grey_question: |
| Map07Special                       | :grey_question: |
| BaronSpecial                       | :grey_question: |
| CyberdemonSpecial                  | :grey_question: |
| SpiderMastermindSpecial            | :grey_question: |
| IronlichSpecial                    | :x:             |
| MinotaurSpecial                    | :x:             |
| DSparilSpecial                     | :x:             |
| SpecialAction_ExitLevel            | :grey_question: |
| SpecialAction_OpenDoor             | :grey_question: |
| SpecialAction_LowerFloor           | :grey_question: |
| SpecialAction_KillMonsters         | :grey_question: |
| ClipMidTextures                    | :grey_question: |
| NoAutoSequences                    | :x:             |
| AutoSequences                      | :x:             |
| StrictMonsterActivation            | :grey_question: |
| LaxMonsterActivation               | :grey_question: |
| MissileShootersActivateImpactLines | :grey_question: |
| MissilesActivateImpactLines        | :grey_question: |
| FallingDamage                      | :x:             |
| OldFallingDamage                   | :x:             |
| ForceFallingDamage                 | :x:             |
| StrifeFallingDamage                | :x:             |
| NoFallingDamage                    | :x:             |
| MonsterFallingDamage               | :x:             |
| ProperMonsterFallingDamage         | :x:             |
| AvoidMelee                         | :grey_question: |
| FilterStarts                       | :grey_question: |
| AllowRespawn                       | :grey_question: |
| TeamPlayOn                         | :x:             |
| TeamPlayOff                        | :x:             |
| NoInventoryBar                     | :x:             |
| KeepFullInventory                  | :x:             |
| InfiniteFlightPowerup              | :x:             |
| NoJump                             | :grey_question: |
| AllowJump                          | :grey_question: |
| NoCrouch                           | :x:             |
| AllowCrouch                        | :x:             |
| NoFreelook                         | :grey_question: |
| AllowFreelook                      | :grey_question: |
| NoInfighting                       | :grey_question: |
| NormalInfighting                   | :grey_question: |
| TotalInfighting                    | :grey_question: |
| CheckSwitchRange                   | :grey_question: |
| NoCheckSwitchRange                 | :grey_question: |
| UnFreezeSinglePlayerConversations  | :x:             |
| NoAllies                           | :grey_question: |
| ResetHealth                        | :grey_question: |
| ResetInventory                     | :grey_question: |
| ResetItems                         | :x:             |
| Grinding_Polyobj                   | :grey_question: |
| No_Grinding_Polyobj                | :grey_question: |
| compat_*                           | :grey_question: |
| DefaultEnvironment                 | :grey_question: |
| NoAutosaveHint                     | :x:             |
| UsePlayerStartZ                    | :grey_question: |
| RandomPlayerStarts                 | :grey_question: |
| PrecacheSounds                     | :grey_question: |
| PrecacheTextures                   | :grey_question: |
| PrecacheClasses                    | :grey_question: |
| ForgetState                        | :grey_question: |
| RememberState                      | :grey_question: |
| SpawnWithWeaponRaised              | :grey_question: |
| ForceFakeContrast                  | :x:             |
| ForceWorldPanning                  | :x:             |
| HazardColor                        | :grey_question: |
| HazardFlash                        | :grey_question: |
| EventHandlers                      | :grey_question: |
| NeedClusterText                    | :x:             |
| NoClusterText                      | :x:             |
| Author                             | :grey_question: |
| EnableSkyboxAO                     | :x:             |
| DisableSkyboxAO                    | :x:             |
| EnableShadowmap                    | :x:             |
| DisableShadowmap                   | :x:             |
| AttenuateLights                    | :x:             |
| SndInfo                            | :grey_question: |
| SoundInfo                          | :grey_question: |
| SndSeq                             | :grey_question: |
| Intro                              | :grey_question: |
| Outro                              | :grey_question: |

#### Notes
- The `lookup` key for map name localization is not supported
- The `tracknum` field for music is not supported
- The `EnterPic` and `ExitPic` keys do not support intermission scripts
