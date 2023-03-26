//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

#ifndef __HEXEN_A_ACTION__
#define __HEXEN_A_ACTION__

#include "p_mobj.h"
#include "p_pspr.h"

extern int localQuakeHappening[MAX_MAXPLAYERS];

dboolean A_LocalQuake(byte * args, mobj_t * victim);
void P_SpawnDirt(mobj_t * actor, fixed_t radius);
void A_BridgeRemove(mobj_t * actor);


void A_FreeTargMobj(mobj_t* mobj);
void A_FlameCheck(mobj_t* mobj);
void A_HideThing(mobj_t* mobj);
void A_UnHideThing(mobj_t* mobj);
void A_RestoreSpecialThing1(mobj_t* mobj);
void A_RestoreSpecialThing2(mobj_t* mobj);
void A_RestoreArtifact(mobj_t* mobj);
void A_Summon(mobj_t* mobj);
void A_ThrustInitUp(mobj_t* mobj);
void A_ThrustInitDn(mobj_t* mobj);
void A_ThrustRaise(mobj_t* mobj);
void A_ThrustBlock(mobj_t* mobj);
void A_ThrustImpale(mobj_t* mobj);
void A_ThrustLower(mobj_t* mobj);
void A_TeloSpawnC(mobj_t* mobj);
void A_TeloSpawnB(mobj_t* mobj);
void A_TeloSpawnA(mobj_t* mobj);
void A_TeloSpawnD(mobj_t* mobj);
void A_CheckTeleRing(mobj_t* mobj);
void A_FogSpawn(mobj_t* mobj);
void A_FogMove(mobj_t* mobj);
void A_Quake(mobj_t* mobj);
void A_ContMobjSound(mobj_t* mobj);
void A_Scream(mobj_t* mobj);
void A_Explode(mobj_t* mobj);
void A_PoisonBagInit(mobj_t* mobj);
void A_PoisonBagDamage(mobj_t* mobj);
void A_PoisonBagCheck(mobj_t* mobj);
void A_CheckThrowBomb(mobj_t* mobj);
void A_NoGravity(mobj_t* mobj);
void A_PotteryExplode(mobj_t* mobj);
void A_PotteryChooseBit(mobj_t* mobj);
void A_PotteryCheck(mobj_t* mobj);
void A_CorpseBloodDrip(mobj_t* mobj);
void A_CorpseExplode(mobj_t* mobj);
void A_LeafSpawn(mobj_t* mobj);
void A_LeafThrust(mobj_t* mobj);
void A_LeafCheck(mobj_t* mobj);
void A_BridgeInit(mobj_t* mobj);
void A_BridgeOrbit(mobj_t* mobj);
void A_TreeDeath(mobj_t* mobj);
void A_PoisonShroom(mobj_t* mobj);
void A_Pain(mobj_t* mobj);
void A_SoAExplode(mobj_t* mobj);
void A_BellReset1(mobj_t* mobj);
void A_BellReset2(mobj_t* mobj);
void A_NoBlocking(mobj_t* mobj);
void A_FPunchAttack(struct player_s* player, pspdef_t * psp);
void A_FAxeAttack(struct player_s* player, pspdef_t * psp);
void A_FHammerAttack(struct player_s* player, pspdef_t * psp);
void A_FHammerThrow(struct player_s* player, pspdef_t * psp);
void A_FSwordAttack(struct player_s* player, pspdef_t * psp);
void A_FSwordFlames(mobj_t* mobj);
void A_CMaceAttack(struct player_s* player, pspdef_t * psp);
void A_CStaffInitBlink(struct player_s* player, pspdef_t * psp);
void A_CStaffCheckBlink(struct player_s* player, pspdef_t * psp);
void A_CStaffCheck(struct player_s* player, pspdef_t * psp);
void A_CStaffAttack(struct player_s* player, pspdef_t * psp);
void A_CStaffMissileSlither(mobj_t* mobj);
void A_CFlameAttack(struct player_s* player, pspdef_t * psp);
void A_CFlameRotate(mobj_t* mobj);
void A_CFlamePuff(mobj_t* mobj);
void A_CFlameMissile(mobj_t* mobj);
void A_CHolyAttack(struct player_s* player, pspdef_t * psp);
void A_CHolyPalette(struct player_s* player, pspdef_t * psp);
void A_CHolySeek(mobj_t* mobj);
void A_CHolyCheckScream(mobj_t* mobj);
void A_CHolyTail(mobj_t* mobj);
void A_CHolySpawnPuff(mobj_t* mobj);
void A_CHolyAttack2(mobj_t* mobj);
void A_MWandAttack(struct player_s* player, pspdef_t * psp);
void A_LightningReady(struct player_s* player, pspdef_t * psp);
void A_MLightningAttack(struct player_s* player, pspdef_t * psp);
void A_LightningZap(mobj_t* mobj);
void A_LightningClip(mobj_t* mobj);
void A_LightningRemove(mobj_t* mobj);
void A_LastZap(mobj_t* mobj);
void A_ZapMimic(mobj_t* mobj);
void A_MStaffAttack(struct player_s* player, pspdef_t * psp);
void A_MStaffPalette(struct player_s* player, pspdef_t * psp);
void A_MStaffWeave(mobj_t* mobj);
void A_MStaffTrack(mobj_t* mobj);
void A_SnoutAttack(struct player_s* player, pspdef_t * psp);
void A_FireConePL1(struct player_s* player, pspdef_t * psp);
void A_ShedShard(mobj_t* mobj);
void A_AddPlayerCorpse(mobj_t* mobj);
void A_SkullPop(mobj_t* mobj);
void A_FreezeDeath(mobj_t* mobj);
void A_FreezeDeathChunks(mobj_t* mobj);
void A_CheckBurnGone(mobj_t* mobj);
void A_CheckSkullFloor(mobj_t* mobj);
void A_CheckSkullDone(mobj_t* mobj);
void A_SpeedFade(mobj_t* mobj);
void A_IceSetTics(mobj_t* mobj);
void A_IceCheckHeadDone(mobj_t* mobj);
void A_PigPain(mobj_t* mobj);
void A_PigLook(mobj_t* mobj);
void A_PigChase(mobj_t* mobj);
void A_FaceTarget(mobj_t* mobj);
void A_PigAttack(mobj_t* mobj);
void A_QueueCorpse(mobj_t* mobj);
void A_Look(mobj_t* mobj);
void A_Chase(mobj_t* mobj);
void A_CentaurAttack(mobj_t* mobj);
void A_CentaurAttack2(mobj_t* mobj);
void A_SetReflective(mobj_t* mobj);
void A_CentaurDefend(mobj_t* mobj);
void A_UnSetReflective(mobj_t* mobj);
void A_CentaurDropStuff(mobj_t* mobj);
void A_CheckFloor(mobj_t* mobj);
void A_DemonAttack1(mobj_t* mobj);
void A_DemonAttack2(mobj_t* mobj);
void A_DemonDeath(mobj_t* mobj);
void A_Demon2Death(mobj_t* mobj);
void A_WraithRaiseInit(mobj_t* mobj);
void A_WraithRaise(mobj_t* mobj);
void A_WraithInit(mobj_t* mobj);
void A_WraithLook(mobj_t* mobj);
void A_WraithChase(mobj_t* mobj);
void A_WraithFX3(mobj_t* mobj);
void A_WraithMelee(mobj_t* mobj);
void A_WraithMissile(mobj_t* mobj);
void A_WraithFX2(mobj_t* mobj);
void A_MinotaurFade1(mobj_t* mobj);
void A_MinotaurFade2(mobj_t* mobj);
void A_MinotaurLook(mobj_t* mobj);
void A_MinotaurChase(mobj_t* mobj);
void A_MinotaurRoam(mobj_t* mobj);
void A_MinotaurAtk1(mobj_t* mobj);
void A_MinotaurDecide(mobj_t* mobj);
void A_MinotaurAtk2(mobj_t* mobj);
void A_MinotaurAtk3(mobj_t* mobj);
void A_MinotaurCharge(mobj_t* mobj);
void A_SmokePuffExit(mobj_t* mobj);
void A_MinotaurFade0(mobj_t* mobj);
void A_MntrFloorFire(mobj_t* mobj);
void A_SerpentChase(mobj_t* mobj);
void A_SerpentHumpDecide(mobj_t* mobj);
void A_SerpentUnHide(mobj_t* mobj);
void A_SerpentRaiseHump(mobj_t* mobj);
void A_SerpentLowerHump(mobj_t* mobj);
void A_SerpentHide(mobj_t* mobj);
void A_SerpentBirthScream(mobj_t* mobj);
void A_SetShootable(mobj_t* mobj);
void A_SerpentCheckForAttack(mobj_t* mobj);
void A_UnSetShootable(mobj_t* mobj);
void A_SerpentDiveSound(mobj_t* mobj);
void A_SerpentWalk(mobj_t* mobj);
void A_SerpentChooseAttack(mobj_t* mobj);
void A_SerpentMeleeAttack(mobj_t* mobj);
void A_SerpentMissileAttack(mobj_t* mobj);
void A_SerpentHeadPop(mobj_t* mobj);
void A_SerpentSpawnGibs(mobj_t* mobj);
void A_SerpentHeadCheck(mobj_t* mobj);
void A_FloatGib(mobj_t* mobj);
void A_DelayGib(mobj_t* mobj);
void A_SinkGib(mobj_t* mobj);
void A_BishopDecide(mobj_t* mobj);
void A_BishopDoBlur(mobj_t* mobj);
void A_BishopSpawnBlur(mobj_t* mobj);
void A_BishopChase(mobj_t* mobj);
void A_BishopAttack(mobj_t* mobj);
void A_BishopAttack2(mobj_t* mobj);
void A_BishopPainBlur(mobj_t* mobj);
void A_BishopPuff(mobj_t* mobj);
void A_SetAltShadow(mobj_t* mobj);
void A_BishopMissileWeave(mobj_t* mobj);
void A_BishopMissileSeek(mobj_t* mobj);
void A_DragonInitFlight(mobj_t* mobj);
void A_DragonFlap(mobj_t* mobj);
void A_DragonFlight(mobj_t* mobj);
void A_DragonAttack(mobj_t* mobj);
void A_DragonPain(mobj_t* mobj);
void A_DragonCheckCrash(mobj_t* mobj);
void A_DragonFX2(mobj_t* mobj);
void A_ESound(mobj_t* mobj);
void A_EttinAttack(mobj_t* mobj);
void A_DropMace(mobj_t* mobj);
void A_FiredRocks(mobj_t* mobj);
void A_SetInvulnerable(mobj_t* mobj);
void A_UnSetInvulnerable(mobj_t* mobj);
void A_FiredChase(mobj_t* mobj);
void A_FiredAttack(mobj_t* mobj);
void A_FiredSplotch(mobj_t* mobj);
void A_SmBounce(mobj_t* mobj);
void A_IceGuyLook(mobj_t* mobj);
void A_IceGuyChase(mobj_t* mobj);
void A_IceGuyAttack(mobj_t* mobj);
void A_IceGuyDie(mobj_t* mobj);
void A_IceGuyMissilePuff(mobj_t* mobj);
void A_IceGuyMissileExplode(mobj_t* mobj);
void A_ClassBossHealth(mobj_t* mobj);
void A_FastChase(mobj_t* mobj);
void A_FighterAttack(mobj_t* mobj);
void A_ClericAttack(mobj_t* mobj);
void A_MageAttack(mobj_t* mobj);
void A_SorcSpinBalls(mobj_t* mobj);
void A_SpeedBalls(mobj_t* mobj);
void A_SpawnFizzle(mobj_t* mobj);
void A_SorcBossAttack(mobj_t* mobj);
void A_SorcBallOrbit(mobj_t* mobj);
void A_SorcBallPop(mobj_t* mobj);
void A_BounceCheck(mobj_t* mobj);
void A_SorcFX1Seek(mobj_t* mobj);
void A_SorcFX2Split(mobj_t* mobj);
void A_SorcFX2Orbit(mobj_t* mobj);
void A_SorcererBishopEntry(mobj_t* mobj);
void A_SpawnBishop(mobj_t* mobj);
void A_SorcFX4Check(mobj_t* mobj);
void A_KoraxStep2(mobj_t* mobj);
void A_KoraxChase(mobj_t* mobj);
void A_KoraxStep(mobj_t* mobj);
void A_KoraxDecide(mobj_t* mobj);
void A_KoraxMissile(mobj_t* mobj);
void A_KoraxCommand(mobj_t* mobj);
void A_KoraxBonePop(mobj_t* mobj);
void A_KSpiritRoam(mobj_t* mobj);
void A_KBoltRaise(mobj_t* mobj);
void A_KBolt(mobj_t* mobj);
void A_BatSpawnInit(mobj_t* mobj);
void A_BatSpawn(mobj_t* mobj);
void A_BatMove(mobj_t* mobj);

#endif
