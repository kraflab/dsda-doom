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
| cluster       | :heavy_check_mark: |
| clearepisodes | :heavy_check_mark: |
| episode       | :heavy_check_mark: |
| map           | :heavy_check_mark: |
| defaultmap    | :heavy_check_mark: |
| adddefaultmap | :heavy_check_mark: |
| clearskills   | :heavy_check_mark: |
| skill         | :heavy_check_mark: |

### Map

**map** **\<MapLump\>** **"\<NiceName\>" { _properties_ }**

Defines a single map.
The **MapLump** is the name of the marker lump for the map (e.g., E1M1).
The **NiceName** is used on the automap and anywhere else to refer to the map in-game.

```c
// Example
map MAP01 "My first map" {
  property = value
}
```

#### Map Properties

| Key | Description |
| --- | --- |
| **LevelNum** | The number used in the **Teleport_NewMap** special and for warping (e.g., to record a demo). The default is based on the **MapLump** (e.g., MAP23 yields 23 and E1M8 yields 8). If multiple maps use the same **LevelNum**, only the last map will keep it. |
| **Next** | The behavior when the normal exit is triggered. The following options are supported: <ul> <li>**"\<MapLump\>":** Enter the given map.</li> <li>**EndPic, "\<Lump\>":** End the game, displaying the given lump as an image.</li> <li>**"EndGame1":** End the game, displaying the image from Doom E1.</li>  <li>**"EndGame2":** End the game, displaying the image from Doom E2.</li>  <li>**"EndGame3":** End the game, displaying the horizontal scroller from Doom E3.</li>  <li>**"EndGame4":** End the game, displaying the image from Doom E4.</li>  <li>**"EndGameC":** End the game, displaying the cast from Doom 2.</li>  <li>**EndGame { _properties_ }:** End the game with custom properties: <ul> <li>**Pic = "\<Lump\>":** Display the given lump as an image. If combined with **Cast**, sets the cast background.</li> <li>**HScroll = "\<RightLump\>", "\<LeftLump\>":** Display the horizontal scroller using the given lumps as images.</li> <li>**Cast:** Display the cast.</li> <li>**Music = "\<Lump\>"[, \<Loop\>]:** Play the given lump as music. Optionally set whether the music should loop (1) or not (0). Music loops by default.</li> </ul> If multiple of **Pic**, **HScroll**, and **Cast** are used, only the last one will be used to decide the end game version. </li> </ul> |
| **SecretNext** | The behavior when the secret exit is triggered. See the description of **Next** for options. |
| **Cluster** | todo |
| **Sky1** | todo |
| **TitlePatch** | todo |
| **Par** | todo |
| **Music** | todo |
| **ExitPic** | todo |
| **EnterPic** | todo |
| **InterMusic** | todo |
| **BorderTexture** | todo |
| **Gravity** | todo |
| **AirControl** | todo |
| **Author** | todo |
| **SpecialAction** | todo |
| **Intermission** | todo |
| **NoIntermission** | todo |
| **EvenLighting** | todo |
| **SmoothLighting** | todo |
| **AllowMonsterTelefrags** | todo |
| **ActivateOwnDeathSpecials** | todo |
| **KillerActivatesDeathSpecials** | todo |
| **StrictMonsterActivation** | todo |
| **LaxMonsterActivation** | todo |
| **MissileShootersActivateImpactLines** | todo |
| **MissilesActivateImpactLines** | todo |
| **FilterStarts** | todo |
| **AllowRespawn** | todo |
| **NoJump** | todo |
| **AllowJump** | todo |
| **CheckSwitchRange** | todo |
| **NoCheckSwitchRange** | todo |
| **ResetHealth** | todo |
| **ResetInventory** | todo |
| **UsePlayerStartZ** | todo |
| **NoPassover** | todo |
| **Passover** | todo |

#### Notes

The `Passover` and `NoPassover` fields are new. ZDoom has different defaults and expectations, where this behavior can be adjusted using zdoom's specific compatibility flags when _necessary_. In dsda-doom this is considered a feature rather than a compatibility and must be turned on explicitly by the author.

### Cluster Properties

| Key               | Status             |
| ----------------- | ------------------ |
| EnterText         | :heavy_check_mark: |
| ExitText          | :heavy_check_mark: |
| Music             | :heavy_check_mark: |
| Flat              | :heavy_check_mark: |
| Pic               | :heavy_check_mark: |

### Episode Properties

| Key               | Status             |
| ----------------- | ------------------ |
| Name              | :heavy_check_mark: |
| PicName           | :heavy_check_mark: |
| Key               | :heavy_check_mark: |

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
