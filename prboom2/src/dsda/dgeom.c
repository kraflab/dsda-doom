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
//	Double-precision geometry functions
//

#include <assert.h>

#include "dgeom.h"
#include "m_bbox.h"

// Signed distance of point from line, with positive being to the right
dfixed_t dgeom_PointDistanceFromLine(const dpoint_t* point, const dline_t* line)
{
  dfixed_t l = (point->y - line->start.y) * (line->end.x - line->start.x);
  dfixed_t r = (point->x - line->start.x) * (line->end.y - line->start.y);
  return (r - l) / dgeom_LineLen(line);
}

// Unsigned squared distance of point from line
dfixed_t dgeom_PointDistanceFromLineSquared(const dpoint_t* point,
                                            const dline_t* line)
{
  dfixed_t l = (point->y - line->start.y) * (line->end.x - line->start.x);
  dfixed_t r = (point->x - line->start.x) * (line->end.y - line->start.y);
  dfixed_t num = r - l;
  dfixed_t denom = dgeom_LineLenSquared(line);

  return num * num / denom;
}

dboolean dgeom_PointIsOnLine(const dpoint_t* point, const dline_t* line,
                             dfixed_t epsilonr2)
{
  return dgeom_DFixedEqual(dgeom_PointDistanceFromLineSquared(point, line), 0,
                           epsilonr2 * epsilonr2);
}

// We specifically test for a particular side instead of returning it
// because a point can be on both sides of a line if it's within epsilon of it
dboolean dgeom_PointIsOnSide(const dpoint_t* point, const dline_t* line,
                       dside_t side, dfixed_t epsilonr2)
{
  dfixed_t dist = dgeom_PointDistanceFromLine(point, line);

  return side == DGEOM_SIDE_RIGHT ? dist > -epsilonr2 : dist < epsilonr2;
}

dboolean dgeom_ParallelLinesHaveSameOrientation(const dline_t* a,
                                                const dline_t* b)
{
  return (a->start.x < a->end.x) == (b->start.x < b->end.x) &&
         (a->start.y < a->end.y) == (b->start.y < b->end.y);
}

dboolean dgeom_CounterOrientedCoincidentSegmentsOverlap(const dline_t* a,
                                                        const dline_t* b,
                                                        dfixed_t epsilon)
{
  return dgeom_DFixedEqual(dgeom_PointDist(&a->start, &b->start) +
                               dgeom_PointDist(&a->end, &b->end),
                           dgeom_LineLen(a) + dgeom_LineLen(b),
                           epsilon * M_SQRT2) &&
         // Don't consider segments joined end-to-end to overlap
         !dgeom_PointsEqual(&a->start, &b->start, epsilon) &&
         !dgeom_PointsEqual(&a->end, &b->end, epsilon);
}

dboolean dgeom_CoincidentSegmentsOverlap(const dline_t* a, const dline_t* b,
                                         dfixed_t epsilonr2)
{
  if (dgeom_ParallelLinesHaveSameOrientation(a, b))
  {
    dline_t r = {b->end, b->start};
    return dgeom_CounterOrientedCoincidentSegmentsOverlap(a, &r, epsilonr2);
  }
  return dgeom_CounterOrientedCoincidentSegmentsOverlap(a, b, epsilonr2);
}


// Calculates line intersection
dboolean dgeom_LineIntersect(const dline_t* l1, const dline_t* l2, dpoint_t* p)
{
  dfixed_t x1 = l1->start.x;
  dfixed_t y1 = l1->start.y;
  dfixed_t x2 = l1->end.x;
  dfixed_t y2 = l1->end.y;
  dfixed_t x3 = l2->start.x;
  dfixed_t y3 = l2->start.y;
  dfixed_t x4 = l2->end.x;
  dfixed_t y4 = l2->end.y;
  dfixed_t xnum, ynum;

  dfixed_t denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

  // FIXME: is this the right epsilon?  Should we only try to
  // avoid division by (near) 0?
  if (fabs(denom) < dgeom_epsilon)
    return false;

  xnum = (x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4);
  ynum = (x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4);

  p->x = xnum / denom;
  p->y = ynum / denom;

  return true;
}

dboolean dgeom_LinesCoincide(const dline_t* l1, const dline_t* l2,
                             dfixed_t epsilonr2)
{
  return dgeom_PointIsOnLine(&l2->start, l1, epsilonr2) &&
         dgeom_PointIsOnLine(&l2->end, l1, epsilonr2);
}

// Clip line segment `seg` by line `clip`, keeping `side`, and returning whether
// the entire segment has been clipped away.
dboolean dgeom_ClipSegmentByLine(dline_t* seg, const dline_t* clip,
                                 dside_t keep)
{
    dpoint_t p;
    dboolean side1 = dgeom_PointIsOnSide(&seg->start, clip, keep, dgeom_epsilonr2);
    dboolean side2 = dgeom_PointIsOnSide(&seg->end, clip, keep, dgeom_epsilonr2);

    // Both on `keep`, don't discard
    if (side1 && side2)
      return false;

    // Neither on `keep`, discard
    if (!side1 && !side2)
      return true;

    if (!dgeom_LineIntersect(seg, clip, &p))
      // Theoretically shouldn't happen, but floating point 🙃. Assume lines are
      // very nearly coincident and don't discard it.
      return false;

    if (side1)
      seg->end = p;
    else
      seg->start = p;

    // If line was reduced to (nearly) a point, toss it
    return dgeom_LineIsPoint(seg, dgeom_epsilon);
}

// Clip a line to the segment of the line falling within the bounding box,
// preserving line direction
void dgeom_ClipLineByBox(const dline_t* line, const fixed_t* bbox, dline_t* seg)
{
    dline_t border_left = {{bbox[BOXLEFT], bbox[BOXBOTTOM]},
                           {bbox[BOXLEFT], bbox[BOXTOP]}};
    dline_t border_right = {{bbox[BOXRIGHT], bbox[BOXBOTTOM]},
                            {bbox[BOXRIGHT], bbox[BOXTOP]}};
    dline_t border_bottom = {{bbox[BOXLEFT], bbox[BOXBOTTOM]},
                             {bbox[BOXRIGHT], bbox[BOXBOTTOM]}};
    dline_t border_top = {{bbox[BOXLEFT], bbox[BOXTOP]},
                          {bbox[BOXRIGHT], bbox[BOXTOP]}};
    dpoint_t p;
    dpoint_t* leftmost;
    dpoint_t* rightmost;
    dpoint_t* bottommost;
    dpoint_t* topmost;
    dline_t res;

    if (line->start.x < line->end.x)
    {
      leftmost = &res.start;
      rightmost = &res.end;
    }
    else
    {
      leftmost = &res.end;
      rightmost = &res.start;
    }

    if (line->start.y < line->end.y)
    {
      bottommost = &res.start;
      topmost = &res.end;
    }
    else
    {
      bottommost = &res.end;
      topmost = &res.start;
    }

    leftmost->x = bottommost->y = INT_MIN;
    rightmost->x = topmost->y = INT_MAX;

    if (dgeom_LineIntersect(line, &border_left, &p) && p.x > leftmost->x)
      *leftmost = p;
    if (dgeom_LineIntersect(line, &border_right, &p) && p.x < rightmost->x)
      *rightmost = p;
    if (dgeom_LineIntersect(line, &border_bottom, &p) && p.y > bottommost->y)
      *bottommost = p;
    if (dgeom_LineIntersect(line, &border_top, &p) && p.y < topmost->y)
      *topmost = p;

    *seg = res;

    // Sanity check: line angle did not change
    assert(fabs(atan2(seg->end.y - seg->start.y, seg->end.x - seg->start.x) -
                atan2(line->end.y - line->start.y,
                      line->end.x - line->start.x)) < epsilon);
}
