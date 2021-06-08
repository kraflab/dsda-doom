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
 *      Player related stuff.
 *      Bobbing POV/weapon, movement.
 *      Pending weapon.
 *
 *-----------------------------------------------------------------------------*/

#include "doomstat.h"
#include "d_event.h"
#include "r_main.h"
#include "p_map.h"
#include "p_spec.h"
#include "p_user.h"
#include "r_demo.h"
#include "r_fps.h"
#include "g_game.h"
#include "e6y.h"//e6y

#include "dsda/settings.h"

// heretic needs
#include "heretic/def.h"
#include "s_sound.h"
#include "sounds.h"
#include "p_inter.h"
#include "m_random.h"

//
// Movement.
//

// 16 pixels of bob

#define MAXBOB  0x100000

dboolean onground; // whether player is on ground or in air

// heretic
int newtorch;      // used in the torch flicker effect.
int newtorchdelta;

angle_t P_PlayerPitch(player_t* player)
{
  return player->mo->pitch - (angle_t)(player->lookdir * ANG1 / M_PI);
}

//
// P_Thrust
// Moves the given origin along a given angle.
//

void P_SideThrust(player_t *player, angle_t angle, fixed_t move)
{
  angle >>= ANGLETOFINESHIFT;

  player->mo->momx += FixedMul(move,finecosine[angle]);
  player->mo->momy += FixedMul(move,finesine[angle]);
}

void P_Thrust(player_t* player,angle_t angle,fixed_t move)
{
  angle >>= ANGLETOFINESHIFT;

  if ((player->mo->flags & MF_FLY) && player->mo->pitch != 0)
  {
    angle_t pitch = player->mo->pitch >> ANGLETOFINESHIFT;
    fixed_t zpush = FixedMul(move, finesine[pitch]);
    player->mo->momz -= zpush;
    move = FixedMul(move, finecosine[pitch]);
  }

  if (player->powers[pw_flight] && !(player->mo->z <= player->mo->floorz))
  {
      player->mo->momx += FixedMul(move, finecosine[angle]);
      player->mo->momy += FixedMul(move, finesine[angle]);
  }
  else if (player->mo->subsector->sector->special == g_special_friction_low)
  {
      player->mo->momx += FixedMul(move >> 2, finecosine[angle]);
      player->mo->momy += FixedMul(move >> 2, finesine[angle]);
  }
  else
  {
      player->mo->momx += FixedMul(move, finecosine[angle]);
      player->mo->momy += FixedMul(move, finesine[angle]);
  }
}


/*
 * P_Bob
 * Same as P_Thrust, but only affects bobbing.
 *
 * killough 10/98: We apply thrust separately between the real physical player
 * and the part which affects bobbing. This way, bobbing only comes from player
 * motion, nothing external, avoiding many problems, e.g. bobbing should not
 * occur on conveyors, unless the player walks on one, and bobbing should be
 * reduced at a regular rate, even on ice (where the player coasts).
 */

static void P_Bob(player_t *player, angle_t angle, fixed_t move)
{
  //e6y
  if (!mbf_features && !prboom_comp[PC_PRBOOM_FRICTION].state)
    return;

  player->momx += FixedMul(move,finecosine[angle >>= ANGLETOFINESHIFT]);
  player->momy += FixedMul(move,finesine[angle]);
}

//
// P_CalcHeight
// Calculate the walking / running height adjustment
//

void P_CalcHeight (player_t* player)
{
  int     angle;
  fixed_t bob;

  // Regular movement bobbing
  // (needs to be calculated for gun swing
  // even if not on ground)
  // OPTIMIZE: tablify angle
  // Note: a LUT allows for effects
  //  like a ramp with low health.


  /* killough 10/98: Make bobbing depend only on player-applied motion.
   *
   * Note: don't reduce bobbing here if on ice: if you reduce bobbing here,
   * it causes bobbing jerkiness when the player moves from ice to non-ice,
   * and vice-versa.
   */

  player->bob = 0;

  if ((player->mo->flags & MF_FLY) && !onground)
  {
    player->bob = FRACUNIT / 2;
  }

  if (mbf_features)
  {
    if (player_bobbing)
    {
      player->bob = (FixedMul(player->momx, player->momx) +
                     FixedMul(player->momy, player->momy)) >> 2;
    }
  }
  else
  {
    if (demo_compatibility || player_bobbing || prboom_comp[PC_FORCE_INCORRECT_BOBBING_IN_BOOM].state)
    {
      player->bob = (FixedMul(player->mo->momx, player->mo->momx) +
        FixedMul(player->mo->momy, player->mo->momy)) >> 2;
    }
  }

  //e6y
  if (!prboom_comp[PC_PRBOOM_FRICTION].state &&
      compatibility_level >= boom_202_compatibility &&
      compatibility_level <= lxdoom_1_compatibility &&
      player->mo->friction > ORIG_FRICTION) // ice?
  {
    if (player->bob > (MAXBOB >> 2))
      player->bob = MAXBOB >> 2;
  }
  else
  {
    if (player->bob > MAXBOB)
      player->bob = MAXBOB;
  }

  if (player->mo->flags2 & MF2_FLY && !onground)
  {
    player->bob = FRACUNIT / 2;
  }

  if ((!onground && !heretic) || player->cheats & CF_NOMOMENTUM)
  {
    player->viewz = player->mo->z + VIEWHEIGHT;

    if (player->viewz > player->mo->ceilingz - 4 * FRACUNIT)
      player->viewz = player->mo->ceilingz - 4 * FRACUNIT;

    return;
  }

  angle = (FINEANGLES / 20 * leveltime) & FINEMASK;
  bob = FixedMul(player->bob / 2, finesine[angle]);

  // move viewheight

  if (player->playerstate == PST_LIVE)
  {
    player->viewheight += player->deltaviewheight;

    if (player->viewheight > VIEWHEIGHT)
    {
      player->viewheight = VIEWHEIGHT;
      player->deltaviewheight = 0;
    }

    if (player->viewheight < VIEWHEIGHT / 2)
    {
      player->viewheight = VIEWHEIGHT / 2;
      if (player->deltaviewheight <= 0)
        player->deltaviewheight = 1;
    }

    if (player->deltaviewheight)
    {
      player->deltaviewheight += FRACUNIT / 4;
      if (!player->deltaviewheight)
        player->deltaviewheight = 1;
    }
  }

  if (player->chickenTics)
  {
    player->viewz = player->mo->z + player->viewheight - (20 * FRACUNIT);
  }
  else
  {
    player->viewz = player->mo->z + player->viewheight + bob;
  }

  if (player->mo->flags2 & MF2_FEETARECLIPPED
      && player->playerstate != PST_DEAD
      && player->mo->z <= player->mo->floorz)
  {
    player->viewz -= FOOTCLIPSIZE;
  }

  if (player->viewz > player->mo->ceilingz - 4 * FRACUNIT)
    player->viewz = player->mo->ceilingz - 4 * FRACUNIT;

  if (heretic && player->viewz < player->mo->floorz + 4 * FRACUNIT)
    player->viewz = player->mo->floorz + 4 * FRACUNIT;
}

//
// P_SetPitch
// Mouse Look Stuff
//
void P_SetPitch(player_t *player)
{
  mobj_t *mo = player->mo;

  if (player == &players[consoleplayer])
  {
    if (!(demoplayback || democontinue))
    {
      if (GetMouseLook())
      {
        if (!mo->reactiontime && (!(automapmode & am_active) || (automapmode & am_overlay)))
        {
          mo->pitch += (mlooky << 16);
          CheckPitch((signed int *)&mo->pitch);
        }
      }
      else
      {
        mo->pitch = 0;
      }

      R_DemoEx_WriteMLook(mo->pitch);
    }
    else
    {
      mo->pitch = R_DemoEx_ReadMLook();
      CheckPitch((signed int *)&mo->pitch);
    }
  }
  else
  {
    mo->pitch = 0;
  }
}

//
// P_MovePlayer
//
// Adds momentum if the player is not in the air
//
// killough 10/98: simplified

void P_MovePlayer (player_t* player)
{
  ticcmd_t *cmd;
  mobj_t *mo;

  if (heretic) return Heretic_P_MovePlayer(player);

  cmd = &player->cmd;
  mo = player->mo;
  mo->angle += cmd->angleturn << 16;

  if (demo_smoothturns && player == &players[displayplayer])
  {
    R_SmoothPlaying_Add(cmd->angleturn << 16);
  }

  onground = mo->z <= mo->floorz;

  if ((player->mo->flags & MF_FLY) && player == &players[consoleplayer] && upmove != 0)
  {
    mo->momz = upmove << 8;
  }

  if (comperr(comperr_allowjump))
  {
    if (upmove > 0 && onground && player == &players[consoleplayer] && !(player->mo->flags & MF_FLY))
    {
      if (!player->jumpTics)
      {
        mo->momz = (7 + default_comperr[comperr_allowjump]) * FRACUNIT;
        player->jumpTics = 18;
      }
    }
  }

  // killough 10/98:
  //
  // We must apply thrust to the player and bobbing separately, to avoid
  // anomalies. The thrust applied to bobbing is always the same strength on
  // ice, because the player still "works just as hard" to move, while the
  // thrust applied to the movement varies with 'movefactor'.

  //e6y
  if ((!demo_compatibility && !mbf_features && !prboom_comp[PC_PRBOOM_FRICTION].state) ||
    (cmd->forwardmove | cmd->sidemove)) // killough 10/98
    {
      if (onground || mo->flags & MF_BOUNCES || (mo->flags & MF_FLY)) // killough 8/9/98
      {
        int friction, movefactor = P_GetMoveFactor(mo, &friction);

        // killough 11/98:
        // On sludge, make bobbing depend on efficiency.
        // On ice, make it depend on effort.

        int bobfactor =
          friction < ORIG_FRICTION ? movefactor : ORIG_FRICTION_FACTOR;

        if (cmd->forwardmove)
        {
          P_Bob(player,mo->angle,cmd->forwardmove*bobfactor);
          P_Thrust(player,mo->angle,cmd->forwardmove*movefactor);
        }

        if (cmd->sidemove)
        {
          P_Bob(player,mo->angle-ANG90,cmd->sidemove*bobfactor);
          P_SideThrust(player,mo->angle-ANG90,cmd->sidemove*movefactor);
        }
      }
      else if (comperr(comperr_allowjump))
      {
        if (!onground)
        {
          if (cmd->forwardmove)
          {
            P_Thrust(player, mo->angle, FRACUNIT >> 8);
          }
          if (cmd->sidemove)
          {
            P_Thrust(player, mo->angle, FRACUNIT >> 8);
          }
        }
      }
      if (mo->state == states+S_PLAY)
        P_SetMobjState(mo,S_PLAY_RUN1);
    }
}

#define ANG5 (ANG90/18)

//
// P_DeathThink
// Fall on your face when dying.
// Decrease POV height to floor height.
//

void P_DeathThink (player_t* player)
{
  angle_t angle;
  angle_t delta;

  P_MovePsprites(player);

  // fall to the ground

  onground = (player->mo->z <= player->mo->floorz);
  if (player->mo->type == g_skullpop_mt)
  {
    // Flying bloody skull
    player->viewheight = 6*FRACUNIT;
    player->deltaviewheight = 0;
    if (onground)
    {
      if (heretic && player->lookdir < 60)
      {
        int lookDelta;

        lookDelta = (60 - player->lookdir) / 8;
        if (lookDelta < 1 && (leveltime & 1))
        {
            lookDelta = 1;
        }
        else if (lookDelta > 6)
        {
            lookDelta = 6;
        }
        player->lookdir += lookDelta;
      }
      else if (!heretic && (int)player->mo->pitch > -(int)ANG1*19)
      {
        player->mo->pitch -= ((int)ANG1*19 - player->mo->pitch) / 8;
      }
    }
  }
  else
  {
    if (player->viewheight > 6*FRACUNIT)
      player->viewheight -= FRACUNIT;

    if (player->viewheight < 6*FRACUNIT)
      player->viewheight = 6*FRACUNIT;

    player->deltaviewheight = 0;

    if (player->lookdir > 0)
    {
        player->lookdir -= 6;
    }
    else if (player->lookdir < 0)
    {
        player->lookdir += 6;
    }
    if (abs(player->lookdir) < 6)
    {
        player->lookdir = 0;
    }
  }

  P_CalcHeight(player);

  if (player->attacker && player->attacker != player->mo)
  {
    angle = R_PointToAngle2(player->mo->x,
                            player->mo->y,
                            player->attacker->x,
                            player->attacker->y);

    delta = angle - player->mo->angle;

    if (delta < ANG5 || delta > (unsigned)-ANG5)
    {
      // Looking at killer,
      //  so fade damage flash down.

      player->mo->angle = angle;

      if (player->damagecount)
        player->damagecount--;
    }
    else if (delta < ANG180)
      player->mo->angle += ANG5;
    else
      player->mo->angle -= ANG5;
  }
  else if (player->damagecount)
    player->damagecount--;

  if (player->cmd.buttons & BT_USE)
  {
    if (heretic)
    {
      if (player == &players[consoleplayer])
      {
        V_SetPalette(0);
        inv_ptr = 0;
        curpos = 0;
        newtorch = 0;
        newtorchdelta = 0;
      }
      // Let the mobj know the player has entered the reborn state.  Some
      // mobjs need to know when it's ok to remove themselves.
      player->mo->special2.i = 666;
    }

    player->playerstate = PST_REBORN;
  }

  R_SmoothPlaying_Reset(player); // e6y
}

//
// P_PlayerThink
//

void P_PlayerThink (player_t* player)
{
  ticcmd_t*    cmd;
  weapontype_t newweapon;

  if (movement_smooth)
  {
    player->prev_viewz = player->viewz;
    player->prev_viewangle = R_SmoothPlaying_Get(player) + viewangleoffset;
    player->prev_viewpitch = P_PlayerPitch(player) + viewpitchoffset;

    if (&players[displayplayer] == player)
    {
      P_ResetWalkcam();
    }
  }

  // killough 2/8/98, 3/21/98:
  if (player->cheats & CF_NOCLIP)
    player->mo->flags |= MF_NOCLIP;
  else
    player->mo->flags &= ~MF_NOCLIP;

  // chain saw run forward

  cmd = &player->cmd;
  if (player->mo->flags & MF_JUSTATTACKED)
  {
    cmd->angleturn = 0;
    cmd->forwardmove = 0xc800 / 512;
    cmd->sidemove = 0;
    player->mo->flags &= ~MF_JUSTATTACKED;
  }

  if (player->playerstate == PST_DEAD)
  {
    P_DeathThink(player);
    return;
  }

  if (player->chickenTics)
  {
      P_ChickenPlayerThink(player);
  }

  if (player->jumpTics)
  {
      player->jumpTics--;
  }
  // Move around.
  // Reactiontime is used to prevent movement
  //  for a bit after a teleport.

  if (player->mo->reactiontime)
    player->mo->reactiontime--;
  else
    P_MovePlayer(player);

  P_SetPitch(player);

  P_CalcHeight (player); // Determines view height and bobbing

  // Determine if there's anything about the sector you're in that's
  // going to affect you, like painful floors.

  if (player->mo->subsector->sector->special)
    P_PlayerInSpecialSector(player);

  if (cmd->arti)
  {                           // Use an artifact
    if (cmd->arti == 0xff)
    {
      P_PlayerNextArtifact(player);
    }
    else
    {
      P_PlayerUseArtifact(player, cmd->arti);
    }
  }

  // heretic_note: is this needed or a defense mechanism?
  // if (cmd->buttons & BT_SPECIAL)
  // {                           // A special event has no other buttons
  //     cmd->buttons = 0;
  // }

  // Check for weapon change.
  if (cmd->buttons & BT_CHANGE)
  {
    // The actual changing of the weapon is done
    //  when the weapon psprite can do it
    //  (read: not in the middle of an attack).

    newweapon = (cmd->buttons & BT_WEAPONMASK) >> BT_WEAPONSHIFT;

    // killough 3/22/98: For demo compatibility we must perform the fist
    // and SSG weapons switches here, rather than in G_BuildTiccmd(). For
    // other games which rely on user preferences, we must use the latter.

    if (demo_compatibility)
    { // compatibility mode -- required for old demos -- killough
      //e6y
      if (!prboom_comp[PC_ALLOW_SSG_DIRECT].state)
        newweapon = (cmd->buttons & BT_WEAPONMASK_OLD)>>BT_WEAPONSHIFT;

      if (
        newweapon == g_wp_fist && player->weaponowned[g_wp_chainsaw]
        && (
          player->readyweapon != g_wp_chainsaw ||
          (!heretic && !player->powers[pw_strength])
        )
      )
        newweapon = g_wp_chainsaw;

      if (!heretic &&
          gamemode == commercial &&
          newweapon == wp_shotgun &&
          player->weaponowned[wp_supershotgun] &&
          player->readyweapon != wp_supershotgun)
        newweapon = wp_supershotgun;
    }

    // killough 2/8/98, 3/22/98 -- end of weapon selection changes

    if (player->weaponowned[newweapon] && newweapon != player->readyweapon)

      // Do not go to plasma or BFG in shareware,
      //  even if cheated.

      // heretic_note: ignoring this...not sure it's worth worrying about
      if ((newweapon != wp_plasma && newweapon != wp_bfg)
          || (gamemode != shareware) )
        player->pendingweapon = newweapon;
  }

  // check for use

  if (cmd->buttons & BT_USE)
  {
    if (!player->usedown)
    {
      P_UseLines(player);
      player->usedown = true;
    }
  }
  else
    player->usedown = false;

  // Chicken counter
  if (player->chickenTics)
  {
    if (player->chickenPeck)
    {                       // Chicken attack counter
      player->chickenPeck -= 3;
    }
    if (!--player->chickenTics)
    {                       // Attempt to undo the chicken
      P_UndoPlayerChicken(player);
    }
  }

  // cycle psprites
  P_MovePsprites (player);

  // Counters, time dependent power ups.

  // Strength counts up to diminish fade.

  if (player->powers[pw_strength])
    player->powers[pw_strength]++;

  // killough 1/98: Make idbeholdx toggle:

  if (player->powers[pw_invulnerability] > 0) // killough
    player->powers[pw_invulnerability]--;

  if (player->powers[pw_invisibility] > 0)    // killough
    if (! --player->powers[pw_invisibility] )
      player->mo->flags &= ~MF_SHADOW;

  if (player->powers[pw_infrared] > 0)        // killough
    player->powers[pw_infrared]--;

  if (player->powers[pw_ironfeet] > 0)        // killough
    player->powers[pw_ironfeet]--;

  if (player->powers[pw_flight])
  {
    if (!--player->powers[pw_flight])
    {
      // haleyjd: removed externdriver crap
      if (player->mo->z != player->mo->floorz)
      {
          player->centering = true;
      }

      player->mo->flags2 &= ~MF2_FLY;
      player->mo->flags &= ~MF_NOGRAVITY;
    }
  }
  if (player->powers[pw_weaponlevel2])
  {
    if (!--player->powers[pw_weaponlevel2])
    {
      if ((player->readyweapon == wp_phoenixrod)
          && (player->psprites[ps_weapon].state
              != &states[HERETIC_S_PHOENIXREADY])
          && (player->psprites[ps_weapon].state
              != &states[HERETIC_S_PHOENIXUP]))
      {
        P_SetPsprite(player, ps_weapon, HERETIC_S_PHOENIXREADY);
        player->ammo[am_phoenixrod] -= USE_PHRD_AMMO_2;
        player->refire = 0;
      }
      else if ((player->readyweapon == wp_gauntlets)
               || (player->readyweapon == wp_staff))
      {
        player->pendingweapon = player->readyweapon;
      }
    }
  }

  if (player->damagecount)
    player->damagecount--;

  if (player->bonuscount)
    player->bonuscount--;

  // Handling colormaps.
  // killough 3/20/98: reformat to terse C syntax
  if (!heretic)
    player->fixedcolormap = dsda_PowerPalette() &&
      (player->powers[pw_invulnerability] > 4*32 ||
      player->powers[pw_invulnerability] & 8) ? INVERSECOLORMAP :
      player->powers[pw_infrared] > 4*32 || player->powers[pw_infrared] & 8;
  else
  {
    if (player->powers[pw_invulnerability])
    {
      if (player->powers[pw_invulnerability] > BLINKTHRESHOLD
          || (player->powers[pw_invulnerability] & 8))
      {
        player->fixedcolormap = INVERSECOLORMAP;
      }
      else
      {
        player->fixedcolormap = 0;
      }
    }
    else if (player->powers[pw_infrared])
    {
      if (player->powers[pw_infrared] <= BLINKTHRESHOLD)
      {
        if (player->powers[pw_infrared] & 8)
        {
          player->fixedcolormap = 0;
        }
        else
        {
          player->fixedcolormap = 1;
        }
      }
      else if (!(leveltime & 16) && player == &players[consoleplayer])
      {
        if (newtorch)
        {
          if (player->fixedcolormap + newtorchdelta > 7
              || player->fixedcolormap + newtorchdelta < 1
              || newtorch == player->fixedcolormap)
          {
            newtorch = 0;
          }
          else
          {
            player->fixedcolormap += newtorchdelta;
          }
        }
        else
        {
          newtorch = (M_Random() & 7) + 1;
          newtorchdelta = (newtorch == player->fixedcolormap) ?
              0 : ((newtorch > player->fixedcolormap) ? 1 : -1);
        }
      }
    }
    else
    {
      player->fixedcolormap = 0;
    }
  }
}

// heretic

#include "p_tick.h"

void P_PlayerNextArtifact(player_t * player);

int P_GetPlayerNum(player_t * player)
{
    int i;

    for (i = 0; i < MAXPLAYERS; i++)
    {
        if (player == &players[i])
        {
            return (i);
        }
    }
    return (0);
}

dboolean P_UndoPlayerChicken(player_t * player)
{
    mobj_t *fog;
    mobj_t *mo;
    mobj_t *pmo;
    fixed_t x;
    fixed_t y;
    fixed_t z;
    angle_t angle;
    int playerNum;
    weapontype_t weapon;
    int oldFlags;
    int oldFlags2;

    pmo = player->mo;
    x = pmo->x;
    y = pmo->y;
    z = pmo->z;
    angle = pmo->angle;
    weapon = pmo->special1.i;
    oldFlags = pmo->flags;
    oldFlags2 = pmo->flags2;
    P_SetMobjState(pmo, HERETIC_S_FREETARGMOBJ);
    mo = P_SpawnMobj(x, y, z, g_mt_player);
    if (P_TestMobjLocation(mo) == false)
    {                           // Didn't fit
        P_RemoveMobj(mo);
        mo = P_SpawnMobj(x, y, z, HERETIC_MT_CHICPLAYER);
        mo->angle = angle;
        mo->health = player->health;
        mo->special1.i = weapon;
        mo->player = player;
        mo->flags = oldFlags;
        mo->flags2 = oldFlags2;
        player->mo = mo;
        player->chickenTics = 2 * 35;
        return (false);
    }
    playerNum = P_GetPlayerNum(player);
    if (playerNum != 0)
    {                           // Set color translation
        mo->flags |= playerNum << MF_TRANSSHIFT;
    }
    mo->angle = angle;
    mo->player = player;
    mo->reactiontime = 18;
    if (oldFlags2 & MF2_FLY)
    {
        mo->flags2 |= MF2_FLY;
        mo->flags |= MF_NOGRAVITY;
    }
    player->chickenTics = 0;
    player->powers[pw_weaponlevel2] = 0;
    player->health = mo->health = MAXHEALTH;
    player->mo = mo;
    angle >>= ANGLETOFINESHIFT;
    fog = P_SpawnMobj(x + 20 * finecosine[angle],
                      y + 20 * finesine[angle], z + TELEFOGHEIGHT, HERETIC_MT_TFOG);
    S_StartSound(fog, heretic_sfx_telept);
    P_PostChickenWeapon(player, weapon);
    return (true);
}

void P_ArtiTele(player_t * player)
{
    int i;
    int selections;
    fixed_t destX;
    fixed_t destY;
    angle_t destAngle;

    if (deathmatch)
    {
        selections = deathmatch_p - deathmatchstarts;
        i = P_Random(pr_heretic) % selections;
        destX = deathmatchstarts[i].x << FRACBITS;
        destY = deathmatchstarts[i].y << FRACBITS;
        destAngle = ANG45 * (deathmatchstarts[i].angle / 45);
    }
    else
    {
        destX = playerstarts[0].x << FRACBITS;
        destY = playerstarts[0].y << FRACBITS;
        destAngle = ANG45 * (playerstarts[0].angle / 45);
    }
    P_Teleport(player->mo, destX, destY, destAngle);
    S_StartSound(NULL, heretic_sfx_wpnup);      // Full volume laugh
}

void P_PlayerNextArtifact(player_t * player)
{
    if (player == &players[consoleplayer])
    {
        inv_ptr--;
        if (inv_ptr < 6)
        {
            curpos--;
            if (curpos < 0)
            {
                curpos = 0;
            }
        }
        if (inv_ptr < 0)
        {
            inv_ptr = player->inventorySlotNum - 1;
            if (inv_ptr < 6)
            {
                curpos = inv_ptr;
            }
            else
            {
                curpos = 6;
            }
        }
        player->readyArtifact = player->inventory[inv_ptr].type;
    }
}

void P_PlayerRemoveArtifact(player_t * player, int slot)
{
    int i;
    player->artifactCount--;
    if (!(--player->inventory[slot].count))
    {                           // Used last of a type - compact the artifact list
        player->readyArtifact = arti_none;
        player->inventory[slot].type = arti_none;
        for (i = slot + 1; i < player->inventorySlotNum; i++)
        {
            player->inventory[i - 1] = player->inventory[i];
        }
        player->inventorySlotNum--;
        if (player == &players[consoleplayer])
        {                       // Set position markers and get next readyArtifact
            inv_ptr--;
            if (inv_ptr < 6)
            {
                curpos--;
                if (curpos < 0)
                {
                    curpos = 0;
                }
            }
            if (inv_ptr >= player->inventorySlotNum)
            {
                inv_ptr = player->inventorySlotNum - 1;
            }
            if (inv_ptr < 0)
            {
                inv_ptr = 0;
            }
            player->readyArtifact = player->inventory[inv_ptr].type;
        }
    }
}

void P_PlayerUseArtifact(player_t * player, artitype_t arti)
{
    int i;

    for (i = 0; i < player->inventorySlotNum; i++)
    {
        if (player->inventory[i].type == arti)
        {                       // Found match - try to use
            if (P_UseArtifact(player, arti))
            {                   // Artifact was used - remove it from inventory
                P_PlayerRemoveArtifact(player, i);
                if (player == &players[consoleplayer])
                {
                    S_StartSound(NULL, heretic_sfx_artiuse);
                    ArtifactFlash = 4;
                }
            }
            else
            {                   // Unable to use artifact, advance pointer
                P_PlayerNextArtifact(player);
            }
            break;
        }
    }
}

dboolean P_UseArtifact(player_t * player, artitype_t arti)
{
    mobj_t *mo;
    angle_t angle;

    switch (arti)
    {
        case arti_invulnerability:
            if (!P_GivePower(player, pw_invulnerability))
            {
                return (false);
            }
            break;
        case arti_invisibility:
            if (!P_GivePower(player, pw_invisibility))
            {
                return (false);
            }
            break;
        case arti_health:
            if (!P_GiveBody(player, 25))
            {
                return (false);
            }
            break;
        case arti_superhealth:
            if (!P_GiveBody(player, 100))
            {
                return (false);
            }
            break;
        case arti_tomeofpower:
            if (player->chickenTics)
            {                   // Attempt to undo chicken
                if (P_UndoPlayerChicken(player) == false)
                {               // Failed
                    P_DamageMobj(player->mo, NULL, NULL, 10000);
                }
                else
                {               // Succeeded
                    player->chickenTics = 0;
                    S_StartSound(player->mo, heretic_sfx_wpnup);
                }
            }
            else
            {
                if (!P_GivePower(player, pw_weaponlevel2))
                {
                    return (false);
                }
                if (player->readyweapon == wp_staff)
                {
                    P_SetPsprite(player, ps_weapon, HERETIC_S_STAFFREADY2_1);
                }
                else if (player->readyweapon == wp_gauntlets)
                {
                    P_SetPsprite(player, ps_weapon, HERETIC_S_GAUNTLETREADY2_1);
                }
            }
            break;
        case arti_torch:
            if (!P_GivePower(player, pw_infrared))
            {
                return (false);
            }
            break;
        case arti_firebomb:
            angle = player->mo->angle >> ANGLETOFINESHIFT;

            // Vanilla bug here:
            // Original code here looks like:
            //   (player->mo->flags2 & MF2_FEETARECLIPPED != 0),
            // Which under C's operator precedence is:
            //   (player->mo->flags2 & (MF2_FEETARECLIPPED != 0)),
            // Which simplifies to:
            //   (player->mo->flags2 & 1),
            mo = P_SpawnMobj(player->mo->x + 24 * finecosine[angle],
                             player->mo->y + 24 * finesine[angle],
                             player->mo->z -
                             15 * FRACUNIT * (player->mo->flags2 & 1),
                             HERETIC_MT_FIREBOMB);
            P_SetTarget(&mo->target, player->mo);
            break;
        case arti_egg:
            mo = player->mo;
            P_SpawnPlayerMissile(mo, HERETIC_MT_EGGFX);
            P_SPMAngle(mo, HERETIC_MT_EGGFX, mo->angle - (ANG45 / 6));
            P_SPMAngle(mo, HERETIC_MT_EGGFX, mo->angle + (ANG45 / 6));
            P_SPMAngle(mo, HERETIC_MT_EGGFX, mo->angle - (ANG45 / 3));
            P_SPMAngle(mo, HERETIC_MT_EGGFX, mo->angle + (ANG45 / 3));
            break;
        case arti_fly:
            if (!P_GivePower(player, pw_flight))
            {
                return (false);
            }
            break;
        case arti_teleport:
            P_ArtiTele(player);
            break;
        default:
            return (false);
    }
    return (true);
}

void Heretic_P_MovePlayer(player_t * player)
{
    int look;
    int fly;
    ticcmd_t *cmd;

    cmd = &player->cmd;
    player->mo->angle += (cmd->angleturn << 16);

    if (demo_smoothturns && player == &players[displayplayer])
    {
      R_SmoothPlaying_Add(cmd->angleturn << 16);
    }

    onground = (player->mo->z <= player->mo->floorz
                || (player->mo->flags2 & MF2_ONMOBJ));

    if (player->chickenTics)
    {                           // Chicken speed
        if (cmd->forwardmove && (onground || player->mo->flags2 & MF2_FLY))
            P_Thrust(player, player->mo->angle, cmd->forwardmove * 2500);
        if (cmd->sidemove && (onground || player->mo->flags2 & MF2_FLY))
            P_Thrust(player, player->mo->angle - ANG90, cmd->sidemove * 2500);
    }
    else
    {                           // Normal speed
        if (cmd->forwardmove && (onground || player->mo->flags2 & MF2_FLY))
            P_Thrust(player, player->mo->angle, cmd->forwardmove * 2048);
        if (cmd->sidemove && (onground || player->mo->flags2 & MF2_FLY))
            P_Thrust(player, player->mo->angle - ANG90, cmd->sidemove * 2048);
    }

    if (cmd->forwardmove || cmd->sidemove)
    {
        if (player->chickenTics)
        {
            if (player->mo->state == &states[HERETIC_S_CHICPLAY])
            {
                P_SetMobjState(player->mo, HERETIC_S_CHICPLAY_RUN1);
            }
        }
        else
        {
            if (player->mo->state == &states[HERETIC_S_PLAY])
            {
                P_SetMobjState(player->mo, HERETIC_S_PLAY_RUN1);
            }
        }
    }

    look = cmd->lookfly & 15;
    if (look > 7)
    {
        look -= 16;
    }
    if (look)
    {
        if (look == TOCENTER)
        {
            player->centering = true;
        }
        else
        {
            player->lookdir += 5 * look;
            if (player->lookdir > 90 || player->lookdir < -110)
            {
                player->lookdir -= 5 * look;
            }
        }
    }
    if (player->centering)
    {
        if (player->lookdir > 0)
        {
            player->lookdir -= 8;
        }
        else if (player->lookdir < 0)
        {
            player->lookdir += 8;
        }
        if (abs(player->lookdir) < 8)
        {
            player->lookdir = 0;
            player->centering = false;
        }
    }
    fly = cmd->lookfly >> 4;
    if (fly > 7)
    {
        fly -= 16;
    }
    if (fly && player->powers[pw_flight])
    {
        if (fly != TOCENTER)
        {
            player->flyheight = fly * 2;
            if (!(player->mo->flags2 & MF2_FLY))
            {
                player->mo->flags2 |= MF2_FLY;
                player->mo->flags |= MF_NOGRAVITY;
            }
        }
        else
        {
            player->mo->flags2 &= ~MF2_FLY;
            player->mo->flags &= ~MF_NOGRAVITY;
        }
    }
    else if (fly > 0)
    {
        P_PlayerUseArtifact(player, arti_fly);
    }
    if (player->mo->flags2 & MF2_FLY)
    {
        player->mo->momz = player->flyheight * FRACUNIT;
        if (player->flyheight)
        {
            player->flyheight /= 2;
        }
    }
}

void P_ChickenPlayerThink(player_t * player)
{
    mobj_t *pmo;

    if (player->health > 0)
    {                           // Handle beak movement
        P_UpdateBeak(player, &player->psprites[ps_weapon]);
    }
    if (player->chickenTics & 15)
    {
        return;
    }
    pmo = player->mo;
    if (!(pmo->momx + pmo->momy) && P_Random(pr_heretic) < 160)
    {                           // Twitch view angle
        pmo->angle += P_SubRandom() << 19;
    }
    if ((pmo->z <= pmo->floorz) && (P_Random(pr_heretic) < 32))
    {                           // Jump and noise
        pmo->momz += FRACUNIT;
        P_SetMobjState(pmo, HERETIC_S_CHICPLAY_PAIN);
        return;
    }
    if (P_Random(pr_heretic) < 48)
    {                           // Just noise
        S_StartSound(pmo, heretic_sfx_chicact);
    }
}
