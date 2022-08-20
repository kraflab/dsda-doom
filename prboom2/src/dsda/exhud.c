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
#include "dsda/args.h"
#include "dsda/global.h"
#include "dsda/hud.h"
#include "dsda/hud_components.h"
#include "dsda/settings.h"
#include "dsda/utility.h"

#include "exhud.h"

#define DSDA_EXHUD_X 2
#define TRACKER_LIMIT 16

typedef enum {
  dsda_tracker_nothing,
  dsda_tracker_line,
  dsda_tracker_line_distance,
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

static int color_default;
static int color_warning;
static int color_alert;

void dsda_InitExHud(patchnum_t* font) {
  int i;

  color_default = g_cr_gray;
  color_warning = g_cr_green;
  color_alert = g_cr_red;

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
          "\x1b%cl %d: %d %d",
          lines[dsda_tracker[i].id].special ? 0x30 + color_warning : 0x30 + color_default,
          dsda_tracker[i].id,
          lines[dsda_tracker[i].id].special,
          lines[dsda_tracker[i].id].player_activations
        );
        break;
      case dsda_tracker_line_distance:
        {
          line_t* line;
          mobj_t* mo;
          double distance;
          double radius;

          line = &lines[dsda_tracker[i].id];
          mo = players[displayplayer].mo;
          radius = (double) mo->radius / FRACUNIT;
          distance = dsda_DistancePointToLine(line->v1->x, line->v1->y, line->v2->x, line->v2->y,
                                              mo->x, mo->y);

          snprintf(
            dsda_exhud_tracker[i].msg,
            sizeof(dsda_exhud_tracker[i].msg),
            "\x1b%cld %d: %.03f",
            distance < radius ? 0x30 + color_warning : 0x30 + color_default,
            dsda_tracker[i].id,
            distance
          );
        }
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
            active ? 0x30 + color_alert : special ? 0x30 + color_warning : 0x30 + color_default,
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
            health > 0 ? 0x30 + color_warning : 0x30 + color_default,
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
            player_damage_last_tic > 0 ? 0x30 + color_warning : 0x30 + color_default,
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

void dsda_WipeTrackers(void) {
  int i;

  for (i = 0; i < TRACKER_LIMIT; ++i)
    if (dsda_tracker[i].type != dsda_tracker_player)
      dsda_WipeTracker(i);
}

static void dsda_ConsolidateTrackers(void) {
  int i;

  for (i = 0; i < TRACKER_LIMIT; ++i)
    if (dsda_tracker[i].type == dsda_tracker_nothing) {
      int j;

      for (j = i + 1; j < TRACKER_LIMIT; ++j)
        if (dsda_tracker[j].type != dsda_tracker_nothing) {
          dsda_tracker[i] = dsda_tracker[j];
          dsda_WipeTracker(j);
          break;
        }

      if (j == TRACKER_LIMIT)
        return;
    }
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

static void dsda_ParseCommandlineTrackers(int arg_id, dboolean (*track)(int)) {
  dsda_arg_t* arg;

  arg = dsda_Arg(arg_id);
  if (arg->found) {
    int i;

    for (i = 0; i < arg->count; ++i)
      track(arg->value.v_int_array[i]);
  }
}

void dsda_ResetTrackers(void) {
  static dboolean first_time = true;

  if (first_time) {
    first_time = false;

    dsda_ParseCommandlineTrackers(dsda_arg_track_line, dsda_TrackLine);
    dsda_ParseCommandlineTrackers(dsda_arg_track_line_distance, dsda_TrackLineDistance);
    dsda_ParseCommandlineTrackers(dsda_arg_track_sector, dsda_TrackSector);
    dsda_ParseCommandlineTrackers(dsda_arg_track_mobj, dsda_TrackMobj);

    if (dsda_Flag(dsda_arg_track_player))
      dsda_TrackPlayer(0);

    return;
  }

  if (gamemap != tracker_map || gameepisode != tracker_episode)
    dsda_WipeTrackers();
  else
    dsda_RefreshTrackers();

  dsda_ConsolidateTrackers();
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
    dsda_ConsolidateTrackers();

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

dboolean dsda_TrackLineDistance(int id) {
  int i;

  if (dsda_StrictMode())
    return false;

  if (id >= numlines || id < 0)
    return false;

  return dsda_AddTracker(dsda_tracker_line_distance, id, NULL);
}

dboolean dsda_UntrackLineDistance(int id) {
  return dsda_RemoveTracker(dsda_tracker_line_distance, id);
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
  dsda_UpdateTrackers();

  dsda_CompositeTimeHC(dsda_exhud_timer.msg, sizeof(dsda_exhud_timer.msg));
  dsda_RefreshHudText(&dsda_exhud_timer);

  dsda_StatTotalsHC(dsda_exhud_max_totals.msg, sizeof(dsda_exhud_max_totals.msg));
  dsda_RefreshHudText(&dsda_exhud_max_totals);
}

void dsda_DrawExHud(void) {
  int i;

  if (!dsda_StrictMode())
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
