## Doom in Hexen

This page tracks support for the "Doom in Hexen" map format and related features, as seen in ZDoom.

### Current Status

Current work is isolated to the initial pass over the level format itself - line and sector specials, necessary changes to internal data structures, etc. Lots of refactoring needs to be done in the engine itself in order to treat different formats correctly without breaking compatibility.

### Features

| Name         | Supported |
|--------------|-----------|
| Levels       | Yes       |
| Poly Objects | No        |
| ACS          | No        |
| MAPINFO      | No        |
| SNDINFO      | No        |
| SNDSEQ       | No        |
| ANIMDEFS     | No        |

### Line Specials

- To Do

### Sector Specials

| Value   | Name                     | Supported |
|---------|--------------------------|-----------|
| *       | Generalized Effects      | Yes       |
| 1       | Light_Phased             | Yes       |
| 2-4     | LightSequence*           | ?         |
| -       | -                        | -         |
| 26-27   | Stairs_Special*          | ?         |
| -       | -                        | -         |
| 40-51   | Wind*                    | Yes       |
| -       | -                        | -         |
| 65      | dLight_Flicker           | Yes       |
| 66      | dLight_StrobeFast        | Yes       |
| 67      | dLight_StrobeSlow        | Yes       |
| 68      | dLight_Strobe_Hurt       | Yes       |
| 69      | dDamage_Hellslime        | Yes       |
| -       | -                        | -         |
| 71      | dDamage_Nukage           | Yes       |
| 72      | dLight_Glow              | Yes       |
| -       | -                        | -         |
| 74      | dSector_DoorCloseIn30    | Yes       |
| 75      | dDamage_End              | Yes       |
| 76      | dLight_StrobeSlowSync    | Yes       |
| 77      | dLight_StrobeFastSync    | Yes       |
| 78      | dSector_DoorRaiseIn5Mins | Yes       |
| 79      | dFriction_Low            | Yes       |
| 80      | dDamage_SuperHellslime   | Yes       |
| 81      | dLight_FireFlicker       | Yes       |
| 82      | dDamage_LavaWimpy        | Yes       |
| 83      | dDamage_LavaHefty        | Yes       |
| 84      | dScroll_EastLavaDamage   | Yes       |
| 85      | hDamage_Sludge           | Yes       |
| -       | -                        | -         |
| 87      | Sector_Outside           | No        |
| -       | -                        | -         |
| 104     | sLight_Strobe_Hurt       | Yes       |
| 105     | sDamage_Hellslime        | Yes       |
| -       | -                        | -         |
| 115     | Damage_InstantDeath      | Yes       |
| 116     | sDamage_SuperHellslime   | Yes       |
| -       | -                        | -         |
| 118     | Scroll_StrifeCurrent     | No        |
| -       | -                        | -         |
| 195     | Sector_Hidden            | Yes       |
| 196     | Sector_Heal              | Yes       |
| 197     | Light_OutdoorLightning   | No        |
| 198-199 | Light_IndoorLightning*   | No        |
| 200     | Sky2                     | No        |
| 201-224 | Scroll*                  | Yes       |
| 225-244 | Carry*                   | Yes       |
