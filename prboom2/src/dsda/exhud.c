//
// Copyright(C) 2021 by Ryan Krafnick
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
//	DSDA Intermission Display
//

#include "hu_lib.h"
#include "hu_stuff.h"
#include "doomstat.h"
#include "p_spec.h"
#include "p_tick.h"
#include "r_state.h"

#include "dsda.h"
#include "dsda/global.h"
#include "dsda/hud.h"
#include "dsda/settings.h"

#include "exhud.h"

#define DSDA_EXHUD_X 2
#define TRACKER_LIMIT 16

typedef enum {
  dsda_tracker_nothing,
  dsda_tracker_line,
  dsda_tracker_sector,
  dsda_tracker_mobj,
  dsda_tracker_player,
} dsda_tracker_type_t;

typedef struct {
  dsda_tracker_type_t type;
  int id;
  mobj_t* mobj;
} dsda_tracker_t;

dsda_text_t dsda_exhud_timer;
dsda_text_t dsda_exhud_max_totals;
dsda_text_t dsda_exhud_tracker[TRACKER_LIMIT];
dsda_tracker_t dsda_tracker[TRACKER_LIMIT];

static int tracker_map;
static int tracker_episode;

void dsda_InitExHud(patchnum_t* font) {
  int i;

  for (i = 0; i < TRACKER_LIMIT; ++i)
    HUlib_initTextLine(
      &dsda_exhud_tracker[i].text,
      DSDA_EXHUD_X,
      200 - g_st_height - 32 - i * 8,
      font,
      HU_FONTSTART,
      g_cr_gray,
      VPT_ALIGN_LEFT_BOTTOM | VPT_EX_TEXT
    );

  HUlib_initTextLine(
    &dsda_exhud_timer.text,
    DSDA_EXHUD_X,
    200 - g_st_height - 16,
    font,
    HU_FONTSTART,
    g_cr_gray,
    VPT_ALIGN_LEFT_BOTTOM | VPT_EX_TEXT
  );

  HUlib_initTextLine(
    &dsda_exhud_max_totals.text,
    DSDA_EXHUD_X,
    200 - g_st_height - 8,
    font,
    HU_FONTSTART,
    g_cr_gray,
    VPT_ALIGN_LEFT_BOTTOM | VPT_EX_TEXT
  );
}

extern int totalleveltimes;

static void dsda_UpdateTrackers(void) {
  int i;

  for (i = 0; i < TRACKER_LIMIT; ++i) {
    switch (dsda_tracker[i].type) {
      case dsda_tracker_nothing:
        dsda_exhud_tracker[i].msg[0] = '\0';
        break;
      case dsda_tracker_line:
        snprintf(
          dsda_exhud_tracker[i].msg,
          sizeof(dsda_exhud_tracker[i].msg),
          "\x1b%cl %d: %d",
          lines[dsda_tracker[i].id].special ? 0x30 + g_cr_gold : 0x30 + g_cr_blue,
          dsda_tracker[i].id,
          lines[dsda_tracker[i].id].special
        );
        break;
      case dsda_tracker_sector:
        {
          dboolean active;
          int special;

          active = P_PlaneActive(&sectors[dsda_tracker[i].id]);
          special = sectors[dsda_tracker[i].id].special;

          snprintf(
            dsda_exhud_tracker[i].msg,
            sizeof(dsda_exhud_tracker[i].msg),
            "\x1b%cs %d: %d %d %d",
            active ? 0x30 + g_cr_red : special ? 0x30 + g_cr_gold : 0x30 + g_cr_blue,
            dsda_tracker[i].id, special, active,
            sectors[dsda_tracker[i].id].floorheight >> FRACBITS
          );
        }
        break;
      case dsda_tracker_mobj:
        {
          int health;

          health = dsda_tracker[i].mobj->health;

          if (dsda_tracker[i].mobj->thinker.function == P_RemoveThinkerDelayed)
            health = 0;

          snprintf(
            dsda_exhud_tracker[i].msg,
            sizeof(dsda_exhud_tracker[i].msg),
            "\x1b%cm %d: %d",
            health > 0 ? 0x30 + g_cr_gold : 0x30 + g_cr_blue,
            dsda_tracker[i].id, health
          );
        }
        break;
      case dsda_tracker_player:
        {
          extern int player_damage_last_tic;

          snprintf(
            dsda_exhud_tracker[i].msg,
            sizeof(dsda_exhud_tracker[i].msg),
            "\x1b%cp: %d",
            player_damage_last_tic > 0 ? 0x30 + g_cr_gold : 0x30 + g_cr_blue,
            player_damage_last_tic
          );
        }
        break;
    }

    dsda_RefreshHudText(&dsda_exhud_tracker[i]);
  }
}

static int dsda_FindTracker(int type, int id) {
  int i;

  for (i = 0; i < TRACKER_LIMIT; ++i)
    if (dsda_tracker[i].type == type && dsda_tracker[i].id == id)
      return i;

  return -1;
}

static mobj_t* dsda_FindMobj(int id) {
  thinker_t* th;
  mobj_t* mobj;

  for (th = thinkercap.next; th != &thinkercap; th = th->next) {
    if (th->function != P_MobjThinker)
      continue;

    mobj = (mobj_t*) th;

    if (mobj->index == id)
      return mobj;
  }

  return NULL;
}

static void dsda_WipeTracker(int i) {
  dsda_tracker[i].type = dsda_tracker_nothing;
  dsda_tracker[i].id = 0;
  dsda_tracker[i].mobj = NULL;
}

static void dsda_WipeTrackers(void) {
  int i;

  for (i = 0; i < TRACKER_LIMIT; ++i)
    if (dsda_tracker[i].type != dsda_tracker_player)
      dsda_WipeTracker(i);
}

static void dsda_RefreshTrackers(void) {
  int i;

  for (i = 0; i < TRACKER_LIMIT; ++i)
    switch (dsda_tracker[i].type) {
      default:
        break;
      case dsda_tracker_mobj:
        dsda_tracker[i].mobj = dsda_FindMobj(dsda_tracker[i].id);
        if (!dsda_tracker[i].mobj)
          dsda_WipeTracker(i);
        break;
    }
}

void dsda_ResetTrackers(void) {
  if (gamemap != tracker_map || gameepisode != tracker_episode)
    dsda_WipeTrackers();
  else
    dsda_RefreshTrackers();
}

static dboolean dsda_AddTracker(int type, int id, mobj_t* mobj) {
  int i;

  tracker_map = gamemap;
  tracker_episode = gameepisode;

  if (dsda_FindTracker(type, id) >= 0)
    return false;

  if ((i = dsda_FindTracker(dsda_tracker_nothing, 0)) >= 0) {
    dsda_tracker[i].type = type;
    dsda_tracker[i].id = id;
    dsda_tracker[i].mobj = mobj;

    return true;
  }

  return false;
}

static dboolean dsda_RemoveTracker(int type, int id) {
  int i;

  if (dsda_StrictMode())
    return false;

  if ((i = dsda_FindTracker(type, id)) >= 0) {
    dsda_WipeTracker(i);

    return true;
  }

  return false;
}

dboolean dsda_TrackLine(int id) {
  int i;

  if (dsda_StrictMode())
    return false;

  if (id >= numlines || id < 0)
    return false;

  return dsda_AddTracker(dsda_tracker_line, id, NULL);
}

dboolean dsda_UntrackLine(int id) {
  return dsda_RemoveTracker(dsda_tracker_line, id);
}

dboolean dsda_TrackSector(int id) {
  int i;

  if (dsda_StrictMode())
    return false;

  if (id >= numsectors || id < 0)
    return false;

  return dsda_AddTracker(dsda_tracker_sector, id, NULL);
}

dboolean dsda_UntrackSector(int id) {
  return dsda_RemoveTracker(dsda_tracker_sector, id);
}

dboolean dsda_TrackMobj(int id) {
  int i;
  mobj_t* mobj = NULL;

  if (dsda_StrictMode())
    return false;

  mobj = dsda_FindMobj(id);

  if (!mobj)
    return false;

  {
    mobj_t* target = NULL;

    // While a mobj is targeted, its address is preserved
    P_SetTarget(&target, mobj);
  }

  return dsda_AddTracker(dsda_tracker_mobj, id, mobj);
}

dboolean dsda_UntrackMobj(int id) {
  return dsda_RemoveTracker(dsda_tracker_mobj, id);
}

dboolean dsda_TrackPlayer(int id) {
  int i;

  if (dsda_StrictMode())
    return false;

  return dsda_AddTracker(dsda_tracker_player, id, NULL);
}

dboolean dsda_UntrackPlayer(int id) {
  return dsda_RemoveTracker(dsda_tracker_player, id);
}

void dsda_UpdateExHud(void) {
  int total_time;

  dsda_UpdateTrackers();

  total_time = hexen ?
               players[consoleplayer].worldTimer :
               totalleveltimes + leveltime;

  // Timer - from hu_stuff.c
  if (total_time != leveltime)
    snprintf(
      dsda_exhud_timer.msg,
      sizeof(dsda_exhud_timer.msg),
      "\x1b%ctime \x1b%c%d:%02d \x1b%c%d:%05.2f ",
      g_cr_gray + 0x30,
      g_cr_gold + 0x30,
      total_time / 35 / 60,
      (total_time % (60 * 35)) / 35,
      g_cr_green + 0x30,
      leveltime / 35 / 60,
      (float)(leveltime % (60 * 35)) / 35
    );
  else
    snprintf(
      dsda_exhud_timer.msg,
      sizeof(dsda_exhud_timer.msg),
      "\x1b%ctime \x1b%c%d:%05.2f ",
      g_cr_gray + 0x30,
      g_cr_green + 0x30,
      leveltime / 35 / 60,
      (float)(leveltime % (60 * 35)) / 35
    );

  dsda_RefreshHudText(&dsda_exhud_timer);

  // Max totals - from hu_stuff.c
  {
    int i;
    char allkills[200], allsecrets[200];
    int playerscount;
    int fullkillcount, fullitemcount, fullsecretcount;
    int color, killcolor, itemcolor, secretcolor;
    int kill_percent_count;
    int allkills_len = 0;
    int allsecrets_len = 0;
    int max_kill_requirement;

    playerscount = 0;
    fullkillcount = 0;
    fullitemcount = 0;
    fullsecretcount = 0;
    kill_percent_count = 0;
    max_kill_requirement = dsda_MaxKillRequirement();

    for (i = 0; i < g_maxplayers; i++) {
      if (playeringame[i]) {
        color = i == displayplayer ? 0x30 + g_cr_green : 0x30 + g_cr_gray;
        if (playerscount==0) {
          allkills_len = sprintf(allkills, "\x1b%c%d", color, players[i].killcount - players[i].maxkilldiscount);
          allsecrets_len = sprintf(allsecrets, "\x1b%c%d", color, players[i].secretcount);
        }
        else {
          if (allkills_len >= 0 && allsecrets_len >=0) {
            allkills_len += sprintf(&allkills[allkills_len], "\x1b%c+%d", color, players[i].killcount - players[i].maxkilldiscount);
            allsecrets_len += sprintf(&allsecrets[allsecrets_len], "\x1b%c+%d", color, players[i].secretcount);
          }
        }
        playerscount++;
        fullkillcount += players[i].killcount - players[i].maxkilldiscount;
        fullitemcount += players[i].itemcount;
        fullsecretcount += players[i].secretcount;
        kill_percent_count += players[i].killcount;
      }
    }

    if (respawnmonsters)
    {
      fullkillcount = kill_percent_count;
      max_kill_requirement = totalkills;
    }

    killcolor = (fullkillcount >= max_kill_requirement ? 0x30 + g_cr_blue : 0x30 + g_cr_gold);
    secretcolor = (fullsecretcount >= totalsecret ? 0x30 + g_cr_blue : 0x30 + g_cr_gold);
    itemcolor = (fullitemcount >= totalitems ? 0x30 + g_cr_blue : 0x30 + g_cr_gold);

    if (playerscount < 2) {
      snprintf(
        dsda_exhud_max_totals.msg,
        sizeof(dsda_exhud_max_totals.msg),
        "\x1b%cK \x1b%c%d/%d \x1b%cI \x1b%c%d/%d \x1b%cS \x1b%c%d/%d",
        0x30 + g_cr_red,
        killcolor, fullkillcount, max_kill_requirement,
        0x30 + g_cr_red,
        itemcolor, players[displayplayer].itemcount, totalitems,
        0x30 + g_cr_red,
        secretcolor, fullsecretcount, totalsecret
      );
    }
    else {
      snprintf(
        dsda_exhud_max_totals.msg,
        sizeof(dsda_exhud_max_totals.msg),
        "\x1b%cK %s \x1b%c%d/%d \x1b%cI \x1b%c%d/%d \x1b%cS %s \x1b%c%d/%d",
        0x30 + g_cr_red,
        allkills, killcolor, fullkillcount, max_kill_requirement,
        0x30 + g_cr_red,
        itemcolor, players[displayplayer].itemcount, totalitems,
        0x30 + g_cr_red,
        allsecrets, secretcolor, fullsecretcount, totalsecret
      );
    }
  }

  dsda_RefreshHudText(&dsda_exhud_max_totals);
}

void dsda_DrawExHud(void) {
  int i;

  for (i = 0; i < TRACKER_LIMIT; ++i)
    HUlib_drawTextLine(&dsda_exhud_tracker[i].text, false);

  HUlib_drawTextLine(&dsda_exhud_timer.text, false);
  HUlib_drawTextLine(&dsda_exhud_max_totals.text, false);
}

void dsda_EraseExHud(void) {
  int i;

  for (i = 0; i < TRACKER_LIMIT; ++i)
    HUlib_eraseTextLine(&dsda_exhud_tracker[i].text);

  HUlib_eraseTextLine(&dsda_exhud_timer.text);
  HUlib_eraseTextLine(&dsda_exhud_max_totals.text);
}
