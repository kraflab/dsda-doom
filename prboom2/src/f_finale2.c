/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *  Copyright 2017 by
 *  Christoph Oelckers
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      Game completion, final screen animation.
 *      This is an alternative version for UMAPINFO defined
 *      intermission so that the feature can be cleanly implemented
 *      without worrying about demo sync implications
 *
 *-----------------------------------------------------------------------------
 */

#include "doomstat.h"
#include "d_event.h"
#include "g_game.h"
#include "v_video.h"
#include "w_wad.h"
#include "s_sound.h"
#include "sounds.h"
#include "d_deh.h"

void F_StartCast (void);
void F_TextWrite(void);
void F_BunnyScroll(void);
void WI_checkForAccelerate(void);
float Get_TextSpeed(void);

extern int finalestage;
extern int finalecount;
extern const char* finaletext;
extern const char* finaleflat;
extern const char* finalepatch;
extern int acceleratestage;
extern int midstage;
extern dboolean secretexit;

#define TEXTSPEED    3
#define TEXTWAIT     250
#define NEWTEXTWAIT  1000

void FMI_StartFinale(void)
{
  if (gamemapinfo->intertextsecret && secretexit && gamemapinfo->intertextsecret[0] != '-') // '-' means that any default intermission was cleared.
  {
    finaletext = gamemapinfo->intertextsecret;
  }
  else if (gamemapinfo->intertext && !secretexit && gamemapinfo->intertext[0] != '-') // '-' means that any default intermission was cleared.
  {
    finaletext = gamemapinfo->intertext;
  }

  if (!finaletext) finaletext = "The End"; // this is to avoid a crash on a missing text in the last map.

  if (gamemapinfo->interbackdrop[0])
  {
    if (W_CheckNumForName(gamemapinfo->interbackdrop) != -1 &&
        (W_CheckNumForName)(gamemapinfo->interbackdrop, ns_flats) == -1)
    {
      finalepatch = gamemapinfo->interbackdrop;
    }
    else
    {
      finaleflat = gamemapinfo->interbackdrop;
    }
  }

  if (!finaleflat) finaleflat = "FLOOR4_8";  // use a single fallback for all maps.
}

int FMI_Ticker(void)
{
  int i;
  if (!demo_compatibility)
    WI_checkForAccelerate(); // killough 3/28/98: check for acceleration
  else
    for (i = 0; i < g_maxplayers; i++)
      if (players[i].cmd.buttons)
        goto next_level;

  // advance animation
  finalecount++;

  if (!finalestage)
  {
    float speed = demo_compatibility ? TEXTSPEED : Get_TextSpeed();
    /* killough 2/28/98: changed to allow acceleration */
    if (
      finalecount > strlen(finaletext) * speed + (midstage ? NEWTEXTWAIT : TEXTWAIT) ||
      (midstage && acceleratestage)
    )
    {

    next_level:
      if (gamemapinfo->endpic[0] && (strcmp(gamemapinfo->endpic, "-") != 0))
      {
        if (!stricmp(gamemapinfo->endpic, "$CAST"))
        {
          F_StartCast();
          return false; // let go of finale ownership
        }
        else
        {
          finalecount = 0;
          finalestage = 1;
          wipegamestate = -1; // force a wipe
          if (!stricmp(gamemapinfo->endpic, "$BUNNY"))
          {
            S_StartMusic(mus_bunny);
          }
          else if (!stricmp(gamemapinfo->endpic, "!"))
          {
            return false; // let go of finale ownership
          }
        }
      }
      else
      {
        gameaction = ga_worlddone; // next level, e.g. MAP07
      }
    }
  }

  return true; // keep finale ownership
}

void FMI_Drawer(void)
{
  if (!finalestage || !gamemapinfo->endpic[0] || (strcmp(gamemapinfo->endpic, "-") == 0))
  {
    F_TextWrite();
  }
  else if (strcmp(gamemapinfo->endpic, "$BUNNY") == 0)
  {
    F_BunnyScroll();
  }
  else
  {
    // e6y: wide-res
    V_FillBorder(-1, 0);
    V_DrawNamePatch(0, 0, 0, gamemapinfo->endpic, CR_DEFAULT, VPT_STRETCH);
  }
}
