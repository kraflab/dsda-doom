## MAPINFO

This page tracks support for the MAPINFO lump, as seen in ZDoom. This is all a work in progress - the status of any feature (including whether or not it is planned) is subject to change.

### Current Status

You can enable MAPINFO parsing via the `-mapinfo` command line option. There is *very limited* support currently and crashing is possible.

### Legend

| Symbol             | Meaning                        |
| ------------------ | ------------------------------ |
| :heavy_check_mark: | Feature is supported           |
| :telescope:        | Feature is planned             |

### Top Level Keys

| Key           | Status          |
| ------------- | --------------- |
| map           | :telescope:     |
| defaultmap    | :telescope:     |
| adddefaultmap | :telescope:     |

### Map Properties

| Key                                | Status          |
| ---------------------------------- | --------------- |
| LevelNum                           | :telescope:     |
| Next                               | :telescope:     |
| SecretNext                         | :telescope:     |
| Cluster                            | :telescope:     |
| Sky1                               | :heavy_check_mark: |
| TitlePatch                         | :heavy_check_mark: |
| Par                                | :heavy_check_mark: |
| Music                              | :heavy_check_mark: |
| ExitPic                            | :telescope:     |
| EnterPic                           | :telescope:     |
| InterMusic                         | :heavy_check_mark: |
| BorderTexture                      | :heavy_check_mark: |
| Gravity                            | :telescope:     |
| AirControl                         | :telescope:     |
| Author                             | :heavy_check_mark: |
| SpecialAction                      | :telescope:     |
| Intermission                       | :telescope:     |
| NoIntermission                     | :telescope:     |
| EvenLighting                       | :telescope:     |
| SmoothLighting                     | :telescope:     |
| AllowMonsterTelefrags              | :telescope:     |
| ActivateOwnDeathSpecials           | :telescope:     |
| KillerActivatesDeathSpecials       | :telescope:     |
| StrictMonsterActivation            | :telescope:     |
| LaxMonsterActivation               | :telescope:     |
| MissileShootersActivateImpactLines | :telescope:     |
| MissilesActivateImpactLines        | :telescope:     |
| FilterStarts                       | :telescope:     |
| AllowRespawn                       | :telescope:     |
| NoJump                             | :telescope:     |
| AllowJump                          | :telescope:     |
| NoFreelook                         | :telescope:     |
| AllowFreelook                      | :telescope:     |
| CheckSwitchRange                   | :telescope:     |
| NoCheckSwitchRange                 | :telescope:     |
| ResetHealth                        | :telescope:     |
| ResetInventory                     | :telescope:     |
| UsePlayerStartZ                    | :telescope:     |
| RandomPlayerStarts                 | :telescope:     |
| ForgetState                        | :telescope:     |
| RememberState                      | :telescope:     |
| NoPassover                         | :telescope:     |
| Passover                           | :telescope:     |
