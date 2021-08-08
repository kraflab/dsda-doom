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
 *      Here is a core component: drawing the floors and ceilings,
 *       while maintaining a per column clipping list only.
 *      Moreover, the sky areas have to be determined.
 *
 * MAXVISPLANES is no longer a limit on the number of visplanes,
 * but a limit on the number of hash slots; larger numbers mean
 * better performance usually but after a point they are wasted,
 * and memory and time overheads creep in.
 *
 * For more information on visplanes, see:
 *
 * http://classicgaming.com/doom/editing/
 *
 * Lee Killough
 *
 *-----------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "z_zone.h"  /* memory allocation wrappers -- killough */

#include "doomstat.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_draw.h"
#include "r_things.h"
#include "r_sky.h"
#include "r_plane.h"
#include "r_main.h"
#include "v_video.h"
#include "lprintf.h"

int Sky1Texture;
int Sky2Texture;
fixed_t Sky1ColumnOffset;
fixed_t Sky2ColumnOffset;
dboolean DoubleSky;

#define MAXVISPLANES 256    /* must be a power of 2 */

static visplane_t *visplanes[MAXVISPLANES];   // killough
static visplane_t *freetail;                  // killough
static visplane_t **freehead = &freetail;     // killough
visplane_t *floorplane, *ceilingplane;

// killough -- hash function for visplanes
// Empirically verified to be fairly uniform:

#define visplane_hash(picnum,lightlevel,height) \
  ((unsigned)((picnum)*3+(lightlevel)+(height)*7) & (MAXVISPLANES-1))

size_t maxopenings;
int *openings,*lastopening; // dropoff overflow

// Clip values are the solid pixel bounding the range.
//  floorclip starts out SCREENHEIGHT
//  ceilingclip starts out -1

// dropoff overflow
// e6y: resolution limitation is removed
int *floorclip = NULL;
int *ceilingclip = NULL;

// spanstart holds the start of a plane span; initialized to 0 at start

// e6y: resolution limitation is removed
static int *spanstart = NULL;                // killough 2/8/98

//
// texture mapping
//

static const lighttable_t **planezlight;
static fixed_t planeheight;

// killough 2/8/98: make variables static

static fixed_t basexscale, baseyscale;
static fixed_t *cachedheight = NULL;
static fixed_t xoffs,yoffs;    // killough 2/28/98: flat offsets

// e6y: resolution limitation is removed
fixed_t *yslope = NULL;
fixed_t *distscale = NULL;

void R_InitPlanesRes(void)
{
  if (floorclip) free(floorclip);
  if (ceilingclip) free(ceilingclip);
  if (spanstart) free(spanstart);

  if (cachedheight) free(cachedheight);

  if (yslope) free(yslope);
  if (distscale) free(distscale);

  floorclip = calloc(1, SCREENWIDTH * sizeof(*floorclip));
  ceilingclip = calloc(1, SCREENWIDTH * sizeof(*ceilingclip));
  spanstart = calloc(1, SCREENHEIGHT * sizeof(*spanstart));

  cachedheight = calloc(1, SCREENHEIGHT * sizeof(*cachedheight));

  yslope = calloc(1, SCREENHEIGHT * sizeof(*yslope));
  distscale = calloc(1, SCREENWIDTH * sizeof(*distscale));
}

void R_InitVisplanesRes(void)
{
  int i;

  freetail = NULL;
  freehead = &freetail;

  for (i = 0; i < MAXVISPLANES; i++)
  {
    visplanes[i] = 0;
  }
}

//
// R_InitPlanes
// Only at game startup.
//
void R_InitPlanes (void)
{
}

//
// R_MapPlane
//
// Uses global vars:
//  planeheight
//  dsvars.source
//  basexscale
//  baseyscale
//  viewx
//  viewy
//  xoffs
//  yoffs
//
// BASIC PRIMITIVE
//

static void R_MapPlane(int y, int x1, int x2, draw_span_vars_t *dsvars)
{
  int_64_t den;
  fixed_t distance;
  unsigned index;

#ifdef RANGECHECK
  if (x2 < x1 || x1<0 || x2>=viewwidth || (unsigned)y>(unsigned)viewheight)
    I_Error ("R_MapPlane: %i, %i at %i",x1,x2,y);
#endif

  // [RH]Instead of using the xtoviewangle array, I calculated the fractional values
  // at the middle of the screen, then used the calculated ds_xstep and ds_ystep
  // to step from those to the proper texture coordinate to start drawing at.
  // That way, the texture coordinate is always calculated by its position
  // on the screen and not by its position relative to the edge of the visplane.
  //
  // Visplanes with the same texture now match up far better than before.
  //
  // See cchest2.wad/map02/room with sector #265
  if (centery == y)
    return;
  den = (int_64_t)FRACUNIT * FRACUNIT * D_abs(centery - y);
  distance = FixedMul (planeheight, yslope[y]);

  dsvars->xstep = (fixed_t)((int_64_t)viewsin * planeheight * viewfocratio / den);
  dsvars->ystep = (fixed_t)((int_64_t)viewcos * planeheight * viewfocratio / den);

  // killough 2/28/98: Add offsets
  dsvars->xfrac =  viewx + xoffs + FixedMul(viewcos, distance) + (x1 - centerx) * dsvars->xstep;
  dsvars->yfrac = -viewy + yoffs - FixedMul(viewsin, distance) + (x1 - centerx) * dsvars->ystep;

  if (drawvars.filterfloor == RDRAW_FILTER_LINEAR) {
    dsvars->xfrac -= (FRACUNIT>>1);
    dsvars->yfrac -= (FRACUNIT>>1);
  }

  if (!(dsvars->colormap = fixedcolormap))
    {
      dsvars->z = distance;
      index = distance >> LIGHTZSHIFT;
      if (index >= MAXLIGHTZ )
        index = MAXLIGHTZ-1;
      dsvars->colormap = planezlight[index];
      dsvars->nextcolormap = planezlight[index+1 >= MAXLIGHTZ ? MAXLIGHTZ-1 : index+1];
    }
  else
   {
      dsvars->z = 0;
   }

  dsvars->y = y;
  dsvars->x1 = x1;
  dsvars->x2 = x2;

  if (V_IsSoftwareMode())
    R_DrawSpan(dsvars);
}

//
// R_ClearPlanes
// At begining of frame.
//

void R_ClearPlanes(void)
{
  int i;

  // opening / clipping determination
  for (i=0 ; i<viewwidth ; i++)
    floorclip[i] = viewheight, ceilingclip[i] = -1;

  for (i=0;i<MAXVISPLANES;i++)    // new code -- killough
    for (*freehead = visplanes[i], visplanes[i] = NULL; *freehead; )
      freehead = &(*freehead)->next;

  lastopening = openings;

  // texture calculation
  memset (cachedheight, 0, SCREENHEIGHT * sizeof(*cachedheight));

  // scale will be unit scale at SCREENWIDTH/2 distance
  basexscale = FixedDiv (viewsin,projection);
  baseyscale = FixedDiv (viewcos,projection);
}

// New function, by Lee Killough

static visplane_t *new_visplane(unsigned hash)
{
  visplane_t *check = freetail;
  if (!check)
  {
    // e6y: resolution limitation is removed
    check = calloc(1, sizeof(*check) + sizeof(*check->top) * (SCREENWIDTH * 2));
    check->bottom = &check->top[SCREENWIDTH + 2];
  }
  else
    if (!(freetail = freetail->next))
      freehead = &freetail;
  check->next = visplanes[hash];
  visplanes[hash] = check;
  return check;
}

/*
 * R_DupPlane
 *
 * cph 2003/04/18 - create duplicate of existing visplane and set initial range
 */
visplane_t *R_DupPlane(const visplane_t *pl, int start, int stop)
{
      int i;
      unsigned hash = visplane_hash(pl->picnum, pl->lightlevel, pl->height);
      visplane_t *new_pl = new_visplane(hash);

      new_pl->height = pl->height;
      new_pl->picnum = pl->picnum;
      new_pl->lightlevel = pl->lightlevel;
      new_pl->special = pl->special;
      new_pl->xoffs = pl->xoffs;           // killough 2/28/98
      new_pl->yoffs = pl->yoffs;
      new_pl->minx = start;
      new_pl->maxx = stop;
      for (i = 0; i != SCREENWIDTH; i++)
        new_pl->top[i] = SHRT_MAX;
      return new_pl;
}
//
// R_FindPlane
//
// killough 2/28/98: Add offsets

visplane_t *R_FindPlane(fixed_t height, int picnum, int lightlevel, int special,
                        fixed_t xoffs, fixed_t yoffs)
{
  visplane_t *check;
  unsigned hash;                      // killough

  if (hexen && special < 150)
  {
    special = 0;
  }

  if (picnum == skyflatnum || picnum & PL_SKYFLAT)
    height = lightlevel = 0;         // killough 7/19/98: most skies map together

  // New visplane algorithm uses hash table -- killough
  hash = visplane_hash(picnum,lightlevel,height);

  for (check=visplanes[hash]; check; check=check->next)  // killough
    if (height == check->height &&
        picnum == check->picnum &&
        lightlevel == check->lightlevel &&
        special == check->special &&
        xoffs == check->xoffs &&      // killough 2/28/98: Add offset checks
        yoffs == check->yoffs)
      return check;

  check = new_visplane(hash);         // killough

  check->height = height;
  check->picnum = picnum;
  check->lightlevel = lightlevel;
  check->special = special;
  check->xoffs = xoffs;               // killough 2/28/98: Save offsets
  check->yoffs = yoffs;
#ifdef GL_DOOM
  if (V_IsSoftwareMode())
#endif
  {
    int i;
    check->minx = viewwidth; // Was SCREENWIDTH -- killough 11/98
    check->maxx = -1;

    for (i = 0; i != SCREENWIDTH; i++)
      check->top[i] = SHRT_MAX;
  }

  return check;
}

//
// R_CheckPlane
//
visplane_t *R_CheckPlane(visplane_t *pl, int start, int stop)
{
  int intrl, intrh, unionl, unionh, x;

  if (start < pl->minx)
    intrl   = pl->minx, unionl = start;
  else
    unionl  = pl->minx,  intrl = start;

  if (stop  > pl->maxx)
    intrh   = pl->maxx, unionh = stop;
  else
    unionh  = pl->maxx, intrh  = stop;

  for (x=intrl ; x <= intrh && pl->top[x] == SHRT_MAX; x++) // dropoff overflow
    ;

  if (x > intrh) { /* Can use existing plane; extend range */
    pl->minx = unionl; pl->maxx = unionh;
    return pl;
  } else /* Cannot use existing plane; create a new one */
    return R_DupPlane(pl,start,stop);
}

//
// R_MakeSpans
//

static void R_MakeSpans(int x, unsigned int t1, unsigned int b1,
                        unsigned int t2, unsigned int b2,
                        draw_span_vars_t *dsvars)
{
  for (; t1 < t2 && t1 <= b1; t1++)
    R_MapPlane(t1, spanstart[t1], x-1, dsvars);
  for (; b1 > b2 && b1 >= t1; b1--)
    R_MapPlane(b1, spanstart[b1] ,x-1, dsvars);
  while (t2 < t1 && t2 <= b2)
    spanstart[t2++] = x;
  while (b2 > b1 && b2 >= t2)
    spanstart[b2--] = x;
}

// New function, by Lee Killough

static void R_DoDrawPlane(visplane_t *pl)
{
  register int x;
  draw_column_vars_t dcvars;
  R_DrawColumn_f colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_STANDARD, drawvars.filterwall, drawvars.filterz);

  R_SetDefaultDrawColumnVars(&dcvars);

  if (pl->minx <= pl->maxx) {
    // hexen_note: Skies
    // if (pl->picnum == skyflatnum)
    // {                       // Sky flat
    //     #define SKYTEXTUREMIDSHIFTED 200
    //
    //     byte *source;
    //     byte *source2;
    //     int offset;
    //     int skyTexture;
    //     int offset2;
    //     int skyTexture2;
    //
    //     if (DoubleSky)
    //     {                   // Render 2 layers, sky 1 in front
    //         offset = Sky1ColumnOffset >> 16;
    //         skyTexture = texturetranslation[Sky1Texture];
    //         offset2 = Sky2ColumnOffset >> 16;
    //         skyTexture2 = texturetranslation[Sky2Texture];
    //         for (x = pl->minx; x <= pl->maxx; x++)
    //         {
    //             dc_yl = pl->top[x];
    //             dc_yh = pl->bottom[x];
    //             if (dc_yl <= dc_yh)
    //             {
    //                 count = dc_yh - dc_yl;
    //                 if (count < 0)
    //                 {
    //                     return;
    //                 }
    //                 angle = (viewangle + xtoviewangle[x])
    //                     >> ANGLETOSKYSHIFT;
    //                 source = R_GetColumn(skyTexture, angle + offset);
    //                 source2 = R_GetColumn(skyTexture2, angle + offset2);
    //                 dest = ylookup[dc_yl] + columnofs[x];
    //                 frac = SKYTEXTUREMIDSHIFTED * FRACUNIT + (dc_yl - centery) * fracstep;
    //                 do
    //                 {
    //                     if (source[frac >> FRACBITS])
    //                     {
    //                         *dest = source[frac >> FRACBITS];
    //                         frac += fracstep;
    //                     }
    //                     else
    //                     {
    //                         *dest = source2[frac >> FRACBITS];
    //                         frac += fracstep;
    //                     }
    //                     dest += SCREENWIDTH;
    //                 }
    //                 while (count--);
    //             }
    //         }
    //         continue;       // Next visplane
    //     }
    //     else
    //     {                   // Render single layer
    //         if (pl->special == 200)
    //         {               // Use sky 2
    //             offset = Sky2ColumnOffset >> 16;
    //             skyTexture = texturetranslation[Sky2Texture];
    //         }
    //         else
    //         {               // Use sky 1
    //             offset = Sky1ColumnOffset >> 16;
    //             skyTexture = texturetranslation[Sky1Texture];
    //         }
    //         for (x = pl->minx; x <= pl->maxx; x++)
    //         {
    //             dc_yl = pl->top[x];
    //             dc_yh = pl->bottom[x];
    //             if (dc_yl <= dc_yh)
    //             {
    //                 count = dc_yh - dc_yl;
    //                 if (count < 0)
    //                 {
    //                     return;
    //                 }
    //                 angle = (viewangle + xtoviewangle[x])
    //                     >> ANGLETOSKYSHIFT;
    //                 source = R_GetColumn(skyTexture, angle + offset);
    //                 dest = ylookup[dc_yl] + columnofs[x];
    //                 frac = SKYTEXTUREMIDSHIFTED * FRACUNIT + (dc_yl - centery) * fracstep;
    //                 do
    //                 {
    //                     *dest = source[frac >> FRACBITS];
    //                     dest += SCREENWIDTH;
    //                     frac += fracstep;
    //                 }
    //                 while (count--);
    //             }
    //         }
    //         continue;       // Next visplane
    //     }
    // }

    if (pl->picnum == skyflatnum || pl->picnum & PL_SKYFLAT) { // sky flat
      int texture;
      const rpatch_t *tex_patch;
      angle_t an, flip;

      // killough 10/98: allow skies to come from sidedefs.
      // Allows scrolling and/or animated skies, as well as
      // arbitrary multiple skies per level without having
      // to use info lumps.

      an = viewangle;

      if (pl->picnum & PL_SKYFLAT)
      {
        // Sky Linedef
        const line_t *l = &lines[pl->picnum & ~PL_SKYFLAT];

        // Sky transferred from first sidedef
        const side_t *s = *l->sidenum + sides;

        // Texture comes from upper texture of reference sidedef
        texture = texturetranslation[s->toptexture];

        // Horizontal offset is turned into an angle offset,
        // to allow sky rotation as well as careful positioning.
        // However, the offset is scaled very small, so that it
        // allows a long-period of sky rotation.

        an += s->textureoffset;

        // Vertical offset allows careful sky positioning.

        dcvars.texturemid = s->rowoffset - 28*FRACUNIT;

        // We sometimes flip the picture horizontally.
        //
        // Doom always flipped the picture, so we make it optional,
        // to make it easier to use the new feature, while to still
        // allow old sky textures to be used.

        flip = l->special==272 ? 0u : ~0u;

        if (skystretch)
        {
          int skyheight = textureheight[texture]>>FRACBITS;
          dcvars.texturemid = (int)((int_64_t)dcvars.texturemid * skyheight / SKYSTRETCH_HEIGHT);
        }
      }
      else
      {    // Normal Doom sky, only one allowed per level
        dcvars.texturemid = skytexturemid;    // Default y-offset
        texture = skytexture;             // Default texture
        flip = 0;                         // Doom flips it
      }

      /* Sky is always drawn full bright, i.e. colormaps[0] is used.
       * Because of this hack, sky is not affected by INVUL inverse mapping.
       * Until Boom fixed this. Compat option added in MBF. */

      if (comp[comp_skymap] || !(dcvars.colormap = fixedcolormap))
        dcvars.colormap = fullcolormap;          // killough 3/20/98

      dcvars.nextcolormap = dcvars.colormap; // for filtering -- POPE

      //dcvars.texturemid = skytexturemid;
      dcvars.texheight = textureheight[texture]>>FRACBITS; // killough

      // proff 09/21/98: Changed for high-res

      // e6y
      // disable sky texture scaling if status bar is used
      // old code: dcvars.iscale = FRACUNIT*200/viewheight;
      dcvars.iscale = skyiscale;

      tex_patch = R_CacheTextureCompositePatchNum(texture);

  // killough 10/98: Use sky scrolling offset, and possibly flip picture
        for (x = pl->minx; (dcvars.x = x) <= pl->maxx; x++)
          if ((dcvars.yl = pl->top[x]) != SHRT_MAX && dcvars.yl <= (dcvars.yh = pl->bottom[x])) // dropoff overflow
            {
              dcvars.source = R_GetTextureColumn(tex_patch, ((an + xtoviewangle[x])^flip) >> ANGLETOSKYSHIFT);
              dcvars.prevsource = R_GetTextureColumn(tex_patch, ((an + xtoviewangle[x-1])^flip) >> ANGLETOSKYSHIFT);
              dcvars.nextsource = R_GetTextureColumn(tex_patch, ((an + xtoviewangle[x+1])^flip) >> ANGLETOSKYSHIFT);
              colfunc(&dcvars);
            }

      R_UnlockTextureCompositePatchNum(texture);

    } else {     // regular flat

      int stop, light;
      draw_span_vars_t dsvars;

      dsvars.source = W_CacheLumpNum(firstflat + flattranslation[pl->picnum]);

      if (heretic)
      {
        switch (pl->special)
        {
          case 20:
          case 21:
          case 22:
          case 23:
          case 24:           // Scroll_East
            dsvars.source = dsvars.source +
              ((63 - ((leveltime >> 1) & 63)) << (pl->special - 20) & 63);
            break;
          case 4:            // Scroll_EastLavaDamage
            dsvars.source = dsvars.source +
              (((63 - ((leveltime >> 1) & 63)) << 3) & 63);
            break;
        }
      }
      else if (hexen)
      {
        int scrollOffset = leveltime >> 1 & 63;

        switch (pl->special)
        {                       // Handle scrolling flats
          case 201:
          case 202:
          case 203:          // Scroll_North_xxx
            dsvars.source = dsvars.source + ((scrollOffset
                                       << (pl->special - 201) & 63) << 6);
            break;
          case 204:
          case 205:
          case 206:          // Scroll_East_xxx
            dsvars.source = dsvars.source + ((63 - scrollOffset)
                                      << (pl->special - 204) & 63);
            break;
          case 207:
          case 208:
          case 209:          // Scroll_South_xxx
            dsvars.source = dsvars.source + (((63 - scrollOffset)
                                       << (pl->special - 207) & 63) << 6);
            break;
          case 210:
          case 211:
          case 212:          // Scroll_West_xxx
            dsvars.source = dsvars.source + (scrollOffset
                                      << (pl->special - 210) & 63);
            break;
          case 213:
          case 214:
          case 215:          // Scroll_NorthWest_xxx
            dsvars.source = dsvars.source + (scrollOffset
                                      << (pl->special - 213) & 63) +
                ((scrollOffset << (pl->special - 213) & 63) << 6);
            break;
          case 216:
          case 217:
          case 218:          // Scroll_NorthEast_xxx
            dsvars.source = dsvars.source + ((63 - scrollOffset)
                                      << (pl->special - 216) & 63) +
                ((scrollOffset << (pl->special - 216) & 63) << 6);
            break;
          case 219:
          case 220:
          case 221:          // Scroll_SouthEast_xxx
            dsvars.source = dsvars.source + ((63 - scrollOffset)
                                      << (pl->special - 219) & 63) +
                (((63 - scrollOffset) << (pl->special - 219) & 63) << 6);
            break;
          case 222:
          case 223:
          case 224:          // Scroll_SouthWest_xxx
            dsvars.source = dsvars.source + (scrollOffset
                                      << (pl->special - 222) & 63) +
                (((63 - scrollOffset) << (pl->special - 222) & 63) << 6);
            break;
          default:
            break;
        }
      }

      xoffs = pl->xoffs;  // killough 2/28/98: Add offsets
      yoffs = pl->yoffs;
      planeheight = D_abs(pl->height-viewz);

      // SoM 10/19/02: deep water colormap fix
      if(fixedcolormap)
        light = (255  >> LIGHTSEGSHIFT);
      else
        light = (pl->lightlevel >> LIGHTSEGSHIFT) + (extralight * LIGHTBRIGHT);

      if(light >= LIGHTLEVELS)
        light = LIGHTLEVELS-1;

      if(light < 0)
        light = 0;

      stop = pl->maxx + 1;
      planezlight = zlight[light];
      pl->top[pl->minx-1] = pl->top[stop] = SHRT_MAX; // dropoff overflow

      for (x = pl->minx ; x <= stop ; x++)
         R_MakeSpans(x,pl->top[x-1],pl->bottom[x-1],
                     pl->top[x],pl->bottom[x], &dsvars);

      W_UnlockLumpNum(firstflat + flattranslation[pl->picnum]);
    }
  }
}

//
// RDrawPlanes
// At the end of each frame.
//

void R_DrawPlanes (void)
{
  visplane_t *pl;
  int i;
  for (i=0;i<MAXVISPLANES;i++)
    for (pl=visplanes[i]; pl; pl=pl->next, rendered_visplanes++)
      R_DoDrawPlane(pl);
}
