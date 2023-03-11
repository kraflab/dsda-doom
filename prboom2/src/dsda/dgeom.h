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
//
//	Double-precision geometry functions
//

#ifndef __DSDA_DGEOM__
#define __DSDA_DGEOM__

#include <math.h>

#include "m_fixed.h"
#include "r_defs.h"
#include "r_state.h"

// fixed_t stored directly in a double (i.e. without dividing out FRACUNIT)
typedef double dfixed_t;

typedef struct dpoint_s
{
  dfixed_t x, y;
} dpoint_t;

// Can be a line (infinitely long) given by the points
// or the segment between start and end
typedef struct dline_s
{
  dpoint_t start, end;
} dline_t;

// Give names to sides to make code very explicit
typedef enum dside_e
{
  DGEOM_SIDE_RIGHT = 0,
  DGEOM_SIDE_LEFT = 1
} dside_t;

#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880
#endif

// Epsilon values used to "round" in various tests.  We don't rescale
// fixed_t values when converting to dfixed_t, so these are all relative
// to FRACUNIT.
// Most test functions take an explicit epsilon parameter which shadows
// `epsilon` or `epsilonr2` to make it clear what default to use.
#define EPSILON_FACTOR (1 << 14);
// Ordinary epsilon, e.g. uncertainty in x or y
static const dfixed_t dgeom_epsilon = (dfixed_t) FRACUNIT / EPSILON_FACTOR;
// Uncertainty in an x,y distance
static const dfixed_t dgeom_epsilonr2 = (dfixed_t) FRACUNIT * M_SQRT2 / EPSILON_FACTOR;
// Uncertainty in a map x or y
static const dfixed_t dgeom_epsilonmap = (dfixed_t) FRACUNIT - 1;
// Uncertainty in a map x,y distance
static const dfixed_t dgeom_epsilonmapr2 = (dfixed_t) FRACUNIT * M_SQRT2 - 1;

//
// Inline functions
// 

// Conversion functions

static inline fixed_t dgeom_DFixedToFixed(dfixed_t v)
{
  return round(v);
}

static inline dpoint_t dgeom_DPointFromVertex(const vertex_t* v)
{
  dpoint_t r = {v->px, v->py};
  return r;
}

static inline dline_t dgeom_DLineFromSeg(const seg_t* s)
{
  dline_t r = {dgeom_DPointFromVertex(s->v1), dgeom_DPointFromVertex(s->v2)};
  return r;
}

// Initialize dline from seg's linedef, oriented to match seg. This is more
// accurate for slope/intercept than the seg's coordinates because those get
// rounded to the nearest map unit during node building.
static inline dline_t dgeom_DLineFromSegLine(const seg_t* s)
{
  const line_t* l = s->linedef;

  if (s->sidedef - sides == l->sidenum[0])
  {
    dline_t r = {dgeom_DPointFromVertex(l->v1), dgeom_DPointFromVertex(l->v2)};
    return r;
  }
  else
  {
    dline_t r = {dgeom_DPointFromVertex(l->v2), dgeom_DPointFromVertex(l->v1)};
    return r;
  }
}

static inline dline_t dgeom_DLineFromISeg(const iseg_t* s)
{
  dline_t r = {{s->x1, s->y1}, {s->x2, s->y2}};
  return r;
}

static inline dline_t dgeom_DLineFromNode(const node_t* n)
{
  dline_t r = {{n->x, n->y}, {n->x + n->dx, n->y + n->dy}};
  return r;
}

// Computation

static inline dboolean dgeom_DFixedEqual(dfixed_t a, dfixed_t b, dfixed_t epsilon)
{
  return fabs(a - b) < epsilon;
}


static inline dfixed_t dgeom_PointDistSquared(const dpoint_t* a, const dpoint_t* b)
{
  dfixed_t dx = b->x - a->x;
  dfixed_t dy = b->y - a->y;
  return dx * dx + dy * dy;
}

static inline dfixed_t dgeom_PointDist(const dpoint_t* a, const dpoint_t* b)
{
  return sqrt(dgeom_PointDistSquared(a, b));
}

static inline dfixed_t dgeom_LineLenSquared(const dline_t* l)
{
  return dgeom_PointDistSquared(&l->start, &l->end);
}

static inline dfixed_t dgeom_LineLen(const dline_t* l)
{
  return sqrt(dgeom_LineLenSquared(l));
}

static inline dboolean dgeom_PointsEqual(const dpoint_t* a, const dpoint_t* b,
                                         dfixed_t epsilon)
{
  return dgeom_DFixedEqual(a->x, b->x, epsilon) && dgeom_DFixedEqual(a->y, b->y, epsilon);
}

static inline dboolean dgeom_LineIsPoint(const dline_t* line, dfixed_t epsilon)
{
  return dgeom_PointsEqual(&line->start, &line->end, epsilon);
}

//
// Non-inline functions
//

// Signed distance of point from line, with positive being to the right
dfixed_t dgeom_PointDistanceFromLine(const dpoint_t* point,
                                     const dline_t* line);

// Unsigned squared distance of point from line
dfixed_t dgeom_PointDistanceFromLineSquared(const dpoint_t* point,
                                            const dline_t* line);

dboolean dgeom_PointIsOnLine(const dpoint_t* point, const dline_t* line,
                             dfixed_t epsilonr2);

// We specifically test for a particular side instead of returning it
// because a point can be on both sides of a line if it's within epsilon of it
dboolean dgeom_PointIsOnSide(const dpoint_t* point, const dline_t* line,
                             dside_t side, dfixed_t epsilonr2);

dboolean dgeom_ParallelLinesHaveSameOrientation(const dline_t* a, const dline_t* b);

dboolean dgeom_CounterOrientedCoincidentSegmentsOverlap(const dline_t* a,
                                                        const dline_t* b,
                                                        dfixed_t epsilon);

dboolean dgeom_CoincidentSegmentsOverlap(const dline_t* a, const dline_t* b,
                                         dfixed_t epsilonr2);

// Calculates line intersection
dboolean dgeom_LineIntersect(const dline_t* l1, const dline_t* l2, dpoint_t* p);

dboolean dgeom_LinesCoincide(const dline_t* l1, const dline_t* l2,
                             dfixed_t epsilonr2);

// Clip line segment `seg` by line `clip`, keeping `side`, and returning whether
// the entire segment has been clipped away.
dboolean dgeom_ClipSegmentByLine(dline_t* seg, const dline_t* clip,
                                 dside_t keep);

// Clip a line to the segment of the line falling within the bounding box,
// preserving line direction
void dgeom_ClipLineByBox(const dline_t* line, const fixed_t* bbox,
                         dline_t* seg);

#endif
