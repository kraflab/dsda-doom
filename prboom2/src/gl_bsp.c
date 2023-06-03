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
 *      BSP traversal, handling of LineSegs for rendering.
 *
 *-----------------------------------------------------------------------------*/

#include "doomstat.h"
#include "m_bbox.h"
#include "p_spec.h"
#include "r_main.h"
#include "r_segs.h"
#include "r_plane.h"
#include "r_things.h"
#include "r_bsp.h" // cph - sanity checking
#include "v_video.h"
#include "lprintf.h"

// Number of chunks in deferred flat rendering list
static unsigned int numdeferred = 0;

// e6y: Check whether the player can look beyond this line
//

static dboolean CheckClip(gl_wall_t* wall, sector_t* frontsector,
                          sector_t* backsector)
{
  static sector_t tempsec_back, tempsec_front;
  side_t* side = &sides[wall->sidedef];

  backsector = R_FakeFlat(backsector, &tempsec_back, NULL, NULL, true);
  frontsector = R_FakeFlat(frontsector, &tempsec_front, NULL, NULL, false);

  // check for closed sectors!
  if (backsector->ceilingheight <= frontsector->floorheight)
  {
    if (side->toptexture == NO_TEXTURE)
      return false;

    if (backsector->ceilingpic == skyflatnum && frontsector->ceilingpic == skyflatnum)
      return false;

    return true;
  }

  if (frontsector->ceilingheight <= backsector->floorheight)
  {
    if (side->bottomtexture == NO_TEXTURE)
      return false;

    // properly render skies (consider door "open" if both floors are sky):
    if (backsector->ceilingpic == skyflatnum && frontsector->ceilingpic == skyflatnum)
      return false;

    return true;
  }

  if (backsector->ceilingheight <= backsector->floorheight)
  {
    // preserve a kind of transparent door/lift special effect:
    if (backsector->ceilingheight < frontsector->ceilingheight)
    {
      if (side->toptexture == NO_TEXTURE)
        return false;
    }
    if (backsector->floorheight > frontsector->floorheight)
    {
      if (side->bottomtexture == NO_TEXTURE)
        return false;
    }
    if (backsector->ceilingpic == skyflatnum && frontsector->ceilingpic == skyflatnum)
      return false;

    if (backsector->floorpic == skyflatnum && frontsector->floorpic == skyflatnum)
      return false;

    return true;
  }

  return false;
}

// Put chunk in deferral list to render its flats later
static void DeferFlats(gl_chunk_t* chunk)
{
  if (chunk->flags & GL_CHUNKF_DEFERRED)
    // Do nothing if chunk is already in deferral list
    return;

  gl_rstate.deferred[numdeferred++] = chunk;
  chunk->flags |= GL_CHUNKF_DEFERRED;
}

static void ApplyBleed(gl_chunk_t* source, bleed_t* bleed)
{
  bleedtarget_t existing;
  bleedtarget_t* target;
  gl_plane_t* plane;
  dboolean ceiling;
  gl_chunk_t* tchunk = &gl_rstate.chunks[bleed->target];

  switch (bleed->type)
  {
  case BLEED_FLOOR_OVER:
    plane = &source->floor;
    if (plane->picnum == -1)
      return;
    if (plane->height <= tchunk->sector->floorheight)
      return;
    target = &tchunk->floorover;
    ceiling = false;
    break;
  case BLEED_FLOOR_UNDER:
    plane = &source->floor;
    if (plane->picnum == -1)
      return;
    if (plane->height >= tchunk->sector->floorheight)
      return;
    target = &tchunk->floorunder;
    ceiling = false;
    break;
  case BLEED_FLOOR_THROUGH:
    plane = &source->floor;
    if (plane->picnum == -1)
      return;
    if (plane->height != tchunk->sector->floorheight)
      return;
    target = &tchunk->floorthrough;
    ceiling = false;
    break;
  case BLEED_CEILING_OVER:
    plane = &source->ceiling;
    if (plane->picnum == -1)
      return;
    if (plane->height >= tchunk->sector->ceilingheight)
      return;
    target = &tchunk->ceilingover;
    ceiling = true;
    break;
  case BLEED_CEILING_UNDER:
    plane = &source->ceiling;
    if (plane->picnum == -1)
      return;
    if (plane->height <= tchunk->sector->ceilingheight)
      return;
    target = &tchunk->ceilingunder;
    ceiling = true;
    break;
  case BLEED_CEILING_THROUGH:
    plane = &source->ceiling;
    if (plane->picnum == -1)
      return;
    if (plane->height != tchunk->sector->ceilingheight)
      return;
    target = &tchunk->ceilingthrough;
    ceiling = true;
    break;
  default:
    // Impossible absent a bug
    abort();
  }

  existing = *target;

  // First or lower-depth candidate always wins
  if (!existing.source || bleed->depth < existing.depth)
  {
    target->source = plane;
    target->depth = bleed->depth;
    DeferFlats(tchunk);
    return;
  }

  if (bleed->depth > existing.depth)
    return;

  // Pick the bleedier sector
  if ((ceiling && existing.source->height > source->sector->ceilingheight) ||
      (!ceiling && existing.source->height < source->sector->floorheight))
  {
    target->source = plane;
    target->depth = bleed->depth;
    DeferFlats(tchunk);
    return;
  }

  // Among equals, arbitrarily pick the source with the higher address to avoid
  // dependence on BSP traversal order
  // FIXME: find a better heuristic
  if (plane > existing.source &&
      ((ceiling && existing.source->height ==
                       source->sector->ceilingheight) ||
       (!ceiling && existing.source->height ==
                        source->sector->floorheight)))
  {
    target->source = plane;
    target->depth = bleed->depth;
    DeferFlats(tchunk);
  }
}

static void AddWall(gl_wall_t *wall)
{
  angle_t angle1 = R_PointToPseudoAngle(wall->x1, wall->y1);
  angle_t angle2 = R_PointToPseudoAngle(wall->x2, wall->y2);
  sector_t* fs = &sectors[wall->frontsec];
  sector_t* bs = (wall->backsec == (unsigned short) -1) ? NULL : &sectors[wall->backsec];

  // Back side, i.e. backface culling	- read: endAngle >= startAngle!
  if (angle2 - angle1 < ANG180 || !wall->linedef)
  {
    return;
  }
  if (!gld_clipper_SafeCheckRange(angle2, angle1))
  {
    return;
  }

  if (!bs)
  {
    gld_clipper_SafeAddClipRange(angle2, angle1);
  }
  else
  {
    if (fs == bs)
    {
      if (texturetranslation[sides[wall->sidedef].midtexture] == NO_TEXTURE)
      {
        //e6y: nothing to do here!
        return;
      }
    }
    if (CheckClip(wall, fs, bs))
    {
      gld_clipper_SafeAddClipRange(angle2, angle1);
    }
  }

  gld_AddWall(wall);
}

//
// R_CheckBBox
// Checks BSP node/subtree bounding box.
// Returns true
//  if some part of the bbox might be visible.
//

static const int checkcoord[12][4] = // killough -- static const
{
  {3,0,2,1},
  {3,0,2,0},
  {3,1,2,0},
  {0},
  {2,0,2,1},
  {0,0,0,0},
  {3,1,3,0},
  {0},
  {2,0,3,1},
  {2,1,3,1},
  {2,1,3,0}
};

// killough 1/28/98: static // CPhipps - const parameter, reformatted
static dboolean GL_CheckBBox(const fixed_t *bspcoord)
{
  angle_t angle1, angle2;
  int        boxpos;
  const int* check;

  // Find the corners of the box
  // that define the edges from current viewpoint.
  boxpos = (viewx <= bspcoord[BOXLEFT] ? 0 : viewx < bspcoord[BOXRIGHT ] ? 1 : 2) +
    (viewy >= bspcoord[BOXTOP ] ? 0 : viewy > bspcoord[BOXBOTTOM] ? 4 : 8);

  if (boxpos == 5)
    return true;

  check = checkcoord[boxpos];

  angle1 = R_PointToPseudoAngle(bspcoord[check[0]], bspcoord[check[1]]);
  angle2 = R_PointToPseudoAngle(bspcoord[check[2]], bspcoord[check[3]]);
  return gld_clipper_SafeCheckRange(angle2, angle1);
}

static void AddPolyWalls(polyobj_t *poly)
{
  int i;
  gl_polyobj_t* gpo = &gl_rstate.polyobjs[poly - polyobjs];

  poly_add_line = true;
  poly_frontsector = poly->subsector->sector;

  for (i = 0; i < gpo->numwalls; ++i)
    AddWall(&gl_rstate.walls[gpo->firstwall + i]);
  poly_add_line = false;
  poly_frontsector = NULL;
}

// Update GL plane info (safe to call multiple times per frame)
static void UpdatePlane(gl_chunk_t* chunk, gl_plane_t* source, dboolean ceiling)
{
  sector_t dummy;
  sector_t* sector;
  int cll, fll;

  if (source->validcount == validcount)
    return;

  sector = R_FakeFlat(chunk->sector, &dummy, &fll, &cll, false);

  if (ceiling)
  {
    source->lightlevel = cll;

    if (sector->ceilingheight > viewz || sector->ceilingpic == skyflatnum ||
        (sector->heightsec != -1 &&
         sectors[sector->heightsec].floorpic == skyflatnum))
    {
      source->height = sector->ceilingheight;
      source->special = sector->special;
      //source->xoffs = sector->ceiling_xoffs;
      //source->yoffs = sector->ceiling_yoffs;
      source->xscale = sector->ceiling_xscale;
      source->yscale = sector->ceiling_yscale;
      source->rotation = sector->ceiling_rotation;
      source->lightlevel = cll;
      source->picnum =
          sector->ceilingpic == skyflatnum && sector->sky & PL_SKYFLAT
              ? sector->sky
              : sector->ceilingpic;
    } else
      source->picnum = -1;
  }
  else
  {
    source->lightlevel = fll;

    if (sector->floorheight < viewz ||
        (sector->heightsec != -1 &&
         sectors[sector->heightsec].ceilingpic == skyflatnum))
    {
      source->height = sector->floorheight;
      source->special = sector->special;
      //source->xoffs = sector->floor_xoffs;
      //source->yoffs = sector->floor_yoffs;
      source->xscale = sector->floor_xscale;
      source->yscale = sector->floor_yscale;
      source->rotation = sector->floor_rotation;
      source->lightlevel = fll;
      source->picnum =
          sector->floorpic == skyflatnum && sector->sky & PL_SKYFLAT
              ? sector->sky
              : sector->floorpic;
    } else
      source->picnum = -1;
  }

  source->validcount = validcount;
}

static void VisitSubsector(int num)
{
  subsector_t *sub;
  gl_chunk_t* chunk;
  int i;

  sub = &gl_rstate.subsectors[num];
  chunk = GL_Chunk(sub->chunk);

  // Update sector
  if (chunk->sector->validcount != validcount)
  {
    chunk->sector->validcount = validcount;

    UpdatePlane(chunk, &chunk->floor, false);
    UpdatePlane(chunk, &chunk->ceiling, true);

    R_AddSprites(chunk->sector,
                 (chunk->floor.lightlevel + chunk->ceiling.lightlevel) / 2);
  }

  // Update flats
  if (chunk->validcount != validcount)
  {
    chunk->validcount = validcount;

    UpdatePlane(chunk, &chunk->floor, false);
    UpdatePlane(chunk, &chunk->ceiling, true);

    if (chunk->flags & GL_CHUNKF_RENDER_FLATS &&
        (chunk->floor.picnum != -1 || chunk->ceiling.picnum != -1))
    {
      DeferFlats(chunk);

      // Apply bleeds
      for (i = 0; i < chunk->numbleeds; ++i)
        ApplyBleed(chunk, &gl_rstate.bleeds[chunk->firstbleed + i]);
    }
  }

  if (sub->poly)
    AddPolyWalls(sub->poly);

  for (i = 0; i < sub->numwalls; ++i)
    AddWall(&gl_rstate.walls[sub->firstwall + i]);
}

static void VisitNode(int bspnum)
{
  while (!(bspnum & NF_SUBSECTOR))
  {
    const node_t *bsp = &gl_rstate.nodes[bspnum];

    int side = R_PointOnSide(viewx, viewy, bsp);
    VisitNode(bsp->children[side]);

    if (!GL_CheckBBox(bsp->bbox[side^1]))
      return;

    bspnum = bsp->children[side^1];
  }
  VisitSubsector(bspnum == -1 ? 0 : bspnum & ~NF_SUBSECTOR);
}

static void ClearBleedTarget(bleedtarget_t* target)
{
  target->source = NULL;
  target->depth = UINT_MAX;
}

// Render flats of chunks now that all bleed effects have been updated
static void RenderDeferred(void)
{
  int i;

  for (i = 0; i < numdeferred; ++i)
  {
    gl_chunk_t* chunk = gl_rstate.deferred[i];
    int chunknum = chunk - gl_rstate.chunks;

    // Clear deferred mark
    chunk->flags &= ~GL_CHUNKF_DEFERRED;

    // Render ordinary planes if activated during BSP traversal

    if (chunk->floor.picnum != -1 && chunk->flags & GL_CHUNKF_RENDER_FLATS)
      gld_AddFlat(chunknum, false, &chunk->floor, chunk->sector);

    if (chunk->ceiling.picnum != -1 && chunk->flags & GL_CHUNKF_RENDER_FLATS)
      gld_AddFlat(chunknum, true, &chunk->ceiling, chunk->sector);
    
    // Render any over/under planes

    // Don't bleed "under" sky, as it will override the sky due to how sky
    // is rendered
    if (chunk->floorover.source && chunk->sector->floorpic != skyflatnum)
    {
      gl_plane_t* source = chunk->floorover.source;
      if (source->picnum != -1)
        gld_AddFlat(chunknum, false, source, chunk->sector);
    }

    if (chunk->floorunder.source)
    {
      gl_plane_t* source = chunk->floorunder.source;
      if (source->picnum != -1)
        gld_AddFlat(chunknum, false, source, chunk->sector);
    }

    if (chunk->ceilingover.source)
    {
      gl_plane_t* source = chunk->ceilingover.source;
      if (source->picnum != -1)
        gld_AddFlat(chunknum, true, source, chunk->sector);
    }

    if (chunk->ceilingunder.source && chunk->sector->ceilingpic != skyflatnum)
    {
      gl_plane_t* source = chunk->ceilingunder.source;
      if (source->picnum != -1)
        gld_AddFlat(chunknum, true, source, chunk->sector);
    }

    // If this was deferred by bleed-through (and not directly), render the
    // bleed source planes

    if (chunk->floorthrough.source && !(chunk->flags & GL_CHUNKF_RENDER_FLATS))
    {
      gl_plane_t* source = chunk->floorthrough.source;
      if (source->picnum != -1)
        gld_AddFlat(chunknum, false, source, chunk->sector);
    }

    if (chunk->ceilingthrough.source && !(chunk->flags & GL_CHUNKF_RENDER_FLATS))
    {
      gl_plane_t* source = chunk->ceilingthrough.source;
      if (source->picnum != -1)
        gld_AddFlat(chunknum, true, source, chunk->sector);
    }

    ClearBleedTarget(&chunk->floorunder);
    ClearBleedTarget(&chunk->floorover);
    ClearBleedTarget(&chunk->floorthrough);
    ClearBleedTarget(&chunk->ceilingunder);
    ClearBleedTarget(&chunk->ceilingover);
    ClearBleedTarget(&chunk->ceilingthrough);

    chunk->validcount = validcount;
  }

  numdeferred = 0;
}

void GL_RenderBSP(void)
{
  VisitNode(gl_rstate.numnodes - 1);
  RenderDeferred();
}
