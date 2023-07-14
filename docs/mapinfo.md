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
| Intermission                       | :telescope:     |
| NoIntermission                     | :telescope:     |
| EvenLighting                       | :heavy_check_mark: |
| SmoothLighting                     | :heavy_check_mark: |
| AllowMonsterTelefrags              | :heavy_check_mark: |
| ActivateOwnDeathSpecials           | :heavy_check_mark: |
| KillerActivatesDeathSpecials       | :heavy_check_mark: |
| StrictMonsterActivation            | :telescope:     |
| LaxMonsterActivation               | :telescope:     |
| MissileShootersActivateImpactLines | :telescope:     |
| MissilesActivateImpactLines        | :telescope:     |
| FilterStarts                       | :telescope:     |
| AllowRespawn                       | :heavy_check_mark: |
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
| NoPassover                         | :heavy_check_mark: |
| Passover                           | :heavy_check_mark: |
