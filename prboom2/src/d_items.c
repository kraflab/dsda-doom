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
 *  Something to do with weapon sprite frames. Don't ask me.
 *
 *-----------------------------------------------------------------------------
 */

// We are referring to sprite numbers.
#include "doomtype.h"
#include "info.h"

#ifdef __GNUG__
#pragma implementation "d_items.h"
#endif
#include "d_items.h"


//
// PSPRITE ACTIONS for waepons.
// This struct controls the weapon animations.
//
// Each entry is:
//  ammo/amunition type
//  upstate
//  downstate
//  readystate
//  atkstate, i.e. attack/fire/hit frame
//  flashstate, muzzle flash
//
weaponinfo_t doom_weaponinfo[NUMWEAPONS+2] =
{
  {
    // fist
    am_noammo,
    S_PUNCHUP,
    S_PUNCHDOWN,
    S_PUNCH,
    S_PUNCH1,
    S_NULL,
    S_NULL,
    1,
    0,
    WPF_FLEEMELEE | WPF_AUTOSWITCHFROM | WPF_NOAUTOSWITCHTO
  },
  {
    // pistol
    am_clip,
    S_PISTOLUP,
    S_PISTOLDOWN,
    S_PISTOL,
    S_PISTOL1,
    S_NULL,
    S_PISTOLFLASH,
    1,
    0,
    WPF_AUTOSWITCHFROM
  },
  {
    // shotgun
    am_shell,
    S_SGUNUP,
    S_SGUNDOWN,
    S_SGUN,
    S_SGUN1,
    S_NULL,
    S_SGUNFLASH1,
    1,
    0,
    WPF_NOFLAG
  },
  {
    // chaingun
    am_clip,
    S_CHAINUP,
    S_CHAINDOWN,
    S_CHAIN,
    S_CHAIN1,
    S_NULL,
    S_CHAINFLASH1,
    1,
    0,
    WPF_NOFLAG
  },
  {
    // missile launcher
    am_misl,
    S_MISSILEUP,
    S_MISSILEDOWN,
    S_MISSILE,
    S_MISSILE1,
    S_NULL,
    S_MISSILEFLASH1,
    1,
    0,
    WPF_NOAUTOFIRE
  },
  {
    // plasma rifle
    am_cell,
    S_PLASMAUP,
    S_PLASMADOWN,
    S_PLASMA,
    S_PLASMA1,
    S_NULL,
    S_PLASMAFLASH1,
    1,
    0,
    WPF_NOFLAG
  },
  {
    // bfg 9000
    am_cell,
    S_BFGUP,
    S_BFGDOWN,
    S_BFG,
    S_BFG1,
    S_NULL,
    S_BFGFLASH1,
    40,
    0,
    WPF_NOAUTOFIRE
  },
  {
    // chainsaw
    am_noammo,
    S_SAWUP,
    S_SAWDOWN,
    S_SAW,
    S_SAW1,
    S_NULL,
    S_NULL,
    1,
    0,
    WPF_NOTHRUST | WPF_FLEEMELEE | WPF_NOAUTOSWITCHTO
  },
  {
    // super shotgun
    am_shell,
    S_DSGUNUP,
    S_DSGUNDOWN,
    S_DSGUN,
    S_DSGUN1,
    S_NULL,
    S_DSGUNFLASH1,
    2,
    0,
    WPF_NOFLAG
  },

  // dseg03:00082D90                 weaponinfo_t <5, 46h, 45h, 43h, 47h, 0>
  // dseg03:00082D90                 weaponinfo_t <1, 22h, 21h, 20h, 23h, 2Fh>
  // dseg03:00082E68 animdefs        dd 0                    ; istexture
  // dseg03:00082E68                 db 'N', 'U', 'K', 'A', 'G', 'E', '3', 2 dup(0); endname
  // dseg03:00082E68                 db 'N', 'U', 'K', 'A', 'G', 'E', '1', 2 dup(0); startname
  // dseg03:00082E68                 dd 8                    ; speed
  // dseg03:00082E68                 dd 0                    ; istexture
  {
    // ololo weapon
    0,
    S_NULL, // states are not used for emulation of weaponinfo overrun
    S_NULL,
    S_NULL,
    S_NULL,
    S_NULL,
    S_NULL,
    0,
    0,
    WPF_NOFLAG
  },
  {
    // preved medved weapon
    0,
    S_NULL,
    S_NULL,
    S_NULL,
    S_NULL,
    S_NULL,
    S_NULL,
    0,
    0,
    WPF_NOFLAG
  },
};

// heretic

#include "heretic/def.h"

weaponinfo_t wpnlev1info[NUMWEAPONS] = {
  {                           // Staff
    am_noammo,                 // ammo
    HERETIC_S_STAFFUP,                 // upstate
    HERETIC_S_STAFFDOWN,               // downstate
    HERETIC_S_STAFFREADY,              // readystate
    HERETIC_S_STAFFATK1_1,             // atkstate
    HERETIC_S_STAFFATK1_1,             // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    0,                                 // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Gold wand
    am_goldwand,               // ammo
    HERETIC_S_GOLDWANDUP,              // upstate
    HERETIC_S_GOLDWANDDOWN,            // downstate
    HERETIC_S_GOLDWANDREADY,           // readystate
    HERETIC_S_GOLDWANDATK1_1,          // atkstate
    HERETIC_S_GOLDWANDATK1_1,          // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_GWND_AMMO_1,                   // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Crossbow
    am_crossbow,               // ammo
    HERETIC_S_CRBOWUP,                 // upstate
    HERETIC_S_CRBOWDOWN,               // downstate
    HERETIC_S_CRBOW1,                  // readystate
    HERETIC_S_CRBOWATK1_1,             // atkstate
    HERETIC_S_CRBOWATK1_1,             // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_CBOW_AMMO_1,                   // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Blaster
    am_blaster,                // ammo
    HERETIC_S_BLASTERUP,               // upstate
    HERETIC_S_BLASTERDOWN,             // downstate
    HERETIC_S_BLASTERREADY,            // readystate
    HERETIC_S_BLASTERATK1_1,           // atkstate
    HERETIC_S_BLASTERATK1_3,           // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_BLSR_AMMO_1,                   // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Skull rod
    am_skullrod,               // ammo
    HERETIC_S_HORNRODUP,               // upstate
    HERETIC_S_HORNRODDOWN,             // downstate
    HERETIC_S_HORNRODREADY,            // readystae
    HERETIC_S_HORNRODATK1_1,           // atkstate
    HERETIC_S_HORNRODATK1_1,           // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_SKRD_AMMO_1,                   // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Phoenix rod
    am_phoenixrod,             // ammo
    HERETIC_S_PHOENIXUP,               // upstate
    HERETIC_S_PHOENIXDOWN,             // downstate
    HERETIC_S_PHOENIXREADY,            // readystate
    HERETIC_S_PHOENIXATK1_1,           // atkstate
    HERETIC_S_PHOENIXATK1_1,           // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_PHRD_AMMO_1,                   // ammopershot
    0,                                 // intflags
    WPF_NOAUTOFIRE
  },
  {                           // Mace
    am_mace,                   // ammo
    HERETIC_S_MACEUP,                  // upstate
    HERETIC_S_MACEDOWN,                // downstate
    HERETIC_S_MACEREADY,               // readystate
    HERETIC_S_MACEATK1_1,              // atkstate
    HERETIC_S_MACEATK1_2,              // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_MACE_AMMO_1,                   // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Gauntlets
    am_noammo,                 // ammo
    HERETIC_S_GAUNTLETUP,              // upstate
    HERETIC_S_GAUNTLETDOWN,            // downstate
    HERETIC_S_GAUNTLETREADY,           // readystate
    HERETIC_S_GAUNTLETATK1_1,          // atkstate
    HERETIC_S_GAUNTLETATK1_3,          // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    0,                                 // ammopershot
    0,                                 // intflags
    WPF_NOTHRUST
  },
  {                           // Beak
    am_noammo,                 // ammo
    HERETIC_S_BEAKUP,                  // upstate
    HERETIC_S_BEAKDOWN,                // downstate
    HERETIC_S_BEAKREADY,               // readystate
    HERETIC_S_BEAKATK1_1,              // atkstate
    HERETIC_S_BEAKATK1_1,              // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    0,                                 // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  }
};

weaponinfo_t wpnlev2info[NUMWEAPONS] = {
  {                           // Staff
    am_noammo,                 // ammo
    HERETIC_S_STAFFUP2,                // upstate
    HERETIC_S_STAFFDOWN2,              // downstate
    HERETIC_S_STAFFREADY2_1,           // readystate
    HERETIC_S_STAFFATK2_1,             // atkstate
    HERETIC_S_STAFFATK2_1,             // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    0,                                 // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Gold wand
    am_goldwand,               // ammo
    HERETIC_S_GOLDWANDUP,              // upstate
    HERETIC_S_GOLDWANDDOWN,            // downstate
    HERETIC_S_GOLDWANDREADY,           // readystate
    HERETIC_S_GOLDWANDATK2_1,          // atkstate
    HERETIC_S_GOLDWANDATK2_1,          // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_GWND_AMMO_2,                   // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Crossbow
    am_crossbow,               // ammo
    HERETIC_S_CRBOWUP,                 // upstate
    HERETIC_S_CRBOWDOWN,               // downstate
    HERETIC_S_CRBOW1,                  // readystate
    HERETIC_S_CRBOWATK2_1,             // atkstate
    HERETIC_S_CRBOWATK2_1,             // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_CBOW_AMMO_2,                   // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Blaster
    am_blaster,                // ammo
    HERETIC_S_BLASTERUP,               // upstate
    HERETIC_S_BLASTERDOWN,             // downstate
    HERETIC_S_BLASTERREADY,            // readystate
    HERETIC_S_BLASTERATK2_1,           // atkstate
    HERETIC_S_BLASTERATK2_3,           // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_BLSR_AMMO_2,                   // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Skull rod
    am_skullrod,               // ammo
    HERETIC_S_HORNRODUP,               // upstate
    HERETIC_S_HORNRODDOWN,             // downstate
    HERETIC_S_HORNRODREADY,            // readystae
    HERETIC_S_HORNRODATK2_1,           // atkstate
    HERETIC_S_HORNRODATK2_1,           // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_SKRD_AMMO_2,                   // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Phoenix rod
    am_phoenixrod,             // ammo
    HERETIC_S_PHOENIXUP,               // upstate
    HERETIC_S_PHOENIXDOWN,             // downstate
    HERETIC_S_PHOENIXREADY,            // readystate
    HERETIC_S_PHOENIXATK2_1,           // atkstate
    HERETIC_S_PHOENIXATK2_2,           // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_PHRD_AMMO_2,                   // ammopershot
    0,                                 // intflags
    WPF_NOAUTOFIRE
  },
  {                           // Mace
    am_mace,                   // ammo
    HERETIC_S_MACEUP,                  // upstate
    HERETIC_S_MACEDOWN,                // downstate
    HERETIC_S_MACEREADY,               // readystate
    HERETIC_S_MACEATK2_1,              // atkstate
    HERETIC_S_MACEATK2_1,              // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    USE_MACE_AMMO_2,                   // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  },
  {                           // Gauntlets
    am_noammo,                 // ammo
    HERETIC_S_GAUNTLETUP2,             // upstate
    HERETIC_S_GAUNTLETDOWN2,           // downstate
    HERETIC_S_GAUNTLETREADY2_1,        // readystate
    HERETIC_S_GAUNTLETATK2_1,          // atkstate
    HERETIC_S_GAUNTLETATK2_3,          // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    0,                                 // ammopershot
    0,                                 // intflags
    WPF_NOTHRUST
  },
  {                           // Beak
    am_noammo,                 // ammo
    HERETIC_S_BEAKUP,                  // upstate
    HERETIC_S_BEAKDOWN,                // downstate
    HERETIC_S_BEAKREADY,               // readystate
    HERETIC_S_BEAKATK2_1,              // atkstate
    HERETIC_S_BEAKATK2_1,              // holdatkstate
    HERETIC_S_NULL,                    // flashstate
    0,                                 // ammopershot
    0,                                 // intflags
    WPF_NOFLAG
  }
};
