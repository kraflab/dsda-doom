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
| Fade                               | :grey_question: |
| FadeTable                          | :grey_question: |
| OutsideFog                         | :grey_question: |
| TitlePatch                         | :grey_question: |
| Par                                | :grey_question: |
| SuckTime                           | :grey_question: |
| NoIntermission                     | :grey_question: |
| Intermission                       | :grey_question: |
| Music                              | :grey_question: |
| CDTrack                            | :grey_question: |
| CDId                               | :grey_question: |
| ExitPic                            | :grey_question: |
| EnterPic                           | :grey_question: |
| InterMusic                         | :grey_question: |
| BorderTexture                      | :grey_question: |
| Lightning                          | :grey_question: |
| EvenLighting                       | :grey_question: |
| SmoothLighting                     | :grey_question: |
| VertWallShade                      | :grey_question: |
| HorizWallShade                     | :grey_question: |
| TeamDamage                         | :grey_question: |
| Gravity                            | :grey_question: |
| AirControl                         | :grey_question: |
| AirSupply                          | :grey_question: |
| F1                                 | :grey_question: |
| MapBackground                      | :grey_question: |
| Translator                         | :grey_question: |
| AllowMonsterTelefrags              | :grey_question: |
| ActivateOwnDeathSpecials           | :grey_question: |
| SpecialAction                      | :grey_question: |
| Map07Special                       | :grey_question: |
| BaronSpecial                       | :grey_question: |
| CyberdemonSpecial                  | :grey_question: |
| SpiderMastermindSpecial            | :grey_question: |
| IronlichSpecial                    | :grey_question: |
| MinotaurSpecial                    | :grey_question: |
| DSparilSpecial                     | :grey_question: |
| SpecialAction_ExitLevel            | :grey_question: |
| SpecialAction_OpenDoor             | :grey_question: |
| SpecialAction_LowerFloor           | :grey_question: |
| SpecialAction_KillMonsters         | :grey_question: |
| ClipMidTextures                    | :grey_question: |
| NoAutoSequences                    | :grey_question: |
| AutoSequences                      | :grey_question: |
| StrictMonsterActivation            | :grey_question: |
| LaxMonsterActivation               | :grey_question: |
| MissileShootersActivateImpactLines | :grey_question: |
| MissilesActivateImpactLines        | :grey_question: |
| FallingDamage                      | :grey_question: |
| OldFallingDamage                   | :grey_question: |
| ForceFallingDamage                 | :grey_question: |
| StrifeFallingDamage                | :grey_question: |
| NoFallingDamage                    | :grey_question: |
| MonsterFallingDamage               | :grey_question: |
| ProperMonsterFallingDamage         | :grey_question: |
| AvoidMelee                         | :grey_question: |
| FilterStarts                       | :grey_question: |
| AllowRespawn                       | :grey_question: |
| TeamPlayOn                         | :grey_question: |
| TeamPlayOff                        | :grey_question: |
| NoInventoryBar                     | :grey_question: |
| KeepFullInventory                  | :grey_question: |
| InfiniteFlightPowerup              | :grey_question: |
| NoJump                             | :grey_question: |
| AllowJump                          | :grey_question: |
| NoCrouch                           | :grey_question: |
| AllowCrouch                        | :grey_question: |
| NoFreelook                         | :grey_question: |
| AllowFreelook                      | :grey_question: |
| NoInfighting                       | :grey_question: |
| NormalInfighting                   | :grey_question: |
| TotalInfighting                    | :grey_question: |
| CheckSwitchRange                   | :grey_question: |
| NoCheckSwitchRange                 | :grey_question: |
| UnFreezeSinglePlayerConversations  | :grey_question: |
| NoAllies                           | :grey_question: |
| ResetHealth                        | :grey_question: |
| ResetInventory                     | :grey_question: |
| ResetItems                         | :grey_question: |
| Grinding_Polyobj                   | :grey_question: |
| No_Grinding_Polyobj                | :grey_question: |
| compat_*                           | :grey_question: |
| DefaultEnvironment                 | :grey_question: |
| NoAutosaveHint                     | :grey_question: |
| UsePlayerStartZ                    | :grey_question: |
| RandomPlayerStarts                 | :grey_question: |
| PrecacheSounds                     | :grey_question: |
| PrecacheTextures                   | :grey_question: |
| PrecacheClasses                    | :grey_question: |
| ForgetState                        | :grey_question: |
| RememberState                      | :grey_question: |
| SpawnWithWeaponRaised              | :grey_question: |
| ForceFakeContrast                  | :grey_question: |
| ForceWorldPanning                  | :grey_question: |
| HazardColor                        | :grey_question: |
| HazardFlash                        | :grey_question: |
| EventHandlers                      | :grey_question: |
| NeedClusterText                    | :grey_question: |
| NoClusterText                      | :grey_question: |
| Author                             | :grey_question: |
| EnableSkyboxAO                     | :grey_question: |
| DisableSkyboxAO                    | :grey_question: |
| EnableShadowmap                    | :grey_question: |
| DisableShadowmap                   | :grey_question: |
| AttenuateLights                    | :grey_question: |
| SndInfo                            | :grey_question: |
| SoundInfo                          | :grey_question: |
| SndSeq                             | :grey_question: |
| Intro                              | :grey_question: |
| Outro                              | :grey_question: |

#### Notes
- The `lookup` key for map name localization is not supported
