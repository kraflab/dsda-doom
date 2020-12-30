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

// P_pspr.c

#include "doomdef.h"
#include "i_system.h"
#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"

// Macros

#define LOWERSPEED FRACUNIT*6
#define RAISESPEED FRACUNIT*6
#define WEAPONBOTTOM 128*FRACUNIT
#define WEAPONTOP 32*FRACUNIT

fixed_t bulletslope;

//---------------------------------------------------------------------------
//
// PROC P_OpenWeapons
//
// Called at level load before things are loaded.
//
//---------------------------------------------------------------------------

void P_OpenWeapons(void)
{
    MaceSpotCount = 0;
}

//---------------------------------------------------------------------------
//
// PROC P_AddMaceSpot
//
//---------------------------------------------------------------------------

void P_AddMaceSpot(mapthing_t * mthing)
{
    if (MaceSpotCount == MAX_MACE_SPOTS)
    {
        I_Error("Too many mace spots.");
    }
    MaceSpots[MaceSpotCount].x = mthing->x << FRACBITS;
    MaceSpots[MaceSpotCount].y = mthing->y << FRACBITS;
    MaceSpotCount++;
}

//---------------------------------------------------------------------------
//
// PROC P_CloseWeapons
//
// Called at level load after things are loaded.
//
//---------------------------------------------------------------------------

void P_CloseWeapons(void)
{
    int spot;

    if (!MaceSpotCount)
    {                           // No maces placed
        return;
    }
    if (!deathmatch && P_Random() < 64)
    {                           // Sometimes doesn't show up if not in deathmatch
        return;
    }
    spot = P_Random() % MaceSpotCount;
    P_SpawnMobj(MaceSpots[spot].x, MaceSpots[spot].y, ONFLOORZ, MT_WMACE);
}

//---------------------------------------------------------------------------
//
// PROC P_SetPsprite
//
//---------------------------------------------------------------------------

void P_SetPsprite(player_t * player, int position, statenum_t stnum)
{
    pspdef_t *psp;
    state_t *state;

    psp = &player->psprites[position];
    do
    {
        if (!stnum)
        {                       // Object removed itself.
            psp->state = NULL;
            break;
        }
        state = &states[stnum];
        psp->state = state;
        psp->tics = state->tics;        // could be 0
        if (state->misc1)
        {                       // Set coordinates.
            psp->sx = state->misc1 << FRACBITS;
            psp->sy = state->misc2 << FRACBITS;
        }
        if (state->action)
        {                       // Call action routine.
            state->action(player, psp);
            if (!psp->state)
            {
                break;
            }
        }
        stnum = psp->state->nextstate;
    }
    while (!psp->tics);         // An initial state of 0 could cycle through.
}

/*
=================
=
= P_CalcSwing
=
=================
*/

/*
fixed_t	swingx, swingy;
void P_CalcSwing (player_t *player)
{
	fixed_t	swing;
	int		angle;

// OPTIMIZE: tablify this

	swing = player->bob;

	angle = (FINEANGLES/70*leveltime)&FINEMASK;
	swingx = FixedMul ( swing, finesine[angle]);

	angle = (FINEANGLES/70*leveltime+FINEANGLES/2)&FINEMASK;
	swingy = -FixedMul ( swingx, finesine[angle]);
}
*/

//---------------------------------------------------------------------------
//
// PROC P_ActivateBeak
//
//---------------------------------------------------------------------------

void P_ActivateBeak(player_t * player)
{
    player->pendingweapon = wp_nochange;
    player->readyweapon = wp_beak;
    player->psprites[ps_weapon].sy = WEAPONTOP;
    P_SetPsprite(player, ps_weapon, S_BEAKREADY);
}

//---------------------------------------------------------------------------
//
// PROC P_PostChickenWeapon
//
//---------------------------------------------------------------------------

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

//---------------------------------------------------------------------------
//
// PROC P_BringUpWeapon
//
// Starts bringing the pending weapon up from the bottom of the screen.
//
//---------------------------------------------------------------------------

void P_BringUpWeapon(player_t * player)
{
    statenum_t new;

    if (player->pendingweapon == wp_nochange)
    {
        player->pendingweapon = player->readyweapon;
    }
    if (player->pendingweapon == wp_gauntlets)
    {
        S_StartSound(player->mo, sfx_gntact);
    }
    if (player->powers[pw_weaponlevel2])
    {
        new = wpnlev2info[player->pendingweapon].upstate;
    }
    else
    {
        new = wpnlev1info[player->pendingweapon].upstate;
    }
    player->pendingweapon = wp_nochange;
    player->psprites[ps_weapon].sy = WEAPONBOTTOM;
    P_SetPsprite(player, ps_weapon, new);
}

//---------------------------------------------------------------------------
//
// FUNC P_CheckAmmo
//
// Returns true if there is enough ammo to shoot.  If not, selects the
// next weapon to use.
//
//---------------------------------------------------------------------------

boolean P_CheckAmmo(player_t * player)
{
    ammotype_t ammo;
    int *ammoUse;
    int count;

    ammo = wpnlev1info[player->readyweapon].ammo;
    if (player->powers[pw_weaponlevel2] && !deathmatch)
    {
        ammoUse = WeaponAmmoUsePL2;
    }
    else
    {
        ammoUse = WeaponAmmoUsePL1;
    }
    count = ammoUse[player->readyweapon];
    if (ammo == am_noammo || player->ammo[ammo] >= count)
    {
        return (true);
    }
    // out of ammo, pick a weapon to change to
    do
    {
        if (player->weaponowned[wp_skullrod]
            && player->ammo[am_skullrod] > ammoUse[wp_skullrod])
        {
            player->pendingweapon = wp_skullrod;
        }
        else if (player->weaponowned[wp_blaster]
                 && player->ammo[am_blaster] > ammoUse[wp_blaster])
        {
            player->pendingweapon = wp_blaster;
        }
        else if (player->weaponowned[wp_crossbow]
                 && player->ammo[am_crossbow] > ammoUse[wp_crossbow])
        {
            player->pendingweapon = wp_crossbow;
        }
        else if (player->weaponowned[wp_mace]
                 && player->ammo[am_mace] > ammoUse[wp_mace])
        {
            player->pendingweapon = wp_mace;
        }
        else if (player->ammo[am_goldwand] > ammoUse[wp_goldwand])
        {
            player->pendingweapon = wp_goldwand;
        }
        else if (player->weaponowned[wp_gauntlets])
        {
            player->pendingweapon = wp_gauntlets;
        }
        else if (player->weaponowned[wp_phoenixrod]
                 && player->ammo[am_phoenixrod] > ammoUse[wp_phoenixrod])
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

//---------------------------------------------------------------------------
//
// PROC P_FireWeapon
//
//---------------------------------------------------------------------------

void P_FireWeapon(player_t * player)
{
    weaponinfo_t *wpinfo;
    statenum_t attackState;

    if (!P_CheckAmmo(player))
    {
        return;
    }
    P_SetMobjState(player->mo, S_PLAY_ATK2);
    wpinfo = player->powers[pw_weaponlevel2] ? &wpnlev2info[0]
        : &wpnlev1info[0];
    attackState = player->refire ? wpinfo[player->readyweapon].holdatkstate
        : wpinfo[player->readyweapon].atkstate;
    P_SetPsprite(player, ps_weapon, attackState);
    P_NoiseAlert(player->mo, player->mo);
    if (player->readyweapon == wp_gauntlets && !player->refire)
    {                           // Play the sound for the initial gauntlet attack
        S_StartSound(player->mo, sfx_gntuse);
    }
}

//---------------------------------------------------------------------------
//
// PROC P_DropWeapon
//
// The player died, so put the weapon away.
//
//---------------------------------------------------------------------------

void P_DropWeapon(player_t * player)
{
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
}

//---------------------------------------------------------------------------
//
// PROC A_WeaponReady
//
// The player can fire the weapon or change to another weapon at this time.
//
//---------------------------------------------------------------------------

void A_WeaponReady(player_t * player, pspdef_t * psp)
{
    int angle;

    if (player->chickenTics)
    {                           // Change to the chicken beak
        P_ActivateBeak(player);
        return;
    }
    // Change player from attack state
    if (player->mo->state == &states[S_PLAY_ATK1]
        || player->mo->state == &states[S_PLAY_ATK2])
    {
        P_SetMobjState(player->mo, S_PLAY);
    }
    // Check for staff PL2 active sound
    if ((player->readyweapon == wp_staff)
        && (psp->state == &states[S_STAFFREADY2_1]) && P_Random() < 128)
    {
        S_StartSound(player->mo, sfx_stfcrk);
    }
    // Put the weapon away if the player has a pending weapon or has
    // died.
    if (player->pendingweapon != wp_nochange || !player->health)
    {
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
        return;
    }

    // Check for fire.  The phoenix rod does not auto fire.
    if (player->cmd.buttons & BT_ATTACK)
    {
        if (!player->attackdown || (player->readyweapon != wp_phoenixrod))
        {
            player->attackdown = true;
            P_FireWeapon(player);
            return;
        }
    }
    else
    {
        player->attackdown = false;
    }

    // Bob the weapon based on movement speed.
    angle = (128 * leveltime) & FINEMASK;
    psp->sx = FRACUNIT + FixedMul(player->bob, finecosine[angle]);
    angle &= FINEANGLES / 2 - 1;
    psp->sy = WEAPONTOP + FixedMul(player->bob, finesine[angle]);
}

//---------------------------------------------------------------------------
//
// PROC P_UpdateBeak
//
//---------------------------------------------------------------------------

void P_UpdateBeak(player_t * player, pspdef_t * psp)
{
    psp->sy = WEAPONTOP + (player->chickenPeck << (FRACBITS - 1));
}

//---------------------------------------------------------------------------
//
// PROC A_ReFire
//
// The player can re fire the weapon without lowering it entirely.
//
//---------------------------------------------------------------------------

void A_ReFire(player_t * player, pspdef_t * psp)
{
    if ((player->cmd.buttons & BT_ATTACK)
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

//---------------------------------------------------------------------------
//
// PROC A_Lower
//
//---------------------------------------------------------------------------

void A_Lower(player_t * player, pspdef_t * psp)
{
    if (player->chickenTics)
    {
        psp->sy = WEAPONBOTTOM;
    }
    else
    {
        psp->sy += LOWERSPEED;
    }
    if (psp->sy < WEAPONBOTTOM)
    {                           // Not lowered all the way yet
        return;
    }
    if (player->playerstate == PST_DEAD)
    {                           // Player is dead, so don't bring up a pending weapon
        psp->sy = WEAPONBOTTOM;
        return;
    }
    if (!player->health)
    {                           // Player is dead, so keep the weapon off screen
        P_SetPsprite(player, ps_weapon, S_NULL);
        return;
    }
    player->readyweapon = player->pendingweapon;
    P_BringUpWeapon(player);
}

//---------------------------------------------------------------------------
//
// PROC A_Raise
//
//---------------------------------------------------------------------------

void A_Raise(player_t * player, pspdef_t * psp)
{
    psp->sy -= RAISESPEED;
    if (psp->sy > WEAPONTOP)
    {                           // Not raised all the way yet
        return;
    }
    psp->sy = WEAPONTOP;
    if (player->powers[pw_weaponlevel2])
    {
        P_SetPsprite(player, ps_weapon,
                     wpnlev2info[player->readyweapon].readystate);
    }
    else
    {
        P_SetPsprite(player, ps_weapon,
                     wpnlev1info[player->readyweapon].readystate);
    }
}

/*
===============
=
= P_BulletSlope
=
= Sets a slope so a near miss is at aproximately the height of the
= intended target
=
===============
*/

void P_BulletSlope(mobj_t * mo)
{
    angle_t an;

//
// see which target is to be aimed at
//
    an = mo->angle;
    bulletslope = P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT);
    if (!linetarget)
    {
        an += 1 << 26;
        bulletslope = P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT);
        if (!linetarget)
        {
            an -= 2 << 26;
            bulletslope = P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT);
        }
        if (!linetarget)
        {
            an += 2 << 26;
            bulletslope = (mo->player->lookdir << FRACBITS) / 173;
        }
    }
}

//****************************************************************************
//
// WEAPON ATTACKS
//
//****************************************************************************

void A_Light0(player_t * player, pspdef_t * psp)
{
    player->extralight = 0;
}

void A_Light1(player_t * player, pspdef_t * psp)
{
    player->extralight = 1;
}

void A_Light2(player_t * player, pspdef_t * psp)
{
    player->extralight = 2;
}

//------------------------------------------------------------------------
//
// PROC P_SetupPsprites
//
// Called at start of level for each player
//
//------------------------------------------------------------------------

void P_SetupPsprites(player_t * player)
{
    int i;

    // Remove all psprites
    for (i = 0; i < NUMPSPRITES; i++)
    {
        player->psprites[i].state = NULL;
    }
    // Spawn the ready weapon
    player->pendingweapon = player->readyweapon;
    P_BringUpWeapon(player);
}

//------------------------------------------------------------------------
//
// PROC P_MovePsprites
//
// Called every tic by player thinking routine
//
//------------------------------------------------------------------------

void P_MovePsprites(player_t * player)
{
    int i;
    pspdef_t *psp;

    psp = &player->psprites[0];
    for (i = 0; i < NUMPSPRITES; i++, psp++)
    {
        if (psp->state != 0)  // a null state means not active
        {
            // drop tic count and possibly change state
            if (psp->tics != -1)        // a -1 tic count never changes
            {
                psp->tics--;
                if (!psp->tics)
                {
                    P_SetPsprite(player, i, psp->state->nextstate);
                }
            }
        }
    }
    player->psprites[ps_flash].sx = player->psprites[ps_weapon].sx;
    player->psprites[ps_flash].sy = player->psprites[ps_weapon].sy;
}
