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
 *      Handling interactions (i.e., collisions).
 *
 *-----------------------------------------------------------------------------*/

#include "doomstat.h"
#include "dstrings.h"
#include "m_random.h"
#include "am_map.h"
#include "r_main.h"
#include "s_sound.h"
#include "sounds.h"
#include "d_deh.h"  // Ty 03/22/98 - externalized strings
#include "p_tick.h"
#include "lprintf.h"

#include "p_inter.h"
#include "p_enemy.h"
#include "hu_tracers.h"

#ifdef __GNUG__
#pragma implementation "p_inter.h"
#endif
#include "p_inter.h"
#include "e6y.h"//e6y
#include "dsda.h"

#include "heretic/def.h"
#include "heretic/sb_bar.h"

// Ty 03/07/98 - add deh externals
// Maximums and such were hardcoded values.  Need to externalize those for
// dehacked support (and future flexibility).  Most var names came from the key
// strings used in dehacked.

int initial_health = 100;
int initial_bullets = 50;
int maxhealth = 100; // was MAXHEALTH as a #define, used only in this module
int maxhealthbonus = 200;
int max_armor = 200;
int green_armor_class = 1;  // these are involved with armortype below
int blue_armor_class = 2;
int max_soul = 200;
int soul_health = 100;
int mega_health = 200;
int god_health = 100;   // these are used in cheats (see st_stuff.c)
int idfa_armor = 200;
int idfa_armor_class = 2;
// not actually used due to pairing of cheat_k and cheat_fa
int idkfa_armor = 200;
int idkfa_armor_class = 2;

int bfgcells = 40;      // used in p_pspr.c
int monsters_infight = 0; // e6y: Dehacked support - monsters infight
// Ty 03/07/98 - end deh externals

// a weapon is found with two clip loads,
// a big item has five clip loads
int maxammo[NUMAMMO]  = {200, 50, 300, 50, 0, 0}; // heretic +2 ammo types
int clipammo[NUMAMMO] = { 10,  4,  20,  1, 0, 0}; // heretic +2 ammo types

//
// GET STUFF
//

// heretic
static weapontype_t GetAmmoChange[] = {
    wp_goldwand,
    wp_crossbow,
    wp_blaster,
    wp_skullrod,
    wp_phoenixrod,
    wp_mace
};

//
// P_GiveAmmo
// Num is the number of clip loads,
// not the individual count (0= 1/2 clip).
// Returns false if the ammo can't be picked up at all
//

static dboolean P_GiveAmmoAutoSwitch(player_t *player, ammotype_t ammo, int oldammo)
{
  int i;

  if (
    weaponinfo[player->readyweapon].flags & WPF_AUTOSWITCHFROM &&
    weaponinfo[player->readyweapon].ammo != ammo
  )
  {
    for (i = NUMWEAPONS - 1; i > player->readyweapon; --i)
    {
      if (
        player->weaponowned[i] &&
        weaponinfo[i].flags & WPF_AUTOSWITCHTO &&
        weaponinfo[i].ammo == ammo &&
        weaponinfo[i].ammopershot > oldammo &&
        weaponinfo[i].ammopershot <= player->ammo[ammo]
      )
      {
        player->pendingweapon = i;
        break;
      }
    }
  }

  return true;
}

static dboolean P_GiveAmmo(player_t *player, ammotype_t ammo, int num)
{
  int oldammo;

  if (ammo == am_noammo)
    return false;

#ifdef RANGECHECK
  if (ammo < 0 || ammo > NUMAMMO)
    I_Error ("P_GiveAmmo: bad type %i", ammo);
#endif

  if ( player->ammo[ammo] == player->maxammo[ammo]  )
    return false;

  if (num)
    num *= clipammo[ammo];
  else
    num = clipammo[ammo]/2;

  // give double ammo in trainer mode, you'll need in nightmare
  if (gameskill == sk_baby || gameskill == sk_nightmare) {
    if (heretic) num += num >> 1;
    else num <<= 1;
  }

  oldammo = player->ammo[ammo];
  player->ammo[ammo] += num;

  if (player->ammo[ammo] > player->maxammo[ammo])
    player->ammo[ammo] = player->maxammo[ammo];

  if (mbf21)
    return P_GiveAmmoAutoSwitch(player, ammo, oldammo);

  // If non zero ammo, don't change up weapons, player was lower on purpose.
  if (oldammo)
    return true;

  // We were down to zero, so select a new weapon.
  // Preferences are not user selectable.

  if (heretic) {
    if (player->readyweapon == wp_staff || player->readyweapon == wp_gauntlets)
    {
        if (player->weaponowned[GetAmmoChange[ammo]])
        {
            player->pendingweapon = GetAmmoChange[ammo];
        }
    }

    return true;
  }

  switch (ammo)
    {
    case am_clip:
      if (player->readyweapon == wp_fist) {
        if (player->weaponowned[wp_chaingun])
          player->pendingweapon = wp_chaingun;
        else
          player->pendingweapon = wp_pistol;
      }
      break;

    case am_shell:
      if (player->readyweapon == wp_fist || player->readyweapon == wp_pistol)
        if (player->weaponowned[wp_shotgun])
          player->pendingweapon = wp_shotgun;
      break;

      case am_cell:
        if (player->readyweapon == wp_fist || player->readyweapon == wp_pistol)
          if (player->weaponowned[wp_plasma])
            player->pendingweapon = wp_plasma;
        break;

      case am_misl:
        if (player->readyweapon == wp_fist)
          if (player->weaponowned[wp_missile])
            player->pendingweapon = wp_missile;
    default:
      break;
    }
  return true;
}

//
// P_GiveWeapon
// The weapon name may have a MF_DROPPED flag ored in.
//

static dboolean P_GiveWeapon(player_t *player, weapontype_t weapon, dboolean dropped)
{
  dboolean gaveammo;
  dboolean gaveweapon;

  if (heretic) return Heretic_P_GiveWeapon(player, weapon);

  if (netgame && deathmatch!=2 && !dropped)
    {
      // leave placed weapons forever on net games
      if (player->weaponowned[weapon])
        return false;

      player->bonuscount += BONUSADD;
      player->weaponowned[weapon] = true;

      P_GiveAmmo(player, weaponinfo[weapon].ammo, deathmatch ? 5 : 2);

      player->pendingweapon = weapon;
      /* cph 20028/10 - for old-school DM addicts, allow old behavior
       * where only consoleplayer's pickup sounds are heard */
      // displayplayer, not consoleplayer, for viewing multiplayer demos
      if (!comp[comp_sound] || player == &players[displayplayer])
        S_StartSound (player->mo, sfx_wpnup|PICKUP_SOUND); // killough 4/25/98
      return false;
    }

  if (weaponinfo[weapon].ammo != am_noammo)
    {
      // give one clip with a dropped weapon,
      // two clips with a found weapon
      gaveammo = P_GiveAmmo (player, weaponinfo[weapon].ammo, dropped ? 1 : 2);
    }
  else
    gaveammo = false;

  if (player->weaponowned[weapon])
    gaveweapon = false;
  else
    {
      gaveweapon = true;
      player->weaponowned[weapon] = true;
      player->pendingweapon = weapon;
    }
  return gaveweapon || gaveammo;
}

//
// P_GiveBody
// Returns false if the body isn't needed at all
//

dboolean P_GiveBody(player_t * player, int num)
{
    int max;

    max = maxhealth;
    if (heretic && player->chickenTics)
    {
        max = MAXCHICKENHEALTH;
    }
    if (player->health >= max)
    {
        return (false);
    }
    player->health += num;
    if (player->health > max)
    {
        player->health = max;
    }
    player->mo->health = player->health;
    return (true);
}

//
// P_GiveArmor
// Returns false if the armor is worse
// than the current armor.
//

static dboolean P_GiveArmor(player_t *player, int armortype)
{
  int hits = armortype*100;
  if (player->armorpoints >= hits)
    return false;   // don't pick up
  player->armortype = armortype;
  player->armorpoints = hits;
  return true;
}

//
// P_GiveCard
//

static void P_GiveCard(player_t *player, card_t card)
{
  if (player->cards[card])
    return;
  player->bonuscount = BONUSADD;
  player->cards[card] = 1;

  if (player == &players[consoleplayer])
    playerkeys |= 1 << card;

  dsda_WatchCard(card);
}

//
// P_GivePower
//
// Rewritten by Lee Killough
//

dboolean P_GivePower(player_t *player, int power)
{
  static const int tics[NUMPOWERS] = {
    INVULNTICS, 1 /* strength */, INVISTICS,
    IRONTICS, 1 /* allmap */, INFRATICS,
    WPNLEV2TICS, FLIGHTTICS, 1 /* shield */, 1 /* health2 */
   };

  if (heretic && tics[power] > 1 && power != pw_ironfeet && player->powers[power] > BLINKTHRESHOLD) return false;

  switch (power)
    {
      case pw_invisibility:
        player->mo->flags |= MF_SHADOW;
        break;
      case pw_allmap:
        if (player->powers[pw_allmap])
          return false;
        break;
      case pw_strength:
        P_GiveBody(player,100);
        break;
      case pw_flight:
        player->mo->flags2 |= MF2_FLY;
        player->mo->flags |= MF_NOGRAVITY;
        if (player->mo->z <= player->mo->floorz)
        {
            player->flyheight = 10;     // thrust the player in the air a bit
        }
        break;
    }

  // Unless player has infinite duration cheat, set duration (killough)

  if (player->powers[power] >= 0)
    player->powers[power] = tics[power];
  return true;
}

//
// P_TouchSpecialThing
//

void P_TouchSpecialThing(mobj_t *special, mobj_t *toucher)
{
  player_t *player;
  int      i;
  int      sound;
  fixed_t  delta = special->z - toucher->z;

  if (heretic) return Heretic_P_TouchSpecialThing(special, toucher);

  if (delta > toucher->height || delta < -8*FRACUNIT)
    return;        // out of reach

  sound = sfx_itemup;
  player = toucher->player;

  // Dead thing touching.
  // Can happen with a sliding player corpse.
  if (toucher->health <= 0)
    return;

    // Identify by sprite.
  switch (special->sprite)
    {
      // armor
    case SPR_ARM1:
      if (!P_GiveArmor (player, green_armor_class))
        return;
      player->message = s_GOTARMOR; // Ty 03/22/98 - externalized
      break;

    case SPR_ARM2:
      if (!P_GiveArmor (player, blue_armor_class))
        return;
      player->message = s_GOTMEGA; // Ty 03/22/98 - externalized
      break;

        // bonus items
    case SPR_BON1:
      player->health++;               // can go over 100%
      if (player->health > (maxhealthbonus))//e6y
        player->health = (maxhealthbonus);//e6y
      player->mo->health = player->health;
      player->message = s_GOTHTHBONUS; // Ty 03/22/98 - externalized
      break;

    case SPR_BON2:
      player->armorpoints++;          // can go over 100%
      // e6y
      // Doom 1.2 does not do check of armor points on overflow.
      // If you set the "IDKFA Armor" to MAX_INT (DWORD at 0x00064B5A -> FFFFFF7F)
      // and pick up one or more armor bonuses, your armor becomes negative
      // and you will die after reception of any damage since this moment.
      // It happens because the taken health damage depends from armor points
      // if they are present and becomes equal to very large value in this case
      if (player->armorpoints > max_armor && compatibility_level != doom_12_compatibility)
        player->armorpoints = max_armor;
      // e6y
      // We always give armor type 1 for the armor bonuses;
      // dehacked only affects the GreenArmor.
      if (!player->armortype)
        player->armortype =
         ((!demo_compatibility || prboom_comp[PC_APPLY_GREEN_ARMOR_CLASS_TO_ARMOR_BONUSES].state) ?
          green_armor_class : 1);
      player->message = s_GOTARMBONUS; // Ty 03/22/98 - externalized
      break;

    case SPR_SOUL:
      player->health += soul_health;
      if (player->health > max_soul)
        player->health = max_soul;
      player->mo->health = player->health;
      player->message = s_GOTSUPER; // Ty 03/22/98 - externalized
      sound = sfx_getpow;
      break;

    case SPR_MEGA:
      if (gamemode != commercial)
        return;
      player->health = mega_health;
      player->mo->health = player->health;
      // e6y
      // We always give armor type 2 for the megasphere;
      // dehacked only affects the MegaArmor.
      P_GiveArmor (player,
         ((!demo_compatibility || prboom_comp[PC_APPLY_BLUE_ARMOR_CLASS_TO_MEGASPHERE].state) ?
          blue_armor_class : 2));
      player->message = s_GOTMSPHERE; // Ty 03/22/98 - externalized
      sound = sfx_getpow;
      break;

        // cards
        // leave cards for everyone
    case SPR_BKEY:
      if (!player->cards[it_bluecard])
        player->message = s_GOTBLUECARD; // Ty 03/22/98 - externalized
      P_GiveCard (player, it_bluecard);
      if (!netgame)
        break;
      return;

    case SPR_YKEY:
      if (!player->cards[it_yellowcard])
        player->message = s_GOTYELWCARD; // Ty 03/22/98 - externalized
      P_GiveCard (player, it_yellowcard);
      if (!netgame)
        break;
      return;

    case SPR_RKEY:
      if (!player->cards[it_redcard])
        player->message = s_GOTREDCARD; // Ty 03/22/98 - externalized
      P_GiveCard (player, it_redcard);
      if (!netgame)
        break;
      return;

    case SPR_BSKU:
      if (!player->cards[it_blueskull])
        player->message = s_GOTBLUESKUL; // Ty 03/22/98 - externalized
      P_GiveCard (player, it_blueskull);
      if (!netgame)
        break;
      return;

    case SPR_YSKU:
      if (!player->cards[it_yellowskull])
        player->message = s_GOTYELWSKUL; // Ty 03/22/98 - externalized
      P_GiveCard (player, it_yellowskull);
      if (!netgame)
        break;
      return;

    case SPR_RSKU:
      if (!player->cards[it_redskull])
        player->message = s_GOTREDSKULL; // Ty 03/22/98 - externalized
      P_GiveCard (player, it_redskull);
      if (!netgame)
        break;
      return;

      // medikits, heals
    case SPR_STIM:
      if (!P_GiveBody (player, 10))
        return;
      player->message = s_GOTSTIM; // Ty 03/22/98 - externalized
      break;

    case SPR_MEDI:
      if (!P_GiveBody (player, 25))
        return;

      if (player->health < 50) // cph - 25 + the 25 just added, thanks to Quasar for reporting this bug
        player->message = s_GOTMEDINEED; // Ty 03/22/98 - externalized
      else
        player->message = s_GOTMEDIKIT; // Ty 03/22/98 - externalized
      break;


      // power ups
    case SPR_PINV:
      if (!P_GivePower (player, pw_invulnerability))
        return;
      player->message = s_GOTINVUL; // Ty 03/22/98 - externalized
      sound = sfx_getpow;
      break;

    case SPR_PSTR:
      if (!P_GivePower (player, pw_strength))
        return;
      player->message = s_GOTBERSERK; // Ty 03/22/98 - externalized
      if (player->readyweapon != wp_fist)
        player->pendingweapon = wp_fist;
      sound = sfx_getpow;
      break;

    case SPR_PINS:
      if (!P_GivePower (player, pw_invisibility))
        return;
      player->message = s_GOTINVIS; // Ty 03/22/98 - externalized
      sound = sfx_getpow;
      break;

    case SPR_SUIT:
      if (!P_GivePower (player, pw_ironfeet))
        return;
      player->message = s_GOTSUIT; // Ty 03/22/98 - externalized
      sound = sfx_getpow;
      break;

    case SPR_PMAP:
      if (!P_GivePower (player, pw_allmap))
        return;
      player->message = s_GOTMAP; // Ty 03/22/98 - externalized
      sound = sfx_getpow;
      break;

    case SPR_PVIS:
      if (!P_GivePower (player, pw_infrared))
        return;
      player->message = s_GOTVISOR; // Ty 03/22/98 - externalized
      sound = sfx_getpow;
      break;

      // ammo
    case SPR_CLIP:
      if (special->flags & MF_DROPPED)
        {
          if (!P_GiveAmmo (player,am_clip,0))
            return;
        }
      else
        {
          if (!P_GiveAmmo (player,am_clip,1))
            return;
        }
      player->message = s_GOTCLIP; // Ty 03/22/98 - externalized
      break;

    case SPR_AMMO:
      if (!P_GiveAmmo (player, am_clip,5))
        return;
      player->message = s_GOTCLIPBOX; // Ty 03/22/98 - externalized
      break;

    case SPR_ROCK:
      if (!P_GiveAmmo (player, am_misl,1))
        return;
      player->message = s_GOTROCKET; // Ty 03/22/98 - externalized
      break;

    case SPR_BROK:
      if (!P_GiveAmmo (player, am_misl,5))
        return;
      player->message = s_GOTROCKBOX; // Ty 03/22/98 - externalized
      break;

    case SPR_CELL:
      if (!P_GiveAmmo (player, am_cell,1))
        return;
      player->message = s_GOTCELL; // Ty 03/22/98 - externalized
      break;

    case SPR_CELP:
      if (!P_GiveAmmo (player, am_cell,5))
        return;
      player->message = s_GOTCELLBOX; // Ty 03/22/98 - externalized
      break;

    case SPR_SHEL:
      if (!P_GiveAmmo (player, am_shell,1))
        return;
      player->message = s_GOTSHELLS; // Ty 03/22/98 - externalized
      break;

    case SPR_SBOX:
      if (!P_GiveAmmo (player, am_shell,5))
        return;
      player->message = s_GOTSHELLBOX; // Ty 03/22/98 - externalized
      break;

    case SPR_BPAK:
      if (!player->backpack)
        {
          for (i=0 ; i<NUMAMMO ; i++)
            player->maxammo[i] *= 2;
          player->backpack = true;
        }
      for (i=0 ; i<NUMAMMO ; i++)
        P_GiveAmmo (player, i, 1);
      player->message = s_GOTBACKPACK; // Ty 03/22/98 - externalized
      break;

        // weapons
    case SPR_BFUG:
      if (!P_GiveWeapon (player, wp_bfg, false) )
        return;
      player->message = s_GOTBFG9000; // Ty 03/22/98 - externalized
      sound = sfx_wpnup;
      break;

    case SPR_MGUN:
      if (!P_GiveWeapon (player, wp_chaingun, (special->flags&MF_DROPPED)!=0) )
        return;
      player->message = s_GOTCHAINGUN; // Ty 03/22/98 - externalized
      sound = sfx_wpnup;
      break;

    case SPR_CSAW:
      if (!P_GiveWeapon (player, wp_chainsaw, false) )
        return;
      player->message = s_GOTCHAINSAW; // Ty 03/22/98 - externalized
      sound = sfx_wpnup;
      break;

    case SPR_LAUN:
      if (!P_GiveWeapon (player, wp_missile, false) )
        return;
      player->message = s_GOTLAUNCHER; // Ty 03/22/98 - externalized
      sound = sfx_wpnup;
      break;

    case SPR_PLAS:
      if (!P_GiveWeapon (player, wp_plasma, false) )
        return;
      player->message = s_GOTPLASMA; // Ty 03/22/98 - externalized
      sound = sfx_wpnup;
      break;

    case SPR_SHOT:
      if (!P_GiveWeapon (player, wp_shotgun, (special->flags&MF_DROPPED)!=0 ) )
        return;
      player->message = s_GOTSHOTGUN; // Ty 03/22/98 - externalized
      sound = sfx_wpnup;
      break;

    case SPR_SGN2:
      if (!P_GiveWeapon(player, wp_supershotgun, (special->flags&MF_DROPPED)!=0))
        return;
      player->message = s_GOTSHOTGUN2; // Ty 03/22/98 - externalized
      sound = sfx_wpnup;
      break;

    default:
      I_Error ("P_SpecialThing: Unknown gettable thing");
    }

  if (special->flags & MF_COUNTITEM)
    player->itemcount++;
  P_RemoveMobj (special);
  player->bonuscount += BONUSADD;

  CheckThingsPickupTracer(special);//e6y

  /* cph 20028/10 - for old-school DM addicts, allow old behavior
   * where only consoleplayer's pickup sounds are heard */
  // displayplayer, not consoleplayer, for viewing multiplayer demos
  if (!comp[comp_sound] || player == &players[displayplayer])
    S_StartSound (player->mo, sound | PICKUP_SOUND);   // killough 4/25/98
}

//
// KillMobj
//
// killough 11/98: make static
static void P_KillMobj(mobj_t *source, mobj_t *target)
{
  mobjtype_t item;
  mobj_t     *mo;
  int xdeath_limit;

  target->flags &= ~(MF_SHOOTABLE|MF_FLOAT|MF_SKULLFLY);

  if (target->type != MT_SKULL)
    target->flags &= ~MF_NOGRAVITY;

  target->flags |= MF_CORPSE|MF_DROPOFF;
  target->height >>= 2;

  // heretic
  target->flags2 &= ~MF2_PASSMOBJ;

  if (compatibility_level == mbf_compatibility &&
      !prboom_comp[PC_MBF_REMOVE_THINKER_IN_KILLMOBJ].state)
  {
    // killough 8/29/98: remove from threaded list
    P_UpdateThinker(&target->thinker);
  }

  if (!((target->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)))
    totallive--;

  dsda_WatchDeath(target);

  if (source && source->player)
  {
    // count for intermission
    if (target->flags & MF_COUNTKILL)
    {
      dsda_WatchKill(source->player, target);
    }
    if (target->player)
    {
      source->player->frags[target->player-players]++;

      if (heretic && target != source)
      {
        if (source->player == &players[consoleplayer])
        {
          S_StartSound(NULL, heretic_sfx_gfrag);
        }
        if (source->player->chickenTics)
        {               // Make a super chicken
          P_GivePower(source->player, pw_weaponlevel2);
        }
      }
    }
  }
  else
    if (target->flags & MF_COUNTKILL) { /* Add to kills tally */
      if ((compatibility_level < lxdoom_1_compatibility) || !netgame) {
        if (!netgame)
        {
          dsda_WatchKill(&players[0], target);
        }
        else
        {
          if (!deathmatch) {
            if (target->lastenemy && target->lastenemy->health > 0 && target->lastenemy->player)
            {
              dsda_WatchKill(target->lastenemy->player, target);
            }
            else
            {
              unsigned int player;
              for (player = 0; player<MAXPLAYERS; player++)
              {
                if (playeringame[player])
                {
                  dsda_WatchKill(&players[player], target);
                  break;
                }
              }
            }
          }
        }
      }
      else
        if (!deathmatch) {
          // try and find a player to give the kill to, otherwise give the
          // kill to a random player.  this fixes the missing monsters bug
          // in coop - rain
          // CPhipps - not a bug as such, but certainly an inconsistency.
          if (target->lastenemy && target->lastenemy->health > 0 && target->lastenemy->player) // Fighting a player
          {
            dsda_WatchKill(target->lastenemy->player, target);
          }
          else {
            // cph - randomely choose a player in the game to be credited
            //  and do it uniformly between the active players
            unsigned int activeplayers = 0, player, i;

            for (player = 0; player<MAXPLAYERS; player++)
              if (playeringame[player])
                activeplayers++;

            if (activeplayers) {
              player = P_Random(pr_friends) % activeplayers;

              for (i=0; i<MAXPLAYERS; i++)
                if (playeringame[i])
                  if (!player--)
                  {
                    dsda_WatchKill(&players[i], target);
                  }
            }
          }
        }
    }

  if (target->player)
  {
    // count environment kills against you
    if (!source)
      target->player->frags[target->player-players]++;

    target->flags &= ~MF_SOLID;

    // heretic
    target->flags2 &= ~MF2_FLY;
    target->player->powers[pw_flight] = 0;
    target->player->powers[pw_weaponlevel2] = 0;

    target->player->playerstate = PST_DEAD;
    P_DropWeapon (target->player);

    // heretic
    if (target->flags2 & MF2_FIREDAMAGE)
    {                       // Player flame death
        P_SetMobjState(target, HERETIC_S_PLAY_FDTH1);
        return;
    }

    if (target->player == &players[consoleplayer] && (automapmode & am_active))
      AM_Stop();    // don't die in auto map; switch view prior to dying
  }

  xdeath_limit = heretic ? (target->info->spawnhealth >> 1) : target->info->spawnhealth;
  if (target->health < -xdeath_limit && target->info->xdeathstate)
    P_SetMobjState (target, target->info->xdeathstate);
  else
    P_SetMobjState (target, target->info->deathstate);

  target->tics -= P_Random(pr_killtics)&3;

  if (heretic) return;

  if (target->tics < 1)
    target->tics = 1;

  // In Chex Quest, monsters don't drop items.
  if (gamemission == chex)
  {
    return;
  }

  // Drop stuff.
  // This determines the kind of object spawned
  // during the death frame of a thing.

  if (target->info->droppeditem != MT_NULL)
  {
    item = target->info->droppeditem;
  }
  else return;

  mo = P_SpawnMobj (target->x,target->y,ONFLOORZ, item);
  mo->flags |= MF_DROPPED;    // special versions of items

#ifdef GL_DOOM
  if (target->momx == 0 && target->momy == 0)
  {
    target->flags |= MF_FOREGROUND;
  }
#endif
}

//
// P_DamageMobj
// Damages both enemies and players
// "inflictor" is the thing that caused the damage
//  creature or missile, can be NULL (slime, etc)
// "source" is the thing to target after taking damage
//  creature or NULL
// Source and inflictor are the same for melee attacks.
// Source can be NULL for slime, barrel explosions
// and other environmental stuff.
//

static dboolean P_InfightingImmune(mobj_t *target, mobj_t *source)
{
  return // not default behaviour, and same group
    mobjinfo[target->type].infighting_group != IG_DEFAULT &&
    mobjinfo[target->type].infighting_group == mobjinfo[source->type].infighting_group;
}

void P_DamageMobj(mobj_t *target,mobj_t *inflictor, mobj_t *source, int damage)
{
  player_t *player;
  dboolean justhit = false;          /* killough 11/98 */

  /* killough 8/31/98: allow bouncers to take damage */
  if (!(target->flags & (MF_SHOOTABLE | MF_BOUNCES)))
    return; // shouldn't happen...

  if (target->health <= 0)
    return;

  if (target->flags & MF_SKULLFLY)
  {
    if (target->type == HERETIC_MT_MINOTAUR) return;
    target->momx = target->momy = target->momz = 0;
  }

  player = target->player;
  if (player && gameskill == sk_baby)
    damage >>= 1;   // take half damage in trainer mode

  // Special damage types
  if (heretic && inflictor)
    switch (inflictor->type)
    {
      case HERETIC_MT_EGGFX:
        if (player)
        {
          P_ChickenMorphPlayer(player);
        }
        else
        {
          P_ChickenMorph(target);
        }
        return;         // Always return
      case HERETIC_MT_WHIRLWIND:
        P_TouchWhirlwind(target);
        return;
      case HERETIC_MT_MINOTAUR:
        if (inflictor->flags & MF_SKULLFLY)
        {               // Slam only when in charge mode
          P_MinotaurSlam(inflictor, target);
          return;
        }
        break;
      case HERETIC_MT_MACEFX4:   // Death ball
        if ((target->flags2 & MF2_BOSS) || target->type == HERETIC_MT_HEAD)
        {               // Don't allow cheap boss kills
          break;
        }
        else if (target->player)
        {               // Player specific checks
          if (target->player->powers[pw_invulnerability])
          {           // Can't hurt invulnerable players
            break;
          }
          if (P_AutoUseChaosDevice(target->player))
          {           // Player was saved using chaos device
            return;
          }
        }
        damage = 10000; // Something's gonna die
        break;
      case HERETIC_MT_PHOENIXFX2:        // Flame thrower
        if (target->player && P_Random(pr_heretic) < 128)
        {               // Freeze player for a bit
          target->reactiontime += 4;
        }
        break;
      case HERETIC_MT_RAINPLR1:  // Rain missiles
      case HERETIC_MT_RAINPLR2:
      case HERETIC_MT_RAINPLR3:
      case HERETIC_MT_RAINPLR4:
        if (target->flags2 & MF2_BOSS)
        {               // Decrease damage for bosses
          damage = (P_Random(pr_heretic) & 7) + 1;
        }
        break;
      case HERETIC_MT_HORNRODFX2:
      case HERETIC_MT_PHOENIXFX1:
        if (target->type == HERETIC_MT_SORCERER2 && P_Random(pr_heretic) < 96)
        {               // D'Sparil teleports away
          P_DSparilTeleport(target);
          return;
        }
        break;
      case HERETIC_MT_BLASTERFX1:
      case HERETIC_MT_RIPPER:
        if (target->type == HERETIC_MT_HEAD)
        {               // Less damage to Ironlich bosses
          damage = P_Random(pr_heretic) & 1;
          if (!damage)
          {
            return;
          }
        }
        break;
      default:
        break;
    }

  // Some close combat weapons should not
  // inflict thrust and push the victim out of reach,
  // thus kick away unless using the chainsaw.

  if (
    inflictor &&
    !(target->flags & MF_NOCLIP) &&
    !(
      source &&
      source->player &&
      weaponinfo[source->player->readyweapon].flags & WPF_NOTHRUST
    ) &&
    !(inflictor->flags2 & MF2_NODMGTHRUST)
  )
  {
    unsigned ang = R_PointToAngle2 (inflictor->x, inflictor->y,
                                    target->x,    target->y);

    fixed_t thrust = damage * (FRACUNIT >> 3) * g_thrust_factor / target->info->mass;

    // make fall forwards sometimes
    if ( damage < 40 && damage > target->health
         && target->z - inflictor->z > 64*FRACUNIT
         && P_Random(pr_damagemobj) & 1)
    {
      ang += ANG180;
      thrust *= 4;
    }

    ang >>= ANGLETOFINESHIFT;

    if (source && source->player && (source == inflictor)
        && source->player->powers[pw_weaponlevel2]
        && source->player->readyweapon == wp_staff)
    {
      // Staff power level 2
      target->momx += FixedMul(10 * FRACUNIT, finecosine[ang]);
      target->momy += FixedMul(10 * FRACUNIT, finesine[ang]);
      if (!(target->flags & MF_NOGRAVITY))
      {
          target->momz += 5 * FRACUNIT;
      }
    }
    else
    {
      target->momx += FixedMul(thrust, finecosine[ang]);
      target->momy += FixedMul(thrust, finesine[ang]);
    }

    /* killough 11/98: thrust objects hanging off ledges */
    if (target->intflags & MIF_FALLING && target->gear >= MAXGEAR)
      target->gear = 0;
  }

  // player specific
  if (player)
  {
    // end of game hell hack
    if (!heretic && target->subsector->sector->special == 11 && damage >= target->health)
      damage = target->health - 1;

    // Below certain threshold,
    // ignore damage in GOD mode, or with INVUL power.
    // killough 3/26/98: make god mode 100% god mode in non-compat mode

    if ((damage < 1000 || (!comp[comp_god] && (player->cheats&CF_GODMODE))) &&
        (player->cheats&CF_GODMODE || player->powers[pw_invulnerability]))
      return;

    if (player->armortype)
    {
      int saved;

      if (heretic)
        saved = player->armortype == 1 ? (damage >> 1) : (damage >> 1) + (damage >> 2);
      else
        saved = player->armortype == 1 ? damage / 3 : damage / 2;

      if (player->armorpoints <= saved)
      {
        // armor is used up
        saved = player->armorpoints;
        player->armortype = 0;
      }
      player->armorpoints -= saved;
      damage -= saved;
    }

    if (heretic && damage >= player->health
        && ((gameskill == sk_baby) || deathmatch) && !player->chickenTics)
    {                       // Try to use some inventory health
      P_AutoUseHealth(player, damage - player->health + 1);
    }

    player->health -= damage;       // mirror mobj health here for Dave
    if (player->health < 0)
      player->health = 0;

    player->attacker = source;
    player->damagecount += damage;  // add damage after armor / invuln

    if (player->damagecount > 100)
      player->damagecount = 100;  // teleport stomp does 10k points...

    if (heretic && player == &players[consoleplayer])
    {
      SB_PaletteFlash();
    }
  }

  if (source && target)
  {
    CheckGivenDamageTracer(source, damage);
  }

  dsda_WatchDamage(target, inflictor, source, damage);

  // do the damage
  target->health -= damage;
  if (target->health <= 0)
  {
    if (heretic) {
      target->special1.i = damage;
      if (target->type == HERETIC_MT_POD && source && source->type != HERETIC_MT_POD)
      {                       // Make sure players get frags for chain-reaction kills
        P_SetTarget(&target->target, source);
      }
      if (player && inflictor && !player->chickenTics)
      {                       // Check for flame death
        if ((inflictor->flags2 & MF2_FIREDAMAGE)
            || ((inflictor->type == HERETIC_MT_PHOENIXFX1)
                && (target->health > -50) && (damage > 25)))
        {
          target->flags2 |= MF2_FIREDAMAGE;
        }
      }
    }

    P_KillMobj (source, target);
    return;
  }

  // killough 9/7/98: keep track of targets so that friends can help friends
  if (mbf_features)
  {
    /* If target is a player, set player's target to source,
     * so that a friend can tell who's hurting a player
     */
    if (player) P_SetTarget(&target->target, source);

    /* killough 9/8/98:
     * If target's health is less than 50%, move it to the front of its list.
     * This will slightly increase the chances that enemies will choose to
     * "finish it off", but its main purpose is to alert friends of danger.
     */
    if (target->health*2 < target->info->spawnhealth)
    {
      thinker_t *cap = &thinkerclasscap[target->flags & MF_FRIEND ?
               th_friends : th_enemies];
      (target->thinker.cprev->cnext = target->thinker.cnext)->cprev =
        target->thinker.cprev;
      (target->thinker.cnext = cap->cnext)->cprev = &target->thinker;
      (target->thinker.cprev = cap)->cnext = &target->thinker;
    }
  }

  if (P_Random (pr_painchance) < target->info->painchance &&
      !(target->flags & MF_SKULLFLY)) //killough 11/98: see below
  {
    if (mbf_features)
      justhit = true;
    else
      target->flags |= MF_JUSTHIT;    // fight back!

    P_SetMobjState(target, target->info->painstate);
  }

  target->reactiontime = 0;           // we're awake now...

  /* killough 9/9/98: cleaned up, made more consistent: */
  //e6y: Monsters could commit suicide in Doom v1.2 if they damaged themselves by exploding a barrel
  if (
    source &&
    (source != target || compatibility_level == doom_12_compatibility) &&
    !(source->flags2 & MF2_DMGIGNORED) &&
    (!target->threshold || target->flags2 & MF2_NOTHRESHOLD) &&
    ((source->flags ^ target->flags) & MF_FRIEND || monster_infighting || !mbf_features) &&
    !(heretic && source->flags2 & MF2_BOSS) &&
    !P_InfightingImmune(target, source)
  )
  {
    /* if not intent on another player, chase after this one
     *
     * killough 2/15/98: remember last enemy, to prevent
     * sleeping early; 2/21/98: Place priority on players
     * killough 9/9/98: cleaned up, made more consistent:
     */

    if (
      !target->lastenemy ||
      target->lastenemy->health <= 0 ||
      (
        !mbf_features ?
        !target->lastenemy->player :
        !((target->flags ^ target->lastenemy->flags) & MF_FRIEND) && target->target != source
      )
    ) // remember last enemy - killough
      P_SetTarget(&target->lastenemy, target->target);

    P_SetTarget(&target->target, source);       // killough 11/98
    target->threshold = BASETHRESHOLD;
    if (target->state == &states[target->info->spawnstate]
        && target->info->seestate != g_s_null)
      P_SetMobjState (target, target->info->seestate);
  }

  /* killough 11/98: Don't attack a friend, unless hit by that friend.
   * cph 2006/04/01 - implicitly this is only if mbf_features */
  if(!demo_compatibility) //e6y
    if (justhit && (target->target == source || !target->target ||
        !(target->flags & target->target->flags & MF_FRIEND)))
      target->flags |= MF_JUSTHIT;    // fight back!
}

// heretic

#include "p_user.h"

#define CHICKENTICS (40*35)

void A_RestoreArtifact(mobj_t * arti)
{
    arti->flags |= MF_SPECIAL;
    P_SetMobjState(arti, arti->info->spawnstate);
    S_StartSound(arti, heretic_sfx_respawn);
}

void A_RestoreSpecialThing1(mobj_t * thing)
{
    if (thing->type == HERETIC_MT_WMACE)
    {                           // Do random mace placement
        P_RepositionMace(thing);
    }
    thing->flags2 &= ~MF2_DONTDRAW;
    S_StartSound(thing, heretic_sfx_respawn);
}

void A_RestoreSpecialThing2(mobj_t * thing)
{
    thing->flags |= MF_SPECIAL;
    P_SetMobjState(thing, thing->info->spawnstate);
}

// heretic

void P_SetMessage(player_t * player, const char *message, dboolean ultmsg)
{
    player->message = message;
}

void Heretic_P_TouchSpecialThing(mobj_t * special, mobj_t * toucher)
{
    int i;
    player_t *player;
    fixed_t delta;
    int sound;

    delta = special->z - toucher->z;
    if (delta > toucher->height || delta < -32 * FRACUNIT)
    {                           // Out of reach
        return;
    }
    if (toucher->health <= 0)
    {                           // Toucher is dead
        return;
    }
    sound = heretic_sfx_itemup;
    player = toucher->player;

    switch (special->sprite)
    {
            // Items
        case HERETIC_SPR_PTN1:         // Item_HealingPotion
            if (!P_GiveBody(player, 10))
            {
                return;
            }
            P_SetMessage(player, DEH_String(HERETIC_TXT_ITEMHEALTH), false);
            break;
        case HERETIC_SPR_SHLD:         // Item_Shield1
            if (!P_GiveArmor(player, 1))
            {
                return;
            }
            P_SetMessage(player, DEH_String(HERETIC_TXT_ITEMSHIELD1), false);
            break;
        case HERETIC_SPR_SHD2:         // Item_Shield2
            if (!P_GiveArmor(player, 2))
            {
                return;
            }
            P_SetMessage(player, DEH_String(HERETIC_TXT_ITEMSHIELD2), false);
            break;
        case HERETIC_SPR_BAGH:         // Item_BagOfHolding
            if (!player->backpack)
            {
                for (i = 0; i < NUMAMMO; i++)
                {
                    player->maxammo[i] *= 2;
                }
                player->backpack = true;
            }
            P_GiveAmmo(player, am_goldwand, AMMO_GWND_WIMPY);
            P_GiveAmmo(player, am_blaster, AMMO_BLSR_WIMPY);
            P_GiveAmmo(player, am_crossbow, AMMO_CBOW_WIMPY);
            P_GiveAmmo(player, am_skullrod, AMMO_SKRD_WIMPY);
            P_GiveAmmo(player, am_phoenixrod, AMMO_PHRD_WIMPY);
            P_SetMessage(player, DEH_String(HERETIC_TXT_ITEMBAGOFHOLDING), false);
            break;
        case HERETIC_SPR_SPMP:         // Item_SuperMap
            if (!P_GivePower(player, pw_allmap))
            {
                return;
            }
            P_SetMessage(player, DEH_String(HERETIC_TXT_ITEMSUPERMAP), false);
            break;

            // Keys
        case HERETIC_SPR_BKYY:         // Key_Blue
            if (!player->cards[key_blue])
            {
                P_SetMessage(player, DEH_String(HERETIC_TXT_GOTBLUEKEY), false);
            }
            P_GiveCard(player, key_blue);
            sound = heretic_sfx_keyup;
            if (!netgame)
            {
                break;
            }
            return;
        case HERETIC_SPR_CKYY:         // Key_Yellow
            if (!player->cards[key_yellow])
            {
                P_SetMessage(player, DEH_String(HERETIC_TXT_GOTYELLOWKEY), false);
            }
            sound = heretic_sfx_keyup;
            P_GiveCard(player, key_yellow);
            if (!netgame)
            {
                break;
            }
            return;
        case HERETIC_SPR_AKYY:         // Key_Green
            if (!player->cards[key_green])
            {
                P_SetMessage(player, DEH_String(HERETIC_TXT_GOTGREENKEY), false);
            }
            sound = heretic_sfx_keyup;
            P_GiveCard(player, key_green);
            if (!netgame)
            {
                break;
            }
            return;

            // Artifacts
        case HERETIC_SPR_PTN2:         // Arti_HealingPotion
            if (P_GiveArtifact(player, arti_health, special))
            {
                P_SetMessage(player, DEH_String(HERETIC_TXT_ARTIHEALTH), false);
                P_SetDormantArtifact(special);
            }
            return;
        case HERETIC_SPR_SOAR:         // Arti_Fly
            if (P_GiveArtifact(player, arti_fly, special))
            {
                P_SetMessage(player, DEH_String(HERETIC_TXT_ARTIFLY), false);
                P_SetDormantArtifact(special);
            }
            return;
        case HERETIC_SPR_INVU:         // Arti_Invulnerability
            if (P_GiveArtifact(player, arti_invulnerability, special))
            {
                P_SetMessage(player, DEH_String(HERETIC_TXT_ARTIINVULNERABILITY), false);
                P_SetDormantArtifact(special);
            }
            return;
        case HERETIC_SPR_PWBK:         // Arti_TomeOfPower
            if (P_GiveArtifact(player, arti_tomeofpower, special))
            {
                P_SetMessage(player, DEH_String(HERETIC_TXT_ARTITOMEOFPOWER), false);
                P_SetDormantArtifact(special);
            }
            return;
        case HERETIC_SPR_INVS:         // Arti_Invisibility
            if (P_GiveArtifact(player, arti_invisibility, special))
            {
                P_SetMessage(player, DEH_String(HERETIC_TXT_ARTIINVISIBILITY), false);
                P_SetDormantArtifact(special);
            }
            return;
        case HERETIC_SPR_EGGC:         // Arti_Egg
            if (P_GiveArtifact(player, arti_egg, special))
            {
                P_SetMessage(player, DEH_String(HERETIC_TXT_ARTIEGG), false);
                P_SetDormantArtifact(special);
            }
            return;
        case HERETIC_SPR_SPHL:         // Arti_SuperHealth
            if (P_GiveArtifact(player, arti_superhealth, special))
            {
                P_SetMessage(player, DEH_String(HERETIC_TXT_ARTISUPERHEALTH), false);
                P_SetDormantArtifact(special);
            }
            return;
        case HERETIC_SPR_TRCH:         // Arti_Torch
            if (P_GiveArtifact(player, arti_torch, special))
            {
                P_SetMessage(player, DEH_String(HERETIC_TXT_ARTITORCH), false);
                P_SetDormantArtifact(special);
            }
            return;
        case HERETIC_SPR_FBMB:         // Arti_FireBomb
            if (P_GiveArtifact(player, arti_firebomb, special))
            {
                P_SetMessage(player, DEH_String(HERETIC_TXT_ARTIFIREBOMB), false);
                P_SetDormantArtifact(special);
            }
            return;
        case HERETIC_SPR_ATLP:         // Arti_Teleport
            if (P_GiveArtifact(player, arti_teleport, special))
            {
                P_SetMessage(player, DEH_String(HERETIC_TXT_ARTITELEPORT), false);
                P_SetDormantArtifact(special);
            }
            return;

            // Ammo
        case HERETIC_SPR_AMG1:         // Ammo_GoldWandWimpy
            if (!P_GiveAmmo(player, am_goldwand, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(HERETIC_TXT_AMMOGOLDWAND1), false);
            break;
        case HERETIC_SPR_AMG2:         // Ammo_GoldWandHefty
            if (!P_GiveAmmo(player, am_goldwand, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(HERETIC_TXT_AMMOGOLDWAND2), false);
            break;
        case HERETIC_SPR_AMM1:         // Ammo_MaceWimpy
            if (!P_GiveAmmo(player, am_mace, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(HERETIC_TXT_AMMOMACE1), false);
            break;
        case HERETIC_SPR_AMM2:         // Ammo_MaceHefty
            if (!P_GiveAmmo(player, am_mace, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(HERETIC_TXT_AMMOMACE2), false);
            break;
        case HERETIC_SPR_AMC1:         // Ammo_CrossbowWimpy
            if (!P_GiveAmmo(player, am_crossbow, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(HERETIC_TXT_AMMOCROSSBOW1), false);
            break;
        case HERETIC_SPR_AMC2:         // Ammo_CrossbowHefty
            if (!P_GiveAmmo(player, am_crossbow, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(HERETIC_TXT_AMMOCROSSBOW2), false);
            break;
        case HERETIC_SPR_AMB1:         // Ammo_BlasterWimpy
            if (!P_GiveAmmo(player, am_blaster, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(HERETIC_TXT_AMMOBLASTER1), false);
            break;
        case HERETIC_SPR_AMB2:         // Ammo_BlasterHefty
            if (!P_GiveAmmo(player, am_blaster, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(HERETIC_TXT_AMMOBLASTER2), false);
            break;
        case HERETIC_SPR_AMS1:         // Ammo_SkullRodWimpy
            if (!P_GiveAmmo(player, am_skullrod, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(HERETIC_TXT_AMMOSKULLROD1), false);
            break;
        case HERETIC_SPR_AMS2:         // Ammo_SkullRodHefty
            if (!P_GiveAmmo(player, am_skullrod, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(HERETIC_TXT_AMMOSKULLROD2), false);
            break;
        case HERETIC_SPR_AMP1:         // Ammo_PhoenixRodWimpy
            if (!P_GiveAmmo(player, am_phoenixrod, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(HERETIC_TXT_AMMOPHOENIXROD1), false);
            break;
        case HERETIC_SPR_AMP2:         // Ammo_PhoenixRodHefty
            if (!P_GiveAmmo(player, am_phoenixrod, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(HERETIC_TXT_AMMOPHOENIXROD2), false);
            break;

            // Weapons
        case HERETIC_SPR_WMCE:         // Weapon_Mace
            if (!P_GiveWeapon(player, wp_mace, false))
            {
                return;
            }
            P_SetMessage(player, DEH_String(HERETIC_TXT_WPNMACE), false);
            sound = heretic_sfx_wpnup;
            break;
        case HERETIC_SPR_WBOW:         // Weapon_Crossbow
            if (!P_GiveWeapon(player, wp_crossbow, false))
            {
                return;
            }
            P_SetMessage(player, DEH_String(HERETIC_TXT_WPNCROSSBOW), false);
            sound = heretic_sfx_wpnup;
            break;
        case HERETIC_SPR_WBLS:         // Weapon_Blaster
            if (!P_GiveWeapon(player, wp_blaster, false))
            {
                return;
            }
            P_SetMessage(player, DEH_String(HERETIC_TXT_WPNBLASTER), false);
            sound = heretic_sfx_wpnup;
            break;
        case HERETIC_SPR_WSKL:         // Weapon_SkullRod
            if (!P_GiveWeapon(player, wp_skullrod, false))
            {
                return;
            }
            P_SetMessage(player, DEH_String(HERETIC_TXT_WPNSKULLROD), false);
            sound = heretic_sfx_wpnup;
            break;
        case HERETIC_SPR_WPHX:         // Weapon_PhoenixRod
            if (!P_GiveWeapon(player, wp_phoenixrod, false))
            {
                return;
            }
            P_SetMessage(player, DEH_String(HERETIC_TXT_WPNPHOENIXROD), false);
            sound = heretic_sfx_wpnup;
            break;
        case HERETIC_SPR_WGNT:         // Weapon_Gauntlets
            if (!P_GiveWeapon(player, wp_gauntlets, false))
            {
                return;
            }
            P_SetMessage(player, DEH_String(HERETIC_TXT_WPNGAUNTLETS), false);
            sound = heretic_sfx_wpnup;
            break;
        default:
            I_Error("Heretic_P_TouchSpecialThing: Unknown gettable thing");
    }
    if (special->flags & MF_COUNTITEM)
    {
        player->itemcount++;
    }
    if (deathmatch && !(special->flags & MF_DROPPED))
    {
        P_HideSpecialThing(special);
    }
    else
    {
        P_RemoveMobj(special);
    }
    player->bonuscount += BONUSADD;
    if (player == &players[consoleplayer])
    {
        S_StartSound(NULL, sound);
        SB_PaletteFlash();
    }
}

dboolean P_GiveArtifact(player_t * player, artitype_t arti, mobj_t * mo)
{
    int i;

    i = 0;
    while (player->inventory[i].type != arti && i < player->inventorySlotNum)
    {
        i++;
    }
    if (i == player->inventorySlotNum)
    {
        player->inventory[i].count = 1;
        player->inventory[i].type = arti;
        player->inventorySlotNum++;
    }
    else
    {
        if (player->inventory[i].count >= 16)
        {                       // Player already has 16 of this item
            return (false);
        }
        player->inventory[i].count++;
    }
    if (player->artifactCount == 0)
    {
        player->readyArtifact = arti;
    }
    player->artifactCount++;
    if (mo && (mo->flags & MF_COUNTITEM))
    {
        player->itemcount++;
    }
    return (true);
}

void P_SetDormantArtifact(mobj_t * arti)
{
    arti->flags &= ~MF_SPECIAL;
    if (deathmatch && (arti->type != HERETIC_MT_ARTIINVULNERABILITY)
        && (arti->type != HERETIC_MT_ARTIINVISIBILITY))
    {
        P_SetMobjState(arti, HERETIC_S_DORMANTARTI1);
    }
    else
    {                           // Don't respawn
        P_SetMobjState(arti, HERETIC_S_DEADARTI1);
    }
    S_StartSound(arti, heretic_sfx_artiup);
}

int GetWeaponAmmo[NUMWEAPONS] = {
    0,                          // staff
    25,                         // gold wand
    10,                         // crossbow
    30,                         // blaster
    50,                         // skull rod
    2,                          // phoenix rod
    50,                         // mace
    0,                          // gauntlets
    0                           // beak
};

int WeaponValue[] = {
    1,                          // staff
    3,                          // goldwand
    4,                          // crossbow
    5,                          // blaster
    6,                          // skullrod
    7,                          // phoenixrod
    8,                          // mace
    2,                          // gauntlets
    0                           // beak
};

dboolean Heretic_P_GiveWeapon(player_t * player, weapontype_t weapon)
{
    dboolean gaveAmmo;
    dboolean gaveWeapon;

    if (netgame && !deathmatch)
    {                           // Cooperative net-game
        if (player->weaponowned[weapon])
        {
            return (false);
        }
        player->bonuscount += BONUSADD;
        player->weaponowned[weapon] = true;
        P_GiveAmmo(player, wpnlev1info[weapon].ammo, GetWeaponAmmo[weapon]);
        player->pendingweapon = weapon;
        if (player == &players[consoleplayer])
        {
            S_StartSound(NULL, heretic_sfx_wpnup);
        }
        return (false);
    }
    gaveAmmo = P_GiveAmmo(player, wpnlev1info[weapon].ammo,
                          GetWeaponAmmo[weapon]);
    if (player->weaponowned[weapon])
    {
        gaveWeapon = false;
    }
    else
    {
        gaveWeapon = true;
        player->weaponowned[weapon] = true;
        if (WeaponValue[weapon] > WeaponValue[player->readyweapon])
        {                       // Only switch to more powerful weapons
            player->pendingweapon = weapon;
        }
    }
    return (gaveWeapon || gaveAmmo);
}

void P_HideSpecialThing(mobj_t * thing)
{
    thing->flags &= ~MF_SPECIAL;
    thing->flags2 |= MF2_DONTDRAW;
    P_SetMobjState(thing, HERETIC_S_HIDESPECIAL1);
}

void P_MinotaurSlam(mobj_t * source, mobj_t * target)
{
    angle_t angle;
    fixed_t thrust;

    angle = R_PointToAngle2(source->x, source->y, target->x, target->y);
    angle >>= ANGLETOFINESHIFT;
    thrust = 16 * FRACUNIT + (P_Random(pr_heretic) << 10);
    target->momx += FixedMul(thrust, finecosine[angle]);
    target->momy += FixedMul(thrust, finesine[angle]);
    P_DamageMobj(target, NULL, NULL, HITDICE(6));
    if (target->player)
    {
        target->reactiontime = 14 + (P_Random(pr_heretic) & 7);
    }
}

void P_TouchWhirlwind(mobj_t * target)
{
    int randVal;

    target->angle += P_SubRandom() << 20;
    target->momx += P_SubRandom() << 10;
    target->momy += P_SubRandom() << 10;
    if (leveltime & 16 && !(target->flags2 & MF2_BOSS))
    {
        randVal = P_Random(pr_heretic);
        if (randVal > 160)
        {
            randVal = 160;
        }
        target->momz += randVal << 10;
        if (target->momz > 12 * FRACUNIT)
        {
            target->momz = 12 * FRACUNIT;
        }
    }
    if (!(leveltime & 7))
    {
        P_DamageMobj(target, NULL, NULL, 3);
    }

    if (target->player) R_SmoothPlaying_Reset(target->player); // e6y
}

dboolean P_ChickenMorphPlayer(player_t * player)
{
    mobj_t *pmo;
    mobj_t *fog;
    mobj_t *chicken;
    fixed_t x;
    fixed_t y;
    fixed_t z;
    angle_t angle;
    int oldFlags2;

    if (player->chickenTics)
    {
        if ((player->chickenTics < CHICKENTICS - TICRATE)
            && !player->powers[pw_weaponlevel2])
        {                       // Make a super chicken
            P_GivePower(player, pw_weaponlevel2);
        }
        return (false);
    }
    if (player->powers[pw_invulnerability])
    {                           // Immune when invulnerable
        return (false);
    }
    pmo = player->mo;
    x = pmo->x;
    y = pmo->y;
    z = pmo->z;
    angle = pmo->angle;
    oldFlags2 = pmo->flags2;
    P_SetMobjState(pmo, HERETIC_S_FREETARGMOBJ);
    fog = P_SpawnMobj(x, y, z + TELEFOGHEIGHT, HERETIC_MT_TFOG);
    S_StartSound(fog, heretic_sfx_telept);
    chicken = P_SpawnMobj(x, y, z, HERETIC_MT_CHICPLAYER);
    chicken->special1.i = player->readyweapon;
    chicken->angle = angle;
    chicken->player = player;
    player->health = chicken->health = MAXCHICKENHEALTH;
    player->mo = chicken;
    player->armorpoints = player->armortype = 0;
    player->powers[pw_invisibility] = 0;
    player->powers[pw_weaponlevel2] = 0;
    if (oldFlags2 & MF2_FLY)
    {
        chicken->flags2 |= MF2_FLY;
    }
    player->chickenTics = CHICKENTICS;
    P_ActivateBeak(player);
    return (true);
}

dboolean P_ChickenMorph(mobj_t * actor)
{
    mobj_t *fog;
    mobj_t *chicken;
    mobj_t *target;
    mobjtype_t moType;
    fixed_t x;
    fixed_t y;
    fixed_t z;
    angle_t angle;
    int ghost;

    if (actor->player)
    {
        return (false);
    }
    moType = actor->type;
    switch (moType)
    {
        case HERETIC_MT_POD:
        case HERETIC_MT_CHICKEN:
        case HERETIC_MT_HEAD:
        case HERETIC_MT_MINOTAUR:
        case HERETIC_MT_SORCERER1:
        case HERETIC_MT_SORCERER2:
            return (false);
        default:
            break;
    }
    x = actor->x;
    y = actor->y;
    z = actor->z;
    angle = actor->angle;
    ghost = actor->flags & MF_SHADOW;
    target = actor->target;
    P_SetMobjState(actor, HERETIC_S_FREETARGMOBJ);
    fog = P_SpawnMobj(x, y, z + TELEFOGHEIGHT, HERETIC_MT_TFOG);
    S_StartSound(fog, heretic_sfx_telept);
    chicken = P_SpawnMobj(x, y, z, HERETIC_MT_CHICKEN);
    chicken->special2.i = moType;
    chicken->special1.i = CHICKENTICS + P_Random(pr_heretic);
    chicken->flags |= ghost;
    P_SetTarget(&chicken->target, target);
    chicken->angle = angle;
    return (true);
}

dboolean P_AutoUseChaosDevice(player_t * player)
{
    int i;

    for (i = 0; i < player->inventorySlotNum; i++)
    {
        if (player->inventory[i].type == arti_teleport)
        {
            P_PlayerUseArtifact(player, arti_teleport);
            player->health = player->mo->health = (player->health + 1) / 2;
            return (true);
        }
    }
    return (false);
}

void P_AutoUseHealth(player_t * player, int saveHealth)
{
    int i;
    int count;
    int normalCount;
    int normalSlot;
    int superCount;
    int superSlot;

    normalCount = 0;
    superCount = 0;
    normalSlot = 0;
    superSlot = 0;

    for (i = 0; i < player->inventorySlotNum; i++)
    {
        if (player->inventory[i].type == arti_health)
        {
            normalSlot = i;
            normalCount = player->inventory[i].count;
        }
        else if (player->inventory[i].type == arti_superhealth)
        {
            superSlot = i;
            superCount = player->inventory[i].count;
        }
    }
    if ((gameskill == sk_baby) && (normalCount * 25 >= saveHealth))
    {                           // Use quartz flasks
        count = (saveHealth + 24) / 25;
        for (i = 0; i < count; i++)
        {
            player->health += 25;
            P_PlayerRemoveArtifact(player, normalSlot);
        }
    }
    else if (superCount * 100 >= saveHealth)
    {                           // Use mystic urns
        count = (saveHealth + 99) / 100;
        for (i = 0; i < count; i++)
        {
            player->health += 100;
            P_PlayerRemoveArtifact(player, superSlot);
        }
    }
    else if ((gameskill == sk_baby)
             && (superCount * 100 + normalCount * 25 >= saveHealth))
    {                           // Use mystic urns and quartz flasks
        count = (saveHealth + 24) / 25;
        saveHealth -= count * 25;
        for (i = 0; i < count; i++)
        {
            player->health += 25;
            P_PlayerRemoveArtifact(player, normalSlot);
        }
        count = (saveHealth + 99) / 100;
        for (i = 0; i < count; i++)
        {
            player->health += 100;
            P_PlayerRemoveArtifact(player, normalSlot);
        }
    }
    player->mo->health = player->health;
}
