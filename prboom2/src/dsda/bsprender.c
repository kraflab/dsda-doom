//
// Copyright(C) 2023 Brian Koropoff
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
//	BSP Analysis
//

#include <assert.h>

#include "m_bbox.h"

#include "bspinternal.h"

static unsigned int numwalls = 0;

static void WallInitFromSeg(gl_wall_t* wall, seg_t* seg)
{
  int side = seg->sidedef == &sides[seg->linedef->sidenum[1]];

  wall->x1 = seg->v1->x;
  wall->y1 = seg->v1->y;
  wall->x2 = seg->v2->x;
  wall->y2 = seg->v2->y;
  wall->linedef = seg->linedef - lines;
  wall->v1id = (side ? seg->linedef->v2 : seg->linedef->v1) - vertexes;
  wall->v2id = (side ? seg->linedef->v1 : seg->linedef->v2) - vertexes;
  wall->alpha = seg->linedef->alpha;
  wall->frontsec = seg->frontsector - sectors;
  wall->backsec =
      seg->backsector ? seg->backsector - sectors : (unsigned short)-1;
  wall->sidedef = seg->sidedef - sides;

  if (side)
    wall->flags |= GL_WALLF_BACK;
  if (seg->linedef->flags & ML_DONTPEGBOTTOM)
    wall->flags |= GL_WALLF_DONTPEGBOTTOM;
  if (seg->linedef->flags & ML_DONTPEGTOP)
    wall->flags |= GL_WALLF_DONTPEGTOP;
  if (seg->linedef->flags & ML_WRAPMIDTEX)
    wall->flags |= GL_WALLF_WRAPMIDTEX;
}

static void AddSubsectorWall(subsector_t* sub, seg_t* seg)
{
  gl_wall_t* wall;
  segmeta_t *segmeta = &dsda_gl_rstate.segmeta[seg - gl_rstate.segs];
  segmeta_t* psegmeta =
      segmeta->partner
          ? &dsda_gl_rstate.segmeta[segmeta->partner - gl_rstate.segs]
          : NULL;
  submeta_t* submeta = &dsda_gl_rstate.submeta[segmeta->subsector - gl_rstate.subsectors] ;
  submeta_t* psubmeta =
      psegmeta
          ? &dsda_gl_rstate.submeta[psegmeta->subsector - gl_rstate.subsectors]
          : NULL;

  if (!seg->linedef)
    return;

  wall = &gl_rstate.walls[numwalls++];
  WallInitFromSeg(wall, seg);

  if ((submeta && submeta->flags & SUBF_MULTISECTOR) ||
      (psubmeta && psubmeta->flags & SUBF_MULTISECTOR))
    // Inhibit stencil bleed if subsectors on either side have an ambiguous
    // sector, as we might bleed the wrong floor/ceiling depending
    // non-deterministically on how node building assigned first segments to
    // subsectors.
    wall->flags |= GL_WALLF_NOBLEED;

  sub->numwalls++;
}

static void AnnotateSubsectorRender(subsector_t* sub)
{
  int i;

  sub->firstwall = numwalls;

  for (i = 0; i < sub->numlines; ++i)
  {
    seg_t* seg = &gl_rstate.segs[sub->firstline + i];

    AddSubsectorWall(sub, seg);
  }
}

static void AddPolyobjWall(gl_polyobj_t* gpo, seg_t* seg)
{
  int i;
  gl_wall_t* wall;
  uint8_t side;

  assert(seg->linedef);

  side = seg->sidedef == &sides[seg->linedef->sidenum[1]];

  for (i = 0; i < gpo->numwalls; ++i)
  {
    wall = &gl_rstate.walls[gpo->firstwall + i];

    if (wall->linedef == seg->linedef - lines &&
        (wall->flags & GL_WALLF_BACK) == side)
      return;
  }

  wall = &gl_rstate.walls[numwalls++];
  WallInitFromSeg(wall, seg);

  gpo->numwalls++;
}

static void AnnotatePolyobjRender(polyobj_t* po, gl_polyobj_t* gpo)
{
  int i;

  gpo->firstwall = numwalls;

  for (i = 0; i < po->numsegs; ++i)
  {
    seg_t* seg = po->segs[i];

    AddPolyobjWall(gpo, seg);
  }
}

void AnnotateRender(void)
{
  int i;

  // Annotate chunks with render info
  for (i = 0; i < gl_rstate.numsubsectors; ++i)
    AnnotateSubsectorRender(&gl_rstate.subsectors[i]);

  // Annotate polyobjs with render info
  gl_rstate.polyobjs = Z_Calloc(sizeof(*gl_rstate.polyobjs), po_NumPolyobjs);
  for (i = 0; i < po_NumPolyobjs; ++i)
    AnnotatePolyobjRender(&polyobjs[i], &gl_rstate.polyobjs[i]);
}

void ClearRender(void)
{
  numwalls = 0;
  if (gl_rstate.walls)
    Z_Free(gl_rstate.walls);
  gl_rstate.walls = NULL;

  if (gl_rstate.polyobjs)
    Z_Free(gl_rstate.polyobjs);
  gl_rstate.polyobjs = NULL;
}
