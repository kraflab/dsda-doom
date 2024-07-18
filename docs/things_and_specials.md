# Things and Specials

This page documents the new thing types and special actions available in advanced map formats in dsda-doom. DSDA-Doom supports both Doom-in-Hexen and UDMF maps. See the [UDMF documentation](./udmf.md) for complete details about the `dsda` namespace.

## Thing Types

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
| 14001-14064 | AmbientSound | Places an ambient sound effect source. |
| 14065 | Custom AmbientSound | Places an ambient sound effect source via special argument. |
| 14100-14164 | MusicChanger | Changes music via DoomEdNum (0 to 64). |
| 14165 | Custom MusicChanger | Changes music via special argument (0 to 64). |

### Notes

- Poly objects have no sound.
- Poly objects may have visual errors in software mode.
- The custom ambient sound object only supports the ID argument.

## Line Specials

You can refer to the zdoom wiki for information on these line specials. Note that the hexen format only supports up to special 255, and argument values from 0 to 255. UDMF does not have these limits. Special behavior matches zdoom except where noted below.

**1: Polyobj_StartLine**

**2: Polyobj_RotateLeft**

**3: Polyobj_RotateRight**

**4: Polyobj_Move**

**5: Polyobj_ExplicitLine**

**7: Polyobj_DoorSwing**

**8: Polyobj_DoorSlide**

**10: Door_Close**

**11: Door_Open**

**12: Door_Raise**

**13: Door_LockedRaise**

**17: Thing_Raise**

**19: Thing_Stop**

**20: Floor_LowerByValue**

**21: Floor_LowerToLowest**

**22: Floor_LowerToNearest**

**23: Floor_RaiseByValue**

**24: Floor_RaiseToHighest**

**25: Floor_RaiseToNearest**

**26: Stairs_BuildDown**

**27: Stairs_BuildUp**

**28: Floor_RaiseAndCrush**

**29: Pillar_Build**

**30: Pillar_Open**

**31: Stairs_BuildDownSync**

**32: Stairs_BuildUpSync**

**33: ForceField**

**34: ClearForceField**

**37: Floor_MoveToValue**

**38: Ceiling_Waggle**

**39: Teleport_ZombieChanger**

**40: Ceiling_LowerByValue**

**41: Ceiling_RaiseByValue**

**42: Ceiling_CrushAndRaise**

**43: Ceiling_LowerAndCrush**

**44: Ceiling_CrushStop**

**45: Ceiling_CrushRaiseAndStay**

**46: Floor_CrushStop**

**47: Ceiling_MoveToValue**

**52: Scroll_Wall**

**53: Line_SetTextureOffset**

**54: Sector_ChangeFlags**
- Silent, friction, and push are the only flags supported in hexen format.
- UDMF additionally supports endgodmode, endlevel, hazard, and noattack.

**55: Line_SetBlocking**
- Hexen format does not support sight, hitscan, sound, and landmonsters.

**56: Line_SetTextureScale**

**58: Sector_CopyScroller**

**59: Polyobj_OR_MoveToSpot**

**60: Plat_PerpetualRaise**

**61: Plat_Stop**

**62: Plat_DownWaitUpStay**

**63: Plat_DownByValue**

**64: Plat_UpWaitDownStay**

**65: Plat_UpByValue**

**66: Floor_LowerInstant**

**67: Floor_RaiseInstant**

**70: Teleport**

**71: Teleport_NoFog**

**72: ThrustThing**
- The thrust limit cannot be removed with the 3rd argument (it is ignored).

**73: DamageThing**

**74: Teleport_NewMap**

**75: Teleport_EndGame**

**76: TeleportOther**

**77: TeleportGroup**

**78: TeleportInSector**

**86: Polyobj_MoveToSpot**

**87: Polyobj_Stop**

**88: Polyobj_MoveTo**

**89: Polyobj_OR_MoveTo**

**90: Polyobj_OR_RotateLeft**

**91: Polyobj_OR_RotateRight**

**92: Polyobj_OR_Move**

**94: Pillar_BuildAndCrush**

**95: FloorAndCeiling_LowerByValue**

**96: FloorAndCeiling_RaiseByValue**

**97: Ceiling_LowerAndCrushDist**

**99: Floor_RaiseAndCrushDoom**

**104: Ceiling_CrushAndRaiseSilentDist**

**105: Door_WaitRaise**

**106: Door_WaitClose**

**110: Light_RaiseByValue**

**111: Light_LowerByValue**

**112: Light_ChangeToValue**

**113: Light_Fade**

**114: Light_Glow**

**115: Light_Flicker**

**116: Light_Strobe**

**117: Light_Stop**

**119: Thing_Damage**

**120: Radius_Quake**

**121: Line_SetIdentification**

**125: Thing_Move**

**127: Thing_SetSpecial**

**128: ThrustThingZ**

**130: Thing_Activate**

**131: Thing_Deactivate**

**132: Thing_Remove**

**133: Thing_Destroy**

**134: Thing_Projectile**

**135: Thing_Spawn**

**136: Thing_ProjectileGravity**

**137: Thing_SpawnNoFog**

**138: Floor_Waggle**

**139: Thing_SpawnFacing**

**154: Teleport_NoStop**

**168: Ceiling_CrushAndRaiseDist**

**169: Generic_Crusher2**

**170: Sector_SetCeilingScale2**

**171: Sector_SetFloorScale2**

**172: Plat_UpNearestWaitDownStay**

**173: NoiseAlert**
- Noise alerts only support the default behavior (the activator is both the target and the emitter).

**176: Thing_ChangeTID**

**177: Thing_Hate**
- Hate can only be applied to a specific actor (type 0).

**178: Thing_ProjectileAimed**

**185: Sector_SetRotation**

**186: Sector_SetCeilingPanning**

**187: Sector_SetFloorPanning**

**188: Sector_SetCeilingScale**

**189: Sector_SetFloorScale**

**192: Ceiling_LowerToHighestFloor**

**193: Ceiling_LowerInstant**

**194: Ceiling_RaiseInstant**

**195: Ceiling_CrushRaiseAndStayA**

**196: Ceiling_CrushAndRaiseA**

**197: Ceiling_CrushAndRaiseSilentA**

**200: Generic_Floor**

**201: Generic_Ceiling**

**202: Generic_Door**

**203: Generic_Lift**

**204: Generic_Stairs**

**205: Generic_Crusher**

**206: Plat_DownWaitUpStayLip**

**207: Plat_PerpetualRaiseLip**

**209: Transfer_Heights**

**214: Sector_SetDamage**

**215: Teleport_Line**

**216: Sector_SetGravity**

**217: Stairs_BuildUpDoom**

**218: Sector_SetWind**

**219: Sector_SetFriction**

**220: Sector_SetCurrent**

**222: Scroll_Texture_Model**

**223: Scroll_Floor**

**224: Scroll_Ceiling**

**227: PointPush_SetForce**

**228: Plat_RaiseAndStayTx0**

**230: Plat_UpByValueStayTx**

**231: Plat_ToggleCeiling**

**232: Light_StrobeDoom**

**233: Light_MinNeighbor**

**234: Light_MaxNeighbor**

**235: Floor_TransferTrigger**

**236: Floor_TransferNumeric**

**238: Floor_RaiseToLowestCeiling**

**239: Floor_RaiseByValueTxTy**

**240: Floor_RaiseByTexture**

**241: Floor_LowerToLowestTxTy**

**242: Floor_LowerToHighest**

**243: Exit_Normal**

**244: Exit_Secret**

**245: Elevator_RaiseToNearest**

**246: Elevator_MoveToFloor**

**247: Elevator_LowerToNearest**

**248: HealThing**

**249: Door_CloseWaitOpen**

**250: Floor_Donut**

**251: FloorAndCeiling_LowerRaise**

**252: Ceiling_RaiseToNearest**

**253: Ceiling_LowerToLowest**

**254: Ceiling_LowerToFloor**

**255: Ceiling_CrushRaiseAndStaySilA**

**256: Floor_LowerToHighestEE**

**257: Floor_RaiseToLowest**

**258: Floor_LowerToLowestCeiling**

**259: Floor_RaiseToCeiling**

**260: Floor_ToCeilingInstant**

**261: Floor_LowerByTexture**

**262: Ceiling_RaiseToHighest**

**263: Ceiling_ToHighestInstant**

**264: Ceiling_LowerToNearest**

**265: Ceiling_RaiseToLowest**

**266: Ceiling_RaiseToHighestFloor**

**267: Ceiling_ToFloorInstant**

**268: Ceiling_RaiseByTexture**

**269: Ceiling_LowerByTexture**

**270: Stairs_BuildDownDoom**

**271: Stairs_BuildUpDoomSync**

**272: Stairs_BuildDownDoomSync**

**273: Stairs_BuildUpDoomCrush**

**275: Floor_Stop**

**276: Ceiling_Stop**

**279: Floor_MoveToValueAndCrush**

**280: Ceiling_MoveToValueAndCrush**

**281: Line_SetAutomapFlags**

**282: Line_SetAutomapStyle**

---

### New Line Specials

**2701: Map_SetColormap(colormap)**
Sets the map colormap.
- _colormap_: the colormap (string argument).

**2702: Sector_SetColormap(colormap, tag)**
Sets the colormap for tagged sectors.
- _colormap_: the colormap (string argument).
- _tag_: The tag of the sector(s) to affect.

**2703: Music_ChangeSong(song, local, loop)**
Changes the current music.
- _song_: the music lump name (string argument).
- _local_: if true, only the activator's music changes.
- _loop_: if true, the music loops.

**2704: Music_Stop(local)**
Stops the current music.
- _local_: if true, only the activator's music stops.

---

### Obsolete Line Specials

The following lines are obsolete in udmf.

**6: Polyobj_MoveTimes8**
- Use 4: Polyobj_Move

**35: Floor_RaiseByValueTimes8**
- Use 23: Floor_RaiseByValue

**36: Floor_LowerByValueTimes8**
- Use 20: Floor_LowerByValue

**68: Floor_MoveToValueTimes8**
- Use 37: Floor_MoveToValue

**69: Ceiling_MoveToValueTimes8**
- Use 47: Ceiling_MoveToValue

**93: Polyobj_OR_MoveTimes8**
- Use 92: Polyobj_OR_Move

**100: Scroll_Texture_Left** \[Obsolete\]
- Use the sidedef scroll properties directly.

**101: Scroll_Texture_Right** \[Obsolete\]
- Use the sidedef scroll properties directly.

**102: Scroll_Texture_Up** \[Obsolete\]
- Use the sidedef scroll properties directly.

**103: Scroll_Texture_Down** \[Obsolete\]
- Use the sidedef scroll properties directly.

**190: Static_Init** \[Obsolete\]
- Use the sector gravity, floor sky, ceiling sky, and damage properties directly.

**198: Ceiling_RaiseByValueTimes8**
- Use 41: Ceiling_RaiseByValue

**199: Ceiling_LowerByValueTimes8**
- Use 40: Ceiling_LowerByValue

**208: TranslucentLine** \[Obsolete\]
- Use the linedef alpha property directly.

**210: Transfer_FloorLight** \[Obsolete\]
- Use the sector floor light property directly.

**211: Transfer_CeilingLight** \[Obsolete\]
- Use the sector ceiling light property directly.

**221: Scroll_Texture_Both** \[Obsolete\]
- Use the sidedef scroll properties directly.

**225: Scroll_Texture_Offsets** \[Obsolete\]
- Use the sidedef scroll properties directly.

## Sector Specials

Many sector specials are legacy effects from the original formats. More control can be obtained from sector properties in UDMF and the above line actions. DSDA-Doom has no concept of "terrain effects" (lava doesn't splash).

| Number | Name | Description |
| --- | --- | --- |
| * | Generalized Effects | Boom's generalized effects. |
| 1 | Light_Phased | Manual phased lighting (set index via lightlevel 0-63). |
| 2-4 | LightSequence* | Automatic phased lighting. |
| 26-27 | Stairs_Special* | Special stair building markers for Stairs_BuildUp* and Stairs_BuildDown*. |
| 40-51 | Wind* | Heretic's wind specials (only affects player). |
| 65 | dLight_Flicker | Random blinking light. |
| 66 | dLight_StrobeFast | Fast strobe. |
| 67 | dLight_StrobeSlow | Slow strobe. |
| 68 | dLight_Strobe_Hurt | Strobe + 20% damage. |
| 69 | dDamage_Hellslime | 10% damage. |
| 71 | dDamage_Nukage | 5% damage. |
| 72 | dLight_Glow | Glowing light. |
| 74 | dSector_DoorCloseIn30 | Door close in 30s. |
| 75 | dDamage_End | Unblockable 20% damage + end level. |
| 76 | dLight_StrobeSlowSync | Slow strobe sync. |
| 77 | dLight_StrobeFastSync | Fast strobe sync. |
| 78 | dSector_DoorRaiseIn5Mins | Door raise in 5m. |
| 79 | dFriction_Low | Low friction (ice). |
| 80 | dDamage_SuperHellslime | 20% damage. |
| 81 | dLight_FireFlicker | Random flickering light. |
| 82 | dDamage_LavaWimpy | Unblockable 5% damage. |
| 83 | dDamage_LavaHefty | Unblockable 8% damage. |
| 84 | dScroll_EastLavaDamage | East scroller + unblockable 5% damage. |
| 85 | hDamage_Sludge | 4% damage. |
| 104 | sLight_Strobe_Hurt | Strobe + 5% damage. |
| 105 | sDamage_Hellslime | 2% hazard (delayed damage). |
| 115 | Damage_InstantDeath | Instant kill. |
| 116 | sDamage_SuperHellslime | 4% hazard (delayed damage). |
| 118 | Scroll_StrifeCurrent | Scroll direction by tag (only affects player). |
| 195 | Sector_Hidden | Hides the sector texture on the textured automap. |
| 196 | Sector_Heal | Slowly heals the player up to 100% health. |
| 201-224 | Scroll* | Carry items on the floor and scroll textures. |
| 225-244 | Carry* | Carry items on the floor (only the east variant scrolls textures). |
