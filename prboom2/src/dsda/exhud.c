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

#include "dsda.h"
#include "dsda/global.h"
#include "dsda/hud.h"

#include "exhud.h"

#define DSDA_EXHUD_X 2

dsda_text_t dsda_exhud_timer;
dsda_text_t dsda_exhud_max_totals;

void dsda_InitExHud(patchnum_t* font) {
  HUlib_initTextLine(
    &dsda_exhud_timer.text,
    DSDA_EXHUD_X,
    200 - g_st_height - 16,
    font,
    HU_FONTSTART,
    g_cr_gray,
    VPT_ALIGN_LEFT_BOTTOM
  );

  HUlib_initTextLine(
    &dsda_exhud_max_totals.text,
    DSDA_EXHUD_X,
    200 - g_st_height - 8,
    font,
    HU_FONTSTART,
    g_cr_gray,
    VPT_ALIGN_LEFT_BOTTOM
  );
}

extern int totalleveltimes;

void dsda_UpdateExHud(void) {
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

  dsda_RefreshHudText(&dsda_exhud_timer);

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

  dsda_RefreshHudText(&dsda_exhud_max_totals);
}

void dsda_DrawExHud(void) {
  HUlib_drawTextLine(&dsda_exhud_timer.text, false);
  HUlib_drawTextLine(&dsda_exhud_max_totals.text, false);
}

void dsda_EraseExHud(void) {
  HUlib_eraseTextLine(&dsda_exhud_timer.text);
  HUlib_eraseTextLine(&dsda_exhud_max_totals.text);
}
