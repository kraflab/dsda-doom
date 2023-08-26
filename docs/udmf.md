## UDMF (Universal Doom Map Format)

This page defines the `dsda` udmf namespace. The features of this namespace form a subset of the `zdoom` namespace - map features understood by dsda-doom should also be understood by gzdoom, but not necessarily vice versa. See the [Doom in Hexen docs](./doom_in_hexen.md) for more information about which specials and thing types are supported.

In principle, udmf features supported in dsda-doom should work the same as they do in gzdoom. However, there may be maps that do not work in one port or the other due to differences in the underlying physics, similar to other map formats. Significant deviation in behaviour is likely a bug.

It's strongly recommended to configure defaults with [MAPINFO](./mapinfo.md) for cross-port consistency.

DSDA-Doom supports the `zdoom` and `dsda` namespaces and the zdbsp extended gl nodes gln, gl2, and gl3.

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

| Field                 | Status             |
| --------------------- | ------------------ |
| offsetx               | :heavy_check_mark: |
| offsety               | :heavy_check_mark: |
| texturetop            | :heavy_check_mark: |
| texturebottom         | :heavy_check_mark: |
| texturemiddle         | :heavy_check_mark: |
| sector                | :heavy_check_mark: |
| scalex_top            | :ice_cream:        |
| scaley_top            | :ice_cream:        |
| scalex_mid            | :ice_cream:        |
| scaley_mid            | :ice_cream:        |
| scalex_bottom         | :ice_cream:        |
| scaley_bottom         | :ice_cream:        |
| offsetx_top           | :heavy_check_mark: |
| offsety_top           | :heavy_check_mark: |
| offsetx_mid           | :heavy_check_mark: |
| offsety_mid           | :heavy_check_mark: |
| offsetx_bottom        | :heavy_check_mark: |
| offsety_bottom        | :heavy_check_mark: |
| light                 | :heavy_check_mark: |
| light_top             | :heavy_check_mark: |
| light_mid             | :heavy_check_mark: |
| light_bottom          | :heavy_check_mark: |
| lightabsolute         | :heavy_check_mark: |
| lightabsolute_top     | :heavy_check_mark: |
| lightabsolute_mid     | :heavy_check_mark: |
| lightabsolute_bottom  | :heavy_check_mark: |
| nofakecontrast        | :heavy_check_mark: |
| smoothlighting        | :heavy_check_mark: |
| clipmidtex            | :heavy_check_mark: |
| wrapmidtex            | :ice_cream:        |

### Vertices

| Field    | Status             |
| -------- | ------------------ |
| x        | :heavy_check_mark: |
| y        | :heavy_check_mark: |

### Sectors

| Field                    | Status             |
| ------------------------ | ------------------ |
| heightfloor              | :heavy_check_mark: |
| heightceiling            | :heavy_check_mark: |
| texturefloor             | :heavy_check_mark: |
| textureceiling           | :heavy_check_mark: |
| lightlevel               | :heavy_check_mark: |
| special                  | :heavy_check_mark: |
| id                       | :heavy_check_mark: |
| xpanningfloor            | :heavy_check_mark: |
| ypanningfloor            | :heavy_check_mark: |
| xpanningceiling          | :heavy_check_mark: |
| ypanningceiling          | :heavy_check_mark: |
| xscalefloor              | :heavy_check_mark: |
| yscalefloor              | :heavy_check_mark: |
| xscaleceiling            | :heavy_check_mark: |
| yscaleceiling            | :heavy_check_mark: |
| rotationfloor            | :heavy_check_mark: |
| rotationceiling          | :heavy_check_mark: |
| lightfloor               | :heavy_check_mark: |
| lightceiling             | :heavy_check_mark: |
| lightfloorabsolute       | :heavy_check_mark: |
| lightceilingabsolute     | :heavy_check_mark: |
| gravity                  | :heavy_check_mark: |
| silent                   | :heavy_check_mark: |
| noattack                 | :heavy_check_mark: |
| hidden                   | :heavy_check_mark: |
| moreids                  | :heavy_check_mark: |
| damageamount             | :heavy_check_mark: |
| damageinterval           | :heavy_check_mark: |
| leakiness                | :heavy_check_mark: |
| damagehazard             | :heavy_check_mark: |

### Things

| Field         | Status             |
| ------------- | ------------------ |
| id            | :heavy_check_mark: |
| x             | :heavy_check_mark: |
| y             | :heavy_check_mark: |
| height        | :heavy_check_mark: |
| angle         | :heavy_check_mark: |
| type          | :heavy_check_mark: |
| skill1        | :heavy_check_mark: |
| skill2        | :heavy_check_mark: |
| skill3        | :heavy_check_mark: |
| skill4        | :heavy_check_mark: |
| skill5        | :heavy_check_mark: |
| ambush        | :heavy_check_mark: |
| single        | :heavy_check_mark: |
| dm            | :heavy_check_mark: |
| coop          | :heavy_check_mark: |
| friend        | :heavy_check_mark: |
| dormant       | :heavy_check_mark: |
| translucent   | :heavy_check_mark: |
| invisible     | :heavy_check_mark: |
| special       | :heavy_check_mark: |
| arg0          | :heavy_check_mark: |
| arg1          | :heavy_check_mark: |
| arg2          | :heavy_check_mark: |
| arg3          | :heavy_check_mark: |
| arg4          | :heavy_check_mark: |
| countsecret   | :heavy_check_mark: |
| gravity       | :heavy_check_mark: |
| health        | :heavy_check_mark: |
| alpha         | :heavy_check_mark: |
