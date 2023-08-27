## UDMF (Universal Doom Map Format)

This page defines the `dsda` udmf namespace. The features of this namespace form a subset of the `zdoom` namespace - map features understood by dsda-doom should also be understood by gzdoom, but not necessarily vice versa. See the [Doom in Hexen docs](./doom_in_hexen.md) for more information about which specials and thing types are supported.

In principle, udmf features supported in dsda-doom should work the same as they do in gzdoom. However, there may be maps that do not work in one port or the other due to differences in the underlying physics, similar to other map formats. Significant deviation in behaviour is likely a bug.

It's strongly recommended to configure defaults with [MAPINFO](./mapinfo.md) for cross-port consistency.

DSDA-Doom supports the `zdoom` and `dsda` namespaces and the zdbsp extended gl nodes gln, gl2, and gl3. Properties marked with a `*` are only supported by the opengl renderer.

### Linedefs

| Property | Description |
| --- | --- |
| id _integer_ | Line ID / tag (default -1). |
| v1 _integer_ | Index of the first vertex. |
| v2 _integer_ | Index of the second vertex. |
| blocking _bool_ | Blocks things. |
| blockmonsters _bool_ | Blocks monsters. |
| blockplayers _bool_ | Blocks players. |
| blockfloaters _bool_ | Blocks floaters. |
| blocklandmonsters _bool_ | Block non-flying monsters. |
| blockprojectiles _bool_ | Blocks projectiles. |
| blockhitscan _bool_ | Blocks hitscan. |
| blockuse _bool_ | Blocks use. |
| blocksound _bool_ | Blocks sound. |
| blocksight _bool_ | Blocks sight. |
| blockeverything _bool_ | Blocks everything. |
| twosided _bool_ | Line has two sides. |
| dontpegtop _bool_ | Line upper texture is unpegged. |
| dontpegbottom _bool_ | Line lower texture is unpegged. |
| secret _bool_ | Line is drawn as one-sided on the map. |
| dontdraw _bool_ | Line never shows on the map. |
| mapped _bool_ | Line starts out mapped. |
| revealed _bool_ | Line starts revealed on the map. |
| jumpover _bool_ | Line is a strife-style railing. |
| playercross _bool_ | Can be activated by player cross. |
| playeruse _bool_ | Can be activated by player use. |
| monstercross _bool_ | Can be activated by monster cross. |
| monsteruse _bool_ | Can be activated by monster use. |
| impact _bool_ | Can be activated by projectile impact. |
| playerpush _bool_ | Can be activated by player push. |
| monsterpush _bool_ | Can be activated by monster push. |
| missilecross _bool_ | Can be activated by projectile cross. |
| anycross _bool_ | Can be activated by any non-projectile cross. |
| monsteractivate _bool_ | Can be activated by monsters. |
| playeruseback _bool_ | Can be activated by player use from the back. |
| firstsideonly _bool_ | Can only be activated from the front. |
| passuse _bool_ | Use action passes through the line. |
| checkswitchrange _bool_ | Switches can only be activated when vertically reachable. |
| locknumber _integer_ | **TODO** |
| repeatspecial _bool_ | Repeatable activation. |
| special _integer_ | Special action. |
| arg0 _integer_ | Special argument 0. |
| arg1 _integer_ | Special argument 1. |
| arg2 _integer_ | Special argument 2. |
| arg3 _integer_ | Special argument 3. |
| arg4 _integer_ | Special argument 4. |
| sidefront _integer_ | Index of the first sidedef. |
| sideback _integer_ | Index of the second sidedef. |
| alpha _float_ | Translucency (default is 1.0). |
| translucent _bool_ | Line is 75% opague. |
| transparent _bool_ | Line is 25% opague. |
| clipmidtex _bool_ | Midtextures are clipped by the floor and ceiling. |
| * wrapmidtex _bool_ | Midtextures are wrapped. |
| midtex3d _bool_ | Actors can walk on the midtexture. |
| midtex3dimpassible _bool_ | The midtexture is impassible. |
| moreids _string_ | Space-separated list of extra IDs. |
| automapstyle _integer_ | **TODO** |
| health _integer_ | Line health (for use with damage / death special). |
| healthgroup _integer_ | Group ID of lines sharing health. |
| damagespecial _bool_ | Special activated when receiving damage that does not reduce health to 0. |
| deathspecial _bool_ | Special activated when health is reduced to 0. |

### Sidedefs

| Property | Description |
| --- | --- |
| offsetx _integer_ | X texture offset. |
| offsety _integer_ | Y texture offset. |
| texturetop _string_ | Upper texture. |
| texturebottom _string_ | Lower texture. |
| texturemiddle _string_ | Middle texture. |
| sector _integer_ | Sector index. |
| * scalex_top _float_ | X scale for upper texture (default is 1.0). |
| * scaley_top _float_ | Y scale for upper texture (default is 1.0). |
| * scalex_mid _float_ | X scale for middle texture (default is 1.0). |
| * scaley_mid _float_ | Y scale for middle texture (default is 1.0). |
| * scalex_bottom _float_ | X scale for bottom texture (default is 1.0). |
| * scaley_bottom _float_ | Y scale for bottom texture (default is 1.0). |
| offsetx_top _float_ | X offset for upper texture. |
| offsety_top _float_ | Y offset for upper texture. |
| offsetx_mid _float_ | X offset for middle texture. |
| offsety_mid _float_ | Y offset for middle texture. |
| offsetx_bottom _float_ | X offset for lower texture. |
| offsety_bottom _float_ | Y offset for lower texture. |
| light _integer_ | Light level. |
| light_top _integer_ | Upper texture light level. |
| light_mid _integer_ | Middle texture light level. |
| light_bottom _integer_ | Lower texture light level. |
| lightabsolute _bool_ | Light is absolute rather than relative to the owning sector. |
| lightabsolute_top _bool_ | Upper texture light is absolute rather than relative to the sidedef light level. |
| lightabsolute_mid _bool_ | Middle texture light is absolute rather than relative to the sidedef light level. |
| lightabsolute_bottom _bool_ | Lower texture light is absolute rather than relative to the sidedef light level. |
| nofakecontrast _bool_ | Disables fake contrast on this sidedef. |
| smoothlighting _bool_ | Enables smooth fake contrast on this sidedef. |
| clipmidtex _bool_ | Middle textures are clipped by the floor and ceiling. |
| * wrapmidtex _bool_ | Middle textures are wrapped. |

### Vertices

| Property | Description |
| --- | --- |
| x _float_ | X coordinate. |
| y _float_ | Y coordinate. |

### Sectors

| Property | Description |
| --- | --- |
| heightfloor _integer_ | **TODO** |
| heightceiling _integer_ | **TODO** |
| texturefloor _string_ | **TODO** |
| textureceiling _string_ | **TODO** |
| lightlevel _integer_ | **TODO** |
| special _integer_ | **TODO** |
| id _integer_ | **TODO** |
| xpanningfloor _float_ | **TODO** |
| ypanningfloor _float_ | **TODO** |
| xpanningceiling _float_ | **TODO** |
| ypanningceiling _float_ | **TODO** |
| xscalefloor _float_ | **TODO** |
| yscalefloor _float_ | **TODO** |
| xscaleceiling _float_ | **TODO** |
| yscaleceiling _float_ | **TODO** |
| rotationfloor _float_ | **TODO** |
| rotationceiling _float_ | **TODO** |
| lightfloor _integer_ | **TODO** |
| lightceiling _integer_ | **TODO** |
| lightfloorabsolute _bool_ | **TODO** |
| lightceilingabsolute _bool_ | **TODO** |
| gravity _float_ | **TODO** |
| silent _bool_ | **TODO** |
| noattack _bool_ | **TODO** |
| hidden _bool_ | **TODO** |
| moreids _string_ | **TODO** |
| damageamount _integer_ | **TODO** |
| damageinterval _integer_ | **TODO** |
| leakiness _integer_ | **TODO** |
| damagehazard _bool_ | **TODO** |

### Things

| Property | Description |
| --- | --- |
| id _integer_ | **TODO** |
| x _float_ | **TODO** |
| y _float_ | **TODO** |
| height _float_ | **TODO** |
| angle _integer_ | **TODO** |
| type _integer_ | **TODO** |
| skill1 _bool_ | **TODO** |
| skill2 _bool_ | **TODO** |
| skill3 _bool_ | **TODO** |
| skill4 _bool_ | **TODO** |
| skill5 _bool_ | **TODO** |
| ambush _bool_ | **TODO** |
| single _bool_ | **TODO** |
| dm _bool_ | **TODO** |
| coop _bool_ | **TODO** |
| friend _bool_ | **TODO** |
| dormant _bool_ | **TODO** |
| translucent _bool_ | **TODO** |
| invisible _bool_ | **TODO** |
| special _integer_ | **TODO** |
| arg0 _integer_ | **TODO** |
| arg1 _integer_ | **TODO** |
| arg2 _integer_ | **TODO** |
| arg3 _integer_ | **TODO** |
| arg4 _integer_ | **TODO** |
| countsecret _bool_ | **TODO** |
| gravity _float_ | **TODO** |
| health _float_ | **TODO** |
| alpha _float_ | **TODO** |
