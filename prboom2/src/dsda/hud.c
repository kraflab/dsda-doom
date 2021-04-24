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
//	DSDA Hud
//

#include "st_stuff.h"
#include "hu_lib.h"
#include "hu_stuff.h"
#include "doomstat.h"

#include "dsda.h"
#include "dsda/global.h"
#include "dsda/settings.h"
#include "dsda/command_display.h"
#include "hud.h"

#define DSDA_TEXT_X 2
#define DSDA_INTERMISSION_TIME_Y 1
#define DSDA_SPLIT_Y 12
#define DSDA_SPLIT_LIFETIME 105
#define DSDA_SPLIT_SIZE 80
#define DSDA_TEXT_SIZE 200

// hook into screen settings
extern int SCREENHEIGHT;
extern int viewheight;

extern int totalleveltimes;

typedef struct {
  hu_textline_t text;
  char msg[DSDA_SPLIT_SIZE];
  int ticks;
} dsda_split_text_t;

typedef struct {
  hu_textline_t text;
  char msg[DSDA_TEXT_SIZE];
} dsda_text_t;

typedef struct {
  const char* msg;
  int default_delay;
  int delay;
} dsda_split_state_t;

dsda_split_state_t dsda_split_state[] = {
  {"Blue Key", 0, 0},
  {"Yellow Key", 0, 0},
  {"Red Key", 0, 0},
  {"Use", 2, 0},
  {"Secret", 0, 0}
};

dsda_split_text_t dsda_split;

dsda_text_t dsda_exhud_timer;
dsda_text_t dsda_exhud_max_totals;
dsda_text_t dsda_intermission_time;

static void dsda_InitExHud(patchnum_t* font) {
  HUlib_initTextLine(
    &dsda_exhud_timer.text,
    DSDA_TEXT_X,
    200 - g_st_height - 16,
    font,
    HU_FONTSTART,
    g_cr_gray,
    VPT_ALIGN_LEFT_BOTTOM
  );

  HUlib_initTextLine(
    &dsda_exhud_max_totals.text,
    DSDA_TEXT_X,
    200 - g_st_height - 8,
    font,
    HU_FONTSTART,
    g_cr_gray,
    VPT_ALIGN_LEFT_BOTTOM
  );
}

void dsda_InitHud(patchnum_t* font) {
  HUlib_initTextLine(
    &dsda_split.text,
    DSDA_TEXT_X,
    DSDA_SPLIT_Y,
    font,
    HU_FONTSTART,
    g_cr_gray,
    VPT_ALIGN_LEFT
  );

  HUlib_initTextLine(
    &dsda_intermission_time.text,
    DSDA_TEXT_X,
    DSDA_INTERMISSION_TIME_Y,
    font,
    HU_FONTSTART,
    g_cr_gray,
    VPT_ALIGN_LEFT
  );

  dsda_InitExHud(font);
  dsda_InitCommandDisplay(font);
}

void dsda_DrawIntermissionTime(void) {
  char* s;

  snprintf(
    dsda_intermission_time.msg,
    sizeof(dsda_intermission_time.msg),
    "%d:%05.2f",
    leveltime / 35 / 60,
    (float)(leveltime % (60 * 35)) / 35
  );

  HUlib_clearTextLine(&dsda_intermission_time.text);

  s = dsda_intermission_time.msg;
  while (*s) HUlib_addCharToTextLine(&dsda_intermission_time.text, *(s++));

  HUlib_drawTextLine(&dsda_intermission_time.text, false);
}

static dboolean dsda_ExHudVisible(void) {
  return dsda_ExHud() && // extended hud turned on
         viewheight != SCREENHEIGHT && // not zoomed in
         (!(automapmode & am_active) || (automapmode & am_overlay)); // automap inactive
}

static dboolean dsda_CommandDisplayVisible(void) {
  return dsda_CommandDisplay() && // command display turned on
         viewheight != SCREENHEIGHT && // not zoomed in
         (!(automapmode & am_active) || (automapmode & am_overlay)); // automap inactive
}

static void dsda_UpdateExHud(void) {
  char* s;

  // Timer - from hu_stuff.c
  if (totalleveltimes)
    snprintf(
      dsda_exhud_timer.msg,
      sizeof(dsda_exhud_timer.msg),
      "\x1b%ctime \x1b%c%d:%02d \x1b%c%d:%05.2f ",
      g_cr_gray + 0x30,
      g_cr_gold + 0x30,
      (totalleveltimes + leveltime) / 35 / 60,
      ((totalleveltimes + leveltime) % (60 * 35)) / 35,
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

  HUlib_clearTextLine(&dsda_exhud_timer.text);

  s = dsda_exhud_timer.msg;
  while (*s) HUlib_addCharToTextLine(&dsda_exhud_timer.text, *(s++));

  // Max totals - from hu_stuff.c
  {
    int i;
    char allkills[200], allsecrets[200];
    int playerscount;
    int fullkillcount, fullitemcount, fullsecretcount;
    int color, killcolor, itemcolor, secretcolor;
    int kill_percent_color, kill_percent_count, kill_percent;
    int allkills_len = 0;
    int allsecrets_len = 0;
    int max_kill_requirement;

    playerscount = 0;
    fullkillcount = 0;
    fullitemcount = 0;
    fullsecretcount = 0;
    kill_percent_count = 0;
    max_kill_requirement = dsda_MaxKillRequirement();

    for (i = 0; i < MAXPLAYERS; i++) {
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
    killcolor = (fullkillcount >= max_kill_requirement ? 0x30 + g_cr_blue : 0x30 + g_cr_gold);
    secretcolor = (fullsecretcount >= totalsecret ? 0x30 + g_cr_blue : 0x30 + g_cr_gold);
    itemcolor = (fullitemcount >= totalitems ? 0x30 + g_cr_blue : 0x30 + g_cr_gold);
    kill_percent_color = (kill_percent_count >= totalkills ? 0x30 + g_cr_blue : 0x30 + g_cr_gold);
    kill_percent = (totalkills == 0 ? 100 : kill_percent_count * 100 / totalkills);
    if (playerscount < 2) {
      snprintf(
        dsda_exhud_max_totals.msg,
        sizeof(dsda_exhud_max_totals.msg),
        "\x1b%cK \x1b%c%d/%d \x1b%c%d%% \x1b%cI \x1b%c%d/%d \x1b%cS \x1b%c%d/%d",
        0x30 + g_cr_red,
        killcolor, fullkillcount, max_kill_requirement,
        kill_percent_color, kill_percent,
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
        "\x1b%cK %s \x1b%c%d/%d \x1b%c%d%% \x1b%cI \x1b%c%d/%d \x1b%cS %s \x1b%c%d/%d",
        0x30 + g_cr_red,
        allkills, killcolor, fullkillcount, max_kill_requirement,
        kill_percent_color, kill_percent,
        0x30 + g_cr_red,
        itemcolor, players[displayplayer].itemcount, totalitems,
        0x30 + g_cr_red,
        allsecrets, secretcolor, fullsecretcount, totalsecret
      );
    }
  }

  HUlib_clearTextLine(&dsda_exhud_max_totals.text);

  s = dsda_exhud_max_totals.msg;
  while (*s) HUlib_addCharToTextLine(&dsda_exhud_max_totals.text, *(s++));
}

void dsda_UpdateHud(void) {
  int i;

  if (dsda_split.ticks > 0) --dsda_split.ticks;

  for (i = 0; i < DSDA_SPLIT_CLASS_COUNT; ++i)
    if (dsda_split_state[i].delay > 0)
      --dsda_split_state[i].delay;

  if (dsda_ExHudVisible()) dsda_UpdateExHud();
}

static void dsda_DrawExHud(void) {
  HUlib_drawTextLine(&dsda_exhud_timer.text, false);
  HUlib_drawTextLine(&dsda_exhud_max_totals.text, false);
}

void dsda_DrawHud(void) {
  if (dsda_split.ticks > 0) HUlib_drawTextLine(&dsda_split.text, false);

  if (dsda_ExHudVisible()) dsda_DrawExHud();
  if (dsda_CommandDisplayVisible()) dsda_DrawCommandDisplay();
}

static void dsda_EraseExHud(void) {
  HUlib_eraseTextLine(&dsda_exhud_timer.text);
  HUlib_eraseTextLine(&dsda_exhud_max_totals.text);
}

void dsda_EraseHud(void) {
  if (dsda_split.ticks > 0) HUlib_eraseTextLine(&dsda_split.text);

  dsda_EraseExHud();
  dsda_EraseCommandDisplay();
}

void dsda_AddSplit(dsda_split_class_t split_class) {
  int minutes;
  float seconds;
  char* s;
  dsda_split_state_t* split_state;

  split_state = &dsda_split_state[split_class];

  if (split_state->delay > 0) {
    split_state->delay = split_state->default_delay;
    return;
  }

  split_state->delay = split_state->default_delay;

  dsda_split.ticks = DSDA_SPLIT_LIFETIME;

  // to match the timer, we use the leveltime value at the end of the frame
  minutes = (leveltime + 1) / 35 / 60;
  seconds = (float)((leveltime + 1) % (60 * 35)) / 35;
  snprintf(
    dsda_split.msg, DSDA_SPLIT_SIZE, "%d:%05.2f - %s",
    minutes, seconds, split_state->msg
  );

  HUlib_clearTextLine(&dsda_split.text);

  s = dsda_split.msg;
  while (*s) HUlib_addCharToTextLine(&dsda_split.text, *(s++));
}
