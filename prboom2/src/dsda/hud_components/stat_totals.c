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
//	DSDA Stat Totals HUD Component
//

#include "stdio.h"

#include "doomstat.h"

#include "dsda.h"
#include "dsda/global.h"

#include "stat_totals.h"

#define STAT_STRING_SIZE 200

void dsda_StatTotalsHC(char* str, size_t max_size) {
  int i;
  char allkills[STAT_STRING_SIZE], allsecrets[STAT_STRING_SIZE];
  int playerscount;
  int fullkillcount, fullitemcount, fullsecretcount;
  int color, killcolor, itemcolor, secretcolor;
  int kill_percent_count;
  int allkills_len, allsecrets_len;
  int max_kill_requirement;

  playerscount = 0;
  fullkillcount = 0;
  fullitemcount = 0;
  fullsecretcount = 0;
  kill_percent_count = 0;
  allkills_len = 0;
  allsecrets_len = 0;
  max_kill_requirement = dsda_MaxKillRequirement();

  for (i = 0; i < g_maxplayers; ++i) {
    if (playeringame[i]) {
      color = (i == displayplayer ? 0x30 + g_cr_green : 0x30 + g_cr_gray);

      if (playerscount == 0) {
        allkills_len = snprintf(
                         allkills,
                         STAT_STRING_SIZE,
                         "\x1b%c%d",
                         color,
                         players[i].killcount - players[i].maxkilldiscount
                       );
        allsecrets_len = snprintf(
                           allsecrets,
                           STAT_STRING_SIZE,
                           "\x1b%c%d",
                           color,
                           players[i].secretcount
                         );
      }
      else {
        if (allkills_len >= 0 && allsecrets_len >= 0) {
          allkills_len += snprintf(
                            &allkills[allkills_len],
                            STAT_STRING_SIZE,
                            "\x1b%c+%d",
                            color,
                            players[i].killcount - players[i].maxkilldiscount
                          );
          allsecrets_len += snprintf(
                              &allsecrets[allsecrets_len],
                              STAT_STRING_SIZE,
                              "\x1b%c+%d",
                              color,
                              players[i].secretcount
                            );
        }
      }

      ++playerscount;
      fullkillcount += players[i].killcount - players[i].maxkilldiscount;
      fullitemcount += players[i].itemcount;
      fullsecretcount += players[i].secretcount;
      kill_percent_count += players[i].killcount;
    }
  }

  if (respawnmonsters) {
    fullkillcount = kill_percent_count;
    max_kill_requirement = totalkills;
  }

  killcolor = (fullkillcount >= max_kill_requirement ? 0x30 + g_cr_blue : 0x30 + g_cr_gold);
  secretcolor = (fullsecretcount >= totalsecret ? 0x30 + g_cr_blue : 0x30 + g_cr_gold);
  itemcolor = (fullitemcount >= totalitems ? 0x30 + g_cr_blue : 0x30 + g_cr_gold);

  if (playerscount < 2) {
    snprintf(
      str,
      max_size,
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
      str,
      max_size,
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
