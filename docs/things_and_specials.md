## Things and Specials

WORK IN PROGRESS

This page documents the new thing types and special actions available in advanced map formats in dsda-doom. DSDA-Doom supports both Doom-in-Hexen and UDMF maps. See the [UDMF documentation](./udmf.md) for complete details about the `dsda` namespace.

### Thing Types

All Vanilla, Boom, and MBF thing types are supported in advanced map formats. This table defines the new entries.

| DoomEdNum | Name | Description |
| --- | --- | --- |
| 9001 | MapSpot | Marks a spot on a map (to be used by special actions). |
| 9013 | MapSpotGravity | Marks a spot on a map, with a marker subject to gravity (to be used by special actions). |
| 9043 | TeleportDest3 | Teleport destination subject to gravity which supports displacement with respect to the floor. |
| 9044 | TeleportDest2 | Teleport destination not subject to gravity which supports displacement with respect to the floor. |
| 9300 | PolyObject Anchor | Anchor defining PolyObject number via angle. |
| 9301 | PolyObject Start Spot (harmless) | Polyobject that aborts movement on touch. |
| 9302 | PolyObject Start Spot (crushing) | Polyobject that crushes. |
| 9303 | PolyObject Start Spot (harmful) | Polyobject that deals damage on touch. |
| 14001-14064 | AmbientSound | Not yet supported! |
| 14065 | Custom AmbientSound | Not yet supported! |
| 14067 | AmbientSoundNoGravity | Not yet supported! |
| 14100-14164 | MusicChanger | Changes music via DoomEdNum (0 to 64). |
| 14165 | Custom MusicChanger | Changes music via special argument (0 to 64). |

### Line Specials

| Value | Name                            | Status             |
| ----- | ------------------------------- | ------------------ |
| 1     | Polyobj_StartLine               | :heavy_check_mark: |
| 2     | Polyobj_RotateLeft              | :heavy_check_mark: |
| 3     | Polyobj_RotateRight             | :heavy_check_mark: |
| 4     | Polyobj_Move                    | :heavy_check_mark: |
| 5     | Polyobj_ExplicitLine            | :heavy_check_mark: |
| 6     | Polyobj_MoveTimes8              | :heavy_check_mark: |
| 7     | Polyobj_DoorSwing               | :heavy_check_mark: |
| 8     | Polyobj_DoorSlide               | :heavy_check_mark: |
| -     | -                               | -                  |
| 10    | Door_Close                      | :heavy_check_mark: |
| 11    | Door_Open                       | :heavy_check_mark: |
| 12    | Door_Raise                      | :heavy_check_mark: |
| 13    | Door_LockedRaise                | :heavy_check_mark: |
| -     | -                               | -                  |
| 17    | Thing_Raise                     | :heavy_check_mark: |
| -     | -                               | -                  |
| 19    | Thing_Stop                      | :heavy_check_mark: |
| 20    | Floor_LowerByValue              | :heavy_check_mark: |
| 21    | Floor_LowerToLowest             | :heavy_check_mark: |
| 22    | Floor_LowerToNearest            | :heavy_check_mark: |
| 23    | Floor_RaiseByValue              | :heavy_check_mark: |
| 24    | Floor_RaiseToHighest            | :heavy_check_mark: |
| 25    | Floor_RaiseToNearest            | :heavy_check_mark: |
| 26    | Stairs_BuildDown                | :heavy_check_mark: |
| 27    | Stairs_BuildUp                  | :heavy_check_mark: |
| 28    | Floor_RaiseAndCrush             | :heavy_check_mark: |
| 29    | Pillar_Build                    | :heavy_check_mark: |
| 30    | Pillar_Open                     | :heavy_check_mark: |
| 31    | Stairs_BuildDownSync            | :heavy_check_mark: |
| 32    | Stairs_BuildUpSync              | :heavy_check_mark: |
| 33    | ForceField                      | :heavy_check_mark: |
| 34    | ClearForceField                 | :heavy_check_mark: |
| 35    | Floor_RaiseByValueTimes8        | :heavy_check_mark: |
| 36    | Floor_LowerByValueTimes8        | :heavy_check_mark: |
| 37    | Floor_MoveToValue               | :heavy_check_mark: |
| 38    | Ceiling_Waggle                  | :heavy_check_mark: |
| 39    | Teleport_ZombieChanger          | :heavy_check_mark: |
| 40    | Ceiling_LowerByValue            | :heavy_check_mark: |
| 41    | Ceiling_RaiseByValue            | :heavy_check_mark: |
| 42    | Ceiling_CrushAndRaise           | :heavy_check_mark: |
| 43    | Ceiling_LowerAndCrush           | :heavy_check_mark: |
| 44    | Ceiling_CrushStop               | :heavy_check_mark: |
| 45    | Ceiling_CrushRaiseAndStay       | :heavy_check_mark: |
| 46    | Floor_CrushStop                 | :heavy_check_mark: |
| 47    | Ceiling_MoveToValue             | :heavy_check_mark: |
| -     | -                               | -                  |
| 52    | Scroll_Wall                     | :heavy_check_mark: |
| 53    | Line_SetTextureOffset           | :heavy_check_mark: |
| 54    | Sector_ChangeFlags              | :warning:          |
| 55    | Line_SetBlocking                | :warning:          |
| 56    | Line_SetTextureScale            | :heavy_check_mark: |
| -     | -                               | -                  |
| 58    | Sector_CopyScroller             | :heavy_check_mark: |
| 59    | Polyobj_OR_MoveToSpot           | :heavy_check_mark: |
| 60    | Plat_PerpetualRaise             | :heavy_check_mark: |
| 61    | Plat_Stop                       | :heavy_check_mark: |
| 62    | Plat_DownWaitUpStay             | :heavy_check_mark: |
| 63    | Plat_DownByValue                | :heavy_check_mark: |
| 64    | Plat_UpWaitDownStay             | :heavy_check_mark: |
| 65    | Plat_UpByValue                  | :heavy_check_mark: |
| 66    | Floor_LowerInstant              | :heavy_check_mark: |
| 67    | Floor_RaiseInstant              | :heavy_check_mark: |
| 68    | Floor_MoveToValueTimes8         | :heavy_check_mark: |
| 69    | Ceiling_MoveToValueTimes8       | :heavy_check_mark: |
| 70    | Teleport                        | :heavy_check_mark: |
| 71    | Teleport_NoFog                  | :heavy_check_mark: |
| 72    | ThrustThing                     | :heavy_check_mark: |
| 73    | DamageThing                     | :heavy_check_mark: |
| 74    | Teleport_NewMap                 | :warning:          |
| 75    | Teleport_EndGame                | :warning:          |
| 76    | TeleportOther                   | :heavy_check_mark: |
| 77    | TeleportGroup                   | :heavy_check_mark: |
| 78    | TeleportInSector                | :heavy_check_mark: |
| -     | -                               | -                  |
| 86    | Polyobj_MoveToSpot              | :heavy_check_mark: |
| 87    | Polyobj_Stop                    | :heavy_check_mark: |
| 88    | Polyobj_MoveTo                  | :heavy_check_mark: |
| 89    | Polyobj_OR_MoveTo               | :heavy_check_mark: |
| 90    | Polyobj_OR_RotateLeft           | :heavy_check_mark: |
| 91    | Polyobj_OR_RotateRight          | :heavy_check_mark: |
| 92    | Polyobj_OR_Move                 | :heavy_check_mark: |
| 93    | Polyobj_OR_MoveTimes8           | :heavy_check_mark: |
| 94    | Pillar_BuildAndCrush            | :heavy_check_mark: |
| 95    | FloorAndCeiling_LowerByValue    | :heavy_check_mark: |
| 96    | FloorAndCeiling_RaiseByValue    | :heavy_check_mark: |
| 97    | Ceiling_LowerAndCrushDist       | :heavy_check_mark: |
| -     | -                               | -                  |
| 99    | Floor_RaiseAndCrushDoom         | :heavy_check_mark: |
| 100   | Scroll_Texture_Left             | :heavy_check_mark: |
| 101   | Scroll_Texture_Right            | :heavy_check_mark: |
| 102   | Scroll_Texture_Up               | :heavy_check_mark: |
| 103   | Scroll_Texture_Down             | :heavy_check_mark: |
| 104   | Ceiling_CrushAndRaiseSilentDist | :heavy_check_mark: |
| 105   | Door_WaitRaise                  | :heavy_check_mark: |
| 106   | Door_WaitClose                  | :heavy_check_mark: |
| -     | -                               | -                  |
| 110   | Light_RaiseByValue              | :heavy_check_mark: |
| 111   | Light_LowerByValue              | :heavy_check_mark: |
| 112   | Light_ChangeToValue             | :heavy_check_mark: |
| 113   | Light_Fade                      | :heavy_check_mark: |
| 114   | Light_Glow                      | :heavy_check_mark: |
| 115   | Light_Flicker                   | :heavy_check_mark: |
| 116   | Light_Strobe                    | :heavy_check_mark: |
| 117   | Light_Stop                      | :heavy_check_mark: |
| -     | -                               | -                  |
| 119   | Thing_Damage                    | :heavy_check_mark: |
| 120   | Radius_Quake                    | :heavy_check_mark: |
| 121   | Line_SetIdentification          | :warning:          |
| -     | -                               | -                  |
| 125   | Thing_Move                      | :heavy_check_mark: |
| -     | -                               | -                  |
| 127   | Thing_SetSpecial                | :heavy_check_mark: |
| 128   | ThrustThingZ                    | :heavy_check_mark: |
| -     | -                               | -                  |
| 130   | Thing_Activate                  | :heavy_check_mark: |
| 131   | Thing_Deactivate                | :heavy_check_mark: |
| 132   | Thing_Remove                    | :heavy_check_mark: |
| 133   | Thing_Destroy                   | :heavy_check_mark: |
| 134   | Thing_Projectile                | :heavy_check_mark: |
| 135   | Thing_Spawn                     | :heavy_check_mark: |
| 136   | Thing_ProjectileGravity         | :heavy_check_mark: |
| 137   | Thing_SpawnNoFog                | :heavy_check_mark: |
| 138   | Floor_Waggle                    | :heavy_check_mark: |
| 139   | Thing_SpawnFacing               | :heavy_check_mark: |
| -     | -                               | -                  |
| 154   | Teleport_NoStop                 | :heavy_check_mark: |
| -     | -                               | -                  |
| 168   | Ceiling_CrushAndRaiseDist       | :heavy_check_mark: |
| 169   | Generic_Crusher2                | :heavy_check_mark: |
| 170   | Sector_SetCeilingScale2         | :heavy_check_mark: |
| 171   | Sector_SetFloorScale2           | :heavy_check_mark: |
| 172   | Plat_UpNearestWaitDownStay      | :heavy_check_mark: |
| 173   | NoiseAlert                      | :warning:          |
| -     | -                               | -                  |
| 176   | Thing_ChangeTID                 | :heavy_check_mark: |
| 177   | Thing_Hate                      | :warning:          |
| 178   | Thing_ProjectileAimed           | :heavy_check_mark: |
| -     | -                               | -                  |
| 185   | Sector_SetRotation              | :heavy_check_mark: |
| 186   | Sector_SetCeilingPanning        | :heavy_check_mark: |
| 187   | Sector_SetFloorPanning          | :heavy_check_mark: |
| 188   | Sector_SetCeilingScale          | :heavy_check_mark: |
| 189   | Sector_SetFloorScale            | :heavy_check_mark: |
| 190   | Static_Init                     | :warning:          |
| -     | -                               | -                  |
| 192   | Ceiling_LowerToHighestFloor     | :heavy_check_mark: |
| 193   | Ceiling_LowerInstant            | :heavy_check_mark: |
| 194   | Ceiling_RaiseInstant            | :heavy_check_mark: |
| 195   | Ceiling_CrushRaiseAndStayA      | :heavy_check_mark: |
| 196   | Ceiling_CrushAndRaiseA          | :heavy_check_mark: |
| 197   | Ceiling_CrushAndRaiseSilentA    | :heavy_check_mark: |
| 198   | Ceiling_RaiseByValueTimes8      | :heavy_check_mark: |
| 199   | Ceiling_LowerByValueTimes8      | :heavy_check_mark: |
| 200   | Generic_Floor                   | :heavy_check_mark: |
| 201   | Generic_Ceiling                 | :heavy_check_mark: |
| 202   | Generic_Door                    | :heavy_check_mark: |
| 203   | Generic_Lift                    | :heavy_check_mark: |
| 204   | Generic_Stairs                  | :heavy_check_mark: |
| 205   | Generic_Crusher                 | :heavy_check_mark: |
| 206   | Plat_DownWaitUpStayLip          | :heavy_check_mark: |
| 207   | Plat_PerpetualRaiseLip          | :heavy_check_mark: |
| 208   | TranslucentLine                 | :warning:          |
| 209   | Transfer_Heights                | :heavy_check_mark: |
| 210   | Transfer_FloorLight             | :heavy_check_mark: |
| 211   | Transfer_CeilingLight           | :heavy_check_mark: |
| -     | -                               | -                  |
| 214   | Sector_SetDamage                | :heavy_check_mark: |
| 215   | Teleport_Line                   | :heavy_check_mark: |
| 216   | Sector_SetGravity               | :heavy_check_mark: |
| 217   | Stairs_BuildUpDoom              | :heavy_check_mark: |
| 218   | Sector_SetWind                  | :heavy_check_mark: |
| 219   | Sector_SetFriction              | :heavy_check_mark: |
| 220   | Sector_SetCurrent               | :heavy_check_mark: |
| 221   | Scroll_Texture_Both             | :heavy_check_mark: |
| 222   | Scroll_Texture_Model            | :heavy_check_mark: |
| 223   | Scroll_Floor                    | :heavy_check_mark: |
| 224   | Scroll_Ceiling                  | :heavy_check_mark: |
| 225   | Scroll_Texture_Offsets          | :heavy_check_mark: |
| -     | -                               | -                  |
| 227   | PointPush_SetForce              | :heavy_check_mark: |
| 228   | Plat_RaiseAndStayTx0            | :heavy_check_mark: |
| -     | -                               | -                  |
| 230   | Plat_UpByValueStayTx            | :heavy_check_mark: |
| 231   | Plat_ToggleCeiling              | :heavy_check_mark: |
| 232   | Light_StrobeDoom                | :heavy_check_mark: |
| 233   | Light_MinNeighbor               | :heavy_check_mark: |
| 234   | Light_MaxNeighbor               | :heavy_check_mark: |
| 235   | Floor_TransferTrigger           | :heavy_check_mark: |
| 236   | Floor_TransferNumeric           | :heavy_check_mark: |
| -     | -                               | -                  |
| 238   | Floor_RaiseToLowestCeiling      | :heavy_check_mark: |
| 239   | Floor_RaiseByValueTxTy          | :heavy_check_mark: |
| 240   | Floor_RaiseByTexture            | :heavy_check_mark: |
| 241   | Floor_LowerToLowestTxTy         | :heavy_check_mark: |
| 242   | Floor_LowerToHighest            | :heavy_check_mark: |
| 243   | Exit_Normal                     | :heavy_check_mark: |
| 244   | Exit_Secret                     | :heavy_check_mark: |
| 245   | Elevator_RaiseToNearest         | :heavy_check_mark: |
| 246   | Elevator_MoveToFloor            | :heavy_check_mark: |
| 247   | Elevator_LowerToNearest         | :heavy_check_mark: |
| 248   | HealThing                       | :heavy_check_mark: |
| 249   | Door_CloseWaitOpen              | :heavy_check_mark: |
| 250   | Floor_Donut                     | :heavy_check_mark: |
| 251   | FloorAndCeiling_LowerRaise      | :heavy_check_mark: |
| 252   | Ceiling_RaiseToNearest          | :heavy_check_mark: |
| 253   | Ceiling_LowerToLowest           | :heavy_check_mark: |
| 254   | Ceiling_LowerToFloor            | :heavy_check_mark: |
| 255   | Ceiling_CrushRaiseAndStaySilA   | :heavy_check_mark: |
| 256   | Floor_LowerToHighestEE          | :heavy_check_mark: |
| 257   | Floor_RaiseToLowest             | :heavy_check_mark: |
| 258   | Floor_LowerToLowestCeiling      | :heavy_check_mark: |
| 259   | Floor_RaiseToCeiling            | :heavy_check_mark: |
| 260   | Floor_ToCeilingInstant          | :heavy_check_mark: |
| 261   | Floor_LowerByTexture            | :heavy_check_mark: |
| 262   | Ceiling_RaiseToHighest          | :heavy_check_mark: |
| 263   | Ceiling_ToHighestInstant        | :heavy_check_mark: |
| 264   | Ceiling_LowerToNearest          | :heavy_check_mark: |
| 265   | Ceiling_RaiseToLowest           | :heavy_check_mark: |
| 266   | Ceiling_RaiseToHighestFloor     | :heavy_check_mark: |
| 267   | Ceiling_ToFloorInstant          | :heavy_check_mark: |
| 268   | Ceiling_RaiseByTexture          | :heavy_check_mark: |
| 269   | Ceiling_LowerByTexture          | :heavy_check_mark: |
| 270   | Stairs_BuildDownDoom            | :heavy_check_mark: |
| 271   | Stairs_BuildUpDoomSync          | :heavy_check_mark: |
| 272   | Stairs_BuildDownDoomSync        | :heavy_check_mark: |
| 273   | Stairs_BuildUpDoomCrush         | :heavy_check_mark: |
| -     | -                               | -                  |
| 275   | Floor_Stop                      | :heavy_check_mark: |
| 276   | Ceiling_Stop                    | :heavy_check_mark: |
| -     | -                               | -                  |
| 279   | Floor_MoveToValueAndCrush       | :heavy_check_mark: |
| 280   | Ceiling_MoveToValueAndCrush     | :heavy_check_mark: |
| 281   | Line_SetAutomapFlags            | :heavy_check_mark: |
| 282   | Line_SetAutomapStyle            | :heavy_check_mark: |
| -     | -                               | -                  |

#### Notes

- Specials above 255 are not accessible in hexen format.
- The static init supports damage, gravity, and skies only.
- Teleporting to a specific map currently...
  - Always triggers the intermission screen
  - Does not know about clusters (e.g., you can't bring keys through the exit)
- There may be undefined behaviour when combining new exit specials with UMAPINFO
- Line flags coming from extra arguments are ignored.
- TranslucentLine does not support additive translucency.
- Line_SetBlocking sight, hitscan, and sound are not possible in hexen format.
- Sector_ChangeFlags only supports silent, friction, and push in hexen format.
  - UDMF additionally supports endgodmode, endlevel, hazard, and noattack.
- NoiseAlert only works with the default arguments.
- Thing_Hate only supports 0 for the third argument ("target only").
- ThrustThing has an implicit speed limit.
- Poly objects have no sound.
- Poly objects may have visual errors in software mode.

### Sector Specials

| Value   | Name                     | Status             |
| ------- | ------------------------ | ------------------ |
| *       | Generalized Effects      | :heavy_check_mark: |
| 1       | Light_Phased             | :heavy_check_mark: |
| 2-4     | LightSequence*           | :heavy_check_mark: |
| -       | -                        | -                  |
| 26-27   | Stairs_Special*          | :heavy_check_mark: |
| -       | -                        | -                  |
| 40-51   | Wind*                    | :heavy_check_mark: |
| -       | -                        | -                  |
| 65      | dLight_Flicker           | :heavy_check_mark: |
| 66      | dLight_StrobeFast        | :heavy_check_mark: |
| 67      | dLight_StrobeSlow        | :heavy_check_mark: |
| 68      | dLight_Strobe_Hurt       | :heavy_check_mark: |
| 69      | dDamage_Hellslime        | :heavy_check_mark: |
| -       | -                        | -                  |
| 71      | dDamage_Nukage           | :heavy_check_mark: |
| 72      | dLight_Glow              | :heavy_check_mark: |
| -       | -                        | -                  |
| 74      | dSector_DoorCloseIn30    | :heavy_check_mark: |
| 75      | dDamage_End              | :heavy_check_mark: |
| 76      | dLight_StrobeSlowSync    | :heavy_check_mark: |
| 77      | dLight_StrobeFastSync    | :heavy_check_mark: |
| 78      | dSector_DoorRaiseIn5Mins | :heavy_check_mark: |
| 79      | dFriction_Low            | :heavy_check_mark: |
| 80      | dDamage_SuperHellslime   | :heavy_check_mark: |
| 81      | dLight_FireFlicker       | :heavy_check_mark: |
| 82      | dDamage_LavaWimpy        | :heavy_check_mark: |
| 83      | dDamage_LavaHefty        | :heavy_check_mark: |
| 84      | dScroll_EastLavaDamage   | :heavy_check_mark: |
| 85      | hDamage_Sludge           | :heavy_check_mark: |
| -       | -                        | -                  |
| 104     | sLight_Strobe_Hurt       | :heavy_check_mark: |
| 105     | sDamage_Hellslime        | :heavy_check_mark: |
| -       | -                        | -                  |
| 115     | Damage_InstantDeath      | :heavy_check_mark: |
| 116     | sDamage_SuperHellslime   | :heavy_check_mark: |
| -       | -                        | -                  |
| 118     | Scroll_StrifeCurrent     | :heavy_check_mark: |
| -       | -                        | -                  |
| 195     | Sector_Hidden            | :heavy_check_mark: |
| 196     | Sector_Heal              | :heavy_check_mark: |
| -       | -                        | -                  |
| 201-224 | Scroll*                  | :heavy_check_mark: |
| 225-244 | Carry*                   | :heavy_check_mark: |

#### Notes

- Terrain effects (e.g., from lava specials) don't exist.
