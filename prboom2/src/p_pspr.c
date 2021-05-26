/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      Weapon sprite animation, weapon objects.
 *      Action functions for weapons.
 *
 *-----------------------------------------------------------------------------*/

#include "doomstat.h"
#include "r_main.h"
#include "p_map.h"
#include "p_inter.h"
#include "p_pspr.h"
#include "p_enemy.h"
#include "p_tick.h"
#include "m_random.h"
#include "s_sound.h"
#include "sounds.h"
#include "d_event.h"
#include "r_demo.h"
#include "g_game.h"
#include "lprintf.h"
#include "e6y.h"//e6y
#include "dsda.h"

#define LOWERSPEED   (FRACUNIT*6)
#define RAISESPEED   (FRACUNIT*6)
#define WEAPONBOTTOM (FRACUNIT*128)
#define WEAPONTOP    (FRACUNIT*32)

#define BFGCELLS bfgcells        /* Ty 03/09/98 externalized in p_inter.c */

// Checking correctness of input parameters for weapon codepointers
// for avoiding crashes when they are used with player/monster states.
#ifdef PRBOOM_DEBUG
  #define CHECK_WEAPON_CODEPOINTER(codepointer, player)\
    if (!player->mo->player) {\
      I_Error("%s: Weapon codepointers cannot be used with player/monster states (incorrect DEH).", codepointer);\
      return;\
    }
#else
  #define CHECK_WEAPON_CODEPOINTER(codepointer, player)
#endif

extern void P_Thrust(player_t *, angle_t, fixed_t);

// The following array holds the recoil values         // phares

static const int recoil_values[] = {    // phares
  10, // wp_fist
  10, // wp_pistol
  30, // wp_shotgun
  10, // wp_chaingun
  100,// wp_missile
  20, // wp_plasma
  100,// wp_bfg
  0,  // wp_chainsaw
  80  // wp_supershotgun
};

//
// P_SetPsprite
//

void P_SetPsprite(player_t *player, int position, statenum_t stnum)
{
	P_SetPspritePtr(player, &player->psprites[position], stnum);
}

//
// P_SetPspritePtr
//

void P_SetPspritePtr(player_t *player, pspdef_t *psp, statenum_t stnum)
{
  do
    {
      state_t *state;

      if (!stnum)
        {
          // object removed itself
          psp->state = NULL;
          break;
        }

      state = &states[stnum];
      psp->state = state;
      psp->tics = state->tics;        // could be 0

      if (state->misc1)
        {
          // coordinate set
          psp->sx = state->misc1 << FRACBITS;
          psp->sy = state->misc2 << FRACBITS;
        }

      // Call action routine.
      // Modified handling.
      if (state->action)
        {
          state->action(player, psp);
          if (!psp->state)
            break;
        }
      stnum = psp->state->nextstate;
    }
  while (!psp->tics);     // an initial state of 0 could cycle through
}

//
// P_BringUpWeapon
// Starts bringing the pending weapon up
// from the bottom of the screen.
// Uses player
//

static void P_BringUpWeapon(player_t *player)
{
  statenum_t newstate;

  if (player->pendingweapon == wp_nochange)
    player->pendingweapon = player->readyweapon;

  if (player->pendingweapon == g_wp_chainsaw)
    S_StartSound(player->mo, g_sfx_sawup);

  if (player->pendingweapon >= NUMWEAPONS)
    lprintf(LO_WARN, "P_BringUpWeapon: weaponinfo overrun has occurred.\n");

  if (player->powers[pw_weaponlevel2])
  {
    newstate = wpnlev2info[player->pendingweapon].upstate;
  }
  else
  {
    newstate = weaponinfo[player->pendingweapon].upstate;
  }

  player->pendingweapon = wp_nochange;
  // killough 12/98: prevent pistol from starting visibly at bottom of screen:
  player->psprites[ps_weapon].sy =
    mbf_features ? WEAPONBOTTOM+FRACUNIT*2 : WEAPONBOTTOM;

  P_SetPsprite(player, ps_weapon, newstate);
}

// The first set is where the weapon preferences from             // killough,
// default.cfg are stored. These values represent the keys used   // phares
// in DOOM2 to bring up the weapon, i.e. 6 = plasma gun. These    //    |
// are NOT the wp_* constants.                                    //    V

int weapon_preferences[2][NUMWEAPONS+1] = {
  {6, 9, 4, 3, 2, 8, 5, 7, 1, 0},  // !compatibility preferences
  {6, 9, 4, 3, 2, 8, 5, 7, 1, 0},  //  compatibility preferences
};

// Center Weapon when Firing.
int weapon_attack_alignment=0;

// [XA] fixed version of P_SwitchWeapon that correctly
// takes each weapon's ammotype and ammopershot into account,
// instead of blindly assuming both.

static int P_SwitchWeaponMBF21(player_t *player)
{
  int *prefer;
  int currentweapon, newweapon;
  int i;
  weapontype_t checkweapon;
  ammotype_t ammotype;

  prefer = weapon_preferences[0];
  currentweapon = player->readyweapon;
  newweapon = currentweapon;
  i = NUMWEAPONS + 1;

  do
  {
    checkweapon = wp_nochange;
    switch (*prefer++)
    {
      case 1:
        if (!player->powers[pw_strength]) // allow chainsaw override
          break;
        // fallthrough
      case 0:
        checkweapon = wp_fist;
        break;
      case 2:
        checkweapon = wp_pistol;
        break;
      case 3:
        checkweapon = wp_shotgun;
        break;
      case 4:
        checkweapon = wp_chaingun;
        break;
      case 5:
        checkweapon = wp_missile;
        break;
      case 6:
        if (gamemode != shareware)
          checkweapon = wp_plasma;
        break;
      case 7:
        if (gamemode != shareware)
          checkweapon = wp_bfg;
        break;
      case 8:
        checkweapon = wp_chainsaw;
        break;
      case 9:
        if (gamemode == commercial)
          checkweapon = wp_supershotgun;
        break;
    }

    if (checkweapon != wp_nochange && player->weaponowned[checkweapon])
    {
      ammotype = weaponinfo[checkweapon].ammo;
      if (ammotype == am_noammo ||
        player->ammo[ammotype] >= weaponinfo[checkweapon].ammopershot)
        newweapon = checkweapon;
    }
  }
  while (newweapon == currentweapon && --i);
  return newweapon;
}

// P_SwitchWeapon checks current ammo levels and gives you the
// most preferred weapon with ammo. It will not pick the currently
// raised weapon. When called from P_CheckAmmo this won't matter,
// because the raised weapon has no ammo anyway. When called from
// G_BuildTiccmd you want to toggle to a different weapon regardless.

int P_SwitchWeapon(player_t *player)
{
  int *prefer;
  int currentweapon, newweapon;
  int i;

  // [XA] use fixed behavior for mbf21. no need
  // for a discrete compat option for this, as
  // it doesn't impact demo playback (weapon
  // switches are saved in the demo itself)
  if (mbf21)
    return P_SwitchWeaponMBF21(player);

  prefer = weapon_preferences[demo_compatibility != 0]; // killough 3/22/98
  currentweapon = player->readyweapon;
  newweapon = currentweapon;
  i = NUMWEAPONS + 1;   // killough 5/2/98

  // killough 2/8/98: follow preferences and fix BFG/SSG bugs

  do
    switch (*prefer++)
      {
      case 1:
        if (!player->powers[pw_strength])      // allow chainsaw override
          break;
        // fallthrough
      case 0:
        newweapon = wp_fist;
        break;
      case 2:
        if (player->ammo[am_clip])
          newweapon = wp_pistol;
        break;
      case 3:
        if (player->weaponowned[wp_shotgun] && player->ammo[am_shell])
          newweapon = wp_shotgun;
        break;
      case 4:
        if (player->weaponowned[wp_chaingun] && player->ammo[am_clip])
          newweapon = wp_chaingun;
        break;
      case 5:
        if (player->weaponowned[wp_missile] && player->ammo[am_misl])
          newweapon = wp_missile;
        break;
      case 6:
        if (player->weaponowned[wp_plasma] && player->ammo[am_cell] &&
            gamemode != shareware)
          newweapon = wp_plasma;
        break;
      case 7:
        if (player->weaponowned[wp_bfg] && gamemode != shareware &&
            player->ammo[am_cell] >= (demo_compatibility ? 41 : 40))
          newweapon = wp_bfg;
        break;
      case 8:
        if (player->weaponowned[wp_chainsaw])
          newweapon = wp_chainsaw;
        break;
      case 9:
        if (player->weaponowned[wp_supershotgun] && gamemode == commercial &&
            player->ammo[am_shell] >= (demo_compatibility ? 3 : 2))
          newweapon = wp_supershotgun;
        break;
      }
  while (newweapon == currentweapon && --i);          // killough 5/2/98
  return newweapon;
}

// killough 5/2/98: whether consoleplayer prefers weapon w1 over weapon w2.
int P_WeaponPreferred(int w1, int w2)
{
  return
    (weapon_preferences[0][0] != ++w2 && (weapon_preferences[0][0] == ++w1 ||
    (weapon_preferences[0][1] !=   w2 && (weapon_preferences[0][1] ==   w1 ||
    (weapon_preferences[0][2] !=   w2 && (weapon_preferences[0][2] ==   w1 ||
    (weapon_preferences[0][3] !=   w2 && (weapon_preferences[0][3] ==   w1 ||
    (weapon_preferences[0][4] !=   w2 && (weapon_preferences[0][4] ==   w1 ||
    (weapon_preferences[0][5] !=   w2 && (weapon_preferences[0][5] ==   w1 ||
    (weapon_preferences[0][6] !=   w2 && (weapon_preferences[0][6] ==   w1 ||
    (weapon_preferences[0][7] !=   w2 && (weapon_preferences[0][7] ==   w1
   ))))))))))))))));
}

//
// P_CheckAmmo
// Returns true if there is enough ammo to shoot.
// If not, selects the next weapon to use.
// (only in demo_compatibility mode -- killough 3/22/98)
//

dboolean P_CheckAmmo(player_t *player)
{
  ammotype_t ammo;
  int count;  // Regular

  if (heretic) return Heretic_P_CheckAmmo(player);

  ammo = weaponinfo[player->readyweapon].ammo;
  if (mbf21)
    count = weaponinfo[player->readyweapon].ammopershot;
  else
    if (player->readyweapon == wp_bfg)  // Minimal amount for one shot varies.
      count = BFGCELLS;
    else
      if (player->readyweapon == wp_supershotgun)        // Double barrel.
        count = 2;
      else
        count = 1;

  // Some do not need ammunition anyway.
  // Return if current ammunition sufficient.

  if (ammo == am_noammo || player->ammo[ammo] >= count)
    return true;

  // Out of ammo, pick a weapon to change to.
  //
  // killough 3/22/98: for old demos we do the switch here and now;
  // for Boom games we cannot do this, and have different player
  // preferences across demos or networks, so we have to use the
  // G_BuildTiccmd() interface instead of making the switch here.

  if (demo_compatibility)
    {
      player->pendingweapon = P_SwitchWeapon(player);      // phares
      // Now set appropriate weapon overlay.
      P_SetPsprite(player,ps_weapon,weaponinfo[player->readyweapon].downstate);
    }

  return false;
}

//
// P_SubtractAmmo
// Subtracts ammo, w/compat handling. In mbf21, use
// readyweapon's "ammopershot" field if it's explicitly
// defined in dehacked; otherwise use the amount specified
// by the codepointer instead (Doom behavior)
//
// [XA] NOTE: this function is for handling Doom's native
// attack pointers; of note, the new A_ConsumeAmmo mbf21
// codepointer does NOT call this function, since it doesn't
// have to worry about any compatibility shenanigans.
//

void P_SubtractAmmo(struct player_s *player, int vanilla_amount)
{
  int amount;
  ammotype_t ammotype = weaponinfo[player->readyweapon].ammo;

  if (mbf21 && ammotype == am_noammo)
    return; // [XA] hmm... I guess vanilla/boom will go out of bounds then?

  if (mbf21 && (weaponinfo[player->readyweapon].intflags & WIF_ENABLEAPS))
    amount = weaponinfo[player->readyweapon].ammopershot;
  else
    amount = vanilla_amount;

  player->ammo[ammotype] -= amount;

  if (mbf21 && player->ammo[ammotype] < 0)
    player->ammo[ammotype] = 0;
}

//
// P_FireWeapon.
//

static void P_FireWeapon(player_t *player)
{
  statenum_t newstate;

  if (!P_CheckAmmo(player))
    return;

  dsda_WatchWeaponFire(player->readyweapon);

  // heretic_note: these can probably be combined in a clean way
  if (heretic)
  {
    weaponinfo_t *wpinfo;

    P_SetMobjState(player->mo, HERETIC_S_PLAY_ATK2);
    wpinfo = player->powers[pw_weaponlevel2] ? &wpnlev2info[0]
                                             : &weaponinfo[0];
    newstate = player->refire ? wpinfo[player->readyweapon].holdatkstate
                              : wpinfo[player->readyweapon].atkstate;
    P_SetPsprite(player, ps_weapon, newstate);
    P_NoiseAlert(player->mo, player->mo);
    if (player->readyweapon == wp_gauntlets && !player->refire)
    {                           // Play the sound for the initial gauntlet attack
        S_StartSound(player->mo, heretic_sfx_gntuse);
    }
  }
  else
  {
    P_SetMobjState(player->mo, S_PLAY_ATK1);
    newstate = weaponinfo[player->readyweapon].atkstate;
    P_SetPsprite(player, ps_weapon, newstate);
    if (!(weaponinfo[player->readyweapon].flags & WPF_SILENT))
      P_NoiseAlert(player->mo, player->mo);
  }
}

//
// P_DropWeapon
// Player died, so put the weapon away.
//

void P_DropWeapon(player_t *player)
{
  statenum_t newstate;
  if (player->powers[pw_weaponlevel2])
  {
    newstate = wpnlev2info[player->readyweapon].downstate;
  }
  else
  {
    newstate = weaponinfo[player->readyweapon].downstate;
  }
  P_SetPsprite(player, ps_weapon, newstate);
}

//
// A_WeaponReady
// The player can fire the weapon
// or change to another weapon at this time.
// Follows after getting weapon up,
// or after previous attack/fire sequence.
//

void A_WeaponReady(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_WeaponReady", player);

// weapon change sequence considered complete
  done_autoswitch = false;

  if (player->chickenTics)
  {                           // Change to the chicken beak
      P_ActivateBeak(player);
      return;
  }

  // get out of attack state
  if (player->mo->state == &states[g_s_play_atk1]
      || player->mo->state == &states[g_s_play_atk2] )
    P_SetMobjState(player->mo, g_s_play);

  if (heretic)
  {
    if (player->readyweapon == wp_staff
        && psp->state == &states[HERETIC_S_STAFFREADY2_1]
        && P_Random(pr_heretic) < 128)
    {
        S_StartSound(player->mo, heretic_sfx_stfcrk);
    }
  }
  else
    if (player->readyweapon == wp_chainsaw && psp->state == &states[S_SAW])
      S_StartSound(player->mo, sfx_sawidl);

  // check for change
  //  if player is dead, put the weapon away

  if (player->pendingweapon != wp_nochange || !player->health)
  {
    // change weapon (pending weapon should already be validated)
    statenum_t newstate;
    if (player->powers[pw_weaponlevel2])
      newstate = wpnlev2info[player->readyweapon].downstate;
    else
      newstate = weaponinfo[player->readyweapon].downstate;
    P_SetPsprite(player, ps_weapon, newstate);
    return;
  }

  // check for fire
  //  the missile launcher and bfg do not auto fire

  if (player->cmd.buttons & BT_ATTACK)
  {
    if (
      !player->attackdown ||
      !(weaponinfo[player->readyweapon].flags & WPF_NOAUTOFIRE)
    )
    {
      player->attackdown = true;
      P_FireWeapon(player);
      return;
    }
  }
  else
    player->attackdown = false;

  // bob the weapon based on movement speed
  {
    int angle = (128*leveltime) & FINEMASK;
    psp->sx = FRACUNIT + FixedMul(player->bob, finecosine[angle]);
    angle &= FINEANGLES/2-1;
    psp->sy = WEAPONTOP + FixedMul(player->bob, finesine[angle]);
  }
}

//
// A_ReFire
// The player can re-fire the weapon
// without lowering it entirely.
//

void A_ReFire(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_ReFire", player);

  // check for fire
  //  (if a weaponchange is pending, let it go through instead)

  if ( (player->cmd.buttons & BT_ATTACK)
       && player->pendingweapon == wp_nochange && player->health)
    {
      player->refire++;
      P_FireWeapon(player);
    }
  else
    {
      player->refire = 0;
      P_CheckAmmo(player);
    }
}

void A_CheckReload(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_CheckReload", player);

  if (!P_CheckAmmo(player) && compatibility_level >= prboom_4_compatibility) {
    /* cph 2002/08/08 - In old Doom, P_CheckAmmo would start the weapon lowering
     * immediately. This was lost in Boom when the weapon switching logic was
     * rewritten. But we must tell Doom that we don't need to complete the
     * reload frames for the weapon here. G_BuildTiccmd will set ->pendingweapon
     * for us later on. */
    P_SetPsprite(player,ps_weapon,weaponinfo[player->readyweapon].downstate);
  }
}

//
// A_Lower
// Lowers current weapon,
//  and changes weapon at bottom.
//

void A_Lower(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_Lower", player);

  if (player->chickenTics)
  {
      psp->sy = WEAPONBOTTOM;
  }
  else
  {
      psp->sy += LOWERSPEED;
  }

  // Is already down.
  if (psp->sy < WEAPONBOTTOM)
    return;

  // Player is dead.
  if (player->playerstate == PST_DEAD)
  {
    psp->sy = WEAPONBOTTOM;
    return;      // don't bring weapon back up
  }

  // The old weapon has been lowered off the screen,
  // so change the weapon and start raising it

  if (!player->health)
  {      // Player is dead, so keep the weapon off screen.
    P_SetPsprite(player,  ps_weapon, g_s_null);
    return;
  }

  player->readyweapon = player->pendingweapon;

  P_BringUpWeapon(player);
}

//
// A_Raise
//

void A_Raise(player_t *player, pspdef_t *psp)
{
  statenum_t newstate;

  CHECK_WEAPON_CODEPOINTER("A_Raise", player);

  psp->sy -= RAISESPEED;

  if (psp->sy > WEAPONTOP)
    return;

  psp->sy = WEAPONTOP;

  // The weapon has been raised all the way,
  //  so change to the ready state.

  if (player->powers[pw_weaponlevel2])
    newstate = wpnlev2info[player->readyweapon].readystate;
  else
    newstate = weaponinfo[player->readyweapon].readystate;

  P_SetPsprite(player, ps_weapon, newstate);
}


// Weapons now recoil, amount depending on the weapon.              // phares
//                                                                  //   |
// The P_SetPsprite call in each of the weapon firing routines      //   V
// was moved here so the recoil could be synched with the
// muzzle flash, rather than the pressing of the trigger.
// The BFG delay caused this to be necessary.

static void A_FireSomething(player_t* player,int adder)
{
  P_SetPsprite(player, ps_flash,
               weaponinfo[player->readyweapon].flashstate+adder);

  // killough 3/27/98: prevent recoil in no-clipping mode
  if (!(player->mo->flags & MF_NOCLIP))
    if (!compatibility && weapon_recoil)
      P_Thrust(player,
               ANG180+player->mo->angle,                          //   ^
               2048*recoil_values[player->readyweapon]);          //   |
}                                                                 // phares

//
// A_GunFlash
//

void A_GunFlash(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_GunFlash", player);

  P_SetMobjState(player->mo, S_PLAY_ATK2);

  A_FireSomething(player,0);                                      // phares
}

//
// WEAPON ATTACKS
//

//
// A_Punch
//

void A_Punch(player_t *player, pspdef_t *psp)
{
  angle_t angle;
  int t, slope, damage, range;

  CHECK_WEAPON_CODEPOINTER("A_Punch", player);

  damage = (P_Random(pr_punch)%10+1)<<1;

  if (player->powers[pw_strength])
    damage *= 10;

  angle = player->mo->angle;

  // killough 5/5/98: remove dependence on order of evaluation:
  t = P_Random(pr_punchangle);
  angle += (t - P_Random(pr_punchangle))<<18;

  range = (mbf21 ? player->mo->info->meleerange : MELEERANGE);

  /* killough 8/2/98: make autoaiming prefer enemies */
  if (!mbf_features ||
      (slope = P_AimLineAttack(player->mo, angle, range, MF_FRIEND),
       !linetarget))
    slope = P_AimLineAttack(player->mo, angle, range, 0);

  P_LineAttack(player->mo, angle, range, slope, damage);

  if (!linetarget)
    return;

  S_StartSound(player->mo, sfx_punch);

  // turn to face target

  player->mo->angle = R_PointToAngle2(player->mo->x, player->mo->y,
                                      linetarget->x, linetarget->y);
  R_SmoothPlaying_Reset(player); // e6y
}

//
// A_Saw
//

void A_Saw(player_t *player, pspdef_t *psp)
{
  int slope, damage, range;
  angle_t angle;
  int t;

  CHECK_WEAPON_CODEPOINTER("A_Saw", player);

  damage = 2*(P_Random(pr_saw)%10+1);
  angle = player->mo->angle;
  // killough 5/5/98: remove dependence on order of evaluation:
  t = P_Random(pr_saw);
  angle += (t - P_Random(pr_saw))<<18;

  // Use meleerange + 1 so that the puff doesn't skip the flash
  range = (mbf21 ? player->mo->info->meleerange : MELEERANGE) + 1;

  /* killough 8/2/98: make autoaiming prefer enemies */
  if (!mbf_features ||
      (slope = P_AimLineAttack(player->mo, angle, range, MF_FRIEND),
       !linetarget))
    slope = P_AimLineAttack(player->mo, angle, range, 0);

  P_LineAttack(player->mo, angle, range, slope, damage);

  if (!linetarget)
    {
      S_StartSound(player->mo, sfx_sawful);
      return;
    }

  S_StartSound(player->mo, sfx_sawhit);

  // turn to face target
  angle = R_PointToAngle2(player->mo->x, player->mo->y,
                          linetarget->x, linetarget->y);

  if (angle - player->mo->angle > ANG180) {
    if (angle - player->mo->angle < -ANG90/20)
      player->mo->angle = angle + ANG90/21;
    else
      player->mo->angle -= ANG90/20;
  } else {
    if (angle - player->mo->angle > ANG90/20)
      player->mo->angle = angle - ANG90/21;
    else
      player->mo->angle += ANG90/20;
  }

  player->mo->flags |= MF_JUSTATTACKED;
  R_SmoothPlaying_Reset(player); // e6y
}

//
// A_FireMissile
//

void A_FireMissile(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_FireMissile", player);

  P_SubtractAmmo(player, 1);
  P_SpawnPlayerMissile(player->mo, MT_ROCKET);
}

//
// A_FireBFG
//

void A_FireBFG(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_FireBFG", player);

  P_SubtractAmmo(player, BFGCELLS);
  P_SpawnPlayerMissile(player->mo, MT_BFG);
}

//
// A_FireOldBFG
//
// This function emulates Doom's Pre-Beta BFG
// By Lee Killough 6/6/98, 7/11/98, 7/19/98, 8/20/98
//
// This code may not be used in other mods without appropriate credit given.
// Code leeches will be telefragged.

int autoaim = 0;  // killough 7/19/98: autoaiming was not in original beta
void A_FireOldBFG(player_t *player, pspdef_t *psp)
{
  int type = MT_PLASMA1;

  if (compatibility_level < mbf_compatibility)
    return;

  CHECK_WEAPON_CODEPOINTER("A_FireOldBFG", player);

  if (weapon_recoil && !(player->mo->flags & MF_NOCLIP))
    P_Thrust(player, ANG180 + player->mo->angle,
    512*recoil_values[wp_plasma]);

  P_SubtractAmmo(player, 1);

  player->extralight = 2;

  do
  {
    mobj_t *th, *mo = player->mo;
    angle_t an = mo->angle;
    angle_t an1 = ((P_Random(pr_bfg)&127) - 64) * (ANG90/768) + an;
    angle_t an2 = ((P_Random(pr_bfg)&127) - 64) * (ANG90/640) + ANG90;
    extern int autoaim;

    if (autoaim/* || !beta_emulation*/)
    {
      // killough 8/2/98: make autoaiming prefer enemies
      uint_64_t mask = mbf_features ? MF_FRIEND : 0;
      fixed_t slope;
      do
      {
        slope = P_AimLineAttack(mo, an, 16*64*FRACUNIT, mask);
        if (!linetarget)
          slope = P_AimLineAttack(mo, an += 1<<26, 16*64*FRACUNIT, mask);
        if (!linetarget)
          slope = P_AimLineAttack(mo, an -= 2<<26, 16*64*FRACUNIT, mask);
        if (!linetarget)
          slope = 0, an = mo->angle;
      }
      while (mask && (mask=0, !linetarget));     // killough 8/2/98
      an1 += an - mo->angle;
      an2 += tantoangle[slope >> DBITS];
    }

    th = P_SpawnMobj(mo->x, mo->y, mo->z + 62*FRACUNIT - player->psprites[ps_weapon].sy, type);
    P_SetTarget(&th->target, mo);
    th->angle = an1;
    th->momx = finecosine[an1>>ANGLETOFINESHIFT] * 25;
    th->momy = finesine[an1>>ANGLETOFINESHIFT] * 25;
    th->momz = finetangent[an2>>ANGLETOFINESHIFT] * 25;
    P_CheckMissileSpawn(th);
  }
  while ((type != MT_PLASMA2) && (type = MT_PLASMA2)); //killough: obfuscated!
}

//
// A_FirePlasma
//

void A_FirePlasma(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_FirePlasma", player);

  P_SubtractAmmo(player, 1);

  A_FireSomething(player,P_Random(pr_plasma)&1);              // phares
  P_SpawnPlayerMissile(player->mo, MT_PLASMA);
}

//
// P_BulletSlope
// Sets a slope so a near miss is at aproximately
// the height of the intended target
//

//e6y static
fixed_t bulletslope;

static void P_BulletSlope(mobj_t *mo)
{
  angle_t an = mo->angle;    // see which target is to be aimed at

  if (comperr(comperr_freeaim))
    bulletslope = finetangent[(ANG90 - mo->pitch) >> ANGLETOFINESHIFT];
  else
  {
    /* killough 8/2/98: make autoaiming prefer enemies */
    uint_64_t mask = mbf_features ? MF_FRIEND : 0;

    do
    {
      bulletslope = P_AimLineAttack(mo, an, 16*64*FRACUNIT, mask);
      if (!linetarget)
        bulletslope = P_AimLineAttack(mo, an += 1<<26, 16*64*FRACUNIT, mask);
      if (!linetarget)
        bulletslope = P_AimLineAttack(mo, an -= 2<<26, 16*64*FRACUNIT, mask);
      if (heretic && !linetarget)
        bulletslope = (mo->player->lookdir << FRACBITS) / 173;
    }
    while (mask && (mask=0, !linetarget));  /* killough 8/2/98 */
  }
}

//
// P_GunShot
//

static void P_GunShot(mobj_t *mo, dboolean accurate)
{
  int damage = 5*(P_Random(pr_gunshot)%3+1);
  angle_t angle = mo->angle;

  if (!accurate)
    {  // killough 5/5/98: remove dependence on order of evaluation:
      int t = P_Random(pr_misfire);
      angle += (t - P_Random(pr_misfire))<<18;
    }

  P_LineAttack(mo, angle, MISSILERANGE, bulletslope, damage);
}

//
// A_FirePistol
//

void A_FirePistol(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_FirePistol", player);

  S_StartSound(player->mo, sfx_pistol);

  P_SetMobjState(player->mo, S_PLAY_ATK2);
  P_SubtractAmmo(player, 1);

  A_FireSomething(player,0);                                      // phares
  P_BulletSlope(player->mo);
  P_GunShot(player->mo, !player->refire);
}

//
// A_FireShotgun
//

void A_FireShotgun(player_t *player, pspdef_t *psp)
{
  int i;

  CHECK_WEAPON_CODEPOINTER("A_FireShotgun", player);

  S_StartSound(player->mo, sfx_shotgn);
  P_SetMobjState(player->mo, S_PLAY_ATK2);

  P_SubtractAmmo(player, 1);

  A_FireSomething(player,0);                                      // phares

  P_BulletSlope(player->mo);

  for (i=0; i<7; i++)
    P_GunShot(player->mo, false);
}

//
// A_FireShotgun2
//

void A_FireShotgun2(player_t *player, pspdef_t *psp)
{
  int i;

  CHECK_WEAPON_CODEPOINTER("A_FireShotgun2", player);

  S_StartSound(player->mo, sfx_dshtgn);
  P_SetMobjState(player->mo, S_PLAY_ATK2);
  P_SubtractAmmo(player, 2);

  A_FireSomething(player,0);                                      // phares

  P_BulletSlope(player->mo);

  for (i=0; i<20; i++)
    {
      int damage = 5*(P_Random(pr_shotgun)%3+1);
      angle_t angle = player->mo->angle;
      // killough 5/5/98: remove dependence on order of evaluation:
      int t = P_Random(pr_shotgun);
      angle += (t - P_Random(pr_shotgun))<<19;
      t = P_Random(pr_shotgun);
      P_LineAttack(player->mo, angle, MISSILERANGE, bulletslope +
                   ((t - P_Random(pr_shotgun))<<5), damage);
    }
}

//
// A_FireCGun
//

void A_FireCGun(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_FireCGun", player);

  if (player->ammo[weaponinfo[player->readyweapon].ammo] || comp[comp_sound])
    S_StartSound(player->mo, sfx_pistol);

  if (!player->ammo[weaponinfo[player->readyweapon].ammo])
    return;

  P_SetMobjState(player->mo, S_PLAY_ATK2);
  P_SubtractAmmo(player, 1);

  A_FireSomething(player,psp->state - &states[S_CHAIN1]);           // phares

  P_BulletSlope(player->mo);

  P_GunShot(player->mo, !player->refire);
}

void A_Light0(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_Light0", player);

  player->extralight = 0;
}

void A_Light1 (player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_Light1", player);

  player->extralight = 1;
}

void A_Light2 (player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_Light2", player);

  player->extralight = 2;
}

//
// A_BFGSpray
// Spawn a BFG explosion on every monster in view
//

void A_BFGSpray(mobj_t *mo)
{
  int i;

  for (i=0 ; i<40 ; i++)  // offset angles from its attack angle
    {
      int j, damage;
      angle_t an = mo->angle - ANG90/2 + ANG90/40*i;

      // mo->target is the originator (player) of the missile

      // killough 8/2/98: make autoaiming prefer enemies
      if (!mbf_features ||
         (P_AimLineAttack(mo->target, an, 16*64*FRACUNIT, MF_FRIEND),
         !linetarget))
        P_AimLineAttack(mo->target, an, 16*64*FRACUNIT, 0);

      if (!linetarget)
        continue;

      P_SpawnMobj(linetarget->x, linetarget->y,
                  linetarget->z + (linetarget->height>>2), MT_EXTRABFG);

      for (damage=j=0; j<15; j++)
        damage += (P_Random(pr_bfg)&7) + 1;

      P_DamageMobj(linetarget, mo->target, mo->target, damage);
    }
}

//
// A_BFGsound
//

void A_BFGsound(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_BFGsound", player);

  S_StartSound(player->mo, sfx_bfg);
}

//
// [XA] New mbf21 codepointers
//

//
// A_WeaponProjectile
// A parameterized player weapon projectile attack. Does not consume ammo.
//   args[0]: Type of actor to spawn
//   args[1]: Angle (degrees, in fixed point), relative to calling player's angle
//   args[2]: Pitch (degrees, in fixed point), relative to calling player's pitch; approximated
//   args[3]: X/Y spawn offset, relative to calling player's angle
//   args[4]: Z spawn offset, relative to player's default projectile fire height
//
void A_WeaponProjectile(player_t *player, pspdef_t *psp)
{
  int type, angle, pitch, spawnofs_xy, spawnofs_z;
  mobj_t *mo;
  int an;

  CHECK_WEAPON_CODEPOINTER("A_WeaponProjectile", player);

  if (!mbf21 || !psp->state || !psp->state->args[0])
    return;

  type        = psp->state->args[0] - 1;
  angle       = psp->state->args[1];
  pitch       = psp->state->args[2];
  spawnofs_xy = psp->state->args[3];
  spawnofs_z  = psp->state->args[4];

  mo = P_SpawnPlayerMissile(player->mo, type);
  if (!mo)
    return;

  // adjust angle
  mo->angle += (unsigned int)(((int_64_t)angle << 16) / 360);
  an = mo->angle >> ANGLETOFINESHIFT;
  mo->momx = FixedMul(mo->info->speed, finecosine[an]);
  mo->momy = FixedMul(mo->info->speed, finesine[an]);

  // adjust pitch (approximated, using Doom's ye olde
  // finetangent table; same method as autoaim)
  mo->momz += FixedMul(mo->info->speed, DegToSlope(pitch));

  // adjust position
  an = (player->mo->angle - ANG90) >> ANGLETOFINESHIFT;
  mo->x += FixedMul(spawnofs_xy, finecosine[an]);
  mo->y += FixedMul(spawnofs_xy, finesine[an]);
  mo->z += spawnofs_z;

  // set tracer to the player's autoaim target,
  // so player seeker missiles prioritizing the
  // baddie the player is actually aiming at. ;)
  mo->tracer = linetarget;
}

//
// A_WeaponBulletAttack
// A parameterized player weapon bullet attack. Does not consume ammo.
//   args[0]: Horizontal spread (degrees, in fixed point)
//   args[1]: Vertical spread (degrees, in fixed point)
//   args[2]: Number of bullets to fire; if not set, defaults to 1
//   args[3]: Base damage of attack (e.g. for 5d3, customize the 5); if not set, defaults to 5
//   args[4]: Attack damage modulus (e.g. for 5d3, customize the 3); if not set, defaults to 3
//
void A_WeaponBulletAttack(player_t *player, pspdef_t *psp)
{
  int hspread, vspread, numbullets, damagebase, damagemod;
  int i, damage, angle, slope;

  CHECK_WEAPON_CODEPOINTER("A_WeaponBulletAttack", player);

  if (!mbf21 || !psp->state)
    return;

  hspread    = psp->state->args[0];
  vspread    = psp->state->args[1];
  numbullets = psp->state->args[2];
  damagebase = psp->state->args[3];
  damagemod  = psp->state->args[4];

  P_BulletSlope(player->mo);

  for (i = 0; i < numbullets; i++)
  {
    damage = (P_Random(pr_mbf21) % damagemod + 1) * damagebase;
    angle = (int)player->mo->angle + P_RandomHitscanAngle(pr_mbf21, hspread);
    slope = bulletslope + P_RandomHitscanSlope(pr_mbf21, vspread);

    P_LineAttack(player->mo, angle, MISSILERANGE, slope, damage);
  }
}

//
// A_WeaponMeleeAttack
// A parameterized player weapon melee attack.
//   args[0]: Base damage of attack (e.g. for 2d10, customize the 2); if not set, defaults to 2
//   args[1]: Attack damage modulus (e.g. for 2d10, customize the 10); if not set, defaults to 10
//   args[2]: Berserk damage multiplier (fixed point); if not set, defaults to 1.0 (no change).
//   args[3]: Sound to play if attack hits
//   args[4]: Range (fixed point); if not set, defaults to player mobj's melee range
//
void A_WeaponMeleeAttack(player_t *player, pspdef_t *psp)
{
  int damagebase, damagemod, zerkfactor, hitsound, range;
  angle_t angle;
  int t, slope, damage;

  CHECK_WEAPON_CODEPOINTER("A_WeaponMeleeAttack", player);

  if (!mbf21 || !psp->state)
    return;

  damagebase = psp->state->args[0];
  damagemod  = psp->state->args[1];
  zerkfactor = psp->state->args[2];
  hitsound   = psp->state->args[3];
  range      = psp->state->args[4];

  if (range == 0)
    range = player->mo->info->meleerange;

  damage = (P_Random(pr_mbf21) % damagemod + 1) * damagebase;
  if (player->powers[pw_strength])
    damage = (damage * zerkfactor) >> FRACBITS;

  // slight randomization; weird vanillaism here. :P
  angle = player->mo->angle;

  t = P_Random(pr_mbf21);
  angle += (t - P_Random(pr_mbf21))<<18;

  // make autoaim prefer enemies
  slope = P_AimLineAttack(player->mo, angle, range, MF_FRIEND);
  if (!linetarget)
    slope = P_AimLineAttack(player->mo, angle, range, 0);

  // attack, dammit!
  P_LineAttack(player->mo, angle, range, slope, damage);

  // missed? ah, welp.
  if (!linetarget)
    return;

  // un-missed!
  S_StartSound(player->mo, hitsound);

  // turn to face target
  player->mo->angle = R_PointToAngle2(player->mo->x, player->mo->y, linetarget->x, linetarget->y);
  R_SmoothPlaying_Reset(player);
}

//
// A_WeaponSound
// Plays a sound. Usable from weapons, unlike A_PlaySound
//   args[0]: ID of sound to play
//   args[1]: If 1, play sound at full volume (may be useful in DM?)
//
void A_WeaponSound(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_WeaponSound", player);

  if (!mbf21 || !psp->state)
    return;

  S_StartSound(psp->state->args[1] ? NULL : player->mo, psp->state->args[0]);
}

//
// A_WeaponAlert
// Alerts monsters to the player's presence. Handy when combined with WPF_SILENT.
//
void A_WeaponAlert(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_WeaponAlert", player);

  if (!mbf21)
    return;

  P_NoiseAlert(player->mo, player->mo);
}

//
// A_WeaponJump
// Jumps to the specified state, with variable random chance.
// Basically the same as A_RandomJump, but for weapons.
//   args[0]: State number
//   args[1]: Chance, out of 255, to make the jump
//
void A_WeaponJump(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_WeaponJump", player);

  if (!mbf21 || !psp->state)
    return;

  if (P_Random(pr_mbf21) < psp->state->args[1])
    P_SetPspritePtr(player, psp, psp->state->args[0]);
}

//
// A_ConsumeAmmo
// Subtracts ammo from the player's "inventory". 'Nuff said.
//   args[0]: Amount of ammo to consume. If zero, use the weapon's ammo-per-shot amount.
//
void A_ConsumeAmmo(player_t *player, pspdef_t *psp)
{
  int amount;
  ammotype_t type;

  CHECK_WEAPON_CODEPOINTER("A_ConsumeAmmo", player);

  if (!mbf21)
    return;

  // don't do dumb things, kids
  type = weaponinfo[player->readyweapon].ammo;
  if (!psp->state || type == am_noammo)
	return;

  // use the weapon's ammo-per-shot amount if zero.
  // to subtract zero ammo, don't call this function. ;)
  if (psp->state->args[0] != 0)
    amount = psp->state->args[0];
  else
    amount = weaponinfo[player->readyweapon].ammopershot;

  // subtract ammo, but don't let it get below zero
  if (player->ammo[type] >= amount)
    player->ammo[type] -= amount;
  else
    player->ammo[type] = 0;
}

//
// A_CheckAmmo
// Jumps to a state if the player's ammo is lower than the specified amount.
//   args[0]: State to jump to
//   args[1]: Minimum required ammo to NOT jump. If zero, use the weapon's ammo-per-shot amount.
//
void A_CheckAmmo(player_t *player, pspdef_t *psp)
{
  int amount;
  ammotype_t type;

  CHECK_WEAPON_CODEPOINTER("A_CheckAmmo", player);

  if (!mbf21)
    return;

  type = weaponinfo[player->readyweapon].ammo;
  if (!psp->state || type == am_noammo)
    return;

  if (psp->state->args[1] != 0)
    amount = psp->state->args[1];
  else
    amount = weaponinfo[player->readyweapon].ammopershot;

  if (player->ammo[type] < amount)
    P_SetPspritePtr(player, psp, psp->state->args[0]);
}

//
// A_RefireTo
// Jumps to a state if the player is holding down the fire button
//   args[0]: State to jump to
//   args[1]: If nonzero, skip the ammo check
//
void A_RefireTo(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_RefireTo", player);

  if (!mbf21 || !psp->state)
    return;

  if ((psp->state->args[1] || P_CheckAmmo(player))
  &&  (player->cmd.buttons & BT_ATTACK)
  &&  (player->pendingweapon == wp_nochange && player->health))
    P_SetPspritePtr(player, psp, psp->state->args[0]);
}

//
// A_GunFlashTo
// Sets the weapon flash layer to the specified state.
//   args[0]: State number
//   args[1]: If nonzero, don't change the player actor state
//
void A_GunFlashTo(player_t *player, pspdef_t *psp)
{
  CHECK_WEAPON_CODEPOINTER("A_GunFlashTo", player);

  if (!mbf21 || !psp->state)
    return;

  if(!psp->state->args[1])
    P_SetMobjState(player->mo, S_PLAY_ATK2);

  P_SetPsprite(player, ps_flash, psp->state->args[0]);
}

//
// P_SetupPsprites
// Called at start of level for each player.
//

void P_SetupPsprites(player_t *player)
{
  int i;

  // remove all psprites
  for (i=0; i<NUMPSPRITES; i++)
    player->psprites[i].state = NULL;

  // spawn the gun
  player->pendingweapon = player->readyweapon;
  P_BringUpWeapon(player);
}

//
// P_MovePsprites
// Called every tic by player thinking routine.
//

void P_MovePsprites(player_t *player)
{
  pspdef_t *psp = player->psprites;
  int i;

  // a null state means not active
  // drop tic count and possibly change state
  // a -1 tic count never changes

  for (i=0; i<NUMPSPRITES; i++, psp++)
    if (psp->state && psp->tics != -1 && !--psp->tics)
      P_SetPsprite(player, i, psp->state->nextstate);

  player->psprites[ps_flash].sx = player->psprites[ps_weapon].sx;
  player->psprites[ps_flash].sy = player->psprites[ps_weapon].sy;
}

// heretic

#include "heretic/def.h"
#include "p_user.h"
#include "p_maputl.h"

#define MAGIC_JUNK 1234
#define FLAME_THROWER_TICS 10*35

#define MAX_MACE_SPOTS 8

static int MaceSpotCount;
static struct
{
    fixed_t x;
    fixed_t y;
} MaceSpots[MAX_MACE_SPOTS];

void A_BeakReady(player_t * player, pspdef_t * psp)
{
    if (player->cmd.buttons & BT_ATTACK)
    {                           // Chicken beak attack
        player->attackdown = true;
        P_SetMobjState(player->mo, HERETIC_S_CHICPLAY_ATK1);
        if (player->powers[pw_weaponlevel2])
        {
            P_SetPsprite(player, ps_weapon, HERETIC_S_BEAKATK2_1);
        }
        else
        {
            P_SetPsprite(player, ps_weapon, HERETIC_S_BEAKATK1_1);
        }
        P_NoiseAlert(player->mo, player->mo);
    }
    else
    {
        if (player->mo->state == &states[HERETIC_S_CHICPLAY_ATK1])
        {                       // Take out of attack state
            P_SetMobjState(player->mo, HERETIC_S_CHICPLAY);
        }
        player->attackdown = false;
    }
}

void A_BeakRaise(player_t * player, pspdef_t * psp)
{
    psp->sy = WEAPONTOP;
    P_SetPsprite(player, ps_weapon,
                 wpnlev1info[player->readyweapon].readystate);
}

extern mobjtype_t PuffType;

void A_BeakAttackPL1(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    int damage;
    int slope;

    damage = 1 + (P_Random(pr_heretic) & 3);
    angle = player->mo->angle;
    slope = P_AimLineAttack(player->mo, angle, MELEERANGE, 0);
    PuffType = HERETIC_MT_BEAKPUFF;
    P_LineAttack(player->mo, angle, MELEERANGE, slope, damage);
    if (linetarget)
    {
        player->mo->angle = R_PointToAngle2(player->mo->x,
                                            player->mo->y, linetarget->x,
                                            linetarget->y);
    }
    S_StartSound(player->mo, heretic_sfx_chicpk1 + (P_Random(pr_heretic) % 3));
    player->chickenPeck = 12;
    psp->tics -= P_Random(pr_heretic) & 7;
}

void A_BeakAttackPL2(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    int damage;
    int slope;

    damage = HITDICE(4);
    angle = player->mo->angle;
    slope = P_AimLineAttack(player->mo, angle, MELEERANGE, 0);
    PuffType = HERETIC_MT_BEAKPUFF;
    P_LineAttack(player->mo, angle, MELEERANGE, slope, damage);
    if (linetarget)
    {
        player->mo->angle = R_PointToAngle2(player->mo->x,
                                            player->mo->y, linetarget->x,
                                            linetarget->y);
    }
    S_StartSound(player->mo, heretic_sfx_chicpk1 + (P_Random(pr_heretic) % 3));
    player->chickenPeck = 12;
    psp->tics -= P_Random(pr_heretic) & 3;
}

void A_StaffAttackPL1(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    int damage;
    int slope;

    damage = 5 + (P_Random(pr_heretic) & 15);
    angle = player->mo->angle;
    angle += P_SubRandom() << 18;
    slope = P_AimLineAttack(player->mo, angle, MELEERANGE, 0);
    PuffType = HERETIC_MT_STAFFPUFF;
    P_LineAttack(player->mo, angle, MELEERANGE, slope, damage);
    if (linetarget)
    {
        //S_StartSound(player->mo, sfx_stfhit);
        // turn to face target
        player->mo->angle = R_PointToAngle2(player->mo->x,
                                            player->mo->y, linetarget->x,
                                            linetarget->y);
        R_SmoothPlaying_Reset(player); // e6y
    }
}

void A_StaffAttackPL2(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    int damage;
    int slope;

    // P_inter.c:P_DamageMobj() handles target momentums
    damage = 18 + (P_Random(pr_heretic) & 63);
    angle = player->mo->angle;
    angle += P_SubRandom() << 18;
    slope = P_AimLineAttack(player->mo, angle, MELEERANGE, 0);
    PuffType = HERETIC_MT_STAFFPUFF2;
    P_LineAttack(player->mo, angle, MELEERANGE, slope, damage);
    if (linetarget)
    {
        //S_StartSound(player->mo, sfx_stfpow);
        // turn to face target
        player->mo->angle = R_PointToAngle2(player->mo->x,
                                            player->mo->y, linetarget->x,
                                            linetarget->y);
        R_SmoothPlaying_Reset(player); // e6y
    }
}

void A_FireBlasterPL1(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;
    angle_t angle;
    int damage;

    mo = player->mo;
    S_StartSound(mo, heretic_sfx_gldhit);
    player->ammo[am_blaster] -= USE_BLSR_AMMO_1;
    P_BulletSlope(mo);
    damage = HITDICE(4);
    angle = mo->angle;
    if (player->refire)
    {
        angle += P_SubRandom() << 18;
    }
    PuffType = HERETIC_MT_BLASTERPUFF1;
    P_LineAttack(mo, angle, MISSILERANGE, bulletslope, damage);
    S_StartSound(player->mo, heretic_sfx_blssht);
}

void A_FireBlasterPL2(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;

    player->ammo[am_blaster] -=
        deathmatch ? USE_BLSR_AMMO_1 : USE_BLSR_AMMO_2;
    mo = P_SpawnPlayerMissile(player->mo, HERETIC_MT_BLASTERFX1);
    if (mo)
    {
        mo->thinker.function = P_BlasterMobjThinker;
    }
    S_StartSound(player->mo, heretic_sfx_blssht);
}

void A_FireGoldWandPL1(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;
    angle_t angle;
    int damage;

    mo = player->mo;
    player->ammo[am_goldwand] -= USE_GWND_AMMO_1;
    P_BulletSlope(mo);
    damage = 7 + (P_Random(pr_heretic) & 7);
    angle = mo->angle;
    if (player->refire)
    {
        angle += P_SubRandom() << 18;
    }
    PuffType = HERETIC_MT_GOLDWANDPUFF1;
    P_LineAttack(mo, angle, MISSILERANGE, bulletslope, damage);
    S_StartSound(player->mo, heretic_sfx_gldhit);
}

void A_FireGoldWandPL2(player_t * player, pspdef_t * psp)
{
    int i;
    mobj_t *mo;
    angle_t angle;
    int damage;
    fixed_t momz;

    mo = player->mo;
    player->ammo[am_goldwand] -=
        deathmatch ? USE_GWND_AMMO_1 : USE_GWND_AMMO_2;
    PuffType = HERETIC_MT_GOLDWANDPUFF2;
    P_BulletSlope(mo);
    momz = FixedMul(mobjinfo[HERETIC_MT_GOLDWANDFX2].speed, bulletslope);
    P_SpawnMissileAngle(mo, HERETIC_MT_GOLDWANDFX2, mo->angle - (ANG45 / 8), momz);
    P_SpawnMissileAngle(mo, HERETIC_MT_GOLDWANDFX2, mo->angle + (ANG45 / 8), momz);
    angle = mo->angle - (ANG45 / 8);
    for (i = 0; i < 5; i++)
    {
        damage = 1 + (P_Random(pr_heretic) & 7);
        P_LineAttack(mo, angle, MISSILERANGE, bulletslope, damage);
        angle += ((ANG45 / 8) * 2) / 4;
    }
    S_StartSound(player->mo, heretic_sfx_gldhit);
}

void A_FireMacePL1B(player_t * player, pspdef_t * psp)
{
    mobj_t *pmo;
    mobj_t *ball;
    angle_t angle;

    if (player->ammo[am_mace] < USE_MACE_AMMO_1)
    {
        return;
    }
    player->ammo[am_mace] -= USE_MACE_AMMO_1;
    pmo = player->mo;

    // Vanilla bug here:
    // Original code here looks like:
    //   (pmo->flags2 & MF2_FEETARECLIPPED != 0)
    // C's operator precedence interprets this as:
    //   (pmo->flags2 & (MF2_FEETARECLIPPED != 0))
    // Which simplifies to:
    //   (pmo->flags2 & 1)
    ball = P_SpawnMobj(pmo->x, pmo->y, pmo->z + 28 * FRACUNIT
                       - FOOTCLIPSIZE * (pmo->flags2 & 1), HERETIC_MT_MACEFX2);

    ball->momz = 2 * FRACUNIT + ((player->lookdir) << (FRACBITS - 5));
    angle = pmo->angle;
    P_SetTarget(&ball->target, pmo);
    ball->angle = angle;
    ball->z += (player->lookdir) << (FRACBITS - 4);
    angle >>= ANGLETOFINESHIFT;
    ball->momx = (pmo->momx >> 1)
        + FixedMul(ball->info->speed, finecosine[angle]);
    ball->momy = (pmo->momy >> 1)
        + FixedMul(ball->info->speed, finesine[angle]);
    S_StartSound(ball, heretic_sfx_lobsht);
    P_CheckMissileSpawn(ball);
}

void A_FireMacePL1(player_t * player, pspdef_t * psp)
{
    mobj_t *ball;

    if (P_Random(pr_heretic) < 28)
    {
        A_FireMacePL1B(player, psp);
        return;
    }
    if (player->ammo[am_mace] < USE_MACE_AMMO_1)
    {
        return;
    }
    player->ammo[am_mace] -= USE_MACE_AMMO_1;
    psp->sx = ((P_Random(pr_heretic) & 3) - 2) * FRACUNIT;
    psp->sy = WEAPONTOP + (P_Random(pr_heretic) & 3) * FRACUNIT;
    ball = P_SPMAngle(player->mo, HERETIC_MT_MACEFX1, player->mo->angle
                      + (((P_Random(pr_heretic) & 7) - 4) << 24));
    if (ball)
    {
        ball->special1.i = 16;    // tics till dropoff
    }
}

void A_MacePL1Check(mobj_t * ball)
{
    angle_t angle;

    if (ball->special1.i == 0)
    {
        return;
    }
    ball->special1.i -= 4;
    if (ball->special1.i > 0)
    {
        return;
    }
    ball->special1.i = 0;
    ball->flags2 |= MF2_LOGRAV;
    angle = ball->angle >> ANGLETOFINESHIFT;
    ball->momx = FixedMul(7 * FRACUNIT, finecosine[angle]);
    ball->momy = FixedMul(7 * FRACUNIT, finesine[angle]);
    ball->momz -= ball->momz >> 1;
}

void A_MaceBallImpact(mobj_t * ball)
{
    if ((ball->z <= ball->floorz) && (P_HitFloor(ball) != FLOOR_SOLID))
    {                           // Landed in some sort of liquid
        P_RemoveMobj(ball);
        return;
    }
    if ((ball->health != MAGIC_JUNK) && (ball->z <= ball->floorz)
        && ball->momz)
    {                           // Bounce
        ball->health = MAGIC_JUNK;
        ball->momz = (ball->momz * 192) >> 8;
        ball->flags2 &= ~MF2_FLOORBOUNCE;
        P_SetMobjState(ball, ball->info->spawnstate);
        S_StartSound(ball, heretic_sfx_bounce);
    }
    else
    {                           // Explode
        ball->flags |= MF_NOGRAVITY;
        ball->flags2 &= ~MF2_LOGRAV;
        S_StartSound(ball, heretic_sfx_lobhit);
    }
}

void A_MaceBallImpact2(mobj_t * ball)
{
    mobj_t *tiny;
    angle_t angle;

    if ((ball->z <= ball->floorz) && (P_HitFloor(ball) != FLOOR_SOLID))
    {                           // Landed in some sort of liquid
        P_RemoveMobj(ball);
        return;
    }
    if ((ball->z != ball->floorz) || (ball->momz < 2 * FRACUNIT))
    {                           // Explode
        ball->momx = ball->momy = ball->momz = 0;
        ball->flags |= MF_NOGRAVITY;
        ball->flags2 &= ~(MF2_LOGRAV | MF2_FLOORBOUNCE);
    }
    else
    {                           // Bounce
        ball->momz = (ball->momz * 192) >> 8;
        P_SetMobjState(ball, ball->info->spawnstate);

        tiny = P_SpawnMobj(ball->x, ball->y, ball->z, HERETIC_MT_MACEFX3);
        angle = ball->angle + ANG90;
        P_SetTarget(&tiny->target, ball->target);
        tiny->angle = angle;
        angle >>= ANGLETOFINESHIFT;
        tiny->momx = (ball->momx >> 1) + FixedMul(ball->momz - FRACUNIT,
                                                  finecosine[angle]);
        tiny->momy = (ball->momy >> 1) + FixedMul(ball->momz - FRACUNIT,
                                                  finesine[angle]);
        tiny->momz = ball->momz;
        P_CheckMissileSpawn(tiny);

        tiny = P_SpawnMobj(ball->x, ball->y, ball->z, HERETIC_MT_MACEFX3);
        angle = ball->angle - ANG90;
        P_SetTarget(&tiny->target, ball->target);
        tiny->angle = angle;
        angle >>= ANGLETOFINESHIFT;
        tiny->momx = (ball->momx >> 1) + FixedMul(ball->momz - FRACUNIT,
                                                  finecosine[angle]);
        tiny->momy = (ball->momy >> 1) + FixedMul(ball->momz - FRACUNIT,
                                                  finesine[angle]);
        tiny->momz = ball->momz;
        P_CheckMissileSpawn(tiny);
    }
}

void A_FireMacePL2(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;

    player->ammo[am_mace] -= deathmatch ? USE_MACE_AMMO_1 : USE_MACE_AMMO_2;
    mo = P_SpawnPlayerMissile(player->mo, HERETIC_MT_MACEFX4);
    if (mo)
    {
        mo->momx += player->mo->momx;
        mo->momy += player->mo->momy;
        mo->momz = 2 * FRACUNIT + ((player->lookdir) << (FRACBITS - 5));
        if (linetarget)
        {
            mo->special1.m = linetarget;
        }
    }
    S_StartSound(player->mo, heretic_sfx_lobsht);
}

void A_DeathBallImpact(mobj_t * ball)
{
    int i;
    mobj_t *target;
    angle_t angle;
    dboolean newAngle;

    if ((ball->z <= ball->floorz) && (P_HitFloor(ball) != FLOOR_SOLID))
    {                           // Landed in some sort of liquid
        P_RemoveMobj(ball);
        return;
    }
    if ((ball->z <= ball->floorz) && ball->momz)
    {                           // Bounce
        newAngle = false;
        target = (mobj_t *) ball->special1.m;
        if (target)
        {
            if (!(target->flags & MF_SHOOTABLE))
            {                   // Target died
                ball->special1.m = NULL;
            }
            else
            {                   // Seek
                angle = R_PointToAngle2(ball->x, ball->y,
                                        target->x, target->y);
                newAngle = true;
            }
        }
        else
        {                       // Find new target
            angle = 0;
            for (i = 0; i < 16; i++)
            {
                P_AimLineAttack(ball, angle, 10 * 64 * FRACUNIT, 0);
                if (linetarget && ball->target != linetarget)
                {
                    ball->special1.m = linetarget;
                    angle = R_PointToAngle2(ball->x, ball->y,
                                            linetarget->x, linetarget->y);
                    newAngle = true;
                    break;
                }
                angle += ANG45 / 2;
            }
        }
        if (newAngle)
        {
            ball->angle = angle;
            angle >>= ANGLETOFINESHIFT;
            ball->momx = FixedMul(ball->info->speed, finecosine[angle]);
            ball->momy = FixedMul(ball->info->speed, finesine[angle]);
        }
        P_SetMobjState(ball, ball->info->spawnstate);
        S_StartSound(ball, heretic_sfx_pstop);
    }
    else
    {                           // Explode
        ball->flags |= MF_NOGRAVITY;
        ball->flags2 &= ~MF2_LOGRAV;
        S_StartSound(ball, heretic_sfx_phohit);
    }
}

void A_SpawnRippers(mobj_t * actor)
{
    unsigned int i;
    angle_t angle;
    mobj_t *ripper;

    for (i = 0; i < 8; i++)
    {
        ripper = P_SpawnMobj(actor->x, actor->y, actor->z, HERETIC_MT_RIPPER);
        angle = i * ANG45;
        P_SetTarget(&ripper->target, actor->target);
        ripper->angle = angle;
        angle >>= ANGLETOFINESHIFT;
        ripper->momx = FixedMul(ripper->info->speed, finecosine[angle]);
        ripper->momy = FixedMul(ripper->info->speed, finesine[angle]);
        P_CheckMissileSpawn(ripper);
    }
}

void A_FireCrossbowPL1(player_t * player, pspdef_t * psp)
{
    mobj_t *pmo;

    pmo = player->mo;
    player->ammo[am_crossbow] -= USE_CBOW_AMMO_1;
    P_SpawnPlayerMissile(pmo, HERETIC_MT_CRBOWFX1);
    P_SPMAngle(pmo, HERETIC_MT_CRBOWFX3, pmo->angle - (ANG45 / 10));
    P_SPMAngle(pmo, HERETIC_MT_CRBOWFX3, pmo->angle + (ANG45 / 10));
}

void A_FireCrossbowPL2(player_t * player, pspdef_t * psp)
{
    mobj_t *pmo;

    pmo = player->mo;
    player->ammo[am_crossbow] -=
        deathmatch ? USE_CBOW_AMMO_1 : USE_CBOW_AMMO_2;
    P_SpawnPlayerMissile(pmo, HERETIC_MT_CRBOWFX2);
    P_SPMAngle(pmo, HERETIC_MT_CRBOWFX2, pmo->angle - (ANG45 / 10));
    P_SPMAngle(pmo, HERETIC_MT_CRBOWFX2, pmo->angle + (ANG45 / 10));
    P_SPMAngle(pmo, HERETIC_MT_CRBOWFX3, pmo->angle - (ANG45 / 5));
    P_SPMAngle(pmo, HERETIC_MT_CRBOWFX3, pmo->angle + (ANG45 / 5));
}

void A_BoltSpark(mobj_t * bolt)
{
    mobj_t *spark;

    if (P_Random(pr_heretic) > 50)
    {
        spark = P_SpawnMobj(bolt->x, bolt->y, bolt->z, HERETIC_MT_CRBOWFX4);
        spark->x += P_SubRandom() << 10;
        spark->y += P_SubRandom() << 10;
    }
}

void A_FireSkullRodPL1(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;

    if (player->ammo[am_skullrod] < USE_SKRD_AMMO_1)
    {
        return;
    }
    player->ammo[am_skullrod] -= USE_SKRD_AMMO_1;
    mo = P_SpawnPlayerMissile(player->mo, HERETIC_MT_HORNRODFX1);
    // Randomize the first frame
    if (mo && P_Random(pr_heretic) > 128)
    {
        P_SetMobjState(mo, HERETIC_S_HRODFX1_2);
    }
}

void A_FireSkullRodPL2(player_t * player, pspdef_t * psp)
{
    player->ammo[am_skullrod] -=
        deathmatch ? USE_SKRD_AMMO_1 : USE_SKRD_AMMO_2;
    P_SpawnPlayerMissile(player->mo, HERETIC_MT_HORNRODFX2);
    // Use MissileMobj instead of the return value from
    // P_SpawnPlayerMissile because we need to give info to the mobj
    // even if it exploded immediately.
    if (netgame)
    {                           // Multi-player game
        MissileMobj->special2.i = P_GetPlayerNum(player);
    }
    else
    {                           // Always use red missiles in single player games
        MissileMobj->special2.i = 2;
    }
    if (linetarget)
    {
        MissileMobj->special1.m = linetarget;
    }
    S_StartSound(MissileMobj, heretic_sfx_hrnpow);
}

void A_SkullRodPL2Seek(mobj_t * actor)
{
    P_SeekerMissile(actor, &actor->special1.m, ANG1_X * 10, ANG1_X * 30, false);
}

void A_AddPlayerRain(mobj_t * actor)
{
    int playerNum;
    player_t *player;

    playerNum = netgame ? actor->special2.i : 0;
    if (!playeringame[playerNum])
    {                           // Player left the game
        return;
    }
    player = &players[playerNum];
    if (player->health <= 0)
    {                           // Player is dead
        return;
    }
    if (player->rain1 && player->rain2)
    {                           // Terminate an active rain
        if (player->rain1->health < player->rain2->health)
        {
            if (player->rain1->health > 16)
            {
                player->rain1->health = 16;
            }
            player->rain1 = NULL;
        }
        else
        {
            if (player->rain2->health > 16)
            {
                player->rain2->health = 16;
            }
            player->rain2 = NULL;
        }
    }
    // Add rain mobj to list
    if (player->rain1)
    {
        player->rain2 = actor;
    }
    else
    {
        player->rain1 = actor;
    }
}

void A_SkullRodStorm(mobj_t * actor)
{
    fixed_t x;
    fixed_t y;
    mobj_t *mo;
    int playerNum;
    player_t *player;

    if (actor->health-- == 0)
    {
        P_SetMobjState(actor, g_s_null);
        playerNum = netgame ? actor->special2.i : 0;
        if (!playeringame[playerNum])
        {                       // Player left the game
            return;
        }
        player = &players[playerNum];
        if (player->health <= 0)
        {                       // Player is dead
            return;
        }
        if (player->rain1 == actor)
        {
            player->rain1 = NULL;
        }
        else if (player->rain2 == actor)
        {
            player->rain2 = NULL;
        }
        return;
    }
    if (P_Random(pr_heretic) < 25)
    {                           // Fudge rain frequency
        return;
    }
    x = actor->x + ((P_Random(pr_heretic) & 127) - 64) * FRACUNIT;
    y = actor->y + ((P_Random(pr_heretic) & 127) - 64) * FRACUNIT;
    mo = P_SpawnMobj(x, y, ONCEILINGZ, HERETIC_MT_RAINPLR1 + actor->special2.i);
    P_SetTarget(&mo->target, actor->target);
    mo->momx = 1;               // Force collision detection
    mo->momz = -mo->info->speed;
    mo->special2.i = actor->special2.i;     // Transfer player number
    P_CheckMissileSpawn(mo);
    if (!(actor->special1.i & 31))
    {
        S_StartSound(actor, heretic_sfx_ramrain);
    }
    actor->special1.i++;
}

void A_RainImpact(mobj_t * actor)
{
    if (actor->z > actor->floorz)
    {
        P_SetMobjState(actor, HERETIC_S_RAINAIRXPLR1_1 + actor->special2.i);
    }
    else if (P_Random(pr_heretic) < 40)
    {
        P_HitFloor(actor);
    }
}

void A_HideInCeiling(mobj_t * actor)
{
    actor->z = actor->ceilingz + 4 * FRACUNIT;
}

void A_FirePhoenixPL1(player_t * player, pspdef_t * psp)
{
    angle_t angle;

    player->ammo[am_phoenixrod] -= USE_PHRD_AMMO_1;
    P_SpawnPlayerMissile(player->mo, HERETIC_MT_PHOENIXFX1);
    angle = player->mo->angle + ANG180;
    angle >>= ANGLETOFINESHIFT;
    player->mo->momx += FixedMul(4 * FRACUNIT, finecosine[angle]);
    player->mo->momy += FixedMul(4 * FRACUNIT, finesine[angle]);
}

void A_PhoenixPuff(mobj_t * actor)
{
    mobj_t *puff;
    angle_t angle;

    P_SeekerMissile(actor, &actor->special1.m, ANG1_X * 5, ANG1_X * 10, false);
    puff = P_SpawnMobj(actor->x, actor->y, actor->z, HERETIC_MT_PHOENIXPUFF);
    angle = actor->angle + ANG90;
    angle >>= ANGLETOFINESHIFT;
    puff->momx = FixedMul((fixed_t)(FRACUNIT * 1.3), finecosine[angle]);
    puff->momy = FixedMul((fixed_t)(FRACUNIT * 1.3), finesine[angle]);
    puff->momz = 0;
    puff = P_SpawnMobj(actor->x, actor->y, actor->z, HERETIC_MT_PHOENIXPUFF);
    angle = actor->angle - ANG90;
    angle >>= ANGLETOFINESHIFT;
    puff->momx = FixedMul((fixed_t)(FRACUNIT * 1.3), finecosine[angle]);
    puff->momy = FixedMul((fixed_t)(FRACUNIT * 1.3), finesine[angle]);
    puff->momz = 0;
}

//
// This function was present in the Heretic 1.0 executable for the
// removed "secondary phoenix flash" object (MT_PHOENIXFX_REMOVED).
// The purpose of this object is unknown, as is this function.
//

void A_RemovedPhoenixFunc(mobj_t *actor)
{
    return;
}

void A_InitPhoenixPL2(player_t * player, pspdef_t * psp)
{
    player->flamecount = FLAME_THROWER_TICS;
}

void A_FirePhoenixPL2(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;
    mobj_t *pmo;
    angle_t angle;
    fixed_t x, y, z;
    fixed_t slope;

    if (--player->flamecount == 0)
    {                           // Out of flame
        P_SetPsprite(player, ps_weapon, HERETIC_S_PHOENIXATK2_4);
        player->refire = 0;
        return;
    }
    pmo = player->mo;
    angle = pmo->angle;
    x = pmo->x + (P_SubRandom() << 9);
    y = pmo->y + (P_SubRandom() << 9);
    z = pmo->z + 26 * FRACUNIT + ((player->lookdir) << FRACBITS) / 173;
    if (pmo->flags2 & MF2_FEETARECLIPPED)
    {
        z -= FOOTCLIPSIZE;
    }
    slope = ((player->lookdir) << FRACBITS) / 173 + (FRACUNIT / 10);
    mo = P_SpawnMobj(x, y, z, HERETIC_MT_PHOENIXFX2);
    P_SetTarget(&mo->target, pmo);
    mo->angle = angle;
    mo->momx = pmo->momx + FixedMul(mo->info->speed,
                                    finecosine[angle >> ANGLETOFINESHIFT]);
    mo->momy = pmo->momy + FixedMul(mo->info->speed,
                                    finesine[angle >> ANGLETOFINESHIFT]);
    mo->momz = FixedMul(mo->info->speed, slope);
    if (!player->refire || !(leveltime % 38))
    {
        S_StartSound(player->mo, heretic_sfx_phopow);
    }
    P_CheckMissileSpawn(mo);
}

void A_ShutdownPhoenixPL2(player_t * player, pspdef_t * psp)
{
    player->ammo[am_phoenixrod] -= USE_PHRD_AMMO_2;
}

void A_FlameEnd(mobj_t * actor)
{
    actor->momz += (fixed_t)(1.5 * FRACUNIT);
}

void A_FloatPuff(mobj_t * puff)
{
    puff->momz += (fixed_t)(1.8 * FRACUNIT);
}

void A_GauntletAttack(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    int damage;
    int slope;
    int randVal;
    fixed_t dist;

    psp->sx = ((P_Random(pr_heretic) & 3) - 2) * FRACUNIT;
    psp->sy = WEAPONTOP + (P_Random(pr_heretic) & 3) * FRACUNIT;
    angle = player->mo->angle;
    if (player->powers[pw_weaponlevel2])
    {
        damage = HITDICE(2);
        dist = 4 * MELEERANGE;
        angle += P_SubRandom() << 17;
        PuffType = HERETIC_MT_GAUNTLETPUFF2;
    }
    else
    {
        damage = HITDICE(2);
        dist = MELEERANGE + 1;
        angle += P_SubRandom() << 18;
        PuffType = HERETIC_MT_GAUNTLETPUFF1;
    }
    slope = P_AimLineAttack(player->mo, angle, dist, 0);
    P_LineAttack(player->mo, angle, dist, slope, damage);
    if (!linetarget)
    {
        if (P_Random(pr_heretic) > 64)
        {
            player->extralight = !player->extralight;
        }
        S_StartSound(player->mo, heretic_sfx_gntful);
        return;
    }
    randVal = P_Random(pr_heretic);
    if (randVal < 64)
    {
        player->extralight = 0;
    }
    else if (randVal < 160)
    {
        player->extralight = 1;
    }
    else
    {
        player->extralight = 2;
    }
    if (player->powers[pw_weaponlevel2])
    {
        P_GiveBody(player, damage >> 1);
        S_StartSound(player->mo, heretic_sfx_gntpow);
    }
    else
    {
        S_StartSound(player->mo, heretic_sfx_gnthit);
    }
    // turn to face target
    angle = R_PointToAngle2(player->mo->x, player->mo->y,
                            linetarget->x, linetarget->y);
    if (angle - player->mo->angle > ANG180)
    {
        if (angle - player->mo->angle < -ANG90 / 20)
            player->mo->angle = angle + ANG90 / 21;
        else
            player->mo->angle -= ANG90 / 20;
    }
    else
    {
        if (angle - player->mo->angle > ANG90 / 20)
            player->mo->angle = angle - ANG90 / 21;
        else
            player->mo->angle += ANG90 / 20;
    }
    player->mo->flags |= MF_JUSTATTACKED;
    R_SmoothPlaying_Reset(player); // e6y
}

void P_RepositionMace(mobj_t * mo)
{
    int spot;
    subsector_t *ss;

    P_UnsetThingPosition(mo);
    spot = P_Random(pr_heretic) % MaceSpotCount;
    mo->x = MaceSpots[spot].x;
    mo->y = MaceSpots[spot].y;
    ss = R_PointInSubsector(mo->x, mo->y);
    mo->z = mo->floorz = ss->sector->floorheight;
    mo->ceilingz = ss->sector->ceilingheight;
    P_SetThingPosition(mo);
}

void P_ActivateBeak(player_t * player)
{
    player->pendingweapon = wp_nochange;
    player->readyweapon = wp_beak;
    player->psprites[ps_weapon].sy = WEAPONTOP;
    P_SetPsprite(player, ps_weapon, HERETIC_S_BEAKREADY);
}

void P_PostChickenWeapon(player_t * player, weapontype_t weapon)
{
    if (weapon == wp_beak)
    {                           // Should never happen
        weapon = wp_staff;
    }
    player->pendingweapon = wp_nochange;
    player->readyweapon = weapon;
    player->psprites[ps_weapon].sy = WEAPONBOTTOM;
    P_SetPsprite(player, ps_weapon, wpnlev1info[weapon].upstate);
}

void P_UpdateBeak(player_t * player, pspdef_t * psp)
{
    psp->sy = WEAPONTOP + (player->chickenPeck << (FRACBITS - 1));
}

dboolean Heretic_P_CheckAmmo(player_t * player)
{
    ammotype_t ammo;
    weaponinfo_t *checkweaponinfo;
    int count;

    ammo = wpnlev1info[player->readyweapon].ammo;
    if (player->powers[pw_weaponlevel2] && !deathmatch)
    {
      checkweaponinfo = wpnlev2info;
    }
    else
    {
      checkweaponinfo = wpnlev1info;
    }
    count = checkweaponinfo[player->readyweapon].ammopershot;
    if (ammo == am_noammo || player->ammo[ammo] >= count)
    {
        return (true);
    }
    // out of ammo, pick a weapon to change to
    do
    {
        if (player->weaponowned[wp_skullrod]
            && player->ammo[am_skullrod] > checkweaponinfo[wp_skullrod].ammopershot)
        {
            player->pendingweapon = wp_skullrod;
        }
        else if (player->weaponowned[wp_blaster]
                 && player->ammo[am_blaster] > checkweaponinfo[wp_blaster].ammopershot)
        {
            player->pendingweapon = wp_blaster;
        }
        else if (player->weaponowned[wp_crossbow]
                 && player->ammo[am_crossbow] > checkweaponinfo[wp_crossbow].ammopershot)
        {
            player->pendingweapon = wp_crossbow;
        }
        else if (player->weaponowned[wp_mace]
                 && player->ammo[am_mace] > checkweaponinfo[wp_mace].ammopershot)
        {
            player->pendingweapon = wp_mace;
        }
        else if (player->ammo[am_goldwand] > checkweaponinfo[wp_goldwand].ammopershot)
        {
            player->pendingweapon = wp_goldwand;
        }
        else if (player->weaponowned[wp_gauntlets])
        {
            player->pendingweapon = wp_gauntlets;
        }
        else if (player->weaponowned[wp_phoenixrod]
                 && player->ammo[am_phoenixrod] > checkweaponinfo[wp_phoenixrod].ammopershot)
        {
            player->pendingweapon = wp_phoenixrod;
        }
        else
        {
            player->pendingweapon = wp_staff;
        }
    }
    while (player->pendingweapon == wp_nochange);
    if (player->powers[pw_weaponlevel2])
    {
        P_SetPsprite(player, ps_weapon,
                     wpnlev2info[player->readyweapon].downstate);
    }
    else
    {
        P_SetPsprite(player, ps_weapon,
                     wpnlev1info[player->readyweapon].downstate);
    }
    return (false);
}

void P_OpenWeapons(void)
{
    MaceSpotCount = 0;
}

void P_AddMaceSpot(const mapthing_t * mthing)
{
    if (MaceSpotCount == MAX_MACE_SPOTS)
    {
        I_Error("Too many mace spots.");
    }
    MaceSpots[MaceSpotCount].x = mthing->x << FRACBITS;
    MaceSpots[MaceSpotCount].y = mthing->y << FRACBITS;
    MaceSpotCount++;
}

void P_CloseWeapons(void)
{
    int spot;

    if (!MaceSpotCount)
    {                           // No maces placed
        return;
    }
    if (!deathmatch && P_Random(pr_heretic) < 64)
    {                           // Sometimes doesn't show up if not in deathmatch
        return;
    }
    spot = P_Random(pr_heretic) % MaceSpotCount;
    P_SpawnMobj(MaceSpots[spot].x, MaceSpots[spot].y, ONFLOORZ, HERETIC_MT_WMACE);
}
