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

**defaultmap { _properties_ }**

Sets the default properties that all maps have (which can be overridden in individual map definitions). Since ZDoom has different default behavior, it's best to have at least this minimal definition for cross-port consistency:

```c
defaultmap {
  ActivateOwnDeathSpecials
  MissilesActivateImpactLines
  NoJump
}
```

**adddefaultmap { _properties_ }**

Appends properties to the default map.

#### Map Properties

| Key | Description |
| --- | --- |
| **LevelNum = \<Int\>** | The number used in the **Teleport_NewMap** special and for warping (e.g., to record a demo). The default is based on the **MapLump** (e.g., MAP23 yields 23 and E1M8 yields 8). If multiple maps use the same **LevelNum**, only the last map will keep it. |
| **Next = _various_** | The behavior when the normal exit is triggered. The following options are supported: <ul> <li>**"\<MapLump\>":** Enter the given map.</li> <li>**EndPic, "\<Lump\>":** End the game, displaying the given lump as an image.</li> <li>**"EndGame1":** End the game, displaying the image from Doom E1.</li>  <li>**"EndGame2":** End the game, displaying the image from Doom E2.</li>  <li>**"EndGame3":** End the game, displaying the horizontal scroller from Doom E3.</li>  <li>**"EndGame4":** End the game, displaying the image from Doom E4.</li>  <li>**"EndGameC":** End the game, displaying the cast from Doom 2.</li>  <li>**EndGame { _properties_ }:** End the game with custom properties: <ul> <li>**Pic = "\<Lump\>":** Display the given lump as an image. If combined with **Cast**, sets the cast background.</li> <li>**HScroll = "\<RightLump\>", "\<LeftLump\>":** Display the horizontal scroller using the given lumps as images.</li> <li>**Cast:** Display the cast.</li> <li>**Music = "\<Lump\>"[, \<Loop\>]:** Play the given lump as music. Optionally set whether the music should loop (1) or not (0). Music loops by default.</li> </ul> If multiple of **Pic**, **HScroll**, and **Cast** are used, only the last one will be used to decide the end game version. </li> </ul> |
| **SecretNext = _various_** | The behavior when the secret exit is triggered. See the description of **Next** for options. |
| **Cluster = \<Int\>** | The cluster number the map belongs to. |
| **Sky1 = "\<Texture\>[, \<ScrollSpeed\>]** | The texture used for the sky. Optionally, supply a scrolling speed (default is 0.0). |
| **TitlePatch = "\<Patch\>"[, \<HideAuthorName\>]** | The title patch used on the intermission screen. By default, a graphic will be constructed from the map's **NiceName**. By default, the author name is also shown on the intermission screen. Use a 1 for **HideAuthorName** to hide it instead. |
| **Par = \<Time\>** | Par time (in seconds) shown on the intermission screen. |
| **Music = "\<Lump\>"** | The music that plays during the map. |
| **ExitPic = "\<Lump\>"** | The background of the "level finished" screen. |
| **EnterPic = "\<Lump\>"** | The background of the "entering level" screen. |
| **InterMusic = "\<Lump\>"** | The intermission music. |
| **BorderTexture = "\<Lump\>"** | The flat (not texture, despite the name) that fills the default status bar edges when the player plays in widescreen. |
| **Gravity = \<Float\>** | The strength of gravity in the map (default 800). |
| **AirControl = \<Float\>** | The amount of control a player has while airborne. A value of 1 means complete control (as if the player was on the ground). If jumping is disabled, the default is 0. When jumping is enabled, the default is 0.00390625 (just enough to jump onto ledges). |
| **Author = "\<Name\>"** | The map author name, which may be used on the intermission screen and in other places. |
| **SpecialAction = "\<MonsterType\>", "\<ActionSpecial\>"[, \<arg1\>, \<arg2\>, \<arg3\>, \<arg4\>, \<arg5\>]** | Assigns an action to the given monster type that is executed when all monsters of that type are dead, with the associated arguments. The monster must call **A_BossDeath** to trigger the action. You can define as many special actions as you want, including multiple actions for a single monster type. |
| **Intermission** | Shows the intermission when exiting the map (default). |
| **NoIntermission** | Disables the intermission when exiting the map. |
| **EvenLighting** | Disables fake contrast (by default, Doom applies different shading on orthogonal walls). |
| **SmoothLighting** | Keeps fake contrast, but applying shading smoothly based on the wall angle. |
| **AllowMonsterTelefrags** | Allows monsters to telefrag other monsters or players. By default, monsters cannot telefrag. |
| **ActivateOwnDeathSpecials** | When a thing with a special dies, it is considered the activator of the special (this is the default). |
| **KillerActivatesDeathSpecials** | When a thing with a special dies, the killer is considered the activator of the special. |
| **StrictMonsterActivation** | Only allow monsters to activate line specials they are explicitly allowed to activate. |
| **LaxMonsterActivation** | Allow monsters to activate certain specials (some doors, lifts, and teleporters), even if they aren't explicitly allowed to (this is the default). |
| **MissileShootersActivateImpactLines** | When a missile triggers an impact line, the shooter is considered the activator. |
| **MissilesActivateImpactLines** | When a missile triggers an impact line, the missile is considered the activator (this is the default). |
| **FilterStarts** | Filter out player starts based on skill and game type settings. By default, player starts are not filtered. |
| **AllowRespawn** | Allow the player to respawn in a single-player game without resetting the map. |
| **NoJump** | Disable jumping (this is the default). |
| **AllowJump** | Enable jumping. |
| **CheckSwitchRange** | Check vertical reachability of switches. |
| **NoCheckSwitchRange** | Do not check vertical reachability of switches (this is the default). |
| **ResetHealth** | Reset player health when entering the map. |
| **ResetInventory** | Reset player inventory when entering the map. |
| **UsePlayerStartZ** | Spawn the player at the player start z height (by default, the player spawns on the floor). |
| **NoPassover** | Turns on infinite thing height (objects cannot move over one another). This is the default. |
| **Passover** | Turns off infinite thing height (objects can move over one another). |

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
