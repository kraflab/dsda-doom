## MAPINFO

MAPINFO is a lump that allows authors to configure various properties of maps, clusters, episodes, and skills. Most features listed here form a subset of gzdoom's MAPINFO specification - they will be understood by both dsda-doom and gzdoom. Some new additions, marked with a :duck:, fill in some gaps. Only the "new" format of mapinfo is valid in dsda-doom (the curly braces). MAPINFO gives access to many features and flags of particular importance to UDMF maps, including its set of special actions and activation rules, jumping, gravity, and air control. It is recommended to only use MAPINFO with UDMF maps - various flags and behavioral features may lead to undefined behavior in legacy formats.

## Usage

By default, dsda-doom does not read MAPINFO. This is done for compatibility purposes, since the MAPINFO lump has been assumed to not be consumed by vanilla-compatible ports. For quick testing, you can temporarily enable MAPINFO parsing via the `-debug_mapinfo` command line option. To enable automatic MAPINFO parsing in your pwad, add a `DSDAPREF` lump containing the line `use_mapinfo` (plain text). See the full specification below.

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

| Property | Description |
| --- | --- |
| **LevelNum = \<Int\>** | The number used in the **Teleport_NewMap** special and for warping (e.g., to record a demo). The default is based on the **MapLump** (e.g., MAP23 yields 23 and E1M8 yields 8). If multiple maps use the same **LevelNum**, only the last map will keep it. |
| **Next = _various_** | The behavior when the normal exit is triggered. The following options are supported: <ul> <li>**"\<MapLump\>":** Enter the given map.</li> <li>**EndPic, "\<Lump\>":** End the game, displaying the given lump as an image.</li> <li>**"EndGame1":** End the game, displaying the image from Doom E1.</li>  <li>**"EndGame2":** End the game, displaying the image from Doom E2.</li>  <li>**"EndGame3":** End the game, displaying the horizontal scroller from Doom E3.</li>  <li>**"EndGame4":** End the game, displaying the image from Doom E4.</li>  <li>**"EndGameC":** End the game, displaying the cast from Doom 2.</li>  <li>**EndGame { _properties_ }:** End the game with custom properties: <ul> <li>**Pic = "\<Lump\>":** Display the given lump as an image. If combined with **Cast**, sets the cast background.</li> <li>**HScroll = "\<RightLump\>", "\<LeftLump\>":** Display the horizontal scroller using the given lumps as images.</li> <li>**Cast:** Display the cast.</li> <li>**Music = "\<Lump\>"[, \<Loop\>]:** Play the given lump as music. Optionally set whether the music should loop (1) or not (0). Music loops by default.</li> </ul> If multiple of **Pic**, **HScroll**, and **Cast** are used, only the last one will be used to decide the end game version. </li> </ul> |
| **SecretNext = _various_** | The behavior when the secret exit is triggered. See the description of **Next** for options. |
| **Cluster = \<Int\>** | The cluster number the map belongs to. |
| **Sky1 = "\<Texture\>"** | The texture used for the sky. |
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
| **NoFreelook** | Disable freelook and manual vertical aiming (this is the default). |
| **AllowFreelook** | Enable freelook and manual vertical aiming. |
| **CheckSwitchRange** | Check vertical reachability of switches. |
| **NoCheckSwitchRange** | Do not check vertical reachability of switches (this is the default). |
| **ResetHealth** | Reset player health when entering the map. |
| **ResetInventory** | Reset player inventory when entering the map. |
| **UsePlayerStartZ** | Spawn the player at the player start z height (by default, the player spawns on the floor). |
| :duck: **NoPassover** | Turns on infinite thing height (objects cannot move over one another). This is the default. |
| :duck: **Passover** | Turns off infinite thing height (objects can move over one another). |
| :duck: **NoGravity** | Turns off gravity ("Gravity = 0" is not cross-port compatible). |
| :duck: **ColorMap = "\<Lump\>"** | The default colormap for the map (if not **COLORMAP**). |
| :duck: **NoVerticalExplosionThrust** | Turns off vertical explosion thrust. This is the default. |
| :duck: **VerticalExplosionThrust** | Turns on vertical explosion thrust. |
| :duck: **ExplodeIn2D** | Explosion damage is based on x and y coordinates. This is the default. |
| :duck: **ExplodeIn3D** | Explosion damage is based on x, y, and z coordinates. |

### Cluster

**cluster \<Number\> { _properties_ }**

Defines a cluster with the given number (used in the map definition). If two cluster definitions use the same **Number**, the second definition will overwrite the first.

#### Cluster Properties

| Property | Description |
| --- | --- |
| **EnterText = "\<Message\>"** | Sets the message to display to the user when they exit another cluster and enter this one. Comma-separated values will create multi-line messages. |
| **ExitText = "\<Message\>"** | Sets the message to display to the user when they exit this cluster and enter another one. Comma-separated values will create multi-line messages. **When leaving a cluster with an exit text and entering a cluster with an enter text, only the enter text is shown**. |
| **Music = "\<Lump\>"** | Sets the music that plays during the text screen. |
| **Flat = "\<Lump\>"** | Sets the background flat used during the text screen. |
| **Pic = "\<Lump\>"** | Sets the background graphic used during the text screen (overrides the **Flat**). |

### Episode

**clearepisodes**

Removes the existing episodes.

**episode \<MapLump\> { _properties_ }**

Defines an episode that starts on the given **MapLump**.

#### Episode Properties

| Property | Description |
| --- | --- |
| **Name = "\<String\>"** | The name of the episode in the menu. |
| **PicName = "\<Lump\>"** | The graphic used for the episode in the menu (replaces the **Name** if every episode has a valid graphic). |
| **Key = "\<Char\>"** | Sets the menu hotkey. |

### Skill

**clearskills**

Removes the existing skill levels.

**skill \<ID\> { _properties_ }**

Defines a new skill level. The **ID** does not show up anywhere in-game. If two definitions use the same **ID**, the second definition will overwrite the first. The default skill level IDs are **baby**, **easy**, **normal**, **hard**, and **nightmare**.

#### Skill Properties

| Property | Description |
| --- | --- |
| **AmmoFactor = \<Float\>** | Sets the ammo pickup multiplier (2.0 means double ammo). |
| **DamageFactor = \<Float\>** | Sets the player damage received multiplier (2.0 means double damage). |
| **ArmorFactor = \<Float\>** | Sets the armor pickup multiplier for armor bonuses and the green armor (2.0 means double armor). |
| **HealthFactor = \<Float\>** | Sets the player healing received multiplier (2.0 means double healing). |
| **MonsterHealth = \<Float\>** | Sets the enemy spawn health multiplier (2.0 means double spawn health). |
| **FriendlyHealth = \<Float\>** | Sets the friend spawn health multiplier (2.0 means double spawn health). |
| **RespawnTime = \<Int\>** | Sets the minimum time in seconds before a monster can respawn (the default is 0, which disables respawning). |
| **SpawnFilter = \<Int\>** | Sets which skill value to filter by when spawning things. For instance, if you want to create a new skill level that spawns the same things as UV, use 4. |
| **Key = "\<Char\>"** | Sets the menu hotkey. |
| **MustConfirm [= "\<Message\>"]** | Prompts the player to confirm when selecting this skill level. Optionally set the message given to the player. |
| **Name = "\<String\>"** | The name of the skill in the menu. |
| **PicName = "\<Lump\>"** | The graphic used for the skill in the menu (replaces the **Name** if every skill has a valid graphic). |
| **TextColor = "\<Color\>"** | Sets the color of the skill in the menu. |
| **SpawnMulti** | Spawns multiplayer objects even in single-player. |
| **FastMonsters** | Turns on fast monsters and projectiles. |
| **InstantReaction** | Allows instant attack reactions by monsters. |
| **NoPain** | Disables the pain state. |
| **DefaultSkill** | Defaults the selector to this skill when entering the skill select. |
| **PlayerRespawn** | Allow the player to respawn without resetting the map in single-player. |
| **EasyBossBrain** | Reduces the rate of boss brain spawns. |
