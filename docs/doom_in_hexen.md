## Doom in Hexen

This page tracks support for the "Doom in Hexen" map format and related features, as seen in ZDoom. This is all a work in progress - the status of any feature (including whether or not it is planned) is subject to change.

### Current Status

Current work is isolated to the initial pass over the level format itself - line and sector specials, necessary changes to internal data structures, etc. Lots of refactoring needs to be done in the engine itself in order to treat different formats correctly without breaking compatibility.

### Legend

| Symbol             | Meaning                        |
| ------------------ | ------------------------------ |
| :heavy_check_mark: | Feature is supported           |
| :warning:          | Feature is partially supported |
| :telescope:        | Feature is planned             |
| :grey_question:    | Feature is under investigation |
| :x:                | Feature is not planned         |

### Major Features

| Name         | Status      |
| ------------ | ----------- |
| Levels       | :warning:   |
| Poly Objects | :telescope: |
| ACS          | :telescope: |
| MAPINFO      | :telescope: |
| SNDINFO      | :telescope: |
| SNDSEQ       | :telescope: |
| ANIMDEFS     | :telescope: |

### Thing Types

| Value       | Name                    | Status             |
| ----------- | ----------------------- | ------------------ |
| 118         | ZBridge                 | :x:                |
| -           | -                       | -                  |
| 888         | MBFHelperDog            | :heavy_check_mark: |
| -           | -                       | -                  |
| 1400-1409   | Sound Sequence          | :telescope:        |
| 1411        | Sound Sequence Override | :telescope:        |
| -           | -                       | -                  |
| 1500-1501   | Line Slopes             | :x:                |
| -           | -                       | -                  |
| 1504-1505   | Vertex Slopes           | :x:                |
| -           | -                       | -                  |
| 4001-4004   | Player 5-8 Start        | :x:                |
| 5001        | PointPusher             | :heavy_check_mark: |
| 5002        | PointPuller             | :heavy_check_mark: |
| -           | -                       | -                  |
| 5010        | Pistol                  | :x:                |
| -           | -                       | -                  |
| 5050        | Stalagmite              | :x:                |
| -           | -                       | -                  |
| 5061        | InvisibleBridge32       | :x:                |
| -           | -                       | -                  |
| 5064        | InvisibleBridge16       | :x:                |
| 5065        | InvisibleBridge8        | :x:                |
| -           | -                       | -                  |
| 9001        | MapSpot                 | :telescope:        |
| -           | -                       | -                  |
| 9013        | MapSpotGravity          | :telescope:        |
| -           | -                       | -                  |
| 9024        | PatrolPoint             | :x:                |
| 9025        | SecurityCamera          | :x:                |
| 9026        | Spark                   | :x:                |
| 9027        | RedParticleFountain     | :x:                |
| 9028        | GreenParticleFountain   | :x:                |
| 9029        | BlueParticleFountain    | :x:                |
| 9030        | YellowParticleFountain  | :x:                |
| 9031        | PurpleParticleFountain  | :x:                |
| 9032        | BlackParticleFountain   | :x:                |
| 9033        | WhiteParticleFountain   | :x:                |
| -           | -                       | -                  |
| 9037        | BetaSkull               | :x:                |
| 9038        | ColorSetter             | :x:                |
| 9039        | FadeSetter              | :x:                |
| 9040        | MapMarker               | :x:                |
| 9041        | SectorFlagSetter        | :x:                |
| -           | -                       | -                  |
| 9043-9044   | TeleportDest*           | :telescope:        |
| 9045        | Waterzone               | :x:                |
| 9046        | SecretTrigger           | :x:                |
| 9047        | PatrolSpecial           | :x:                |
| 9048        | SoundEnvironment        | :x:                |
| -           | -                       | -                  |
| 9050-9061   | Stealth*                | :x:                |
| -           | -                       | -                  |
| 9070        | InterpolationPoint      | :x:                |
| 9071        | PathFollower            | :x:                |
| 9072        | MovingCamera            | :x:                |
| 9073        | AimingCamera            | :x:                |
| 9074        | ActorMover              | :x:                |
| 9075        | InterpolationSpecial    | :x:                |
| 9076        | HateTarget              | :x:                |
| 9077        | UpperStackLookOnly      | :x:                |
| 9078        | LowerStackLookOnly      | :x:                |
| -           | -                       | -                  |
| 9080        | SkyViewpoint            | :x:                |
| 9081        | SkyPicker               | :x:                |
| 9082        | SectorSilencer          | :x:                |
| 9083        | SkyCamCompat            | :x:                |
| -           | -                       | -                  |
| 9100        | ScriptedMarine          | :x:                |
| 9101-9111   | Marine*                 | :x:                |
| -           | -                       | -                  |
| 9200        | Decal                   | :x:                |
| -           | -                       | -                  |
| 9300-9303   | PolyObject*             | :telescope:        |
| -           | -                       | -                  |
| 9500-9503   | Slopes                  | :x:                |
| -           | -                       | -                  |
| 9510-9511   | Copy Planes             | :x:                |
| -           | -                       | -                  |
| 9982        | SecActEyesAboveC        | :x:                |
| 9983        | SecActEyesBelowC        | :x:                |
| -           | -                       | -                  |
| 9988        | CustomSprite            | :x:                |
| 9989        | SecActHitFakeFloor      | :x:                |
| 9990        | InvisibleBridge         | :x:                |
| 9991        | CustomBridge            | :x:                |
| 9992        | SecActEyesSurface       | :x:                |
| 9993        | SecActEyesDive          | :x:                |
| 9994        | SecActUseWall           | :x:                |
| 9995        | SecActUse               | :x:                |
| 9996        | SecActHitCeil           | :x:                |
| 9997        | SecActExit              | :x:                |
| 9998        | SecActEnter             | :x:                |
| 9999        | SecActHitFloor          | :x:                |
| -           | -                       | -                  |
| 14001-14064 | AmbientSound            | :telescope:        |
| 14065       | Custom AmbientSound     | :telescope:        |
| 14066       | SoundSequence           | :telescope:        |
| 14067       | AmbientSoundNoGravity   | :telescope:        |
| -           | -                       | -                  |
| 14101-14164 | MusicChanger            | :heavy_check_mark: |
| 14165       | Custom MusicChanger     | :telescope:        |

### Spawn Flags

| Name        | Status             |
| ----------- | ------------------ |
| Dormant     | :warning:          |
| Translucent | :heavy_check_mark: |
| Invisible   | :heavy_check_mark: |
| Friendly    | :heavy_check_mark: |
| Stand Still | :x:                |

#### Notes
- Dormant monsters still bleed.

### Line Triggers

| Name                       | Status             |
| -------------------------- | ------------------ |
| Player Walkover            | :heavy_check_mark: |
| Player Use                 | :heavy_check_mark: |
| Monster Walkover           | :heavy_check_mark: |
| Projectile Hits            | :heavy_check_mark: |
| Player Bumps               | :heavy_check_mark: |
| Projectile Crosses         | :heavy_check_mark: |
| Player Use (Pass Through)  | :heavy_check_mark: |
| Projectile Hits or Crosses | :heavy_check_mark: |

### Line Flags

| Name              | Status             |
| ----------------- | ------------------ |
| Repeatable Action | :heavy_check_mark: |
| Monster Activates | :heavy_check_mark: |
| Block Everything  | :heavy_check_mark: |
| Block Players     | :heavy_check_mark: |

### Line Specials

| Value | Name                            | Status             |
| ----- | ------------------------------- | ------------------ |
| 1     | Polyobj_StartLine               | :telescope:        |
| 2     | Polyobj_RotateLeft              | :telescope:        |
| 3     | Polyobj_RotateRight             | :telescope:        |
| 4     | Polyobj_Move                    | :telescope:        |
| 5     | Polyobj_ExplicitLine            | :telescope:        |
| 6     | Polyobj_MoveTimes8              | :telescope:        |
| 7     | Polyobj_DoorSwing               | :telescope:        |
| 8     | Polyobj_DoorSlide               | :telescope:        |
| 9     | Line_Horizon                    | :telescope:        |
| 10    | Door_Close                      | :heavy_check_mark: |
| 11    | Door_Open                       | :heavy_check_mark: |
| 12    | Door_Raise                      | :heavy_check_mark: |
| 13    | Door_LockedRaise                | :heavy_check_mark: |
| 14    | Door_Animated                   | :x:                |
| 15    | Autosave                        | :x:                |
| 16    | Transfer_WallLight              | :grey_question:    |
| 17    | Thing_Raise                     | :grey_question:    |
| 18    | StartConversation               | :x:                |
| 19    | Thing_Stop                      | :grey_question:    |
| 20    | Floor_LowerByValue              | :grey_question:    |
| 21    | Floor_LowerToLowest             | :grey_question:    |
| 22    | Floor_LowerToNearest            | :grey_question:    |
| 23    | Floor_RaiseByValue              | :grey_question:    |
| 24    | Floor_RaiseToHighest            | :grey_question:    |
| 25    | Floor_RaiseToNearest            | :grey_question:    |
| 26    | Stairs_BuildDown                | :grey_question:    |
| 27    | Stairs_BuildUp                  | :grey_question:    |
| 28    | Floor_RaiseAndCrush             | :grey_question:    |
| 29    | Pillar_Build                    | :grey_question:    |
| 30    | Pillar_Open                     | :grey_question:    |
| 31    | Stairs_BuildDownSync            | :grey_question:    |
| 32    | Stairs_BuildUpSync              | :grey_question:    |
| 33    | ForceField                      | :grey_question:    |
| 34    | ClearForceField                 | :grey_question:    |
| 35    | Floor_RaiseByValueTimes8        | :grey_question:    |
| 36    | Floor_LowerByValueTimes8        | :grey_question:    |
| 37    | Floor_MoveToValue               | :grey_question:    |
| 38    | Ceiling_Waggle                  | :grey_question:    |
| 39    | Teleport_ZombieChanger          | :grey_question:    |
| 40    | Ceiling_LowerByValue            | :grey_question:    |
| 41    | Ceiling_RaiseByValue            | :grey_question:    |
| 42    | Ceiling_CrushAndRaise           | :grey_question:    |
| 43    | Ceiling_LowerAndCrush           | :grey_question:    |
| 44    | Ceiling_CrushStop               | :grey_question:    |
| 45    | Ceiling_CrushRaiseAndStay       | :grey_question:    |
| 46    | Floor_CrushStop                 | :grey_question:    |
| 47    | Ceiling_MoveToValue             | :grey_question:    |
| 48    | Sector_Attach3dMidtex           | :x:                |
| 49    | GlassBreak                      | :grey_question:    |
| 50    | ExtraFloor_LightOnly            | :grey_question:    |
| 51    | Sector_SetLink                  | :grey_question:    |
| 52    | Scroll_Wall                     | :grey_question:    |
| 53    | Line_SetTextureOffset           | :grey_question:    |
| 54    | Sector_ChangeFlags              | :grey_question:    |
| 55    | Line_SetBlocking                | :grey_question:    |
| 56    | Line_SetTextureScale            | :grey_question:    |
| 57    | Sector_SetPortal                | :x:                |
| 58    | Sector_CopyScroller             | :heavy_check_mark: |
| 59    | Polyobj_OR_MoveToSpot           | :grey_question:    |
| 60    | Plat_PerpetualRaise             | :grey_question:    |
| 61    | Plat_Stop                       | :grey_question:    |
| 62    | Plat_DownWaitUpStay             | :grey_question:    |
| 63    | Plat_DownByValue                | :grey_question:    |
| 64    | Plat_UpWaitDownStay             | :grey_question:    |
| 65    | Plat_UpByValue                  | :grey_question:    |
| 66    | Floor_LowerInstant              | :grey_question:    |
| 67    | Floor_RaiseInstant              | :grey_question:    |
| 68    | Floor_MoveToValueTimes8         | :grey_question:    |
| 69    | Ceiling_MoveToValueTimes8       | :grey_question:    |
| 70    | Teleport                        | :grey_question:    |
| 71    | Teleport_NoFog                  | :grey_question:    |
| 72    | ThrustThing                     | :grey_question:    |
| 73    | DamageThing                     | :grey_question:    |
| 74    | Teleport_NewMap                 | :grey_question:    |
| 75    | Teleport_EndGame                | :grey_question:    |
| 76    | TeleportOther                   | :grey_question:    |
| 77    | TeleportGroup                   | :grey_question:    |
| 78    | TeleportInSector                | :grey_question:    |
| 79    | Thing_SetConversation           | :x:                |
| 80    | ACS_Execute                     | :telescope:        |
| 81    | ACS_Suspend                     | :telescope:        |
| 82    | ACS_Terminate                   | :telescope:        |
| 83    | ACS_LockedExecute               | :telescope:        |
| 84    | ACS_ExecuteWithResult           | :telescope:        |
| 85    | ACS_LockedExecuteDoor           | :telescope:        |
| 86    | Polyobj_MoveToSpot              | :telescope:        |
| 87    | Polyobj_Stop                    | :telescope:        |
| 88    | Polyobj_MoveTo                  | :telescope:        |
| 89    | Polyobj_OR_MoveTo               | :telescope:        |
| 90    | Polyobj_OR_RotateLeft           | :telescope:        |
| 91    | Polyobj_OR_RotateRight          | :telescope:        |
| 92    | Polyobj_OR_Move                 | :telescope:        |
| 93    | Polyobj_OR_MoveTimes8           | :telescope:        |
| 94    | Pillar_BuildAndCrush            | :grey_question:    |
| 95    | FloorAndCeiling_LowerByValue    | :grey_question:    |
| 96    | FloorAndCeiling_RaiseByValue    | :grey_question:    |
| 97    | Ceiling_LowerAndCrushDist       | :grey_question:    |
| 98    | Sector_SetTranslucent           | :grey_question:    |
| 99    | Floor_RaiseAndCrushDoom         | :grey_question:    |
| 100   | Scroll_Texture_Left             | :warning:          |
| 101   | Scroll_Texture_Right            | :warning:          |
| 102   | Scroll_Texture_Up               | :warning:          |
| 103   | Scroll_Texture_Down             | :warning:          |
| 104   | Ceiling_CrushAndRaiseSilentDist | :grey_question:    |
| 105   | Door_WaitRaise                  | :heavy_check_mark: |
| 106   | Door_WaitClose                  | :heavy_check_mark: |
| 107   | Line_SetPortalTarget            | :x:                |
| -     | -                               | -                  |
| 109   | Light_ForceLightning            | :grey_question:    |
| 110   | Light_RaiseByValue              | :grey_question:    |
| 111   | Light_LowerByValue              | :grey_question:    |
| 112   | Light_ChangeToValue             | :grey_question:    |
| 113   | Light_Fade                      | :grey_question:    |
| 114   | Light_Glow                      | :grey_question:    |
| 115   | Light_Flicker                   | :grey_question:    |
| 116   | Light_Strobe                    | :grey_question:    |
| 117   | Light_Stop                      | :grey_question:    |
| 118   | Plane_Copy                      | :grey_question:    |
| 119   | Thing_Damage                    | :grey_question:    |
| 120   | Radius_Quake                    | :grey_question:    |
| 121   | Line_SetIdentification          | :grey_question:    |
| -     | -                               | -                  |
| 125   | Thing_Move                      | :grey_question:    |
| -     | -                               | -                  |
| 127   | Thing_SetSpecial                | :grey_question:    |
| 128   | ThrustThingZ                    | :grey_question:    |
| 129   | UsePuzzleItem                   | :grey_question:    |
| 130   | Thing_Activate                  | :grey_question:    |
| 131   | Thing_Deactivate                | :grey_question:    |
| 132   | Thing_Remove                    | :grey_question:    |
| 133   | Thing_Destroy                   | :grey_question:    |
| 134   | Thing_Projectile                | :grey_question:    |
| 135   | Thing_Spawn                     | :grey_question:    |
| 136   | Thing_ProjectileGravity         | :grey_question:    |
| 137   | Thing_SpawnNoFog                | :grey_question:    |
| 138   | Floor_Waggle                    | :grey_question:    |
| 139   | Thing_SpawnFacing               | :grey_question:    |
| 140   | Sector_ChangeSound              | :grey_question:    |
| -     | -                               | -                  |
| 154   | Teleport_NoStop                 | :grey_question:    |
| -     | -                               | -                  |
| 157   | SetGlobalFogParameter           | :grey_question:    |
| 158   | FS_Execute                      | :x:                |
| 159   | Sector_SetPlaneReflection       | :grey_question:    |
| 160   | Sector_Set3DFloor               | :x:                |
| 161   | Sector_SetContents              | :grey_question:    |
| -     | -                               | -                  |
| 168   | Ceiling_CrushAndRaiseDist       | :grey_question:    |
| 169   | Generic_Crusher2                | :grey_question:    |
| 170   | Sector_SetCeilingScale2         | :grey_question:    |
| 171   | Sector_SetFloorScale2           | :grey_question:    |
| 172   | Plat_UpNearestWaitDownStay      | :grey_question:    |
| 173   | NoiseAlert                      | :grey_question:    |
| 174   | SendToCommunicator              | :x:                |
| 175   | Thing_ProjectileIntercept       | :grey_question:    |
| 176   | Thing_ChangeTID                 | :grey_question:    |
| 177   | Thing_Hate                      | :grey_question:    |
| 178   | Thing_ProjectileAimed           | :grey_question:    |
| 179   | ChangeSkill                     | :grey_question:    |
| 180   | Thing_SetTranslation            | :grey_question:    |
| 181   | Plane_Align                     | :grey_question:    |
| 182   | Line_Mirror                     | :x:                |
| 183   | Line_AlignCeiling               | :grey_question:    |
| 184   | Line_AlignFloor                 | :grey_question:    |
| 185   | Sector_SetRotation              | :grey_question:    |
| 186   | Sector_SetCeilingPanning        | :grey_question:    |
| 187   | Sector_SetFloorPanning          | :grey_question:    |
| 188   | Sector_SetCeilingScale          | :grey_question:    |
| 189   | Sector_SetFloorScale            | :grey_question:    |
| 190   | Static_Init                     | :warning:          |
| 191   | SetPlayerProperty               | :grey_question:    |
| 192   | Ceiling_LowerToHighestFloor     | :grey_question:    |
| 193   | Ceiling_LowerInstant            | :grey_question:    |
| 194   | Ceiling_RaiseInstant            | :grey_question:    |
| 195   | Ceiling_CrushRaiseAndStayA      | :grey_question:    |
| 196   | Ceiling_CrushAndRaiseA          | :grey_question:    |
| 197   | Ceiling_CrushAndRaiseSilentA    | :grey_question:    |
| 198   | Ceiling_RaiseByValueTimes8      | :grey_question:    |
| 199   | Ceiling_LowerByValueTimes8      | :grey_question:    |
| 200   | Generic_Floor                   | :grey_question:    |
| 201   | Generic_Ceiling                 | :grey_question:    |
| 202   | Generic_Door                    | :heavy_check_mark: |
| 203   | Generic_Lift                    | :grey_question:    |
| 204   | Generic_Stairs                  | :grey_question:    |
| 205   | Generic_Crusher                 | :grey_question:    |
| 206   | Plat_DownWaitUpStayLip          | :grey_question:    |
| 207   | Plat_PerpetualRaiseLip          | :grey_question:    |
| 208   | TranslucentLine                 | :grey_question:    |
| 209   | Transfer_Heights                | :heavy_check_mark: |
| 210   | Transfer_FloorLight             | :heavy_check_mark: |
| 211   | Transfer_CeilingLight           | :heavy_check_mark: |
| 212   | Sector_SetColor                 | :x:                |
| 213   | Sector_SetFade                  | :grey_question:    |
| 214   | Sector_SetDamage                | :grey_question:    |
| 215   | Teleport_Line                   | :grey_question:    |
| 216   | Sector_SetGravity               | :grey_question:    |
| 217   | Stairs_BuildUpDoom              | :grey_question:    |
| 218   | Sector_SetWind                  | :heavy_check_mark: |
| 219   | Sector_SetFriction              | :heavy_check_mark: |
| 220   | Sector_SetCurrent               | :heavy_check_mark: |
| 221   | Scroll_Texture_Both             | :heavy_check_mark: |
| 222   | Scroll_Texture_Model            | :heavy_check_mark: |
| 223   | Scroll_Floor                    | :heavy_check_mark: |
| 224   | Scroll_Ceiling                  | :heavy_check_mark: |
| 225   | Scroll_Texture_Offsets          | :warning:          |
| 226   | ACS_ExecuteAlways               | :telescope:        |
| 227   | PointPush_SetForce              | :heavy_check_mark: |
| 228   | Plat_RaiseAndStayTx0            | :grey_question:    |
| 229   | Thing_SetGoal                   | :x:                |
| 230   | Plat_UpByValueStayTx            | :grey_question:    |
| 231   | Plat_ToggleCeiling              | :grey_question:    |
| 232   | Light_StrobeDoom                | :grey_question:    |
| 233   | Light_MinNeighbor               | :grey_question:    |
| 234   | Light_MaxNeighbor               | :grey_question:    |
| 235   | Floor_TransferTrigger           | :grey_question:    |
| 236   | Floor_TransferNumeric           | :grey_question:    |
| 237   | ChangeCamera                    | :x:                |
| 238   | Floor_RaiseToLowestCeiling      | :grey_question:    |
| 239   | Floor_RaiseByValueTxTy          | :grey_question:    |
| 240   | Floor_RaiseByTexture            | :grey_question:    |
| 241   | Floor_LowerToLowestTxTy         | :grey_question:    |
| 242   | Floor_LowerToHighest            | :grey_question:    |
| 243   | Exit_Normal                     | :grey_question:    |
| 244   | Exit_Secret                     | :grey_question:    |
| 245   | Elevator_RaiseToNearest         | :grey_question:    |
| 246   | Elevator_MoveToFloor            | :grey_question:    |
| 247   | Elevator_LowerToNearest         | :grey_question:    |
| 248   | HealThing                       | :grey_question:    |
| 249   | Door_CloseWaitOpen              | :heavy_check_mark: |
| 250   | Floor_Donut                     | :grey_question:    |
| 251   | FloorAndCeiling_LowerRaise      | :grey_question:    |
| 252   | Ceiling_RaiseToNearest          | :grey_question:    |
| 253   | Ceiling_LowerToLowest           | :grey_question:    |
| 254   | Ceiling_LowerToFloor            | :grey_question:    |
| 255   | Ceiling_CrushRaiseAndStaySilA   | :grey_question:    |
| 256   | Floor_LowerToHighestEE          | :grey_question:    |
| 257   | Floor_RaiseToLowest             | :grey_question:    |
| 258   | Floor_LowerToLowestCeiling      | :grey_question:    |
| 259   | Floor_RaiseToCeiling            | :grey_question:    |
| 260   | Floor_ToCeilingInstant          | :grey_question:    |
| 261   | Floor_LowerByTexture            | :grey_question:    |
| 262   | Ceiling_RaiseToHighest          | :grey_question:    |
| 263   | Ceiling_ToHighestInstant        | :grey_question:    |
| 264   | Ceiling_LowerToNearest          | :grey_question:    |
| 265   | Ceiling_RaiseToLowest           | :grey_question:    |
| 266   | Ceiling_RaiseToHighestFloor     | :grey_question:    |
| 267   | Ceiling_ToFloorInstant          | :grey_question:    |
| 268   | Ceiling_RaiseByTexture          | :grey_question:    |
| 269   | Ceiling_LowerByTexture          | :grey_question:    |
| 270   | Stairs_BuildDownDoom            | :grey_question:    |
| 271   | Stairs_BuildUpDoomSync          | :grey_question:    |
| 272   | Stairs_BuildDownDoomSync        | :grey_question:    |

#### Notes

- Wall scrollers cannot distinguish between top, bottom, and mid textures.
- The static init supports damage, gravity, and skies only.

### Sector Specials

| Value   | Name                     | Status             |
| ------- | ------------------------ | ------------------ |
| *       | Generalized Effects      | :heavy_check_mark: |
| 1       | Light_Phased             | :heavy_check_mark: |
| 2-4     | LightSequence*           | :grey_question:    |
| -       | -                        | -                  |
| 26-27   | Stairs_Special*          | :grey_question:    |
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
| 87      | Sector_Outside           | :x:                |
| -       | -                        | -                  |
| 104     | sLight_Strobe_Hurt       | :heavy_check_mark: |
| 105     | sDamage_Hellslime        | :heavy_check_mark: |
| -       | -                        | -                  |
| 115     | Damage_InstantDeath      | :heavy_check_mark: |
| 116     | sDamage_SuperHellslime   | :heavy_check_mark: |
| -       | -                        | -                  |
| 118     | Scroll_StrifeCurrent     | :grey_question:    |
| -       | -                        | -                  |
| 195     | Sector_Hidden            | :heavy_check_mark: |
| 196     | Sector_Heal              | :heavy_check_mark: |
| 197     | Light_OutdoorLightning   | :x:                |
| 198-199 | Light_IndoorLightning*   | :x:                |
| 200     | Sky2                     | :telescope:        |
| 201-224 | Scroll*                  | :heavy_check_mark: |
| 225-244 | Carry*                   | :heavy_check_mark: |

#### Notes

- Terrain effects (e.g., from lava specials) don't exist.
