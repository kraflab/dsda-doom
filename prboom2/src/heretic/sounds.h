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

// sounds.h

#ifndef __SOUNDSH__
#define __SOUNDSH__

#include "i_sound.h"

#define MAX_SND_DIST 	1600
#define MAX_CHANNELS	16

// Music identifiers

typedef enum
{
    heretic_mus_e1m1,
    heretic_mus_e1m2,
    heretic_mus_e1m3,
    heretic_mus_e1m4,
    heretic_mus_e1m5,
    heretic_mus_e1m6,
    heretic_mus_e1m7,
    heretic_mus_e1m8,
    heretic_mus_e1m9,

    heretic_mus_e2m1,
    heretic_mus_e2m2,
    heretic_mus_e2m3,
    heretic_mus_e2m4,
    heretic_mus_e2m5,
    heretic_mus_e2m6,
    heretic_mus_e2m7,
    heretic_mus_e2m8,
    heretic_mus_e2m9,

    heretic_mus_e3m1,
    heretic_mus_e3m2,
    heretic_mus_e3m3,
    heretic_mus_e3m4,
    heretic_mus_e3m5,
    heretic_mus_e3m6,
    heretic_mus_e3m7,
    heretic_mus_e3m8,
    heretic_mus_e3m9,

    heretic_mus_e4m1,
    heretic_mus_e4m2,
    heretic_mus_e4m3,
    heretic_mus_e4m4,
    heretic_mus_e4m5,
    heretic_mus_e4m6,
    heretic_mus_e4m7,
    heretic_mus_e4m8,
    heretic_mus_e4m9,

    heretic_mus_e5m1,
    heretic_mus_e5m2,
    heretic_mus_e5m3,
    heretic_mus_e5m4,
    heretic_mus_e5m5,
    heretic_mus_e5m6,
    heretic_mus_e5m7,
    heretic_mus_e5m8,
    heretic_mus_e5m9,

    heretic_mus_e6m1,
    heretic_mus_e6m2,
    heretic_mus_e6m3,

    heretic_mus_titl,
    heretic_mus_intr,
    heretic_mus_cptd,
    HERETIC_NUMMUSIC
} heretic_musicenum_t;

typedef struct
{
    mobj_t *mo;
    int sound_id;
    int handle;
    int pitch;
    int priority;
} heretic_channel_t;

typedef struct
{
    int id;
    unsigned short priority;
    char *name;
    mobj_t *mo;
    int distance;
} ChanInfo_t;

typedef struct
{
    int channelCount;
    int musicVolume;
    int soundVolume;
    ChanInfo_t chan[8];
} SoundInfo_t;

// Sound identifiers

typedef enum
{
    heretic_sfx_None,
    heretic_sfx_gldhit,
    heretic_sfx_gntful,
    heretic_sfx_gnthit,
    heretic_sfx_gntpow,
    heretic_sfx_gntact,
    heretic_sfx_gntuse,
    heretic_sfx_phosht,
    heretic_sfx_phohit,
    heretic_sfx_phopow,
    heretic_sfx_lobsht,
    heretic_sfx_lobhit,
    heretic_sfx_lobpow,
    heretic_sfx_hrnsht,
    heretic_sfx_hrnhit,
    heretic_sfx_hrnpow,
    heretic_sfx_ramphit,
    heretic_sfx_ramrain,
    heretic_sfx_bowsht,
    heretic_sfx_stfhit,
    heretic_sfx_stfpow,
    heretic_sfx_stfcrk,
    heretic_sfx_impsit,
    heretic_sfx_impat1,
    heretic_sfx_impat2,
    heretic_sfx_impdth,
    heretic_sfx_impact,
    heretic_sfx_imppai,
    heretic_sfx_mumsit,
    heretic_sfx_mumat1,
    heretic_sfx_mumat2,
    heretic_sfx_mumdth,
    heretic_sfx_mumact,
    heretic_sfx_mumpai,
    heretic_sfx_mumhed,
    heretic_sfx_bstsit,
    heretic_sfx_bstatk,
    heretic_sfx_bstdth,
    heretic_sfx_bstact,
    heretic_sfx_bstpai,
    heretic_sfx_clksit,
    heretic_sfx_clkatk,
    heretic_sfx_clkdth,
    heretic_sfx_clkact,
    heretic_sfx_clkpai,
    heretic_sfx_snksit,
    heretic_sfx_snkatk,
    heretic_sfx_snkdth,
    heretic_sfx_snkact,
    heretic_sfx_snkpai,
    heretic_sfx_kgtsit,
    heretic_sfx_kgtatk,
    heretic_sfx_kgtat2,
    heretic_sfx_kgtdth,
    heretic_sfx_kgtact,
    heretic_sfx_kgtpai,
    heretic_sfx_wizsit,
    heretic_sfx_wizatk,
    heretic_sfx_wizdth,
    heretic_sfx_wizact,
    heretic_sfx_wizpai,
    heretic_sfx_minsit,
    heretic_sfx_minat1,
    heretic_sfx_minat2,
    heretic_sfx_minat3,
    heretic_sfx_mindth,
    heretic_sfx_minact,
    heretic_sfx_minpai,
    heretic_sfx_hedsit,
    heretic_sfx_hedat1,
    heretic_sfx_hedat2,
    heretic_sfx_hedat3,
    heretic_sfx_heddth,
    heretic_sfx_hedact,
    heretic_sfx_hedpai,
    heretic_sfx_sorzap,
    heretic_sfx_sorrise,
    heretic_sfx_sorsit,
    heretic_sfx_soratk,
    heretic_sfx_soract,
    heretic_sfx_sorpai,
    heretic_sfx_sordsph,
    heretic_sfx_sordexp,
    heretic_sfx_sordbon,
    heretic_sfx_sbtsit,
    heretic_sfx_sbtatk,
    heretic_sfx_sbtdth,
    heretic_sfx_sbtact,
    heretic_sfx_sbtpai,
    heretic_sfx_plroof,
    heretic_sfx_plrpai,
    heretic_sfx_plrdth,                 // Normal
    heretic_sfx_gibdth,                 // Extreme
    heretic_sfx_plrwdth,                // Wimpy
    heretic_sfx_plrcdth,                // Crazy
    heretic_sfx_itemup,
    heretic_sfx_wpnup,
    heretic_sfx_telept,
    heretic_sfx_doropn,
    heretic_sfx_dorcls,
    heretic_sfx_dormov,
    heretic_sfx_artiup,
    heretic_sfx_switch,
    heretic_sfx_pstart,
    heretic_sfx_pstop,
    heretic_sfx_stnmov,
    heretic_sfx_chicpai,
    heretic_sfx_chicatk,
    heretic_sfx_chicdth,
    heretic_sfx_chicact,
    heretic_sfx_chicpk1,
    heretic_sfx_chicpk2,
    heretic_sfx_chicpk3,
    heretic_sfx_keyup,
    heretic_sfx_ripslop,
    heretic_sfx_newpod,
    heretic_sfx_podexp,
    heretic_sfx_bounce,
    heretic_sfx_volsht,
    heretic_sfx_volhit,
    heretic_sfx_burn,
    heretic_sfx_splash,
    heretic_sfx_gloop,
    heretic_sfx_respawn,
    heretic_sfx_blssht,
    heretic_sfx_blshit,
    heretic_sfx_chat,
    heretic_sfx_artiuse,
    heretic_sfx_gfrag,
    heretic_sfx_waterfl,

    // Monophonic sounds

    heretic_sfx_wind,
    heretic_sfx_amb1,
    heretic_sfx_amb2,
    heretic_sfx_amb3,
    heretic_sfx_amb4,
    heretic_sfx_amb5,
    heretic_sfx_amb6,
    heretic_sfx_amb7,
    heretic_sfx_amb8,
    heretic_sfx_amb9,
    heretic_sfx_amb10,
    heretic_sfx_amb11,
    HERETIC_NUMSFX
} heretic_sfxenum_t;

extern sfxinfo_t heretic_S_sfx[];
extern musicinfo_t heretic_S_music[][2];

#endif
