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

// A 2D point
typedef struct dpoint_s
{
  dfixed_t x, y;
} dpoint_t;

// Can be a 2D line (infinitely long) given by the points or the segment between
// start and end
typedef struct dline_s
{
  dpoint_t start, end;
} dline_t;

// Side of a line when situated at `start` looking toward `end`
typedef enum dside_e
{
  DGEOM_SIDE_RIGHT = 0,
  DGEOM_SIDE_LEFT = 1
} dside_t;

#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880
#endif

// Epsilon values used to "round" in various tests.  We don't rescale fixed_t
// values when converting to dfixed_t, so these all contained a factor of
// FRACUNIT. Most test functions take an explicit epsilon parameter named
// `epsilon` or `epsilonr2` to make it clear what default to use.
#define DGEOM_EPSILON_FACTOR (1 << 13)
// Ordinary epsilon, e.g. uncertainty in x or y
#define DGEOM_EPSILON ((dfixed_t)FRACUNIT / DGEOM_EPSILON_FACTOR)
// Uncertainty in an x,y distance
#define DGEOM_EPSILONR2 ((dfixed_t)FRACUNIT * M_SQRT2 / DGEOM_EPSILON_FACTOR)

//
// Inline functions
// 

// Conversion functions

static inline fixed_t dgeom_FixedFromDFixed(dfixed_t v)
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

static inline dline_t dgeom_DLineFromNode(const node_t* n)
{
  dline_t r = {{n->x, n->y}, {n->x + n->dx, n->y + n->dy}};
  return r;
}

// Computation

// Equality test within `epsilon`
static inline dboolean dgeom_DFixedEqual(dfixed_t a, dfixed_t b,
                                         dfixed_t epsilon)
{
  return fabs(a - b) < epsilon;
}

// Squared distance between two points
static inline dfixed_t dgeom_PointDistSquared(const dpoint_t* a,
                                              const dpoint_t* b)
{
  dfixed_t dx = b->x - a->x;
  dfixed_t dy = b->y - a->y;
  return dx * dx + dy * dy;
}

// Distance between two points
static inline dfixed_t dgeom_PointDist(const dpoint_t* a, const dpoint_t* b)
{
  return sqrt(dgeom_PointDistSquared(a, b));
}

// Squared length of line
static inline dfixed_t dgeom_LineLenSquared(const dline_t* l)
{
  return dgeom_PointDistSquared(&l->start, &l->end);
}

// Length of line
static inline dfixed_t dgeom_LineLen(const dline_t* l)
{
  return sqrt(dgeom_LineLenSquared(l));
}

// Equality of points within `epsilon` in both dimensions
static inline dboolean dgeom_PointsEqual(const dpoint_t* a, const dpoint_t* b,
                                         dfixed_t epsilon)
{
  return dgeom_DFixedEqual(a->x, b->x, epsilon) &&
         dgeom_DFixedEqual(a->y, b->y, epsilon);
}

// Returns whether a line is a single point (zero length) within epsilon
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

// Returns whether a point is within `epsilonr2` of being on line
dboolean dgeom_PointIsOnLine(const dpoint_t* point, const dline_t* line,
                             dfixed_t epsilonr2);


// Returns whether a point is within `epsilonr2` of being on the given side of a line
dboolean dgeom_PointIsOnSide(const dpoint_t* point, const dline_t* line,
                             dside_t side, dfixed_t epsilonr2);

// Calculates the intersection of two lines, returning `false` if they do not
// intersect at a unique point
dboolean dgeom_LineIntersect(const dline_t* l1, const dline_t* l2, dpoint_t* p);

// Returns whether the two lines coincide (have the same orientation and
// position, i.e. are the same line)
dboolean dgeom_LinesCoincide(const dline_t* l1, const dline_t* l2,
                             dfixed_t epsilonr2);

// Clip a line to the segment of the line falling within the bounding box,
// preserving line direction
void dgeom_ClipLineByBox(const dline_t* line, const fixed_t* bbox,
                         dline_t* seg);

#endif
