/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
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
 *
 *---------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#ifndef CALLBACK
#define CALLBACK
#endif

#include <assert.h>
#include <math.h>

#include "gl_opengl.h"

#include "z_zone.h"
#include "doomstat.h"
#include "gl_intern.h"
#include "gl_struct.h"
#include "p_maputl.h"
#include "r_main.h"
#include "am_map.h"
#include "lprintf.h"
#include "dsda/traverse.h"

static FILE *levelinfo;

static int gld_max_vertexes = 0;
static int gld_num_vertexes = 0;

static int triangulate_subsectors = 0;

// this is the list for all sectors to the loops
GLSector *sectorloops;

// this is the list for all subsectors to the loops
// uses by textured automap
GLMapSubsector *subsectorloops;

static void gld_AddGlobalVertexes(int count)
{
  if ((gld_num_vertexes+count)>=gld_max_vertexes)
  {
    gld_max_vertexes+=count+1024;
    flats_vbo = Z_Realloc(flats_vbo, gld_max_vertexes * sizeof(flats_vbo[0]));
  }
}

static void gld_TurnOnSubsectorTriangulation(void)
{
  triangulate_subsectors = 1;
}

static void gld_TurnOffSubsectorTriangulation(void)
{
  triangulate_subsectors = 0;
}

static dboolean gld_TriangulateSubsector(subsector_t *ssec)
{
  sector_t *container = ssec->gl_pp.sector;
  return !(ssec->sector->flags & SECTOR_IS_CLOSED) ||
         (container && !(container->flags & SECTOR_IS_CLOSED)) ||
         triangulate_subsectors;
}

static int gld_SubsectorLoopIndex(subsector_t *ssec)
{
  // Give triangles to ultimate container of self-referencing sector
  if (ssec->gl_pp.sector)
    return ssec->gl_pp.sector->iSectorID;

  return ssec->sector->iSectorID;
}

static void gld_SetupSubsectorLoop(subsector_t *ssec, int iSubsectorID, int numedgepoints)
{
  GLLoopDef **loop;
  int *loopcount;

  if (triangulate_subsectors)
  {
    loop = &subsectorloops[ iSubsectorID ].loops;
    loopcount = &subsectorloops[ iSubsectorID ].loopcount;
  }
  else
  {
    int loop_index = gld_SubsectorLoopIndex(ssec);

    loop = &sectorloops[ loop_index ].loops;
    loopcount = &sectorloops[ loop_index ].loopcount;
  }

  (*loopcount)++;
  (*loop) = Z_Realloc((*loop), sizeof(GLLoopDef)*(*loopcount));
  ((*loop)[(*loopcount) - 1]).index = iSubsectorID;
  ((*loop)[(*loopcount) - 1]).mode = GL_TRIANGLE_FAN;
  ((*loop)[(*loopcount) - 1]).vertexcount = numedgepoints;
  ((*loop)[(*loopcount) - 1]).vertexindex = gld_num_vertexes;
}

/*****************************
 *
 * FLATS
 *
 *****************************/

/* proff - 05/15/2000
 * The idea and algorithm to compute the flats with nodes and subsectors is
 * originaly from JHexen. I have redone it.
 */

#define FIX2DBL(x)    ((double)(x))
#define MAX_CC_SIDES  128

static dboolean gld_PointOnSide(vertex_t *p, divline_t *d)
{
  // We'll return false if the point c is on the left side.
  return ((FIX2DBL(d->y)-FIX2DBL(p->y))*FIX2DBL(d->dx)-(FIX2DBL(d->x)-FIX2DBL(p->x))*FIX2DBL(d->dy) >= 0);
}

// Lines start-end and fdiv must intersect.
static void gld_CalcIntersectionVertex(vertex_t *s, vertex_t *e, divline_t *d, vertex_t *i)
{
  double ax = FIX2DBL(s->x), ay = FIX2DBL(s->y), bx = FIX2DBL(e->x), by = FIX2DBL(e->y);
  double cx = FIX2DBL(d->x), cy = FIX2DBL(d->y), dx = cx+FIX2DBL(d->dx), dy = cy+FIX2DBL(d->dy);
  double r = ((ay-cy)*(dx-cx)-(ax-cx)*(dy-cy)) / ((bx-ax)*(dy-cy)-(by-ay)*(dx-cx));
  i->x = (fixed_t)((double)s->x + r*((double)e->x-(double)s->x));
  i->y = (fixed_t)((double)s->y + r*((double)e->y-(double)s->y));
}

#undef FIX2DBL

// Returns a pointer to the list of points. It must be used.
//
static vertex_t *gld_FlatEdgeClipper(int *numpoints, vertex_t *points, int numclippers, divline_t *clippers)
{
  unsigned char sidelist[MAX_CC_SIDES];
  int       i, k, num = *numpoints;

  // We'll clip the polygon with each of the divlines. The left side of
  // each divline is discarded.
  for(i=0; i<numclippers; i++)
  {
    divline_t *curclip = &clippers[i];

    // First we'll determine the side of each vertex. Points are allowed
    // to be on the line.
    for(k=0; k<num; k++)
      sidelist[k] = gld_PointOnSide(&points[k], curclip);

    for(k=0; k<num; k++)
    {
      int startIdx = k, endIdx = k+1;
      // Check the end index.
      if(endIdx == num) endIdx = 0; // Wrap-around.
      // Clipping will happen when the ends are on different sides.
      if(sidelist[startIdx] != sidelist[endIdx])
      {
        vertex_t newvert;

        gld_CalcIntersectionVertex(&points[startIdx], &points[endIdx], curclip, &newvert);

        // Add the new vertex. Also modify the sidelist.
        points = (vertex_t*)Z_Realloc(points,(++num)*sizeof(vertex_t));
        if(num >= MAX_CC_SIDES)
          I_Error("gld_FlatEdgeClipper: Too many points in carver");

        // Make room for the new vertex.
        memmove(&points[endIdx+1], &points[endIdx],
          (num - endIdx-1)*sizeof(vertex_t));
        memcpy(&points[endIdx], &newvert, sizeof(newvert));

        memmove(&sidelist[endIdx+1], &sidelist[endIdx], num-endIdx-1);
        sidelist[endIdx] = 1;

        // Skip over the new vertex.
        k++;
      }
    }

    // Now we must discard the points that are on the wrong side.
    for(k=0; k<num; k++)
      if(!sidelist[k])
      {
        memmove(&points[k], &points[k+1], (num - k-1)*sizeof(vertex_t));
        memmove(&sidelist[k], &sidelist[k+1], num - k-1);
        num--;
        k--;
      }
  }
  // Screen out consecutive identical points.
  for(i=0; i<num; i++)
  {
    int previdx = i-1;
    if(previdx < 0) previdx = num - 1;
    if(points[i].x == points[previdx].x
      && points[i].y == points[previdx].y)
    {
      // This point (i) must be removed.
      memmove(&points[i], &points[i+1], sizeof(vertex_t)*(num-i-1));
      num--;
      i--;
    }
  }
  *numpoints = num;
  return points;
}

static void gld_FlatConvexCarver(int ssidx, int num, divline_t *list)
{
  subsector_t *ssec=&subsectors[ssidx];
  int numclippers = num+ssec->numlines;
  divline_t *clippers;
  int i, numedgepoints;
  vertex_t *edgepoints;

  clippers=(divline_t*)Z_Malloc(numclippers*sizeof(divline_t));
  if (!clippers)
    return;
  for(i=0; i<num; i++)
  {
    clippers[i].x = list[num-i-1].x;
    clippers[i].y = list[num-i-1].y;
    clippers[i].dx = list[num-i-1].dx;
    clippers[i].dy = list[num-i-1].dy;
  }
  for(i=num; i<numclippers; i++)
  {
    seg_t *seg = &segs[ssec->firstline+i-num];
    clippers[i].x = seg->v1->x;
    clippers[i].y = seg->v1->y;
    clippers[i].dx = seg->v2->x-seg->v1->x;
    clippers[i].dy = seg->v2->y-seg->v1->y;
  }

  // Setup the 'worldwide' polygon.
  numedgepoints = 4;
  edgepoints = (vertex_t*)Z_Malloc(numedgepoints*sizeof(vertex_t));

  edgepoints[0].x = INT_MIN;
  edgepoints[0].y = INT_MAX;

  edgepoints[1].x = INT_MAX;
  edgepoints[1].y = INT_MAX;

  edgepoints[2].x = INT_MAX;
  edgepoints[2].y = INT_MIN;

  edgepoints[3].x = INT_MIN;
  edgepoints[3].y = INT_MIN;

  // Do some clipping, <snip> <snip>
  edgepoints = gld_FlatEdgeClipper(&numedgepoints, edgepoints, numclippers, clippers);

  if(!numedgepoints)
  {
    if (levelinfo) fprintf(levelinfo, "All carved away: subsector %lli - sector %i\n", ssec-subsectors, ssec->sector->iSectorID);
  }
  else
  {
    if (numedgepoints >= 3)
    {
      gld_AddGlobalVertexes(numedgepoints);

      if (flats_vbo)
      {
        gld_SetupSubsectorLoop(ssec, ssidx, numedgepoints);

        for(i = 0;  i < numedgepoints; i++)
        {
          flats_vbo[gld_num_vertexes].u = ( (float)edgepoints[i].x/(float)FRACUNIT)/64.0f;
          flats_vbo[gld_num_vertexes].v = (-(float)edgepoints[i].y/(float)FRACUNIT)/64.0f;
          flats_vbo[gld_num_vertexes].x = -(float)edgepoints[i].x/MAP_SCALE;
          flats_vbo[gld_num_vertexes].y = 0.0f;
          flats_vbo[gld_num_vertexes].z =  (float)edgepoints[i].y/MAP_SCALE;
          gld_num_vertexes++;
        }
      }
    }
  }
  // We're done, free the edgepoints memory.
  Z_Free(edgepoints);
  Z_Free(clippers);
}

static void gld_CarveFlats(int bspnode, int numdivlines, divline_t *divlines)
{
  node_t    *nod;
  divline_t *childlist, *dl;
  int     childlistsize = numdivlines+1;

  // If this is a subsector we are dealing with, begin carving with the
  // given list.
  if (bspnode & NF_SUBSECTOR)
  {
    // We have arrived at a subsector. The divline list contains all
    // the partition lines that carve out the subsector.
    // special case for trivial maps (no nodes, single subsector)
    int ssidx = (numnodes != 0) ? bspnode & (~NF_SUBSECTOR) : 0;

    if (gld_TriangulateSubsector(&subsectors[ssidx]))
      gld_FlatConvexCarver(ssidx, numdivlines, divlines);

    return;
  }

  // Get a pointer to the node.
  nod = nodes + bspnode;

  // Allocate a new list for each child.
  childlist = (divline_t*)Z_Malloc(childlistsize*sizeof(divline_t));

  // Copy the previous lines.
  if(divlines) memcpy(childlist,divlines,numdivlines*sizeof(divline_t));

  dl = childlist + numdivlines;
  dl->x = nod->x;
  dl->y = nod->y;
  // The right child gets the original line (LEFT side clipped).
  dl->dx = nod->dx;
  dl->dy = nod->dy;
  gld_CarveFlats(nod->children[0],childlistsize,childlist);

  // The left side. We must reverse the line, otherwise the wrong
  // side would get clipped.
  dl->dx = -nod->dx;
  dl->dy = -nod->dy;
  gld_CarveFlats(nod->children[1],childlistsize,childlist);

  // We are finishing with this node, free the allocated list.
  Z_Free(childlist);
}

struct tctx
{
  struct traverse_ctx ctx;
  GLUtesselator *tess;
};

// ntessBegin
//
// called when the tesselation of a new loop starts

static void CALLBACK ntessBegin(GLenum type, void* data)
{
  struct tctx* ctx = (struct tctx*) data;
  int secnum = ctx->ctx.sector - sectors;

#ifdef PRBOOM_DEBUG
  if (levelinfo)
  {
    if (type==GL_TRIANGLES)
      fprintf(levelinfo, "\t\tBegin: GL_TRIANGLES\n");
    else
    if (type==GL_TRIANGLE_FAN)
      fprintf(levelinfo, "\t\tBegin: GL_TRIANGLE_FAN\n");
    else
    if (type==GL_TRIANGLE_STRIP)
      fprintf(levelinfo, "\t\tBegin: GL_TRIANGLE_STRIP\n");
    else
      fprintf(levelinfo, "\t\tBegin: unknown\n");
  }
#endif
  // increase loopcount for currentsector
  sectorloops[secnum].loopcount++;
  // reallocate to get space for another loop
  sectorloops[secnum].loops =
      Z_Realloc(sectorloops[secnum].loops,
                sizeof(GLLoopDef) * sectorloops[secnum].loopcount);
  // set initial values for current loop
  // currentloop is -> sectorloops[secnum].loopcount-1
  sectorloops[secnum].loops[sectorloops[secnum].loopcount - 1].index = -1;
  sectorloops[secnum].loops[sectorloops[secnum].loopcount - 1].mode = type;
  sectorloops[secnum].loops[sectorloops[secnum].loopcount - 1].vertexcount = 0;
  sectorloops[secnum].loops[sectorloops[secnum].loopcount - 1].vertexindex =
      gld_num_vertexes;
}

// ntessError
//
// called when the tesselation failes (DEBUG only)

static void CALLBACK ntessError(GLenum error, void* data)
{
#ifdef PRBOOM_DEBUG
  const GLubyte *estring;
  estring = gluErrorString(error);
  fprintf(levelinfo, "\t\tTessellation Error: %s\n", estring);
#endif
}

// ntessCombine
//
// called when the two or more vertexes are on the same coordinate

static void CALLBACK ntessCombine(GLdouble coords[3], vertex_t* vert[4],
                                  GLfloat w[4], void** dataOut, void* data)
{
#ifdef PRBOOM_DEBUG
  if (levelinfo)
  {
    fprintf(levelinfo,
            "\t\tVertexCombine Coords: x %10.5f, y %10.5f z %10.5f\n",
            coords[0], coords[1], coords[2]);
    if (vert[0])
      fprintf(levelinfo, "\t\tVertexCombine Vert1 : x %10i, y %10i p %p\n",
              vert[0]->x >> FRACBITS, vert[0]->y >> FRACBITS, vert[0]);
    if (vert[1])
      fprintf(levelinfo, "\t\tVertexCombine Vert2 : x %10i, y %10i p %p\n",
              vert[1]->x >> FRACBITS, vert[1]->y >> FRACBITS, vert[1]);
    if (vert[2])
      fprintf(levelinfo, "\t\tVertexCombine Vert3 : x %10i, y %10i p %p\n",
              vert[2]->x >> FRACBITS, vert[2]->y >> FRACBITS, vert[2]);
    if (vert[3])
      fprintf(levelinfo, "\t\tVertexCombine Vert4 : x %10i, y %10i p %p\n",
              vert[3]->x >> FRACBITS, vert[3]->y >> FRACBITS, vert[3]);
  }
#endif
  // just return the first vertex, because all vertexes are on the same coordinate
  *dataOut = vert[0];
}

// ntessVertex
//
// called when a vertex is found

static void CALLBACK ntessVertex( vertex_t *vert, void* data)
{
  struct tctx* ctx = (struct tctx*) data;
  int secnum = ctx->ctx.sector - sectors;
#ifdef PRBOOM_DEBUG
  if (levelinfo)
    fprintf(levelinfo, "\t\tVertex : x %10i, y %10i\n", vert->x >> FRACBITS,
            vert->y >> FRACBITS);
#endif
  // increase vertex count
  sectorloops[secnum].loops[sectorloops[secnum].loopcount - 1].vertexcount++;

  // increase vertex count
  gld_AddGlobalVertexes(1);
  // add the new vertex (vert is the second argument of gluTessVertex)
  flats_vbo[gld_num_vertexes].u=( (float)vert->x/(float)FRACUNIT)/64.0f;
  flats_vbo[gld_num_vertexes].v=(-(float)vert->y/(float)FRACUNIT)/64.0f;
  flats_vbo[gld_num_vertexes].x=-(float)vert->x/MAP_SCALE;
  flats_vbo[gld_num_vertexes].y=0.0f;
  flats_vbo[gld_num_vertexes].z= (float)vert->y/MAP_SCALE;
  gld_num_vertexes++;
}

// ntessEnd
//
// called when the tesselation of a the current loop ends (DEBUG only)

static void CALLBACK ntessEnd(void* data)
{
#ifdef PRBOOM_DEBUG
  if (levelinfo)
    fprintf(levelinfo, "\t\tEnd loopcount %i vertexcount %i\n",
            sectorloops[currentsector].loopcount,
            sectorloops[currentsector]
                .loops[sectorloops[currentsector].loopcount - 1]
                .vertexcount);
#endif
}

static void gld_PathCallback(sector_t* sector, struct tpath* path, void* data)
{
  // Indicate that these lines have neighbors (are not isolated)
  if (path->prev->line)
  {
    path->line->r_flags |= RF_LINKED;
    path->prev->line->r_flags |= RF_LINKED;
  }
}

static void gld_CycleCallback(sector_t* sector, struct tpath* cycle, void* data)
{
  struct tctx* ctx = (struct tctx*) data;
  struct tpath* cur;
  dboolean ambig = true;

  for (cur = cycle; cur; cur = cur->next)
  {
    if (cur->line->frontsector != cur->line->backsector)
    {
      ambig = false;
      break;
    }
  }

  for (cur = cycle; cur; cur = cur->next)
  {
    // Mark which direction we traversed the line.
    if (cur->line->frontsector == cur->line->backsector)
    {
      // Bi-directional line, detect direction
      if (ambig)
        // Without at least one single-directional line in the cycle,
        // the cycle direction is ambiguous
        cur->line->cycle_amb = true;
      else if (cur->v == cur->line->v2)
        cur->line->cycle_fw = true;
      else
        cur->line->cycle_bw = true;
    }
    else
      // Single-direction lines are always traversed forwards
      cur->line->cycle_fw = true;
  }
}

static void gld_TessCallback(sector_t* sector, struct tpath* cycle, void* data)
{
  struct tctx* ctx = (struct tctx*) data;
  struct tpath* cur;
  double v[3];

  for (cur = cycle; cur; cur = cur->next)
  {
    // Don't tesselate already-tesselated cycles (we can visit
    // cycles multiple times currently) or cycles with bi-directional
    // lines in them
    if (cur->line->frontsector == cur->line->backsector)
      return;
  }

  gluTessBeginContour(ctx->tess);
  for (cur = cycle; cur; cur = cur->next)
  {
    v[0] = -(double)cur->v->x / (double)MAP_SCALE;
    v[1] = 0.0;
    v[2] = (double)cur->v->y / (double)MAP_SCALE;
    gluTessVertex(ctx->tess, v, cur->v);
  }
  gluTessEndContour(ctx->tess);
}

// Detect sectors on each side of line according to BSP tree.
// This projects a fraction of a unit orthogonally from the line
// midpoint and does a point-in-sector test.  It is often inaccurate,
// so don't depend on it alone for anything critical.
static void gld_BSPLineSides(line_t* line, subsector_t** s1, subsector_t** s2)
{
  angle_t ang = R_PointToAngle2(line->v1->x, line->v1->y, line->v2->x, line->v2->y) + ANG90;
  fixed_t offsx = finecosine[ang >> ANGLETOFINESHIFT] >> 5;
  fixed_t offsy = finesine[ang >> ANGLETOFINESHIFT] >> 5;
  fixed_t xmid = (line->v1->x + line->v2->x) >> 1;
  fixed_t ymid = (line->v1->y + line->v2->y) >> 1;
  fixed_t xp1 = xmid + offsx;
  fixed_t yp1 = ymid + offsy;
  fixed_t xp2 = xmid - offsx;
  fixed_t yp2 = ymid - offsy;

  *s1 = R_PointInSubsector(xp1, yp1);
  *s2 = R_PointInSubsector(xp2, yp2);
}

// Disambiguate interior lines and self-referencing lines
// using BSP tree.  This is not completely accurate, so
// it's only used when graph cycle tests are inconclusive.
static dboolean gld_BSPLineIsInterior(sector_t* sector, line_t* line)
{
  subsector_t* s1;
  subsector_t* s2;

  gld_BSPLineSides(line, &s1, &s2);
  return s1->sector == sector && s2->sector == sector;
}

/*
// Traverse sector, detecting and marking elementary cycles and tessellating
// them into triangles.  Also mark lines with at least one neighbor (RF_LINKED).
//
// The sector is treated as the directed graph of its sidedefs.  This means
// most lines can only be traversed one direction.  Lines tagged with the sector
// on both sides can be traversed backwards, which is marked separately.
// Trivial cycles from doubling back on such a line are not counted.
// Cycles can't repeat vertices.
//
// For example, consider the following sector.  Vertices are marked by +; lines
// by |, \, and -; direction by <, >, v, ^, and * (bi-directional).
//
//             B
//   +-<-+-<-+-*-+
//   |   |   |   |\
//   v  A*   ^   v ^
//   |   |   |   |  \
//   +->-+->-+   +->-+
//
// Line B will be marked as not part of any cycle, since it is not part of
// a closed loop.
//
// Line A is part of two cycles with opposite directions, so it will be marked
// as part of both a forward and backward cycle.  This indicates that it is
// an interior line (and therefore not a self-referencing trick).
//
// Cycles involving only bi-directional lines require some care.  They could
// be interior lines, but they could also be the perimeter of a self-referencing
// sector.  These cycles are marked as ambiguous and distinguised geometrically
// with a BSP query later on.  Unfortunately, this geometric check is also not
// reliable, which is why we need to do cycle analysis in the first place.
// Both checks together are more accurate than either alone.
//
// The algorithm proceeds by depth-first search.  Cycles are detected by
// recording the current path and checking for a duplicate vertex.
//
// FIXME: Implement Johnson cycle search for better time complexity.
// This doesn't seem to be necessary in practice so far based on testing
// maps with large/complex sectors.
*/
static void gld_TraverseSector(sector_t* sector)
{
  struct tctx ctx;
  int i;

  // Initialize traversal
  for (i = 0; i < sector->linecount; ++i)
  {
    sector->lines[i]->cycle_fw = false;
    sector->lines[i]->cycle_bw = false;
    sector->lines[i]->cycle_amb = false;
  }

  // Pass 1 -- detect cycles and classify lines/sector
  ctx.ctx.sector = sector;
  ctx.ctx.ccb = gld_CycleCallback;
  ctx.ctx.pcb = gld_PathCallback;
  ctx.ctx.data = &ctx;

  dsda_TraverseSectorGraph(&ctx.ctx);

  // Mark all real (not perfidious mapper trick) lines.
  //
  // The sector is real if all its lines that participate in cycles are real
  // This means sectors that are so broken they don't have cycles are
  // vacuously real, which is a conservative assumption since many maps
  // have extremely broken sectors.
  //
  // We also check for closedness here.
  sector->flags |= SECTOR_IS_REAL;
  sector->flags |= SECTOR_IS_CLOSED;
  for (i = 0; i < sector->linecount; ++i)
  {
    line_t* l = sector->lines[i];
    // Skip lines that were not in a cycle
    if (!l->cycle_fw && !l->cycle_bw && !l->cycle_amb)
    {
      // If there is a line that could otherwise be tessellated
      // that was not part of a cycle, we can't depend on
      // tessellation and need to fall back on subsector
      // triangulation
      if (l->frontsector != l->backsector)
        sector->flags &= ~SECTOR_IS_CLOSED;
      continue;
    }

    // A line is real if it isn't self-referencing or it's interior
    // to the sector.
    // A line is interior if it was traversed in both directions
    // by different cycles, or its cycle direction was ambiguous
    // and the BSP tree indicates the same sector lies
    // on both sides of it.
    if (l->frontsector != l->backsector ||
        (l->cycle_fw && l->cycle_bw) ||
        (l->cycle_amb && gld_BSPLineIsInterior(sector, l)))
      l->r_flags |= RF_REAL;
    else
      sector->flags &= ~SECTOR_IS_REAL;
  }

  // Pass 2 - tesselate sector

  // Don't try to tesselate unclosed sectors
  if (!(sector->flags & SECTOR_IS_CLOSED))
    return;

  ctx.tess = gluNewTess();
  if (!ctx.tess)
    I_Error("gld_PreprocessSectors: Not enough memory for GLU tessellator");
  gluTessCallback(ctx.tess, GLU_TESS_BEGIN_DATA, (void (*)())ntessBegin);
  gluTessCallback(ctx.tess, GLU_TESS_VERTEX_DATA, (void (*)())ntessVertex);
  gluTessCallback(ctx.tess, GLU_TESS_ERROR_DATA, (void (*)())ntessError);
  gluTessCallback(ctx.tess, GLU_TESS_COMBINE_DATA, (void (*)())ntessCombine);
  gluTessCallback(ctx.tess, GLU_TESS_END_DATA, (void (*)())ntessEnd);

  ctx.ctx.ccb = gld_TessCallback;
  ctx.ctx.pcb = NULL;

  gluTessBeginPolygon(ctx.tess, &ctx);
  dsda_TraverseSectorGraph(&ctx.ctx);
  gluTessEndPolygon(ctx.tess);

  gluDeleteTess(ctx.tess);
}

// Classify all sectors and their lines as dangling/interior/real
static void gld_TraverseSectors(void)
{
  int i;

  for (i = 0; i < numsectors; i++)
    gld_TraverseSector(&sectors[i]);
}

// Find subsector at end of container chain, but return `NULL` upon encountering
// `subsector` to avoid creating a cycle.  If `strict`, also return `NULL` upon
// encountering the same sector.
static subsector_t* gld_ChaseContainer(subsector_t* subsector,
                                       subsector_t* container, dboolean strict)
{
  if (container == NULL || container == subsector ||
      (strict && container->sector == subsector->sector))
    return NULL;
  // Chase down target recursively
  while (container->gl_pp.subsector & CONTAINER_SUBSECTOR)
  {
    container = (subsector_t*) (container->gl_pp.subsector & ~CONTAINER_SUBSECTOR);
    // Avoid cycle creation
    if (container == NULL || container == subsector ||
        (strict && container->sector == subsector->sector))
      return NULL;
  }
  return container;
}

static void gld_ResolveContainer(subsector_t* subsector)
{
  // Try to find sector link at the end of the chain if
  // we link to another subsector
  if (subsector->gl_pp.subsector & CONTAINER_SUBSECTOR)
  {
    subsector_t* chase = gld_ChaseContainer(NULL, subsector, false);
    if (chase)
      subsector->gl_pp = chase->gl_pp;
    else
      subsector->gl_pp.sector = NULL;
  }
  // Container better be fully resolved now (either NULL, or a sector)
  assert(subsector->gl_pp.sector == NULL ||
         !(subsector->gl_pp.subsector & CONTAINER_SUBSECTOR));
}

// Try to find a real, non-closed container directly across
// a fake line.  It doesn't make sense to give triangles to
// fake sectors (their flats should be invisible), and we
// don't bother with closed containers because GLU tessellation
// should have succeeded and obviated the need to piece together
// triangles from subsectors.
static sector_t* gld_FindContainer(subsector_t* subsector)
{
  int i;

  for (i = 0; i < subsector->numlines; ++i)
  {
    seg_t* s = &segs[subsector->firstline + i];
    line_t* l = s->linedef;
    subsector_t* s1;
    subsector_t* s2;

    // Only consider fake lines
    if (l->r_flags & RF_REAL)
      continue;

    // Look for container by doing a point lookup to the sides of the line in
    // the BSP tree.  Note that this isn't always accurate, presumably due to
    // vagaries in the node building process, fixed point precision, etc.  This
    // can be seen with sector 34 in AV MAP01, where we fail to find the tiny
    // sectors above and below its self-referencing lines.
    gld_BSPLineSides(l, &s1, &s2);

    if (s1->sector != subsector->sector && s1->sector->flags & SECTOR_IS_REAL &&
        !(s1->sector->flags & SECTOR_IS_CLOSED))
      return s1->sector;
    if (s2->sector != subsector->sector && s2->sector->flags & SECTOR_IS_REAL &&
        !(s2->sector->flags & SECTOR_IS_CLOSED))
      return s2->sector;
  }

  return NULL;
}

// Try to find a neighboring subsector so we can inherit its container.
// If `strict`, only find subsectors in a different sector.
static subsector_t* gld_InheritContainer(subsector_t* subsector, dboolean strict)
{
  int i;

  for (i = 0; i < subsector->numlines; ++i)
  {
    seg_t* s = &segs[subsector->firstline + i];
    line_t* l = s->linedef;
    subsector_t* s1;
    subsector_t* s2;

    gld_BSPLineSides(l, &s1, &s2);

    if ((s1 = gld_ChaseContainer(subsector, s1, strict)))
      return s1;
    if ((s2 = gld_ChaseContainer(subsector, s2, strict)))
      return s2;
  }

  return NULL;
}

// Find container of a subsector of a self-referencing sector, if applicable.
// We try to find a neighboring real sector across a seg, then fall back
// on inheriting from a neighboring fake subsector (first in a different
// sector, and finally in the same sector).  This ensures that the
// inheritance tree we build always terminates in a valid containing
// sector when possible.  The inheritance tree gets collapsed later
// in `gld_ResolveContainer`.
static union container_u gld_SubsectorContainer(subsector_t* subsector)
{
  union container_u result;
  subsector_t* inherit;

  // Invariant: subsector hasn't had container assigned yet
  assert(subsector->gl_pp.sector == NULL);

  result.sector = NULL;

  // Real sectors are the opposite of self-referencing
  if (subsector->sector->flags & SECTOR_IS_REAL)
    return result;

  // Prefer to find a real container across a self-referencing linedef
  if ((result.sector = gld_FindContainer(subsector)))
    return result;

  // Failing that, inherit container from a fake neighbor from a different sector
  if ((inherit = gld_InheritContainer(subsector, true)))
  {
      result.subsector = (uintptr_t) inherit | CONTAINER_SUBSECTOR;
      return result;
  }

  // Failing that, inherit container from a fake neighbor, even from
  // the same sector
  if ((inherit = gld_InheritContainer(subsector, false)))
  {
      result.subsector = (uintptr_t) inherit | CONTAINER_SUBSECTOR;
      return result;
  }

  // No luck
  return result;
}

static void gld_ResolveContainers(void)
{
  // Perform final resolution of containers for subsectors that have them
  int i;

  for (i = 0; i < numsubsectors; i++)
    gld_ResolveContainer(&subsectors[i]);
}

/********************************************
 * Name     : gld_GetSubSectorVertices      *
 * created  : 08/13/00                      *
 * modified : 09/18/00, adapted for PrBoom  *
 * author   : figgi                         *
 * what     : prepares subsectorvertices    *
 *            (glnodes only)                *
 ********************************************/

static void gld_GetSubSectorVertices(void)
{
  int      i, j;
  int      numedgepoints;

  for(i = 0; i < numsubsectors; i++)
  {
    subsector_t* ssector = &subsectors[i];

    if (gld_TriangulateSubsector(ssector))
      continue;

    numedgepoints  = ssector->numlines;

    gld_AddGlobalVertexes(numedgepoints);

    if (flats_vbo)
    {
      gld_SetupSubsectorLoop(ssector, i, numedgepoints);

      for (j = 0; j < numedgepoints; j++)
      {
        flats_vbo[gld_num_vertexes].u =( (float)(segs[ssector->firstline + j].v1->x)/FRACUNIT)/64.0f;
        flats_vbo[gld_num_vertexes].v =(-(float)(segs[ssector->firstline + j].v1->y)/FRACUNIT)/64.0f;
        flats_vbo[gld_num_vertexes].x = -(float)(segs[ssector->firstline + j].v1->x)/MAP_SCALE;
        flats_vbo[gld_num_vertexes].y = 0.0f;
        flats_vbo[gld_num_vertexes].z =  (float)(segs[ssector->firstline + j].v1->y)/MAP_SCALE;
        gld_num_vertexes++;
      }
    }
  }
}

// gld_PreprocessLevel
//
// this checks all sectors if they are closed and calls gld_PrecalculateSector to
// calculate the loops for every sector
// the idea to check for closed sectors is from DEU. check next commentary
/*
      Note from RQ:
      This is a very simple idea, but it works!  The first test (above)
      checks that all Sectors are closed.  But if a closed set of LineDefs
      is moved out of a Sector and has all its "external" SideDefs pointing
      to that Sector instead of the new one, then we need a second test.
      That's why I check if the SideDefs facing each other are bound to
      the same Sector.

      Other note from RQ:
      Nowadays, what makes the power of a good editor is its automatic tests.
      So, if you are writing another Doom editor, you will probably want
      to do the same kind of tests in your program.  Fine, but if you use
      these ideas, don't forget to credit DEU...  Just a reminder... :-)
*/
// so I credited DEU

//
// e6y
// All sectors which are inside 64x64 square of map grid should be marked here.
// GL_CLAMP instead of GL_REPEAT should be used for them for avoiding seams
//

static void gld_MarkSectorsForClamp(void)
{
  int i;

  for (i = 0; i < numsectors; i++)
  {
    int loopnum; // current loop number
    GLLoopDef *currentloop; // the current loop
    GLfloat minu, maxu, minv, maxv;
    dboolean fail;

    minu = minv = 65535;
    maxu = maxv = -65535;
    fail = false;

    for (loopnum = 0; !fail && loopnum < sectorloops[i].loopcount; loopnum++)
    {
      int vertexnum;
      // set the current loop
      currentloop = &sectorloops[i].loops[loopnum];
      if (!currentloop)
        continue;
      for (vertexnum = currentloop->vertexindex; !fail && vertexnum<(currentloop->vertexindex+currentloop->vertexcount); vertexnum++)
      {
        vbo_xyz_uv_t *vbo;
        vbo = &flats_vbo[vertexnum];

        if (vbo->u < minu) minu = (float)floor(vbo->u);
        if (vbo->v < minv) minv = (float)floor(vbo->v);
        if (vbo->u > maxu) maxu = vbo->u;
        if (vbo->v > maxv) maxv = vbo->v;

        fail = (maxu - minu > 1.0f || maxv - minv > 1.0f);
      }
    }

    if (!fail)
    {
      sectorloops[i].flags = SECTOR_CLAMPXY;

      for (loopnum=0; loopnum<sectorloops[i].loopcount; loopnum++)
      {
        int vertexnum;
        // set the current loop
        currentloop = &sectorloops[i].loops[loopnum];
        if (!currentloop)
          continue;
        for (vertexnum = currentloop->vertexindex; vertexnum < (currentloop->vertexindex+currentloop->vertexcount); vertexnum++)
        {
          flats_vbo[vertexnum].u = flats_vbo[vertexnum].u - minu;
          flats_vbo[vertexnum].v = flats_vbo[vertexnum].v - minv;
        }
      }
    }
  }
}

static void gld_PreprocessSectors(void)
{
  int v1num;
  int v2num;
  int i;
  int j;

  if (numsectors)
  {
    sectorloops=Z_Malloc(sizeof(GLSector)*numsectors);
    if (!sectorloops)
      I_Error("gld_PreprocessSectors: Not enough memory for array sectorloops");
    memset(sectorloops, 0, sizeof(GLSector)*numsectors);
  }

  if (numsubsectors)
  {
    subsectorloops=Z_Malloc(sizeof(GLMapSubsector)*numsubsectors);
    if (!subsectorloops)
      I_Error("gld_PreprocessSectors: Not enough memory for array subsectorloops");
    memset(subsectorloops, 0, sizeof(GLMapSubsector)*numsubsectors);
  }

  if (numsegs)
  {
    segrendered=Z_Calloc(numsegs, sizeof(byte));
    if (!segrendered)
      I_Error("gld_PreprocessSectors: Not enough memory for array segrendered");
  }

  if (numlines)
  {
    linerendered[0]=Z_Calloc(numlines, sizeof(byte));
    linerendered[1]=Z_Calloc(numlines, sizeof(byte));
    if (!linerendered[0] || !linerendered[1])
      I_Error("gld_PreprocessSectors: Not enough memory for array linerendered");
  }

  flats_vbo = NULL;
  gld_max_vertexes=0;
  gld_num_vertexes=0;

  if (numvertexes)
  {
    gld_AddGlobalVertexes(numvertexes*2);
  }

  gld_TraverseSectors();

  for (i = 0; i < numsubsectors; ++i)
    subsectors[i].gl_pp = gld_SubsectorContainer(&subsectors[i]);

  gld_ResolveContainers();

  // figgi -- adapted for glnodes
  if (numnodes)
  {
    if (!use_gl_nodes)
      gld_CarveFlats(numnodes-1, 0, 0);
    else
      gld_GetSubSectorVertices();
  }

  gld_ProcessTexturedMap();

  if (levelinfo) fclose(levelinfo);

  //e6y: for seamless rendering
  gld_MarkSectorsForClamp();
}

static void gld_PreprocessSegs(void)
{
  int i;

  gl_segs=Z_Malloc(numsegs*sizeof(GLSeg));
  for (i=0; i<numsegs; i++)
  {
    gl_segs[i].x1=-(float)segs[i].v1->x/(float)MAP_SCALE;
    gl_segs[i].z1= (float)segs[i].v1->y/(float)MAP_SCALE;
    gl_segs[i].x2=-(float)segs[i].v2->x/(float)MAP_SCALE;
    gl_segs[i].z2= (float)segs[i].v2->y/(float)MAP_SCALE;
  }

  gl_lines=Z_Malloc(numlines*sizeof(GLSeg));
  for (i=0; i<numlines; i++)
  {
    gl_lines[i].x1=-(float)lines[i].v1->x/(float)MAP_SCALE;
    gl_lines[i].z1= (float)lines[i].v1->y/(float)MAP_SCALE;
    gl_lines[i].x2=-(float)lines[i].v2->x/(float)MAP_SCALE;
    gl_lines[i].z2= (float)lines[i].v2->y/(float)MAP_SCALE;
  }
}

void gld_PreprocessLevel(void)
{
  // e6y: speedup of level reloading
  // Do not preprocess GL data twice for same level
  if (!gl_preprocessed)
  {
    int i;
    static int numsectors_prev = 0;
    static int numsubsectors_prev = 0;

    Z_Free(gl_segs);
    Z_Free(gl_lines);

    Z_Free(flats_vbo);
    flats_vbo = NULL;

    Z_Free(segrendered);
    Z_Free(linerendered[0]);
    Z_Free(linerendered[1]);

    for (i = 0; i < numsectors_prev; i++)
    {
      Z_Free(sectorloops[i].loops);
    }
    Z_Free(sectorloops);
    for (i = 0; i < numsubsectors_prev; i++)
    {
      Z_Free(subsectorloops[i].loops);
    }
    Z_Free(subsectorloops);

    gld_Precache();
    gld_PreprocessSectors();
    gld_PreprocessFakeSectors();
    gld_PreprocessSegs();

    numsectors_prev = numsectors;
    numsubsectors_prev = numsubsectors;
  }
  else
  {
    gld_PreprocessFakeSectors();

    memset(segrendered, 0, numsegs*sizeof(segrendered[0]));
    memset(linerendered[0], 0, numlines*sizeof(linerendered[0][0]));
    memset(linerendered[1], 0, numlines*sizeof(linerendered[1][0]));
  }

  gld_ResetTexturedAutomap();

  gld_FreeDrawInfo();

  if (!gl_preprocessed)
  {
    if (gl_ext_arb_vertex_buffer_object)
    {
      if (flats_vbo_id)
      {
        // delete VBO when already exists
        GLEXT_glDeleteBuffersARB(1, &flats_vbo_id);
      }
      // generate a new VBO and get the associated ID
      GLEXT_glGenBuffersARB(1, &flats_vbo_id);
      // bind VBO in order to use
      GLEXT_glBindBufferARB(GL_ARRAY_BUFFER, flats_vbo_id);
      // upload data to VBO
      GLEXT_glBufferDataARB(GL_ARRAY_BUFFER,
        gld_num_vertexes * sizeof(flats_vbo[0]),
        flats_vbo, GL_STATIC_DRAW_ARB);

      Z_Free(flats_vbo);
      flats_vbo = NULL;

      // bind VBO in order to use
      GLEXT_glBindBufferARB(GL_ARRAY_BUFFER, flats_vbo_id);
    }

    glVertexPointer(3, GL_FLOAT, sizeof(flats_vbo[0]), flats_vbo_x);
    glTexCoordPointer(2, GL_FLOAT, sizeof(flats_vbo[0]), flats_vbo_u);
  }

  //e6y
  gld_PreprocessDetail();
  gld_InitVertexData();

  gl_preprocessed = true;
}

/*****************************
 *
 * AUTOMAP
 *
 *****************************/

void gld_ProcessTexturedMap(void)
{
  extern int map_textured;

  if (map_textured && subsectorloops && subsectorloops[0].loops == NULL)
  {
    gld_TurnOnSubsectorTriangulation();

    if (!use_gl_nodes)
      gld_CarveFlats(numnodes-1, 0, 0);
    else
      gld_GetSubSectorVertices();

    gld_TurnOffSubsectorTriangulation();
  }
}
