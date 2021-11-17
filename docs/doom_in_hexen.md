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
| 16    | Transfer_WallLight              | :telescope:        |
| 17    | Thing_Raise                     | :telescope:        |
| 18    | StartConversation               | :x:                |
| 19    | Thing_Stop                      | :telescope:        |
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
| 39    | Teleport_ZombieChanger          | :telescope:        |
| 40    | Ceiling_LowerByValue            | :heavy_check_mark: |
| 41    | Ceiling_RaiseByValue            | :heavy_check_mark: |
| 42    | Ceiling_CrushAndRaise           | :heavy_check_mark: |
| 43    | Ceiling_LowerAndCrush           | :heavy_check_mark: |
| 44    | Ceiling_CrushStop               | :heavy_check_mark: |
| 45    | Ceiling_CrushRaiseAndStay       | :heavy_check_mark: |
| 46    | Floor_CrushStop                 | :heavy_check_mark: |
| 47    | Ceiling_MoveToValue             | :heavy_check_mark: |
| 48    | Sector_Attach3dMidtex           | :x:                |
| 49    | GlassBreak                      | :telescope:        |
| 50    | ExtraFloor_LightOnly            | :x:                |
| 51    | Sector_SetLink                  | :x:                |
| 52    | Scroll_Wall                     | :warning:          |
| 53    | Line_SetTextureOffset           | :warning:          |
| 54    | Sector_ChangeFlags              | :telescope:        |
| 55    | Line_SetBlocking                | :warning:          |
| 56    | Line_SetTextureScale            | :x:                |
| 57    | Sector_SetPortal                | :x:                |
| 58    | Sector_CopyScroller             | :heavy_check_mark: |
| 59    | Polyobj_OR_MoveToSpot           | :telescope:        |
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
| 72    | ThrustThing                     | :telescope:        |
| 73    | DamageThing                     | :telescope:        |
| 74    | Teleport_NewMap                 | :telescope:        |
| 75    | Teleport_EndGame                | :telescope:        |
| 76    | TeleportOther                   | :telescope:        |
| 77    | TeleportGroup                   | :telescope:        |
| 78    | TeleportInSector                | :telescope:        |
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
| 94    | Pillar_BuildAndCrush            | :heavy_check_mark: |
| 95    | FloorAndCeiling_LowerByValue    | :heavy_check_mark: |
| 96    | FloorAndCeiling_RaiseByValue    | :heavy_check_mark: |
| 97    | Ceiling_LowerAndCrushDist       | :heavy_check_mark: |
| 98    | Sector_SetTranslucent           | :x:                |
| 99    | Floor_RaiseAndCrushDoom         | :heavy_check_mark: |
| 100   | Scroll_Texture_Left             | :warning:          |
| 101   | Scroll_Texture_Right            | :warning:          |
| 102   | Scroll_Texture_Up               | :warning:          |
| 103   | Scroll_Texture_Down             | :warning:          |
| 104   | Ceiling_CrushAndRaiseSilentDist | :heavy_check_mark: |
| 105   | Door_WaitRaise                  | :heavy_check_mark: |
| 106   | Door_WaitClose                  | :heavy_check_mark: |
| 107   | Line_SetPortalTarget            | :x:                |
| -     | -                               | -                  |
| 109   | Light_ForceLightning            | :telescope:        |
| 110   | Light_RaiseByValue              | :heavy_check_mark: |
| 111   | Light_LowerByValue              | :heavy_check_mark: |
| 112   | Light_ChangeToValue             | :heavy_check_mark: |
| 113   | Light_Fade                      | :heavy_check_mark: |
| 114   | Light_Glow                      | :heavy_check_mark: |
| 115   | Light_Flicker                   | :heavy_check_mark: |
| 116   | Light_Strobe                    | :heavy_check_mark: |
| 117   | Light_Stop                      | :heavy_check_mark: |
| 118   | Plane_Copy                      | :x:                |
| 119   | Thing_Damage                    | :telescope:        |
| 120   | Radius_Quake                    | :telescope:        |
| 121   | Line_SetIdentification          | :warning:          |
| -     | -                               | -                  |
| 125   | Thing_Move                      | :telescope:        |
| -     | -                               | -                  |
| 127   | Thing_SetSpecial                | :telescope:        |
| 128   | ThrustThingZ                    | :telescope:        |
| 129   | UsePuzzleItem                   | :x:                |
| 130   | Thing_Activate                  | :telescope:        |
| 131   | Thing_Deactivate                | :telescope:        |
| 132   | Thing_Remove                    | :telescope:        |
| 133   | Thing_Destroy                   | :telescope:        |
| 134   | Thing_Projectile                | :telescope:        |
| 135   | Thing_Spawn                     | :telescope:        |
| 136   | Thing_ProjectileGravity         | :telescope:        |
| 137   | Thing_SpawnNoFog                | :telescope:        |
| 138   | Floor_Waggle                    | :heavy_check_mark: |
| 139   | Thing_SpawnFacing               | :telescope:        |
| 140   | Sector_ChangeSound              | :telescope:        |
| -     | -                               | -                  |
| 154   | Teleport_NoStop                 | :heavy_check_mark: |
| -     | -                               | -                  |
| 157   | SetGlobalFogParameter           | :telescope:        |
| 158   | FS_Execute                      | :x:                |
| 159   | Sector_SetPlaneReflection       | :x:                |
| 160   | Sector_Set3DFloor               | :x:                |
| 161   | Sector_SetContents              | :x:                |
| -     | -                               | -                  |
| 168   | Ceiling_CrushAndRaiseDist       | :heavy_check_mark: |
| 169   | Generic_Crusher2                | :heavy_check_mark: |
| 170   | Sector_SetCeilingScale2         | :x:                |
| 171   | Sector_SetFloorScale2           | :x:                |
| 172   | Plat_UpNearestWaitDownStay      | :heavy_check_mark: |
| 173   | NoiseAlert                      | :warning:          |
| 174   | SendToCommunicator              | :x:                |
| 175   | Thing_ProjectileIntercept       | :telescope:        |
| 176   | Thing_ChangeTID                 | :telescope:        |
| 177   | Thing_Hate                      | :telescope:        |
| 178   | Thing_ProjectileAimed           | :telescope:        |
| 179   | ChangeSkill                     | :x:                |
| 180   | Thing_SetTranslation            | :telescope:        |
| 181   | Plane_Align                     | :x:                |
| 182   | Line_Mirror                     | :x:                |
| 183   | Line_AlignCeiling               | :x:                |
| 184   | Line_AlignFloor                 | :x:                |
| 185   | Sector_SetRotation              | :x:                |
| 186   | Sector_SetCeilingPanning        | :heavy_check_mark: |
| 187   | Sector_SetFloorPanning          | :heavy_check_mark: |
| 188   | Sector_SetCeilingScale          | :x:                |
| 189   | Sector_SetFloorScale            | :x:                |
| 190   | Static_Init                     | :warning:          |
| 191   | SetPlayerProperty               | :x:                |
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
| 212   | Sector_SetColor                 | :x:                |
| 213   | Sector_SetFade                  | :x:                |
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
| 225   | Scroll_Texture_Offsets          | :warning:          |
| 226   | ACS_ExecuteAlways               | :telescope:        |
| 227   | PointPush_SetForce              | :heavy_check_mark: |
| 228   | Plat_RaiseAndStayTx0            | :heavy_check_mark: |
| 229   | Thing_SetGoal                   | :x:                |
| 230   | Plat_UpByValueStayTx            | :heavy_check_mark: |
| 231   | Plat_ToggleCeiling              | :heavy_check_mark: |
| 232   | Light_StrobeDoom                | :heavy_check_mark: |
| 233   | Light_MinNeighbor               | :heavy_check_mark: |
| 234   | Light_MaxNeighbor               | :heavy_check_mark: |
| 235   | Floor_TransferTrigger           | :heavy_check_mark: |
| 236   | Floor_TransferNumeric           | :heavy_check_mark: |
| 237   | ChangeCamera                    | :x:                |
| 238   | Floor_RaiseToLowestCeiling      | :heavy_check_mark: |
| 239   | Floor_RaiseByValueTxTy          | :heavy_check_mark: |
| 240   | Floor_RaiseByTexture            | :heavy_check_mark: |
| 241   | Floor_LowerToLowestTxTy         | :heavy_check_mark: |
| 242   | Floor_LowerToHighest            | :heavy_check_mark: |
| 243   | Exit_Normal                     | :warning:          |
| 244   | Exit_Secret                     | :warning:          |
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

#### Notes

- Specials above 255 aren't accessible yet (hexen format only supports special < 256).
- Wall scrollers cannot distinguish between top, bottom, and mid textures.
- The static init supports damage, gravity, and skies only.
- The position argument for map exits is currently ignored.
- Line flags coming from extra arguments are ignored.
- Line translucency works like boom (fixed alpha).
- Line_SetBlocking only applies flags for creatures, players, monsters, sound, and everything.
- NoiseAlert only works with the default arguments.
- Teleport destination thing tags are currently ignored.

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
| 87      | Sector_Outside           | :x:                |
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
| 197     | Light_OutdoorLightning   | :x:                |
| 198-199 | Light_IndoorLightning*   | :x:                |
| 200     | Sky2                     | :telescope:        |
| 201-224 | Scroll*                  | :heavy_check_mark: |
| 225-244 | Carry*                   | :heavy_check_mark: |

#### Notes

- Terrain effects (e.g., from lava specials) don't exist.
