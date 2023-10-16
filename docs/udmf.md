## UDMF (Universal Doom Map Format)

This page defines the `dsda` udmf namespace. Most features of this namespace form a subset of the `zdoom` namespace - they will be understood by both dsda-doom and gzdoom. Some new additions, marked with a :duck:, fill in some gaps. See the [things and specials doc](./things_and_specials.md) for more information about which specials and thing types are supported.

In principle, udmf features supported by both dsda-doom and gzdoom should work the same in each. However, there may be maps that do not work in one port or the other due to differences in the underlying physics, similar to other map formats. Significant deviation in behaviour is likely a bug.

It's strongly recommended to configure defaults with [MAPINFO](./mapinfo.md) for cross-port consistency.

DSDA-Doom supports the `zdoom` and `dsda` namespaces and the zdbsp extended gl nodes gln, gl2, and gl3. Properties marked with a `*` are only supported by the opengl renderer.

### Linedefs

| Property | Description |
| --- | --- |
| **id** _integer_ | Line ID / tag (default -1). |
| **v1** _integer_ | Index of the first vertex. |
| **v2** _integer_ | Index of the second vertex. |
| **blocking** _bool_ | Blocks things. |
| **blockmonsters** _bool_ | Blocks monsters. |
| **blockplayers** _bool_ | Blocks players. |
| **blockfloaters** _bool_ | Blocks floaters. |
| **blocklandmonsters** _bool_ | Block non-flying monsters. |
| **blockprojectiles** _bool_ | Blocks projectiles. |
| **blockhitscan** _bool_ | Blocks hitscan. |
| **blockuse** _bool_ | Blocks use. |
| **blocksound** _bool_ | Blocks sound. |
| **blocksight** _bool_ | Blocks sight. |
| **blockeverything** _bool_ | Blocks everything. |
| **twosided** _bool_ | Line has two sides. |
| **dontpegtop** _bool_ | Line upper texture is unpegged. |
| **dontpegbottom** _bool_ | Line lower texture is unpegged. |
| **secret** _bool_ | Line is drawn as one-sided on the map. |
| **dontdraw** _bool_ | Line never shows on the map. |
| **mapped** _bool_ | Line starts out mapped. |
| **revealed** _bool_ | Line starts revealed on the map. |
| **jumpover** _bool_ | Line is a strife-style railing. |
| **playercross** _bool_ | Can be activated by player cross. |
| **playeruse** _bool_ | Can be activated by player use. |
| **monstercross** _bool_ | Can be activated by monster cross. |
| **monsteruse** _bool_ | Can be activated by monster use. |
| **impact** _bool_ | Can be activated by projectile impact. |
| **playerpush** _bool_ | Can be activated by player push. |
| **monsterpush** _bool_ | Can be activated by monster push. |
| **missilecross** _bool_ | Can be activated by projectile cross. |
| **anycross** _bool_ | Can be activated by any non-projectile cross. |
| **monsteractivate** _bool_ | Can be activated by monsters. |
| **playeruseback** _bool_ | Can be activated by player use from the back. |
| **firstsideonly** _bool_ | Can only be activated from the front. |
| **passuse** _bool_ | Use action passes through the line. |
| **checkswitchrange** _bool_ | Switches can only be activated when vertically reachable. |
| **locknumber** _integer_ | <ul> <li> 0: No lock. </li> <li> 1: Red key card. </li> <li> 2: Blue key card. </li> <li> 3: Yellow key card. </li> <li> 4: Red skull key. </li> <li> 5: Blue skull key. </li> <li> 6: Yellow skull key. </li> <li> 100: Any key. </li> <li> 101: All 6 keys. </li> <li> 129: Any red key. </li> <li> 130: Any blue key. </li> <li> 131: Any yellow key. </li> <li> 132: Red card or skull. </li> <li> 133: Blue card or skull. </li> <li> 134: Yellow card or skull. </li> <li> 229: Any key of each color. </li> </ul> |
| **repeatspecial** _bool_ | Repeatable activation. |
| **special** _integer_ | Special action. |
| **arg0** _integer_ | Special argument 0. |
| **arg1** _integer_ | Special argument 1. |
| **arg2** _integer_ | Special argument 2. |
| **arg3** _integer_ | Special argument 3. |
| **arg4** _integer_ | Special argument 4. |
| **arg0str** _string_ | Special argument 0 as a string (for certain actions). |
| **sidefront** _integer_ | Index of the first sidedef. |
| **sideback** _integer_ | Index of the second sidedef. |
| **alpha** _float_ | Translucency (default is 1.0). |
| **translucent** _bool_ | Line is 75% opague. |
| **transparent** _bool_ | Line is 25% opague. |
| **clipmidtex** _bool_ | Midtextures are clipped by the floor and ceiling. |
| * **wrapmidtex** _bool_ | Midtextures are wrapped. |
| **midtex3d** _bool_ | Actors can walk on the midtexture. |
| **midtex3dimpassible** _bool_ | The midtexture is impassible. |
| **moreids** _string_ | Space-separated list of extra IDs. |
| **automapstyle** _integer_ | <ul> <li> 0: Automatic (based on properties). </li> <li> 1: One-sided. </li> <li> 2: Two-sided. </li> <li> 3: Different floor levels. </li> <li> 4: Different ceiling levels. </li> <li> 5: Reserved / unused (3D floor border). </li> <li> 6: Wall with special non-door action. </li> <li> 7: Secret. </li> <li> 8: Unseen. </li> <li> 9: Locked. </li> <li> 10: Intra-level teleporter. </li> <li> 11: Map exit. </li> <li> 12: Unseen secret. </li> <li> 13: Reserved / unused (portal). </li> </ul> |
| **health** _integer_ | Line health (for use with damage / death special). |
| **healthgroup** _integer_ | Group ID of lines sharing health. |
| **damagespecial** _bool_ | Special activated when receiving damage that does not reduce health to 0. |
| **deathspecial** _bool_ | Special activated when health is reduced to 0. |

### Sidedefs

| Property | Description |
| --- | --- |
| **offsetx** _integer_ | X texture offset. |
| **offsety** _integer_ | Y texture offset. |
| **texturetop** _string_ | Upper texture. |
| **texturebottom** _string_ | Lower texture. |
| **texturemiddle** _string_ | Middle texture. |
| **sector** _integer_ | Sector index. |
| * **scalex_top** _float_ | X scale for upper texture (default is 1.0). |
| * **scaley_top** _float_ | Y scale for upper texture (default is 1.0). |
| * **scalex_mid** _float_ | X scale for middle texture (default is 1.0). |
| * **scaley_mid** _float_ | Y scale for middle texture (default is 1.0). |
| * **scalex_bottom** _float_ | X scale for bottom texture (default is 1.0). |
| * **scaley_bottom** _float_ | Y scale for bottom texture (default is 1.0). |
| **offsetx_top** _float_ | X offset for upper texture. |
| **offsety_top** _float_ | Y offset for upper texture. |
| **offsetx_mid** _float_ | X offset for middle texture. |
| **offsety_mid** _float_ | Y offset for middle texture. |
| **offsetx_bottom** _float_ | X offset for lower texture. |
| **offsety_bottom** _float_ | Y offset for lower texture. |
| **light** _integer_ | Light level. |
| **light_top** _integer_ | Upper texture light level. |
| **light_mid** _integer_ | Middle texture light level. |
| **light_bottom** _integer_ | Lower texture light level. |
| **lightabsolute** _bool_ | Light is absolute rather than relative to the owning sector. |
| **lightabsolute_top** _bool_ | Upper texture light is absolute rather than relative to the sidedef light level. |
| **lightabsolute_mid** _bool_ | Middle texture light is absolute rather than relative to the sidedef light level. |
| **lightabsolute_bottom** _bool_ | Lower texture light is absolute rather than relative to the sidedef light level. |
| **nofakecontrast** _bool_ | Disables fake contrast on this sidedef. |
| **smoothlighting** _bool_ | Enables smooth fake contrast on this sidedef. |
| **clipmidtex** _bool_ | Middle textures are clipped by the floor and ceiling. |
| * **wrapmidtex** _bool_ | Middle textures are wrapped. |
| :duck: **xscroll** _float_ | X units per frame to scroll line textures. |
| :duck: **yscroll** _float_ | Y units per frame to scroll line textures. |
| :duck: **xscrolltop** _float_ | X units per frame to scroll upper texture. |
| :duck: **yscrolltop** _float_ | Y units per frame to scroll upper texture. |
| :duck: **xscrollmid** _float_ | X units per frame to scroll middle texture. |
| :duck: **yscrollmid** _float_ | Y units per frame to scroll middle texture. |
| :duck: **xscrollbottom** _float_ | X units per frame to scroll lower texture. |
| :duck: **yscrollbottom** _float_ | Y units per frame to scroll lower texture. |

### Vertices

| Property | Description |
| --- | --- |
| **x** _float_ | X coordinate. |
| **y** _float_ | Y coordinate. |

### Sectors

| Property | Description |
| --- | --- |
| **id** _integer_ | Sector ID / tag. |
| **heightfloor** _integer_ | Floor height. |
| **heightceiling** _integer_ | Ceiling height. |
| **texturefloor** _string_ | Floor texture (flat). |
| **textureceiling** _string_ | Ceiling texture (flat). |
| **lightlevel** _integer_ | Light level. |
| **special** _integer_ | Sector special. |
| **xpanningfloor** _float_ | X offset for floor texture. |
| **ypanningfloor** _float_ | Y offset for floor texture. |
| **xpanningceiling** _float_ | X offset for ceiling texture. |
| **ypanningceiling** _float_ | Y offset for ceiling texture. |
| **xscalefloor** _float_ | X scale for floor texture. |
| **yscalefloor** _float_ | Y scale for floor texture. |
| **xscaleceiling** _float_ | X scale for ceiling texture. |
| **yscaleceiling** _float_ | Y scale for ceiling texture. |
| **rotationfloor** _float_ | Rotation of floor texture in degrees. |
| **rotationceiling** _float_ | Rotation of ceiling texture in degrees. |
| **lightfloor** _integer_ | Floor light level. |
| **lightceiling** _integer_ | Ceiling light level. |
| **lightfloorabsolute** _bool_ | Floor light is absolute rather than relative to the sector level. |
| **lightceilingabsolute** _bool_ | Ceiling light is absolute rather than relative to the sector level. |
| **gravity** _float_ | Sector gravity (default is 1.0). |
| **silent** _bool_ | Actors in the sector make no sound. |
| **noattack** _bool_ | Monsters in the sector do not attack. |
| **hidden** _bool_ | Sector will not be drawn on the map. |
| **moreids** _string_ | Space-separated list of extra IDs. |
| **damageamount** _integer_ | Damage inflicted by the sector. Negative numbers heal. |
| **damageinterval** _integer_ | Interval between damage applications (in tics, default is 32). |
| **leakiness** _integer_ | Probability of damage leaking through radiation suit (0 = never, 256 = always). |
| **damagehazard** _bool_ | Use strife-style delayed damage behavior. |
| :duck: **xscrollfloor** _float_ | X units per frame to scroll the floor. |
| :duck: **yscrollfloor** _float_ | Y units per frame to scroll the floor. |
| :duck: **scrollfloormode** _integer_ | Floor scroll mode (see table below). |
| :duck: **xscrollceiling** _float_ | X map units per frame to scroll the ceiling. |
| :duck: **yscrollceiling** _float_ | Y map units per frame to scroll the ceiling. |
| :duck: **scrollceilingmode** _integer_ | Ceiling scroll mode (see table below). |
| :duck: **xthrust** _float_ | X thrust magnitude (map units per frame^2). |
| :duck: **ythrust** _float_ | Y thrust magnitude (map units per frame^2). |
| :duck: **thrustgroup** _integer_ | Thrust group (see table below). |
| :duck: **thrustlocation** _integer_ | Thrust location (see table below). |
| :duck: **colormap** _string_ | Sector colormap. |
| :duck: **skyfloor** _string_ | Floor sky texture. |
| :duck: **skyceiling** _string_ | Ceiling sky texture. |
| :duck: **frictionfactor** _float_ | Friction factor. Ranges from 0 to 1 (default is 0.90625). |
| :duck: **movefactor** _float_ | Multiplier for actor ground movement, normally derived from the friction factor (default is 0.03125). |

#### Scroll Mode

| Flag | Meaning                |
| ---- | ---------------------- |
| 1    | Affect textures        |
| 2    | Affect static objects  |
| 4    | Affect players         |
| 8    | Affect monsters        |

#### Thrust Group

| Flag | Meaning                  |
| ---- | ------------------------ |
| 1    | Affect static objects    |
| 2    | Affect players           |
| 4    | Affect monsters          |
| 8    | Affect projectiles       |
| 16   | Affect WINDTHRUST actors |

#### Thrust Location

| Flag | Meaning                |
| ---- | ---------------------- |
| 1    | Affect grounded actors |
| 2    | Affect airborne actors |
| 4    | Affect ceiling actors  |

### Things

| Property | Description |
| --- | --- |
| **id** _integer_ | Thing ID / tag. |
| **x** _float_ | X coordinate. |
| **y** _float_ | Y coordinate. |
| **height** _float_ | Z displacement relative to the floor (or ceiling for SPAWNCEILING things). |
| **angle** _integer_ | Facing angle in degrees. |
| **type** _integer_ | DoomEdNum. |
| **skill1** _bool_ | Spawn in skill 1. |
| **skill2** _bool_ | Spawn in skill 2. |
| **skill3** _bool_ | Spawn in skill 3. |
| **skill4** _bool_ | Spawn in skill 4. |
| **skill5** _bool_ | Spawn in skill 5. |
| **ambush** _bool_ | Thing is deaf. |
| **single** _bool_ | Spawn in single-player mode. |
| **dm** _bool_ | Spawn in deathmatch mode. |
| **coop** _bool_ | Spawn in coop mode. |
| **friend** _bool_ | Spawn as a friend. |
| **dormant** _bool_ | Spawn dormant. |
| **alpha** _float_ | Translucency (default is 1.0). |
| **translucent** _bool_ | Thing is 66% opague. |
| **invisible** _bool_ | Thing is invisible. |
| **special** _integer_ | Special action. |
| **arg0** _integer_ | Special argument 0. |
| **arg1** _integer_ | Special argument 1. |
| **arg2** _integer_ | Special argument 2. |
| **arg3** _integer_ | Special argument 3. |
| **arg4** _integer_ | Special argument 4. |
| **arg0str** _string_ | Special argument 0 as a string (for certain actions). |
| **countsecret** _bool_ | Picking this thing up counts as a secret. |
| **gravity** _float_ | Thing gravity. Positive values are multiplicative and negative values are absolute (default is 1.0). |
| **health** _float_ | Thing health. Positive values are multiplicative and negative values are absolute (default is 1.0). |
