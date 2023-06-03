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
  if (fabs(denom) < DGEOM_EPSILON)
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
                      line->end.x - line->start.x)) < DGEOM_EPSILON);
}
