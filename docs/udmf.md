## UDMF (Universal Doom Map Format)

This page tracks support for the universal doom map format, as seen in ZDoom. It is currently possible to parse and load maps that don't violate existing engine constraints (for instance, using flats as textures). The different features are being evaluated and implemented one by one. The status for any feature, including whether or not it will be supported in the future, is subject to change. See the [Doom in Hexen docs](./doom_in_hexen.md) for more information about which specials and thing types are supported.

### Legend

| Symbol             | Meaning                        |
| ------------------ | ------------------------------ |
| :heavy_check_mark: | Feature is fully supported     |
| :warning:          | Feature is partially supported |
| :grey_question:    | Feature is under investigation |
| :telescope:        | Feature is planned             |
| :comet:            | Feature is planned later       |
| :x:                | Feature is not planned         |

### Namespaces

| Name    | Status    |
| ------- | --------- |
| doom    | :x:       |
| heretic | :x:       |
| hexen   | :x:       |
| strife  | :x:       |
| zdoom   | :warning: |

### Nodes

| Name          | Status             |
| ------------- | ------------------ |
| ZDBSP X/Z GLN | :heavy_check_mark: |
| ZDBSP X/Z GL2 | :grey_question:    |
| ZDBSP X/Z GL3 | :grey_question:    |

### Linedefs

| Field              | Status             |
| ------------------ | ------------------ |
| id                 | :heavy_check_mark: |
| v1                 | :heavy_check_mark: |
| v2                 | :heavy_check_mark: |
| blocking           | :heavy_check_mark: |
| blockmonsters      | :heavy_check_mark: |
| twosided           | :heavy_check_mark: |
| dontpegtop         | :heavy_check_mark: |
| dontpegbottom      | :heavy_check_mark: |
| secret             | :heavy_check_mark: |
| blocksound         | :heavy_check_mark: |
| dontdraw           | :heavy_check_mark: |
| mapped             | :heavy_check_mark: |
| passuse            | :heavy_check_mark: |
| translucent        | :grey_question:    |
| jumpover           | :grey_question:    |
| blockfloaters      | :grey_question:    |
| playercross        | :heavy_check_mark: |
| playeruse          | :heavy_check_mark: |
| monstercross       | :heavy_check_mark: |
| monsteruse         | :grey_question:    |
| impact             | :heavy_check_mark: |
| playerpush         | :heavy_check_mark: |
| monsterpush        | :grey_question:    |
| missilecross       | :heavy_check_mark: |
| repeatspecial      | :heavy_check_mark: |
| special            | :heavy_check_mark: |
| arg0               | :warning:          |
| arg1               | :warning:          |
| arg2               | :warning:          |
| arg3               | :warning:          |
| arg4               | :warning:          |
| sidefront          | :heavy_check_mark: |
| sideback           | :heavy_check_mark: |
| comment            | :x:                |
| alpha              | :x:                |
| renderstyle        | :x:                |
| playeruseback      | :grey_question:    |
| anycross           | :warning:          |
| monsteractivate    | :heavy_check_mark: |
| blockplayers       | :heavy_check_mark: |
| blockeverything    | :heavy_check_mark: |
| firstsideonly      | :grey_question:    |
| zoneboundary       | :grey_question:    |
| clipmidtex         | :grey_question:    |
| wrapmidtex         | :grey_question:    |
| midtex3d           | :grey_question:    |
| midtex3dimpassible | :grey_question:    |
| checkswitchrange   | :grey_question:    |
| blockprojectiles   | :grey_question:    |
| blockuse           | :grey_question:    |
| blocksight         | :grey_question:    |
| blockhitscan       | :grey_question:    |
| locknumber         | :grey_question:    |
| arg0str            | :grey_question:    |
| moreids            | :grey_question:    |
| transparent        | :grey_question:    |
| automapstyle       | :grey_question:    |
| revealed           | :grey_question:    |
| noskywalls         | :grey_question:    |
| drawfullheight     | :grey_question:    |
| health             | :grey_question:    |
| healthgroup        | :grey_question:    |
| damagespecial      | :grey_question:    |
| deathspecial       | :grey_question:    |
| blocklandmonsters  | :grey_question:    |

#### Notes
- `arg*` fields currently have hexen's limit of 0-255.
- `anycross` may not support every case (under investigation).

### Sidedefs

| Field                 | Status             |
| --------------------- | ------------------ |
| offsetx               | :heavy_check_mark: |
| offsety               | :heavy_check_mark: |
| texturetop            | :heavy_check_mark: |
| texturebottom         | :heavy_check_mark: |
| texturemiddle         | :heavy_check_mark: |
| sector                | :heavy_check_mark: |
| comment               | :x:                |
| scalex_top            | :grey_question:    |
| scaley_top            | :grey_question:    |
| scalex_mid            | :grey_question:    |
| scaley_mid            | :grey_question:    |
| scalex_bottom         | :grey_question:    |
| scaley_bottom         | :grey_question:    |
| offsetx_top           | :heavy_check_mark: |
| offsety_top           | :heavy_check_mark: |
| offsetx_mid           | :heavy_check_mark: |
| offsety_mid           | :heavy_check_mark: |
| offsetx_bottom        | :heavy_check_mark: |
| offsety_bottom        | :heavy_check_mark: |
| light                 | :heavy_check_mark: |
| light_top             | :grey_question:    |
| light_mid             | :grey_question:    |
| light_bottom          | :grey_question:    |
| lightabsolute         | :heavy_check_mark: |
| lightabsolute_top     | :grey_question:    |
| lightabsolute_mid     | :grey_question:    |
| lightabsolute_bottom  | :grey_question:    |
| lightfog              | :grey_question:    |
| nofakecontrast        | :grey_question:    |
| smoothlighting        | :grey_question:    |
| clipmidtex            | :grey_question:    |
| wrapmidtex            | :grey_question:    |
| nodecals              | :grey_question:    |
| nogradient_top        | :x:                |
| flipgradient_top      | :x:                |
| clampgradient_top     | :x:                |
| useowncolors_top      | :x:                |
| uppercolor_top        | :x:                |
| lowercolor_top        | :x:                |
| nogradient_mid        | :x:                |
| flipgradient_mid      | :x:                |
| clampgradient_mid     | :x:                |
| useowncolors_mid      | :x:                |
| uppercolor_mid        | :x:                |
| lowercolor_mid        | :x:                |
| nogradient_bottom     | :x:                |
| flipgradient_bottom   | :x:                |
| clampgradient_bottom  | :x:                |
| useowncolors_bottom   | :x:                |
| uppercolor_bottom     | :x:                |
| lowercolor_bottom     | :x:                |
| useowncoloradd_top    | :x:                |
| useowncoloradd_mid    | :x:                |
| useowncoloradd_bottom | :x:                |
| coloradd_top          | :x:                |
| coloradd_mid          | :x:                |
| coloradd_bottom       | :x:                |
| colorization_top      | :x:                |
| colorization_mid      | :x:                |
| colorization_bottom   | :x:                |

### Vertices

| Field    | Status             |
| -------- | ------------------ |
| x        | :heavy_check_mark: |
| y        | :heavy_check_mark: |
| zfloor   | :x:                |
| zceiling | :x:                |

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
| comment                  | :x:                |
| xpanningfloor            | :heavy_check_mark: |
| ypanningfloor            | :heavy_check_mark: |
| xpanningceiling          | :heavy_check_mark: |
| ypanningceiling          | :heavy_check_mark: |
| xscalefloor              | :grey_question:    |
| yscalefloor              | :grey_question:    |
| xscaleceiling            | :grey_question:    |
| yscaleceiling            | :grey_question:    |
| rotationfloor            | :grey_question:    |
| rotationceiling          | :grey_question:    |
| ceilingplane_a           | :x:                |
| ceilingplane_b           | :x:                |
| ceilingplane_c           | :x:                |
| ceilingplane_d           | :x:                |
| floorplane_a             | :x:                |
| floorplane_b             | :x:                |
| floorplane_c             | :x:                |
| floorplane_d             | :x:                |
| lightfloor               | :grey_question:    |
| lightceiling             | :grey_question:    |
| lightfloorabsolute       | :grey_question:    |
| lightceilingabsolute     | :grey_question:    |
| alphafloor               | :x:                |
| alphaceiling             | :x:                |
| renderstylefloor         | :x:                |
| renderstyleceiling       | :x:                |
| gravity                  | :heavy_check_mark: |
| lightcolor               | :x:                |
| fadecolor                | :x:                |
| desaturation             | :x:                |
| silent                   | :grey_question:    |
| nofallingdamage          | :grey_question:    |
| dropactors               | :grey_question:    |
| norespawn                | :grey_question:    |
| soundsequence            | :x:                |
| hidden                   | :grey_question:    |
| waterzone                | :grey_question:    |
| moreids                  | :grey_question:    |
| damageamount             | :grey_question:    |
| damagetype               | :x:                |
| damageinterval           | :grey_question:    |
| leakiness                | :grey_question:    |
| damageterraineffect      | :grey_question:    |
| damagehazard             | :grey_question:    |
| floorterrain             | :x:                |
| ceilingterrain           | :x:                |
| portal_ceil_blocksound   | :x:                |
| portal_ceil_disabled     | :x:                |
| portal_ceil_nopass       | :x:                |
| portal_ceil_norender     | :x:                |
| portal_ceil_overlaytype  | :x:                |
| portal_floor_blocksound  | :x:                |
| portal_floor_disabled    | :x:                |
| portal_floor_nopass      | :x:                |
| portal_floor_norender    | :x:                |
| portal_floor_overlaytype | :x:                |

### Things

| Field         | Status             |
| ------------- | ------------------ |
| id            | :heavy_check_mark: |
| x             | :heavy_check_mark: |
| y             | :heavy_check_mark: |
| height        | :heavy_check_mark: |
| angle         | :heavy_check_mark: |
| type          | :heavy_check_mark: |
| skill1        | :warning:          |
| skill2        | :warning:          |
| skill3        | :heavy_check_mark: |
| skill4        | :warning:          |
| skill5        | :warning:          |
| ambush        | :heavy_check_mark: |
| single        | :heavy_check_mark: |
| dm            | :heavy_check_mark: |
| coop          | :heavy_check_mark: |
| friend        | :heavy_check_mark: |
| dormant       | :heavy_check_mark: |
| class1        | :x:                |
| class2        | :x:                |
| class3        | :x:                |
| standing      | :grey_question:    |
| strifeally    | :grey_question:    |
| translucent   | :heavy_check_mark: |
| invisible     | :heavy_check_mark: |
| special       | :heavy_check_mark: |
| arg0          | :warning:          |
| arg1          | :warning:          |
| arg2          | :warning:          |
| arg3          | :warning:          |
| arg4          | :warning:          |
| comment       | :x:                |
| skill6-16     | :x:                |
| class4-16     | :x:                |
| conversation  | :x:                |
| countsecret   | :grey_question:    |
| arg0str       | :grey_question:    |
| gravity       | :grey_question:    |
| health        | :grey_question:    |
| renderstyle   | :x:                |
| fillcolor     | :x:                |
| alpha         | :x:                |
| score         | :x:                |
| pitch         | :x:                |
| roll          | :x:                |
| scalex        | :grey_question:    |
| scaley        | :grey_question:    |
| scale         | :grey_question:    |
| floatbobphase | :grey_question:    |

#### Notes
- `skill1-2` and `skill4-5` are currently collapsed to the existing `easy` and `hard` flags.
- `arg*` fields currently have hexen's limit of 0-255.
