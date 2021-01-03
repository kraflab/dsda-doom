//
// Copyright(C) 2020 by Ryan Krafnick
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
// DESCRIPTION:
//	DSDA Global - define top level globals for doom vs heretic
//

#include <stdlib.h>
#include <string.h>

#include "m_argv.h"
#include "info.h"
#include "d_items.h"
#include "p_inter.h"
#include "sounds.h"
#include "heretic/def.h"

#include "global.h"

#define IGNORE_VALUE -1

state_t* states;
int num_states;

const char** sprnames;
int num_sprites;

mobjinfo_t* mobjinfo;
int num_mobj_types;
int mobj_types_zero;
int mobj_types_max;

sfxinfo_t* S_sfx;
int num_sfx;
musicinfo_t* S_music;
int num_music;

weaponinfo_t* weaponinfo;

int g_mt_player;
int g_mt_tfog;
int g_skullpop_mt;

int g_wp_fist;
int g_wp_chainsaw;

int g_telefog_height;
int g_thrust_factor;
int g_fuzzy_aim_shift;
int g_special_friction_low;

int g_s_play_atk1;
int g_s_play_atk2;
int g_s_play_run1;
int g_s_play;

int g_sfx_sawup;
int g_sfx_telept;
int g_sfx_stnmov;

extern const char** S_music_files;

static void dsda_AllocateMobjInfo(int zero, int max, int count) {
  num_mobj_types = count;
  mobj_types_zero = zero;
  mobj_types_max = max;
  
  mobjinfo = malloc(sizeof(mobjinfo_t) * num_mobj_types);
  memset(mobjinfo, 0, sizeof(mobjinfo_t) * num_mobj_types);
}

static void dsda_SetStates(state_t* state_list, int count) {
  states = state_list;
  num_states = count;
}

static void dsda_SetSpriteNames(const char** sprite_name_list, int count) {
  sprnames = sprite_name_list;
  num_sprites = count;
}

static void dsda_SetSfx(sfxinfo_t* sfx_list, int count) {
  S_sfx = sfx_list;
  num_sfx = count;
}

static void dsda_SetMusic(musicinfo_t* music_list, int count) {
  S_music = music_list;
  num_music = count;
  S_music_files = malloc(sizeof(char *) * num_music);
  memset(S_music_files, 0, sizeof(char *) * num_music);
} 

static void dsda_InitDoom(void) {
  int i;
  doom_mobjinfo_t* mobjinfo_p;

  dsda_AllocateMobjInfo(0, NUMMOBJTYPES, NUMMOBJTYPES);
  dsda_SetStates(doom_states, NUMSTATES);
  dsda_SetSpriteNames(doom_sprnames, NUMSPRITES);
  dsda_SetSfx(doom_S_sfx, NUMSFX);
  dsda_SetMusic(doom_S_music, NUMMUSIC);
  
  weaponinfo = doom_weaponinfo;
  
  g_mt_player = MT_PLAYER;
  g_mt_tfog = MT_TFOG;
  g_skullpop_mt = MT_GIBDTH;
  
  g_wp_fist = wp_fist;
  g_wp_chainsaw = wp_chainsaw;
  
  g_telefog_height = 0;
  g_thrust_factor = 100;
  g_fuzzy_aim_shift = 20;
  g_special_friction_low = IGNORE_VALUE;
  
  g_s_play_atk1 = S_PLAY_ATK1;
  g_s_play_atk2 = S_PLAY_ATK2;
  g_s_play_run1 = S_PLAY_RUN1;
  g_s_play = S_PLAY;
  
  g_sfx_sawup = sfx_sawup;
  g_sfx_telept = sfx_telept;
  g_sfx_stnmov = sfx_stnmov;

  // convert doom mobj types to shared type
  for (i = 0; i < NUMMOBJTYPES; ++i) {
    mobjinfo_p = &doom_mobjinfo[i];
    
    mobjinfo[i].doomednum    = mobjinfo_p->doomednum;
    mobjinfo[i].spawnstate   = mobjinfo_p->spawnstate;
    mobjinfo[i].spawnhealth  = mobjinfo_p->spawnhealth;
    mobjinfo[i].seestate     = mobjinfo_p->seestate;
    mobjinfo[i].seesound     = mobjinfo_p->seesound;
    mobjinfo[i].reactiontime = mobjinfo_p->reactiontime;
    mobjinfo[i].attacksound  = mobjinfo_p->attacksound;
    mobjinfo[i].painstate    = mobjinfo_p->painstate;
    mobjinfo[i].painchance   = mobjinfo_p->painchance;
    mobjinfo[i].painsound    = mobjinfo_p->painsound;
    mobjinfo[i].meleestate   = mobjinfo_p->meleestate;
    mobjinfo[i].missilestate = mobjinfo_p->missilestate;
    mobjinfo[i].deathstate   = mobjinfo_p->deathstate;
    mobjinfo[i].xdeathstate  = mobjinfo_p->xdeathstate;
    mobjinfo[i].deathsound   = mobjinfo_p->deathsound;
    mobjinfo[i].speed        = mobjinfo_p->speed;
    mobjinfo[i].radius       = mobjinfo_p->radius;
    mobjinfo[i].height       = mobjinfo_p->height;
    mobjinfo[i].mass         = mobjinfo_p->mass;
    mobjinfo[i].damage       = mobjinfo_p->damage;
    mobjinfo[i].activesound  = mobjinfo_p->activesound;
    mobjinfo[i].flags        = mobjinfo_p->flags;
    mobjinfo[i].raisestate   = mobjinfo_p->raisestate;
    mobjinfo[i].droppeditem  = mobjinfo_p->droppeditem;
    mobjinfo[i].crashstate   = 0; // not in doom
    mobjinfo[i].flags2       = 0; // not in doom
  }
}

static void dsda_InitHeretic(void) {
  int i, j;
  heretic_mobjinfo_t* mobjinfo_p;

  dsda_AllocateMobjInfo(HERETIC_MT_ZERO, HERETIC_NUMMOBJTYPES, TOTAL_NUMMOBJTYPES);
  dsda_SetStates(heretic_states, HERETIC_NUMSTATES);
  dsda_SetSpriteNames(heretic_sprnames, HERETIC_NUMSPRITES);
  dsda_SetSfx(heretic_S_sfx, HERETIC_NUMSFX);
  dsda_SetMusic(heretic_S_music, HERETIC_NUMMUSIC);
  
  // HERETIC_TODO: of course, 2 levels requires complete rework...
  weaponinfo = wpnlev1info;
  
  g_mt_player = HERETIC_MT_PLAYER;
  g_mt_tfog = HERETIC_MT_TFOG;
  g_skullpop_mt = HERETIC_MT_BLOODYSKULL;
  
  g_wp_fist = wp_staff;
  g_wp_chainsaw = wp_gauntlets;
  
  g_telefog_height = TELEFOGHEIGHT;
  g_thrust_factor = 150;
  g_fuzzy_aim_shift = 21;
  g_special_friction_low = 15;
  
  g_s_play_atk1 = HERETIC_S_PLAY_ATK1;
  g_s_play_atk2 = HERETIC_S_PLAY_ATK2;
  g_s_play_run1 = HERETIC_S_PLAY_RUN1;
  g_s_play = HERETIC_S_PLAY;
  
  g_sfx_sawup = heretic_sfx_gntact;
  g_sfx_telept = heretic_sfx_telept;
  g_sfx_stnmov = heretic_sfx_dormov;
  
  // convert heretic mobj types to shared type
  for (i = 0; i < HERETIC_NUMMOBJTYPES - HERETIC_MT_ZERO; ++i) {
    mobjinfo_p = &heretic_mobjinfo[i];
    
    j = i + HERETIC_MT_ZERO;
    mobjinfo[j].doomednum    = mobjinfo_p->doomednum;
    mobjinfo[j].spawnstate   = mobjinfo_p->spawnstate;
    mobjinfo[j].spawnhealth  = mobjinfo_p->spawnhealth;
    mobjinfo[j].seestate     = mobjinfo_p->seestate;
    mobjinfo[j].seesound     = mobjinfo_p->seesound;
    mobjinfo[j].reactiontime = mobjinfo_p->reactiontime;
    mobjinfo[j].attacksound  = mobjinfo_p->attacksound;
    mobjinfo[j].painstate    = mobjinfo_p->painstate;
    mobjinfo[j].painchance   = mobjinfo_p->painchance;
    mobjinfo[j].painsound    = mobjinfo_p->painsound;
    mobjinfo[j].meleestate   = mobjinfo_p->meleestate;
    mobjinfo[j].missilestate = mobjinfo_p->missilestate;
    mobjinfo[j].deathstate   = mobjinfo_p->deathstate;
    mobjinfo[j].xdeathstate  = mobjinfo_p->xdeathstate;
    mobjinfo[j].deathsound   = mobjinfo_p->deathsound;
    mobjinfo[j].speed        = mobjinfo_p->speed;
    mobjinfo[j].radius       = mobjinfo_p->radius;
    mobjinfo[j].height       = mobjinfo_p->height;
    mobjinfo[j].mass         = mobjinfo_p->mass;
    mobjinfo[j].damage       = mobjinfo_p->damage;
    mobjinfo[j].activesound  = mobjinfo_p->activesound;
    mobjinfo[j].flags        = mobjinfo_p->flags;
    mobjinfo[j].raisestate   = 0; // not in heretic
    mobjinfo[j].droppeditem  = 0; // not in heretic
    mobjinfo[j].crashstate   = mobjinfo_p->crashstate;
    mobjinfo[j].flags2       = mobjinfo_p->flags2;
  }
  
  // heretic doesn't use "clip" concept
  for (i = 0; i < NUMAMMO; ++i) clipammo[i] = 1;
  
  // so few it's not worth implementing a pointer swap
  maxammo[0] = 100; // gold wand
  maxammo[1] = 50;  // crossbow
  maxammo[2] = 200; // blaster
  maxammo[3] = 200; // skull rod
  maxammo[4] = 20;  // phoenix rod
  maxammo[5] = 150; // mace
}

void dsda_InitGlobal(void) {
  heretic = M_CheckParm ("-heretic");
  
  heretic ? dsda_InitHeretic() : dsda_InitDoom();
}
