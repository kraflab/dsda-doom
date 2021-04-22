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
#include "p_spec.h"
#include "sounds.h"
#include "d_main.h"
#include "v_video.h"
#include "hu_stuff.h"
#include "heretic/def.h"

#include "global.h"

#define IGNORE_VALUE -1

const demostate_t (*demostates)[4];
extern const demostate_t doom_demostates[][4];
extern const demostate_t heretic_demostates[][4];

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
int g_mt_blood;
int g_skullpop_mt;

int g_wp_fist;
int g_wp_chainsaw;
int g_wp_pistol;

int g_telefog_height;
int g_thrust_factor;
int g_fuzzy_aim_shift;
int g_special_friction_low;

int g_s_play_atk1;
int g_s_play_atk2;
int g_s_play_run1;
int g_s_play;
int g_s_null;

int g_sfx_sawup;
int g_sfx_telept;
int g_sfx_stnmov;
int g_sfx_stnmov_plats;
int g_sfx_swtchn;
int g_sfx_swtchx;
int g_sfx_dorcls;
int g_sfx_doropn;
int g_sfx_pstart;
int g_sfx_pstop;
int g_sfx_itemup;
int g_sfx_pistol;
int g_sfx_oof;
int g_sfx_menu;

int g_door_normal;
int g_door_raise_in_5_mins;
int g_door_open;

int g_st_height;
int g_border_offset;
int g_mf_translucent;
int g_mf_shadow;

int g_cr_gray;
int g_cr_green;
int g_cr_gold;
int g_cr_red;
int g_cr_blue;

const char* g_menu_flat;
patchnum_t* g_menu_font;
int g_menu_save_page_size;
int g_menu_font_spacing;
int g_menu_cr_title;
int g_menu_cr_set;
int g_menu_cr_item;
int g_menu_cr_hilite;
int g_menu_cr_select;
int g_menu_cr_disable;

extern patchnum_t hu_font[HU_FONTSIZE];
extern patchnum_t hu_font2[HU_FONTSIZE];

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
}

static void dsda_InitDoom(void) {
  int i;
  doom_mobjinfo_t* mobjinfo_p;

  dsda_AllocateMobjInfo(0, NUMMOBJTYPES, NUMMOBJTYPES);
  dsda_SetStates(doom_states, NUMSTATES);
  dsda_SetSpriteNames(doom_sprnames, NUMSPRITES);
  dsda_SetSfx(doom_S_sfx, NUMSFX);
  dsda_SetMusic(doom_S_music, NUMMUSIC);

  demostates = doom_demostates;

  weaponinfo = doom_weaponinfo;

  g_mt_player = MT_PLAYER;
  g_mt_tfog = MT_TFOG;
  g_mt_blood = MT_BLOOD;
  g_skullpop_mt = MT_GIBDTH;

  g_wp_fist = wp_fist;
  g_wp_chainsaw = wp_chainsaw;
  g_wp_pistol = wp_pistol;

  g_telefog_height = 0;
  g_thrust_factor = 100;
  g_fuzzy_aim_shift = 20;
  g_special_friction_low = IGNORE_VALUE;

  g_s_play_atk1 = S_PLAY_ATK1;
  g_s_play_atk2 = S_PLAY_ATK2;
  g_s_play_run1 = S_PLAY_RUN1;
  g_s_play = S_PLAY;
  g_s_null = S_NULL;

  g_sfx_sawup = sfx_sawup;
  g_sfx_telept = sfx_telept;
  g_sfx_stnmov = sfx_stnmov;
  g_sfx_stnmov_plats = sfx_stnmov;
  g_sfx_swtchn = sfx_swtchn;
  g_sfx_swtchx = sfx_swtchx;
  g_sfx_dorcls = sfx_dorcls;
  g_sfx_doropn = sfx_doropn;
  g_sfx_pstart = sfx_pstart;
  g_sfx_pstop = sfx_pstop;
  g_sfx_itemup = sfx_itemup;
  g_sfx_pistol = sfx_pistol;
  g_sfx_oof = sfx_oof;
  g_sfx_menu = sfx_pstop;

  g_door_normal = normal;
  g_door_raise_in_5_mins = raiseIn5Mins;
  g_door_open = openDoor;

  g_st_height = 32;
  g_border_offset = 8;
  g_mf_translucent = MF_TRANSLUCENT;
  g_mf_shadow = MF_SHADOW;

  g_cr_gray = CR_GRAY;
  g_cr_green = CR_GREEN;
  g_cr_gold = CR_GOLD;
  g_cr_red = CR_RED;
  g_cr_blue = CR_BLUE;

  g_menu_flat = "FLOOR4_6";
  g_menu_font = hu_font;
  g_menu_save_page_size = 7;
  g_menu_font_spacing = -1;
  g_menu_cr_title = CR_GOLD;
  g_menu_cr_set = CR_GREEN;
  g_menu_cr_item = CR_RED;
  g_menu_cr_hilite = CR_ORANGE;
  g_menu_cr_select = CR_GRAY;
  g_menu_cr_disable = CR_GRAY;

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

    // mbf21
    mobjinfo[i].infighting_group = IG_DEFAULT;
    mobjinfo[i].projectile_group = PG_DEFAULT;
    mobjinfo[i].splash_group = SG_DEFAULT;
    mobjinfo[i].ripsound = sfx_None;
  }

  // don't want to reorganize info.c structure for a few tweaks...
  mobjinfo[MT_VILE].flags2    = MF2_SHORTMRANGE | MF2_DMGIGNORED | MF2_NOTHRESHOLD;
  mobjinfo[MT_CYBORG].flags2  = MF2_NORADIUSDMG | MF2_HIGHERMPROB | MF2_RANGEHALF |
                                MF2_BOSS | MF2_E2M8BOSS | MF2_E4M6BOSS;
  mobjinfo[MT_SPIDER].flags2  = MF2_NORADIUSDMG | MF2_RANGEHALF | MF2_BOSS |
                                MF2_E3M8BOSS | MF2_E4M8BOSS;
  mobjinfo[MT_SKULL].flags2   = MF2_RANGEHALF;
  mobjinfo[MT_FATSO].flags2   = MF2_MAP07BOSS1;
  mobjinfo[MT_BABY].flags2    = MF2_MAP07BOSS2;
  mobjinfo[MT_BRUISER].flags2 = MF2_E1M8BOSS;
  mobjinfo[MT_UNDEAD].flags2  = MF2_LONGMELEE | MF2_RANGEHALF;

  mobjinfo[MT_BARREL].flags2 = MF2_NEUTRAL_SPLASH;

  mobjinfo[MT_BRUISER].projectile_group = PG_BARON;
  mobjinfo[MT_KNIGHT].projectile_group = PG_BARON;
}

static void dsda_InitHeretic(void) {
  int i, j;
  heretic_mobjinfo_t* mobjinfo_p;

  dsda_AllocateMobjInfo(HERETIC_MT_ZERO, HERETIC_NUMMOBJTYPES, TOTAL_NUMMOBJTYPES);
  dsda_SetStates(heretic_states, HERETIC_NUMSTATES);
  dsda_SetSpriteNames(heretic_sprnames, HERETIC_NUMSPRITES);
  dsda_SetSfx(heretic_S_sfx, HERETIC_NUMSFX);
  dsda_SetMusic(heretic_S_music, HERETIC_NUMMUSIC);

  demostates = heretic_demostates;

  weaponinfo = wpnlev1info;

  g_mt_player = HERETIC_MT_PLAYER;
  g_mt_tfog = HERETIC_MT_TFOG;
  g_mt_blood = HERETIC_MT_BLOOD;
  g_skullpop_mt = HERETIC_MT_BLOODYSKULL;

  g_wp_fist = wp_staff;
  g_wp_chainsaw = wp_gauntlets;
  g_wp_pistol = wp_goldwand;

  g_telefog_height = TELEFOGHEIGHT;
  g_thrust_factor = 150;
  g_fuzzy_aim_shift = 21;
  g_special_friction_low = 15;

  g_s_play_atk1 = HERETIC_S_PLAY_ATK1;
  g_s_play_atk2 = HERETIC_S_PLAY_ATK2;
  g_s_play_run1 = HERETIC_S_PLAY_RUN1;
  g_s_play = HERETIC_S_PLAY;
  g_s_null = HERETIC_S_NULL;

  g_sfx_sawup = heretic_sfx_gntact;
  g_sfx_telept = heretic_sfx_telept;
  g_sfx_stnmov = heretic_sfx_dormov;
  g_sfx_stnmov_plats = heretic_sfx_stnmov;
  g_sfx_swtchn = heretic_sfx_switch;
  g_sfx_swtchx = heretic_sfx_switch;
  g_sfx_dorcls = heretic_sfx_doropn;
  g_sfx_doropn = heretic_sfx_doropn;
  g_sfx_pstart = heretic_sfx_pstart;
  g_sfx_pstop = heretic_sfx_pstop;
  g_sfx_itemup = heretic_sfx_itemup;
  g_sfx_pistol = heretic_sfx_gldhit;
  g_sfx_oof = heretic_sfx_plroof;
  g_sfx_menu = heretic_sfx_dorcls;

  g_door_normal = vld_normal;
  g_door_raise_in_5_mins = vld_raiseIn5Mins;
  g_door_open = vld_open;

  g_st_height = 42;
  g_border_offset = 4;
  g_mf_translucent = MF_SHADOW;
  g_mf_shadow = 0; // doesn't exist in heretic

  g_cr_gray = CR_TAN;
  g_cr_green = CR_YELLOW;
  g_cr_gold = CR_ORANGE;
  g_cr_red = CR_GOLD;
  g_cr_blue = CR_BROWN;

  g_menu_flat = "FLOOR30";
  g_menu_font = hu_font2;
  g_menu_save_page_size = 5;
  g_menu_font_spacing = 0;
  g_menu_cr_title = g_cr_gold;
  g_menu_cr_set = g_cr_green;
  g_menu_cr_item = g_cr_red;
  g_menu_cr_hilite = g_cr_blue;
  g_menu_cr_select = g_cr_gray;
  g_menu_cr_disable = g_cr_gray;

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

    // mbf21
    mobjinfo[j].infighting_group = IG_DEFAULT;
    mobjinfo[j].projectile_group = PG_DEFAULT;
    mobjinfo[j].splash_group = SG_DEFAULT;
    mobjinfo[i].ripsound = heretic_sfx_None;
  }

  // don't want to reorganize info.c structure for a few tweaks...
  mobjinfo[HERETIC_MT_SORCERER2].infighting_group = IG_WIZARD;
  mobjinfo[HERETIC_MT_WIZARD].infighting_group = IG_WIZARD;

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

static dboolean dsda_AutoDetectHeretic(void)
{
  int i, length;
  i = M_CheckParm("-iwad");
  if (i && (++i < myargc)) {
    length = strlen(myargv[i]);
    if (length >= 11 && !strnicmp(myargv[i] + length - 11, "heretic.wad", 11))
      return true;
  }

  return false;
}

void dsda_InitGlobal(void) {
  heretic = M_CheckParm("-heretic") || dsda_AutoDetectHeretic();

  heretic ? dsda_InitHeretic() : dsda_InitDoom();
}
