/* Emacs style mode select   -*- C -*-
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
 *  Sky rendering. The DOOM sky is a texture map like any
 *  wall, wrapping around. A 1024 columns equal 360 degrees.
 *  The default sky map is 256 columns and repeats 4 times
 *  on a 320 screen?
 *
 *-----------------------------------------------------------------------------*/

#include "r_sky.h"
#include "r_main.h"
#include "e6y.h"

#include "dsda/configuration.h"
#include "dsda/mapinfo.h"
#include "dsda/settings.h"

//
// sky mapping
//
int skyflatnum;
int skytexture;
int skytexturemid;

int skystretch;
fixed_t freelookviewheight;

//
// R_InitSkyMap
// Called whenever the view size changes.
//
void R_InitSkyMap(void)
{
  int r_stretchsky;

  r_stretchsky = dsda_IntConfig(dsda_config_render_stretchsky);

  if (raven || !dsda_MouseLook())
  {
    skystretch = false;
    skytexturemid = (raven ? 200 : 100) * FRACUNIT;
    skyiscale = (200 << FRACBITS) / SCREENHEIGHT;
  }
  else
  {
    int skyheight;

    if (!textureheight)
      return;

    // There are various combinations for sky rendering depending on how tall the sky is:
    //        h <  128: Unstretched and tiled, centered on horizon
    // 128 <= h <  200: Can possibly be stretched. When unstretched, the baseline is
    //                  28 rows below the horizon so that the top of the texture
    //                  aligns with the top of the screen when looking straight ahead.
    //                  When stretched, it is scaled to 228 pixels with the baseline
    //                  in the same location as an unstretched 128-tall sky, so the top
    //					of the texture aligns with the top of the screen when looking
    //                  fully up.
    //        h == 200: Unstretched, baseline is on horizon, and top is at the top of
    //                  the screen when looking fully up.
    //        h >  200: Unstretched, but the baseline is shifted down so that the top
    //                  of the texture is at the top of the screen when looking fully up.

    skyheight = textureheight[skytexture]>>FRACBITS;
    skystretch = false;
    skytexturemid = 0;
    if (skyheight >= 128 && skyheight < 200)
    {
      skystretch = (r_stretchsky && skyheight >= 128);
      skytexturemid = -28*FRACUNIT;
    }
    else if (skyheight > 200)
    {
      skytexturemid = (200 - skyheight) << FRACBITS;
    }

    skyiscale = (200 << FRACBITS) / SCREENHEIGHT;

    if (skystretch)
    {
      skyiscale = (fixed_t)((int64_t)skyiscale * skyheight / SKYSTRETCH_HEIGHT);
      skytexturemid = (int)((int64_t)skytexturemid * skyheight / SKYSTRETCH_HEIGHT);
    }
    else
    {
      skytexturemid = 100*FRACUNIT;
    }
  }
}
