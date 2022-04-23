//
// Copyright(C) 2022 by Ryan Krafnick
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
//  DSDA MapInfo U
//

#include "doomstat.h"
#include "g_game.h"
#include "lprintf.h"
#include "m_argv.h"
#include "p_spec.h"
#include "p_tick.h"
#include "r_state.h"
#include "s_sound.h"
#include "sounds.h"
#include "umapinfo.h"
#include "v_video.h"
#include "w_wad.h"

#include "dsda/global.h"
#include "dsda/map_format.h"
#include "dsda/mapinfo.h"

#include "u.h"

static struct MapEntry* gamemapinfo;
static struct MapEntry* lastmapinfo;
static struct MapEntry* nextmapinfo;

static struct MapEntry* dsda_UMapEntry(int gameepisode, int gamemap)
{
  char lumpname[9];
  unsigned i;

  if (gamemode == commercial)
    snprintf(lumpname, 9, "MAP%02d", gamemap);
  else
    snprintf(lumpname, 9, "E%dM%d", gameepisode, gamemap);

  for (i = 0; i < Maps.mapcount; i++)
    if (!stricmp(lumpname, Maps.maps[i].mapname))
      return &Maps.maps[i];

  return NULL;
}

int dsda_UFirstMap(int* episode, int* map) {
  return false;
}

int dsda_UNewGameMap(int* episode, int* map) {
  return false;
}

int dsda_UResolveWarp(int arg_p, int* episode, int* map) {
  return false;
}

int dsda_UNextMap(int* episode, int* map) {
  const char *name = NULL;

  if (!gamemapinfo)
    return false;

  if (gamemapinfo->nextsecret[0])
    name = gamemapinfo->nextsecret;
  else if (gamemapinfo->nextmap[0])
    name = gamemapinfo->nextmap;
  else if (gamemapinfo->endpic[0] && gamemapinfo->endpic[0] != '-')
  {
    *episode = 1;
    *map = 1;

    return true;
  }

  if (name)
    return dsda_NameToMap(name, episode, map);

  return false;
}

int dsda_UShowNextLocBehaviour(int* behaviour) {
  if (!gamemapinfo)
    return false;

  if (gamemapinfo->endpic[0])
    *behaviour = WI_SHOW_NEXT_DONE;
  else
    *behaviour = WI_SHOW_NEXT_LOC | WI_SHOW_NEXT_EPISODAL;

  return true;
}

int dsda_USkipDrawShowNextLoc(int* skip) {
  if (!gamemapinfo)
    return false;

  *skip = (gamemapinfo->endpic[0] && strcmp(gamemapinfo->endpic, "-") != 0);

  return true;
}

void dsda_UUpdateMapInfo(void) {
  gamemapinfo = dsda_UMapEntry(gameepisode, gamemap);
}

void dsda_UUpdateLastMapInfo(void) {
  lastmapinfo = gamemapinfo;
  nextmapinfo = NULL;
}

void dsda_UUpdateNextMapInfo(void) {
  nextmapinfo = dsda_UMapEntry(wminfo.nextep + 1, wminfo.next + 1);
}

int dsda_UResolveCLEV(int* clev, int* episode, int* map) {
  if (dsda_UMapEntry(*episode, *map)) {
    *clev = true;

    return true;
  }

  return false;
}

int dsda_UResolveINIT(int* init) {
  return false;
}

int dsda_UMusicIndexToLumpNum(int* lump, int music_index) {
  return false;
}

int dsda_UMapMusic(int* music_index, int* music_lump) {
  int lump;

  if (!gamemapinfo)
    return false;

  if (!gamemapinfo->music[0])
    return false;

  lump = W_CheckNumForName(gamemapinfo->music);

  if (lump < 0)
    return false;

  *music_index = -1;
  *music_lump = lump;

  return true;
}

int dsda_UInterMusic(int* music_index, int* music_lump) {
  int lump;

  if (!gamemapinfo)
    return false;

  if (!gamemapinfo->intermusic[0])
    return false;

  lump = W_CheckNumForName(gamemapinfo->intermusic);

  if (lump < 0)
    return false;

  *music_index = -1;
  *music_lump = lump;

  return true;
}

extern int finalestage;
extern int finalecount;
extern const char* finaletext;
extern const char* finaleflat;
extern const char* finalepatch;
extern int acceleratestage;
extern int midstage;

int dsda_UStartFinale(void) {
  if (!gamemapinfo)
    return false;

  // '-' means that any default intermission was cleared.
  if (gamemapinfo->intertextsecret && secretexit && gamemapinfo->intertextsecret[0] != '-')
    finaletext = gamemapinfo->intertextsecret;
  else if (gamemapinfo->intertext && !secretexit && gamemapinfo->intertext[0] != '-')
    finaletext = gamemapinfo->intertext;

  // this is to avoid a crash on a missing text in the last map.
  if (!finaletext)
    finaletext = "The End";

  if (gamemapinfo->interbackdrop[0]) {
    if (W_CheckNumForName(gamemapinfo->interbackdrop) != -1 &&
        (W_CheckNumForName)(gamemapinfo->interbackdrop, ns_flats) == -1)
      finalepatch = gamemapinfo->interbackdrop;
    else
      finaleflat = gamemapinfo->interbackdrop;
  }

  if (!finaleflat)
    finaleflat = "FLOOR4_8"; // use a single fallback for all maps.

  return true;
}

int dsda_UFTicker(void) {
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
        finalecount > strlen(finaletext) * speed + (midstage ? NEWTEXTWAIT : TEXTWAIT) ||
        (midstage && acceleratestage)
      )
        next_level = true;
    }
  }

  if (next_level) {
    if (gamemapinfo->endpic[0] && (strcmp(gamemapinfo->endpic, "-") != 0)) {
      if (!stricmp(gamemapinfo->endpic, "$CAST")) {
        F_StartCast();
        return false; // let go of finale ownership
      }
      else {
        finalecount = 0;
        finalestage = 1;
        wipegamestate = -1; // force a wipe
        if (!stricmp(gamemapinfo->endpic, "$BUNNY"))
          S_StartMusic(mus_bunny);
        else if (!stricmp(gamemapinfo->endpic, "!"))
          return false; // let go of finale ownership
      }
    }
    else
      gameaction = ga_worlddone; // next level, e.g. MAP07
  }

  return true; // keep finale ownership
}

void dsda_UFDrawer(void) {
  void F_TextWrite(void);
  void F_BunnyScroll(void);

  if (!finalestage || !gamemapinfo->endpic[0] || (strcmp(gamemapinfo->endpic, "-") == 0))
    F_TextWrite();
  else if (strcmp(gamemapinfo->endpic, "$BUNNY") == 0)
    F_BunnyScroll();
  else {
    // e6y: wide-res
    V_FillBorder(-1, 0);
    V_DrawNamePatch(0, 0, 0, gamemapinfo->endpic, CR_DEFAULT, VPT_STRETCH);
  }
}

// numbossactions == 0 means to use the defaults.
// numbossactions == -1 means to do nothing.
// positive values mean to check the list of boss actions and run all that apply.
int dsda_UBossAction(mobj_t* mo) {
  int i;
  line_t junk;
  thinker_t* th;

  if (!gamemapinfo || !gamemapinfo->numbossactions)
    return false;

  if (gamemapinfo->numbossactions < 0)
    return true;

  // make sure there is a player alive for victory
  for (i = 0; i < g_maxplayers; i++)
    if (playeringame[i] && players[i].health > 0)
      break;

  if (i == g_maxplayers)
    return true; // no one left alive, so do not end game

  for (i = 0; i < gamemapinfo->numbossactions; i++)
    if (gamemapinfo->bossactions[i].type == mo->type)
      break;

  if (i >= gamemapinfo->numbossactions)
    return true; // no matches found

  // scan the remaining thinkers to see
  // if all bosses are dead
  for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    if (th->function == P_MobjThinker) {
      mobj_t* mo2 = (mobj_t*) th;

      if (mo2 != mo && mo2->type == mo->type && mo2->health > 0)
        return true; // other boss not dead
    }

  for (i = 0; i < gamemapinfo->numbossactions; i++) {
    if (gamemapinfo->bossactions[i].type == mo->type) {
      junk = *lines;
      junk.special = (short) gamemapinfo->bossactions[i].special;
      junk.tag = (short) gamemapinfo->bossactions[i].tag;

      // use special semantics for line activation to block problem types.
      if (!P_UseSpecialLine(mo, &junk, 0, true))
        map_format.cross_special_line(&junk, 0, mo, true);
    }
  }

  return true;
}

int dsda_UHUTitle(const char** title) {
  void HU_AddCharToTitle(char s);

  const char* s;

  if (!gamemapinfo || !gamemapinfo->levelname)
    return false;

  if (gamemapinfo->label)
    s = gamemapinfo->label;
  else
    s = gamemapinfo->mapname;

  if (s == gamemapinfo->mapname || strcmp(s, "-") != 0) {
    while (*s)
      HU_AddCharToTitle(*(s++));

    HU_AddCharToTitle(':');
    HU_AddCharToTitle(' ');
  }

  *title = gamemapinfo->levelname;

  return true;
}

int dsda_USkyTexture(int* sky) {
  if (!gamemapinfo || !gamemapinfo->skytexture[0])
    return false;

  *sky = R_TextureNumForName(gamemapinfo->skytexture);

  return true;
}

int dsda_UPrepareInitNew(void) {
  return false;
}

int dsda_UPrepareIntermission(int* result) {
  const char *next = "";

  if (!gamemapinfo)
    return false;

  if (
    gamemapinfo->endpic[0] &&
    strcmp(gamemapinfo->endpic, "-") != 0 &&
    gamemapinfo->nointermission
  ) {
    *result = DC_VICTORY;

    return true;
  }

  if (secretexit)
    next = gamemapinfo->nextsecret;

  if (next[0] == 0)
    next = gamemapinfo->nextmap;

  if (next[0]) {
    dsda_NameToMap(next, &wminfo.nextep, &wminfo.next);

    wminfo.nextep--;
    wminfo.next--;

    if (wminfo.nextep != wminfo.epsd)
    {
      int i;

      for (i = 0; i < g_maxplayers; i++)
        players[i].didsecret = false;
    }

    wminfo.didsecret = players[consoleplayer].didsecret;
    wminfo.partime = gamemapinfo->partime;

    *result = 0;

    return true;
  }

  return false;
}

int dsda_UPrepareFinale(int* result) {
  if (!gamemapinfo)
    return false;

  if (gamemapinfo->intertextsecret && secretexit) {
    if (gamemapinfo->intertextsecret[0] != '-') // '-' means that any default intermission was cleared.
      *result = WD_START_FINALE;
    else
      *result = 0;

    return true;
  }
  else if (gamemapinfo->intertext && !secretexit) {
    if (gamemapinfo->intertext[0] != '-') // '-' means that any default intermission was cleared.
      *result = WD_START_FINALE;
    else
      *result = 0;

    return true;
  }
  else if (gamemapinfo->endpic[0] && strcmp(gamemapinfo->endpic, "-") != 0) {
    *result = WD_VICTORY;

    return true;
  }

  return false;
}

void dsda_ULoadMapInfo(void) {
  int p;

  if (M_CheckParm("-nomapinfo"))
    return;

  p = -1;
  while ((p = W_ListNumFromName("UMAPINFO", p)) >= 0) {
    const unsigned char * lump = (const unsigned char *) W_CacheLumpNum(p);
    ParseUMapInfo(lump, W_LumpLength(p), I_Error);
  }
}

int dsda_UExitPic(const char** exit_pic) {
  if (!lastmapinfo || !lastmapinfo->exitpic[0])
    return false;

  *exit_pic = lastmapinfo->exitpic;

  return true;
}

int dsda_UEnterPic(const char** enter_pic) {
  if (!nextmapinfo || !nextmapinfo->enterpic[0])
    return false;

  *enter_pic = nextmapinfo->enterpic;

  return true;
}

int dsda_UPrepareEntering(void) {
  extern const char *el_levelname;
  extern const char *el_levelpic;

  if (!nextmapinfo)
    return false;

  if (nextmapinfo->levelname && nextmapinfo->levelpic[0] == 0) {
    el_levelname = nextmapinfo->levelname;
    el_levelpic = NULL;

    return true;
  }
  else if (nextmapinfo->levelpic[0]) {
    el_levelname = NULL;
    el_levelpic = nextmapinfo->levelpic;

    return true;
  }

  return false;
}

int dsda_UPrepareFinished(void) {
  extern const char *lf_levelname;
  extern const char *lf_levelpic;

  if (!lastmapinfo)
    return false;

  if (lastmapinfo->levelname && lastmapinfo->levelpic[0] == 0) {
    lf_levelname = lastmapinfo->levelname;
    lf_levelpic = NULL;

    return true;
  }
  else if (lastmapinfo->levelpic[0]) {
    lf_levelname = NULL;
    lf_levelpic = lastmapinfo->levelpic;

    return true;
  }

  return false;
}

int dsda_UMapLightning(int* lightning) {
  return false;
}

int dsda_UApplyFadeTable(void) {
  return false;
}

int dsda_UMapCluster(int* cluster, int map) {
  return false;
}

int dsda_USky1Texture(short* texture) {
  return false;
}

int dsda_USky2Texture(short* texture) {
  return false;
}

int dsda_UInitSky(void) {
  return false;
}
