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
/*
========================
=
= IN_lude.c
=
========================
*/

#include "am_map.h"
#include "doomstat.h"
#include "d_event.h"
#include "s_sound.h"
#include "sounds.h"
#include "i_system.h"
#include "i_video.h"
#include "v_video.h"
#include "lprintf.h"
#include "w_wad.h"
#include "g_game.h"

#include "dsda/exhud.h"
#include "dsda/font.h"
#include "dsda/mapinfo.h"
#include "dsda/mapinfo/u.h"

#include "heretic/def.h"
#include "heretic/dstrings.h"
#include "heretic/mn_menu.h"
#include "heretic/sb_bar.h"

#include "in_lude.h"

extern dboolean BorderNeedRefresh;

typedef enum
{
    SINGLE,
    COOPERATIVE,
    DEATHMATCH
} gametype_t;

static void IN_WaitStop(void);
static void IN_Stop(void);
static void IN_CheckForSkip(void);
static void IN_InitStats(void);
static void IN_DrawOldLevel(void);
static void IN_DrawYAH(void);
static void IN_DrawStatBack(void);
static void IN_DrawSingleStats(void);
static void IN_DrawCoopStats(void);
static void IN_DrawDMStats(void);
static void IN_DrawNumber(int val, int x, int y, int digits);
static void IN_DrawTime(int x, int y, int h, int m, int s);
static void IN_DrTextB(const char *text, int x, int y);
static void IN_DrawLevelname(const char *patch, const char *levelname, int y);

// contains information passed into intermission
static wbstartstruct_t* wbs;

static int prevmap;
static int nextmap;
static dboolean intermission;
static dboolean skipintermission;
dboolean finalintermission;
static int interstate = 0;
enum
{
  IN_STATS = 0,
  IN_FINISHED,
  IN_ENTERING,
  IN_ENTERING_DELAY,
} interstate_e;
static int intertime = -1;
static int oldintertime = 0;
static gametype_t gametype;

static int cnt;

static int hours;
static int minutes;
static int seconds;

// [crispy] Show total time on intermission
static int totalHours;
static int totalMinutes;
static int totalSeconds;

static int parHours;
static int parMinutes;
static int parSeconds;

static int slaughterboy;        // in DM, the player with the most kills

static int killPercent[MAX_MAXPLAYERS];
static int bonusPercent[MAX_MAXPLAYERS];
static int secretPercent[MAX_MAXPLAYERS];

static int FontBNumbers[10];

static int FontBLump;
static int patchFaceOkayBase;
static int patchFaceDeadBase;

static signed int totalFrags[MAX_MAXPLAYERS];
static fixed_t dSlideX[MAX_MAXPLAYERS];
static fixed_t dSlideY[MAX_MAXPLAYERS];

static const char *KillersText[] = { "K", "I", "L", "L", "E", "R", "S" };

extern const char *LevelNames[];

extern const char *lf_levelname;
extern const char *lf_levelpic;
extern const char *lf_author;
extern const char *exitpic;
extern const char *el_levelname;
extern const char *el_levelpic;
extern const char *el_author;
extern const char *enterpic;

typedef struct
{
    int x;
    int y;
} yahpt_t;

static yahpt_t YAHspot[3][9] = {
    {
     {172, 78},
     {86, 90},
     {73, 66},
     {159, 95},
     {148, 126},
     {132, 54},
     {131, 74},
     {208, 138},
     {52, 101}
     },
    {
     {218, 57},
     {137, 81},
     {155, 124},
     {171, 68},
     {250, 86},
     {136, 98},
     {203, 90},
     {220, 140},
     {279, 106}
     },
    {
     {86, 99},
     {124, 103},
     {154, 79},
     {202, 83},
     {178, 59},
     {142, 58},
     {219, 66},
     {247, 57},
     {107, 80}
     }
};

static const char *NameForMap(int map)
{
    const char *name = LevelNames[(gameepisode - 1) * 9 + map - 1];

    if (strlen(name) < 7)
    {
        return "";
    }
    return name + 7;
}

static dboolean IN_HasInterpic()
{
  return (gameepisode > 1 && gameepisode < 3) ||
          W_LumpNameExists(enterpic) || W_LumpNameExists(exitpic);
}

static void IN_DrawInterpic(void)
{
  // e6y: wide-res
  V_ClearBorder();

  if (W_LumpNameExists(enterpic))
  {
    V_DrawNamePatchFS(0, 0, 0, enterpic, CR_DEFAULT, VPT_STRETCH);
  }
  else if (W_LumpNameExists(exitpic))
  {
    V_DrawNamePatchFS(0, 0, 0, exitpic, CR_DEFAULT, VPT_STRETCH);
  }
  else
  {
    if (gameepisode < 1 || gameepisode > 3) return;

    const char name[9];
    snprintf(name, sizeof(name), "MAPE%d", gameepisode);

    V_DrawNamePatchFS(0, 0, 0, name, CR_DEFAULT, VPT_STRETCH);
  }
}

static void IN_DrawBeenThere(int i)
{
  V_DrawNamePatch(
    YAHspot[gameepisode - 1][i].x, YAHspot[gameepisode - 1][i].y, 0,
    "IN_X", CR_DEFAULT, VPT_STRETCH
  );
}

static void IN_DrawGoingThere(int i)
{
  V_DrawNamePatch(
    YAHspot[gameepisode - 1][i].x, YAHspot[gameepisode - 1][i].y, 0,
    "IN_YAH", CR_DEFAULT, VPT_STRETCH
  );
}

static void IN_InitLumps(void)
{
  int i, base;

  base = W_GetNumForName("FONTB16");
  for (i = 0; i < 10; i++)
  {
      FontBNumbers[i] = base + i;
  }

  FontBLump = W_GetNumForName("FONTB_S") + 1;
  patchFaceOkayBase = W_GetNumForName("FACEA0");
  patchFaceDeadBase = W_GetNumForName("FACEB0");
}

static void IN_InitVariables(wbstartstruct_t* wbstartstruct)
{
  wbs = wbstartstruct;
  prevmap = wbs->last + 1;
  nextmap = wbs->next + 1;

  int behaviour;
  dsda_ShowNextLocBehaviour(&behaviour);
  finalintermission = (behaviour & WI_SHOW_NEXT_DONE);
}

//========================================================================
//
// IN_Start
//
//========================================================================

void IN_Start(wbstartstruct_t* wbstartstruct)
{
    V_SetPalette(0);
    IN_InitVariables(wbstartstruct);
    IN_InitLumps();
    IN_InitStats();
    intermission = true;
    interstate = -1;
    skipintermission = false;
    intertime = 0;
    oldintertime = 0;
    AM_Stop(false);
    S_ChangeMusic(heretic_mus_intr, true);
}

//========================================================================
//
// IN_WaitStop
//
//========================================================================

void IN_WaitStop(void)
{
    if (!--cnt)
    {
        IN_Stop();
        G_WorldDone();
    }
}

//========================================================================
//
// IN_Stop
//
//========================================================================

void IN_Stop(void)
{
    intermission = false;
    SB_Start();
    BorderNeedRefresh = true;
}

//========================================================================
//
// IN_InitStats
//
//      Initializes the stats for single player mode
//========================================================================

void IN_InitStats(void)
{
    int i;
    int j;
    signed int slaughterfrags;
    int posnum;
    int slaughtercount;
    int playercount;
    int count;

    if (!netgame)
    {
        gametype = SINGLE;
        count = leveltime / 35;
        hours = count / 3600;
        count -= hours * 3600;
        minutes = count / 60;
        count -= minutes * 60;
        seconds = count;

        // [crispy] Show total time on intermission
        count = wbs->totaltimes / 35;
        totalHours = count / 3600;
        count -= totalHours * 3600;
        totalMinutes = count / 60;
        count -= totalMinutes * 60;
        totalSeconds = count;

        if (wbs->modified_partime)
        {
            count = wbs->partime / 35;
            parHours = count / 3600;
            count -= parHours * 3600;
            parMinutes = count / 60;
            count -= parMinutes * 60;
            parSeconds = count;
        }
    }
    else if (netgame && !deathmatch)
    {
        gametype = COOPERATIVE;
        memset(killPercent, 0, MAX_MAXPLAYERS * sizeof(int));
        memset(bonusPercent, 0, MAX_MAXPLAYERS * sizeof(int));
        memset(secretPercent, 0, MAX_MAXPLAYERS * sizeof(int));
        for (i = 0; i < g_maxplayers; i++)
        {
            if (playeringame[i])
            {
                if (totalkills)
                {
                    killPercent[i] = players[i].killcount * 100 / totalkills;
                }
                if (totalitems)
                {
                    bonusPercent[i] = players[i].itemcount * 100 / totalitems;
                }
                if (totalsecret)
                {
                    secretPercent[i] =
                        players[i].secretcount * 100 / totalsecret;
                }
            }
        }
    }
    else
    {
        gametype = DEATHMATCH;
        slaughterboy = 0;
        slaughterfrags = -9999;
        posnum = 0;
        playercount = 0;
        slaughtercount = 0;
        for (i = 0; i < g_maxplayers; i++)
        {
            totalFrags[i] = 0;
            if (playeringame[i])
            {
                playercount++;
                for (j = 0; j < g_maxplayers; j++)
                {
                    if (playeringame[j])
                    {
                        totalFrags[i] += players[i].frags[j];
                    }
                }
                dSlideX[i] = (43 * posnum * FRACUNIT) / 20;
                dSlideY[i] = (36 * posnum * FRACUNIT) / 20;
                posnum++;
            }
            if (totalFrags[i] > slaughterfrags)
            {
                slaughterboy = 1 << i;
                slaughterfrags = totalFrags[i];
                slaughtercount = 1;
            }
            else if (totalFrags[i] == slaughterfrags)
            {
                slaughterboy |= 1 << i;
                slaughtercount++;
            }
        }
        if (playercount == slaughtercount)
        {                       // don't do the slaughter stuff if everyone is equal
            slaughterboy = 0;
        }
    }
}

//========================================================================
//
// IN_Ticker
//
//========================================================================

void IN_Ticker(void)
{
    if (!intermission)
    {
        return;
    }
    if (interstate == IN_ENTERING_DELAY)
    {
        IN_WaitStop();
        return;
    }
    IN_CheckForSkip();
    intertime++;
    if (oldintertime < intertime)
    {
        interstate++;

        // [crispy] skip "now entering" if it's the final intermission
        if (interstate >= IN_FINISHED && finalintermission)
        {
            IN_Stop();
            G_WorldDone();
            return;
        }

        if (!IN_HasInterpic() && interstate >= IN_FINISHED)
        {                       // Extended Wad levels:  skip directly to the next level
            interstate = IN_ENTERING_DELAY;
        }
        switch (interstate)
        {
            case IN_STATS:
                oldintertime = intertime + 300;
                if (!IN_HasInterpic())
                {
                    oldintertime = intertime + 1200;
                }
                break;
            case IN_FINISHED:
                oldintertime = intertime + 200;
                break;
            case IN_ENTERING:
                oldintertime = INT_MAX;
                break;
            case IN_ENTERING_DELAY:
                cnt = 10;
                break;
            default:
                break;
        }
    }
    if (skipintermission)
    {
        if (interstate == IN_STATS && intertime < 150)
        {
            intertime = 150;
            skipintermission = false;
            return;
        }
        // [crispy] skip "now entering" if it's the final intermission
        else if (finalintermission)
        {
            IN_Stop();
            G_WorldDone();
            return;
        }
        else if (interstate < IN_ENTERING && IN_HasInterpic())
        {
            interstate = IN_ENTERING;
            skipintermission = false;
            S_StartVoidSound(heretic_sfx_dorcls);
            return;
        }
        interstate = IN_ENTERING_DELAY;
        cnt = 10;
        skipintermission = false;
        S_StartVoidSound(heretic_sfx_dorcls);
    }
}

//========================================================================
//
// IN_CheckForSkip
//
//      Check to see if any player hit a key
//========================================================================

void IN_CheckForSkip(void)
{
    int i;
    player_t *player;

    for (i = 0, player = players; i < g_maxplayers; i++, player++)
    {
        if (playeringame[i])
        {
            if (player->cmd.buttons & BT_ATTACK)
            {
                if (!player->attackdown)
                {
                    skipintermission = 1;
                }
                player->attackdown = true;
            }
            else
            {
                player->attackdown = false;
            }
            if (player->cmd.buttons & BT_USE)
            {
                if (!player->usedown)
                {
                    skipintermission = 1;
                }
                player->usedown = true;
            }
            else
            {
                player->usedown = false;
            }
        }
    }
}

//========================================================================
//
// IN_Drawer
//
//========================================================================

void IN_Drawer(void)
{
    static int oldinterstate;

    if (!intermission)
    {
        return;
    }
    if (interstate == IN_ENTERING_DELAY)
    {
        return;
    }

    if (oldinterstate != IN_ENTERING && interstate == IN_ENTERING)
    {
        S_StartVoidSound(heretic_sfx_pstop);
    }
    oldinterstate = interstate;
    switch (interstate)
    {
        case -1:
        case IN_STATS:                // draw stats
            dsda_PrepareFinished();
            IN_DrawStatBack();
            switch (gametype)
            {
                case SINGLE:
                    IN_DrawSingleStats();
                    break;
                case COOPERATIVE:
                    IN_DrawCoopStats();
                    break;
                case DEATHMATCH:
                    IN_DrawDMStats();
                    break;
            }
            break;
        case IN_FINISHED:                // leaving old level
            if (IN_HasInterpic())
            {
                IN_DrawInterpic();
                IN_DrawOldLevel();
            }
            break;
        case IN_ENTERING:                // going to the next level
            if (IN_HasInterpic() && !finalintermission)
            {
                IN_DrawInterpic();
                IN_DrawYAH();
            }
            break;
        case IN_ENTERING_DELAY:                // waiting before going to the next level
            if (IN_HasInterpic())
            {
                IN_DrawInterpic();
            }
            break;
        default:
            I_Error("IN_lude:  Intermission state out of range.\n");
            break;
    }
}

//========================================================================
//
// IN_DrawStatBack
//
//========================================================================

void IN_DrawStatBack(void)
{
    // e6y: wide-res
    V_ClearBorder();

    if (exitpic)
    {
        V_DrawNamePatch(0, 0, 0, exitpic, CR_DEFAULT, VPT_STRETCH);
    }
    else
    {
        V_DrawBackground("FLOOR16", 0);
    }
}

//========================================================================
//
// IN_DrawOldLevel
//
//========================================================================

void IN_DrawOldLevel(void)
{
    int x;

    const char *prev_level_name = lf_levelname ? lf_levelname : NameForMap(prevmap);
    IN_DrawLevelname(lf_levelpic, prev_level_name, 3);
    x = 160 - MN_TextAWidth("FINISHED") / 2;
    MN_DrTextA("FINISHED", x, 25);

    if (exitpic) return;

    if (prevmap == 9)
    {
        for (int i = 0; i < nextmap - 1; i++)
        {
            IN_DrawBeenThere(i);
        }
        if (!(intertime & 16))
        {
            IN_DrawBeenThere(8);
        }
    }
    else
    {
        for (int i = 0; i < prevmap - 1; i++)
        {
            IN_DrawBeenThere(i);
        }
        if (players[consoleplayer].didsecret)
        {
            IN_DrawBeenThere(8);
        }
        if (!(intertime & 16))
        {
            IN_DrawBeenThere(prevmap - 1);
        }
    }
}

//========================================================================
//
// IN_DrawYAH
//
//========================================================================

void IN_DrawYAH(void)
{
    const char *level_name = el_levelname ? el_levelname : NameForMap(nextmap);

    int x = 160 - MN_TextAWidth("NOW ENTERING:") / 2;
    MN_DrTextA("NOW ENTERING:", x, 10);
    IN_DrawLevelname(el_levelpic, level_name, 20);

    if (prevmap == 9)
    {
        prevmap = nextmap - 1;
    }
    if (enterpic || exitpic) return;

    for (int i = 0; i < prevmap; i++)
    {
        IN_DrawBeenThere(i);
    }
    if (players[consoleplayer].didsecret)
    {
        IN_DrawBeenThere(8);
    }
    if (!(intertime & 16) || interstate == IN_ENTERING_DELAY)
    {                           // draw the destination 'X'
        IN_DrawGoingThere(nextmap - 1);
    }
}

//========================================================================
//
// IN_DrawSingleStats
//
//========================================================================

void IN_DrawSingleStats(void)
{
    int x;
    static int sounds;

    // [crispy] offset the stats for Ep.4 and up, to make room for level time
    int yoffset = 3;

    const char *prev_level_name = lf_levelname ? lf_levelname : NameForMap(prevmap);
    IN_DrawLevelname(lf_levelpic, prev_level_name, yoffset);
    yoffset += 20;

    if (lf_author)
    {
        x = 160 - MN_TextAWidth(lf_author) / 2;
        MN_DrTextA(lf_author, x, yoffset);
        yoffset += (5 * hud_font.height / 4);
    }

    x = 160 - MN_TextAWidth("FINISHED") / 2;
    MN_DrTextA("FINISHED", x, yoffset);

    if (gamemode == retail && !IN_HasInterpic())
    {
        yoffset -= 20;
    }

    IN_DrTextB("KILLS", 50, yoffset + 25);
    IN_DrTextB("ITEMS", 50, yoffset + 50);
    IN_DrTextB("SECRETS", 50, yoffset + 75);

    dsda_DrawExIntermission();

    if (intertime < 30)
    {
        sounds = 0;
        return;
    }
    if (sounds < 1 && intertime >= 30)
    {
        S_StartVoidSound(heretic_sfx_dorcls);
        sounds++;
    }
    IN_DrawNumber(players[consoleplayer].killcount, 200, yoffset + 25, 3);
    V_DrawShadowedNamePatch(237, yoffset + 25, "FONTB15");
    IN_DrawNumber(totalkills, 248, yoffset + 25, 3);
    if (intertime < 60)
    {
        return;
    }
    if (sounds < 2 && intertime >= 60)
    {
        S_StartVoidSound(heretic_sfx_dorcls);
        sounds++;
    }
    IN_DrawNumber(players[consoleplayer].itemcount, 200, yoffset + 50, 3);
    V_DrawShadowedNamePatch(237, yoffset + 50, "FONTB15");
    IN_DrawNumber(totalitems, 248, yoffset + 50, 3);
    if (intertime < 90)
    {
        return;
    }
    if (sounds < 3 && intertime >= 90)
    {
        S_StartVoidSound(heretic_sfx_dorcls);
        sounds++;
    }
    IN_DrawNumber(players[consoleplayer].secretcount, 200, yoffset + 75, 3);
    V_DrawShadowedNamePatch(237, yoffset + 75, "FONTB15");
    IN_DrawNumber(totalsecret, 248, yoffset + 75, 3);
    if (intertime < 150)
    {
        return;
    }
    if (sounds < 4 && intertime >= 150)
    {
        S_StartVoidSound(heretic_sfx_dorcls);
        sounds++;
    }

    dsda_PrepareEntering();

    // [crispy] ignore "now entering" if it's the final intermission
    if (gamemode != retail || IN_HasInterpic() || finalintermission)
    {
        yoffset = 30;
    }
    else
    {
        yoffset = 0;

        const char *next_level_name = NameForMap(nextmap);
        if (el_levelname) next_level_name = el_levelname;

        x = 160 - MN_TextAWidth("NOW ENTERING:") / 2;
        MN_DrTextA("NOW ENTERING:", x, 160);
        x = 160 - MN_TextBWidth(next_level_name) / 2;
        IN_DrTextB(next_level_name, x, 170);
        skipintermission = false;
    }

    if (wbs->modified_partime)
    {
        IN_DrTextB("TIME", 8, 120 + yoffset);
        IN_DrawTime(55, 120 + yoffset, hours, minutes, seconds);
        IN_DrTextB("TOTAL", 8, 140 + yoffset);
        IN_DrawTime(55, 140 + yoffset, totalHours, totalMinutes, totalSeconds);

        IN_DrTextB("PAR", 170, 120 + yoffset);
        IN_DrawTime(218, 120 + yoffset, parHours, parMinutes, parSeconds);
    }
    else
    {
        IN_DrTextB("TIME", 85, 120 + yoffset);
        IN_DrawTime(155, 120 + yoffset, hours, minutes, seconds);
        IN_DrTextB("TOTAL", 85, 140 + yoffset);
        IN_DrawTime(155, 140 + yoffset, totalHours, totalMinutes, totalSeconds);
    }

}

//========================================================================
//
// IN_DrawCoopStats
//
//========================================================================

void IN_DrawCoopStats(void)
{
    const char *level_name = NameForMap(prevmap);
    if (lf_levelname) level_name = lf_levelname;

    int x = 160 - MN_TextBWidth(level_name) / 2;
    IN_DrTextB(level_name, x, 3);
    x = 160 - MN_TextAWidth("FINISHED") / 2;
    MN_DrTextA("FINISHED", x, 25);

    IN_DrTextB("KILLS", 95, 35);
    IN_DrTextB("BONUS", 155, 35);
    IN_DrTextB("SECRET", 232, 35);

    static int sounds;
    int ypos = 50;
    for (int i = 0; i < g_maxplayers; i++)
    {
        if (playeringame[i])
        {
            V_DrawShadowedNumPatch(25, ypos, patchFaceOkayBase + i);
            if (intertime < 40)
            {
                sounds = 0;
                ypos += 37;
                continue;
            }
            else if (intertime >= 40 && sounds < 1)
            {
                S_StartVoidSound(heretic_sfx_dorcls);
                sounds++;
            }
            IN_DrawNumber(killPercent[i], 85, ypos + 10, 3);
            V_DrawShadowedNamePatch(121, ypos + 10, "FONTB05");
            IN_DrawNumber(bonusPercent[i], 160, ypos + 10, 3);
            V_DrawShadowedNamePatch(196, ypos + 10, "FONTB05");
            IN_DrawNumber(secretPercent[i], 237, ypos + 10, 3);
            V_DrawShadowedNamePatch(273, ypos + 10, "FONTB05");
            ypos += 37;
        }
    }
}

//========================================================================
//
// IN_DrawDMStats
//
//========================================================================

void IN_DrawDMStats(void)
{
    int i;
    int j;
    int ypos;
    int xpos;
    int kpos;

    static int sounds;

    xpos = 90;
    ypos = 55;

    IN_DrTextB("TOTAL", 265, 30);
    MN_DrTextA("VICTIMS", 140, 8);
    for (i = 0; i < 7; i++)
    {
        MN_DrTextA(KillersText[i], 10, 80 + 9 * i);
    }
    if (intertime < 20)
    {
        for (i = 0; i < g_maxplayers; i++)
        {
            if (playeringame[i])
            {
                V_DrawShadowedNumPatch(
                  40,
                  ((ypos << FRACBITS) + dSlideY[i] * intertime) >> FRACBITS,
                  patchFaceOkayBase + i
                );
                V_DrawShadowedNumPatch(
                  ((xpos << FRACBITS) + dSlideX[i] * intertime) >> FRACBITS,
                  18,
                  patchFaceDeadBase + i
                );
            }
        }
        sounds = 0;
        return;
    }
    if (intertime >= 20 && sounds < 1)
    {
        S_StartVoidSound(heretic_sfx_dorcls);
        sounds++;
    }
    if (intertime >= 100 && slaughterboy && sounds < 2)
    {
        S_StartVoidSound(heretic_sfx_wpnup);
        sounds++;
    }
    for (i = 0; i < g_maxplayers; i++)
    {
        if (playeringame[i])
        {
            if (intertime < 100 || i == consoleplayer)
            {
                V_DrawShadowedNumPatch(40, ypos, patchFaceOkayBase + i);
                V_DrawShadowedNumPatch(xpos, 18, patchFaceDeadBase + i);
            }
            else
            {
                V_DrawTLNumPatch(40, ypos, patchFaceOkayBase + i);
                V_DrawTLNumPatch(xpos, 18, patchFaceDeadBase + i);
            }
            kpos = 86;
            for (j = 0; j < g_maxplayers; j++)
            {
                if (playeringame[j])
                {
                    IN_DrawNumber(players[i].frags[j], kpos, ypos + 10, 3);
                    kpos += 43;
                }
            }
            if (slaughterboy & (1 << i))
            {
                if (!(intertime & 16))
                {
                    IN_DrawNumber(totalFrags[i], 263, ypos + 10, 3);
                }
            }
            else
            {
                IN_DrawNumber(totalFrags[i], 263, ypos + 10, 3);
            }
            ypos += 36;
            xpos += 43;
        }
    }
}

//========================================================================
//
// IN_DrawTime
//
//========================================================================

// [crispy] always draw seconds; don't 0-pad minutes with no hour
void IN_DrawTime(int x, int y, int h, int m, int s)
{
    if (h)
    {
        IN_DrawNumber(h, x, y, 2);
        IN_DrTextB(":", x + 26, y);
    }
    x += 34;
    if (h || m > 9)
    {
        IN_DrawNumber(m, x, y, 2);
    }
    else if (m)
    {
        IN_DrawNumber(m, x + 12, y, 1);
    }
    x += 34;
    {
        IN_DrTextB(":", x - 8, y);
        IN_DrawNumber(s, x, y, 2);
    }
}

//========================================================================
//
// IN_DrawNumber
//
//========================================================================

void IN_DrawNumber(int val, int x, int y, int digits)
{
    int lump;
    int xpos;
    int oldval;
    int realdigits;
    dboolean neg;

    oldval = val;
    xpos = x;
    neg = false;
    realdigits = 1;

    if (val < 0)
    {                           //...this should reflect negative frags
        val = -val;
        neg = true;
        if (val > 99)
        {
            val = 99;
        }
    }
    if (val > 9)
    {
        realdigits++;
        if (digits < realdigits)
        {
            realdigits = digits;
            val = 9;
        }
    }
    if (val > 99)
    {
        realdigits++;
        if (digits < realdigits)
        {
            realdigits = digits;
            val = 99;
        }
    }
    if (val > 999)
    {
        realdigits++;
        if (digits < realdigits)
        {
            realdigits = digits;
            val = 999;
        }
    }
    if (digits == 4)
    {
        lump = FontBNumbers[val / 1000];
        V_DrawShadowedNumPatch(xpos + 6 - R_NumPatchWidth(lump) / 2 - 12, y, lump);
    }
    if (digits > 2)
    {
        if (realdigits > 2)
        {
            lump = FontBNumbers[val / 100];
            V_DrawShadowedNumPatch(xpos + 6 - R_NumPatchWidth(lump) / 2, y, lump);
        }
        xpos += 12;
    }
    val = val % 100;
    if (digits > 1)
    {
        if (val > 9)
        {
            lump = FontBNumbers[val / 10];
            V_DrawShadowedNumPatch(xpos + 6 - R_NumPatchWidth(lump) / 2, y, lump);
        }
        else if (digits == 2 || oldval > 99)
        {
            V_DrawShadowedNumPatch(xpos, y, FontBNumbers[0]);
        }
        xpos += 12;
    }
    val = val % 10;
    lump = FontBNumbers[val];
    V_DrawShadowedNumPatch(xpos + 6 - R_NumPatchWidth(lump) / 2, y, lump);
    if (neg)
    {
        V_DrawShadowedNamePatch(
          xpos + 6 - R_NamePatchWidth("FONTB13") / 2 - 12 * (realdigits),
          y,
          "FONTB13"
        );
    }
}

//========================================================================
//
// IN_DrTextB
//
//========================================================================

void IN_DrTextB(const char *text, int x, int y)
{
    char c;

    while ((c = *text++) != 0)
    {
        if (c < 33)
        {
            x += 8;
        }
        else
        {
            if (c > 90) c -= 32;
            int lump = FontBLump + c - 33;
            V_DrawShadowedNumPatch(x, y, lump);
            x += R_NumPatchWidth(lump) - 1;
        }
    }
}

void IN_DrawLevelname(const char *patch, const char *levelname, int y)
{
  int x;
  if (W_LumpNameExists(patch))
  {
    x = 160 - V_NamePatchWidth(patch) / 2;
    V_DrawNamePatch(x, y, 0, patch, CR_DEFAULT, VPT_STRETCH);
  }
  else
  {
    x = 160 - MN_TextBWidth(levelname) / 2;
    IN_DrTextB(levelname, x, y);
  }
}
