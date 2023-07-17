//
// Copyright(C) 2023 by Ryan Krafnick
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
//  DSDA MapInfo Doom
//

#include "doomstat.h"
#include "g_game.h"
#include "lprintf.h"
#include "p_enemy.h"
#include "r_data.h"
#include "s_sound.h"
#include "sounds.h"
#include "v_video.h"
#include "w_wad.h"

#include "dsda/args.h"
#include "dsda/map_format.h"
#include "dsda/mapinfo.h"
#include "dsda/mapinfo/doom/parser.h"

#include "doom.h"

static const doom_mapinfo_map_t* last_map;
static const doom_mapinfo_map_t* next_map;
static const doom_mapinfo_map_t* current_map;
static const doom_mapinfo_map_next_t* end_data;

static doom_mapinfo_map_next_t default_end_data = {
  .end = dmi_end_game_cast
};

static doom_mapinfo_map_t* dsda_DoomMapEntry(int gamemap) {
  int i;

  for (i = 0; i < doom_mapinfo.num_maps; ++i)
    if (gamemap == doom_mapinfo.maps[i].level_num)
      return &doom_mapinfo.maps[i];

  return NULL;
}

static doom_mapinfo_map_t* dsda_DoomMapEntryByName(const char* name) {
  int i;

  for (i = 0; i < doom_mapinfo.num_maps; ++i)
    if (!stricmp(doom_mapinfo.maps[i].lump_name, name))
      return &doom_mapinfo.maps[i];

  return NULL;
}

int dsda_DoomFirstMap(int* episode, int* map) {
  return false; // TODO
}

int dsda_DoomNewGameMap(int* episode, int* map) {
  return false; // TODO
}

int dsda_DoomResolveWarp(int* args, int arg_count, int* episode, int* map) {
  const doom_mapinfo_map_t* entry;

  if (!doom_mapinfo.loaded)
    return false;

  entry = dsda_DoomMapEntry(*map);

  if (!entry)
    return false;

  *map = entry->level_num;
  *episode = 1; // TODO: warp episode

  return true;
}

int dsda_DoomNextMap(int* episode, int* map) {
  const char* next = NULL;
  const doom_mapinfo_map_t* entry = NULL;

  if (!current_map)
    return false;

  if (current_map->secret_next.map)
    next = current_map->secret_next.map;

  if (!next)
    next = current_map->next.map;

  if (next)
    entry = dsda_DoomMapEntryByName(next);

  // TODO: next map default
  if (!entry) {
    *episode = 1;
    *map = 1;
  }
  else {
    *map = entry->level_num;
    *episode = 1; // TODO: next map episode
  }

  return true;
}

int dsda_DoomShowNextLocBehaviour(int* behaviour) {
  if (!current_map)
    return false;

  if (end_data)
    *behaviour = WI_SHOW_NEXT_DONE;
  else
    *behaviour = WI_SHOW_NEXT_LOC | WI_SHOW_NEXT_EPISODAL;

  return true;
}

int dsda_DoomSkipDrawShowNextLoc(int* skip) {
  if (!current_map)
    return false;

  *skip = !!end_data;

  return true;
}

void dsda_DoomUpdateMapInfo(void) {
  current_map = dsda_DoomMapEntry(gamemap);
}

void dsda_DoomUpdateLastMapInfo(void) {
  last_map = current_map;
  next_map = NULL;
}

void dsda_DoomUpdateNextMapInfo(void) {
  next_map = dsda_DoomMapEntry(wminfo.next + 1);
}

int dsda_DoomResolveCLEV(int* clev, int* episode, int* map) {
  return false; // TODO
}

int dsda_DoomResolveINIT(int* init) {
  return false; // TODO
}

int dsda_DoomMusicIndexToLumpNum(int* lump, int music_index) {
  return false; // TODO
}

int dsda_DoomMapMusic(int* music_index, int* music_lump) {
  int lump;

  if (!current_map || !current_map->music)
    return false;

  lump = W_CheckNumForName(current_map->music);

  if (lump == LUMP_NOT_FOUND)
    return false;

  *music_index = -1;
  *music_lump = lump;

  return true;
}

int dsda_DoomIntermissionMusic(int* music_index, int* music_lump) {
  int lump;

  if (!last_map || !last_map->inter_music)
    return false;

  lump = W_CheckNumForName(last_map->inter_music);

  if (lump == LUMP_NOT_FOUND)
    return false;

  *music_index = -1;
  *music_lump = lump;

  return true;
}

int dsda_DoomInterMusic(int* music_index, int* music_lump) {
  return false; // TODO
}

extern int finalestage;
extern int finalecount;
extern const char* finaletext;
extern const char* finaleflat;
extern const char* finalepatch;
extern int acceleratestage;
extern int midstage;

// TODO: use end_game everywhere and collapse all the finale implementations

int dsda_DoomStartFinale(void) {
  if (!current_map)
    return false;

  // TODO: cluster end text
  finaletext = NULL;
  finalepatch = NULL;
  finaleflat = NULL;

  return true;
}

int dsda_DoomFTicker(void) {
  void WI_checkForAccelerate(void);
  float Get_TextSpeed(void);
  void F_StartCast (void);

  int next_level = false;
  const int TEXTSPEED = 3;
  const int TEXTWAIT = 250;
  const int NEWTEXTWAIT = 1000;

  if (!demo_compatibility)
    WI_checkForAccelerate();
  else {
    int i;

    for (i = 0; i < g_maxplayers; i++)
      if (players[i].cmd.buttons)
        next_level = true;
  }

  if (!next_level) {
    // advance animation
    finalecount++;

    if (!finalestage) {
      float speed = demo_compatibility ? TEXTSPEED : Get_TextSpeed();

      if (
        !finaletext ||
        finalecount > strlen(finaletext) * speed + (midstage ? NEWTEXTWAIT : TEXTWAIT) ||
        (midstage && acceleratestage)
      )
        next_level = true;
    }
  }

  if (next_level) {
    if (end_data) {
      if (end_data->end == dmi_end_game_cast) {
        F_StartCast(); // TODO: need to support cast in doom 1
        return false; // let go of finale ownership
      }
      else {
        finalecount = 0;
        finalestage = 1;
        wipegamestate = -1; // force a wipe
        if (end_data->end == dmi_end_game_scroll && gamemode != commercial)
          S_StartMusic(mus_bunny); // TODO: what music in doom 2?
        return true; // keep finale ownership
      }
    }
    else
      gameaction = ga_worlddone; // next level, e.g. MAP07
  }

  return true; // keep finale ownership
}

void dsda_DoomFDrawer(void) {
  void F_TextWrite(void);
  void F_BunnyScroll(void);

  if (!finalestage || !end_data)
    F_TextWrite();
  else if (end_data->end == dmi_end_game_scroll)
    F_BunnyScroll(); // TODO: need to support bunny in doom 2
  else {
    // e6y: wide-res
    V_ClearBorder();
    V_DrawNamePatch(0, 0, 0, end_data->end_pic, CR_DEFAULT, VPT_STRETCH);
  }
}

int dsda_DoomBossAction(mobj_t* mo) {
  int i;

  if (!current_map)
    return false;

  for (i = 0; i < current_map->num_special_actions; ++i)
    if (current_map->special_actions[i].monster_type == mo->type)
      break;

  if (i == current_map->num_special_actions)
    return true;

  if (!P_CheckBossDeath(mo))
    return true;

  for (i = 0; i < current_map->num_special_actions; ++i)
    if (current_map->special_actions[i].monster_type == mo->type)
      map_format.execute_line_special(
        current_map->special_actions[i].action_special,
        current_map->special_actions[i].special_args,
        NULL, 0, mo
      );

  return true;
}

int dsda_DoomMapLumpName(const char** name, int episode, int map) {
  const doom_mapinfo_map_t* target_map;

  if (!doom_mapinfo.loaded)
    return false;

  target_map = dsda_DoomMapEntry(map);

  if (!target_map)
    return false;

  *name = target_map->lump_name;

  return true;
}

int dsda_DoomHUTitle(dsda_string_t* str) {
  if (!current_map)
    return false;

  dsda_StringPrintF(str, "%s", current_map->nice_name);

  return true;
}

int dsda_DoomSkyTexture(int* sky) {
  if (!current_map || !current_map->sky1.lump)
    return false;

  *sky = R_TextureNumForName(current_map->sky1.lump);

  return true;
}

int dsda_DoomPrepareInitNew(void) {
  return false; // TODO
}

int dsda_DoomPrepareIntermission(int* result) {
  const doom_mapinfo_map_t* map = NULL;
  const doom_mapinfo_map_next_t* next = NULL;

  if (!current_map)
    return false;

  // TODO: NoIntermission

  // TODO: check that teleport new map resets secretexit
  if (leave_data.map > 0)
    map = dsda_DoomMapEntry(leave_data.map); // TODO: what if this map doesn't exist?
  else {
    if (secretexit)
      next = &current_map->secret_next;
    else // TODO: check default for secret exit without SecretNext data
      next = &current_map->next;

    if (next->map)
      map = dsda_DoomMapEntryByName(next->map);
  }

  if (current_map->par) {
    wminfo.partime = current_map->par;
    wminfo.modified_partime = true;
  }

  if (map) {
    wminfo.next = map->level_num - 1;
    wminfo.nextep = 0; // TODO: next ep
  }
  else {
    wminfo.next = 0;
    wminfo.nextep = 0;
  }

  // TODO: this should happen somewhere else
  if (wminfo.nextep != wminfo.epsd) {
    int i;

    for (i = 0; i < g_maxplayers; i++)
      players[i].didsecret = false;
  }

  wminfo.didsecret = players[consoleplayer].didsecret;

  if (!map) {
    end_data = next;

    if (!end_data)
      end_data = &default_end_data;
  }
  else
    end_data = NULL;

  *result = 0;

  return true;
}

int dsda_DoomPrepareFinale(int* result) {
  if (!current_map)
    return false;

  // TODO: check cluster finish text

  if (end_data)
    *result = WD_VICTORY;
  else
    *result = 0;

  return true;
}

void dsda_DoomLoadMapInfo(void) {
  int p;

  if (!dsda_Flag(dsda_arg_debug_mapinfo))
    return;

  p = -1;
  while ((p = W_ListNumFromName("MAPINFO", p)) >= 0) {
    const unsigned char* lump = (const unsigned char *) W_LumpByNum(p);
    dsda_ParseDoomMapInfo(lump, W_LumpLength(p), I_Error);
  }
}

int dsda_DoomExitPic(const char** exit_pic) {
  if (!current_map || !current_map->exit_pic)
    return false;

  *exit_pic = current_map->exit_pic;

  return true;
}

int dsda_DoomEnterPic(const char** enter_pic) {
  if (!current_map || !current_map->enter_pic)
    return false;

  *enter_pic = current_map->enter_pic;

  return true;
}

int dsda_DoomBorderTexture(const char** border_texture) {
  if (!current_map || !current_map->border_texture)
    return false;

  *border_texture = current_map->border_texture;

  return true;
}

int dsda_DoomPrepareEntering(void) {
  extern const char *el_levelname;
  extern const char *el_levelpic;
  extern const char *el_author;

  if (!next_map)
    return false;

  el_author = (next_map->flags & DMI_SHOW_AUTHOR) ? next_map->author : NULL;

  if (next_map->title_patch) {
    el_levelname = NULL;
    el_levelpic = next_map->title_patch;
  }
  else {
    el_levelname = next_map->nice_name;
    el_levelpic = NULL;
  }

  return true;
}

int dsda_DoomPrepareFinished(void) {
  extern const char *lf_levelname;
  extern const char *lf_levelpic;
  extern const char *lf_author;

  if (!last_map)
    return false;

  lf_author = (last_map->flags & DMI_SHOW_AUTHOR) ? last_map->author : NULL;

  if (last_map->title_patch) {
    lf_levelname = NULL;
    lf_levelpic = last_map->title_patch;
  }
  else {
    lf_levelname = last_map->nice_name;
    lf_levelpic = NULL;
  }

  return true;
}

int dsda_DoomMapLightning(int* lightning) {
  return false; // TODO
}

int dsda_DoomApplyFadeTable(void) {
  return false; // TODO
}

int dsda_DoomMapCluster(int* cluster, int map) {
  if (!current_map)
    return false;

  *cluster = current_map->cluster;

  return true;
}

int dsda_DoomSky1Texture(short* texture) {
  return false; // TODO
}

int dsda_DoomSky2Texture(short* texture) {
  return false; // TODO
}

int dsda_DoomGravity(fixed_t* gravity) {
  if (!current_map || !current_map->gravity)
    return false;

  *gravity = dsda_StringToFixed(current_map->gravity) / 800;

  return true;
}

int dsda_DoomAirControl(fixed_t* air_control) {
  if (!current_map || !current_map->air_control)
    return false;

  *air_control = dsda_StringToFixed(current_map->air_control);

  return true;
}

int dsda_DoomInitSky(void) {
  return false; // TODO
}

int dsda_DoomMapFlags(map_info_flags_t* flags) {
  if (!current_map)
    return false;

  *flags = current_map->flags;

  return true;
}
