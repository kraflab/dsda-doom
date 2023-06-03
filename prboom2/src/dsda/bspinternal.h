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

#ifndef __DSDA_BSPINTERNAL__
#define __DSDA_BSPINTERNAL__

// Internal bsp*.c header, not for inclusion elsewhere

#include "bsp.h"

// Iterator over perimeter of chunk
typedef struct piter_s
{
  // Chunk being iterated
  chunkmeta_t* chunkmeta;
  // Front subsector
  subsector_t* frontsub;
  // Front segment
  seg_t* frontseg;
  // Front segment
  segmeta_t* frontsegmeta;
  // Front side (NULL if both front and back segs are minisegs)
  side_t* frontside;
  // Back equivalents to the above
  subsector_t* backsub;
  seg_t* backseg;
  segmeta_t* backsegmeta;
  side_t* backside;
  // Back chunk, sector for convenience
  chunkmeta_t* backchunkmeta;
  sector_t* backsector;
  // Internal
  int i;
} piter_t;

// Chunk perimeter iteration.  Use like:
//
// for (IterPerimeter(&iter, sub);
//      IterPerimeterValid(&iter);
//      IterPerimeterNext(&iter)) ...

void IterPerimeterNext(piter_t* iter);

static inline dboolean IterPerimeterValid(const piter_t* iter)
{
  return iter->i != iter->chunkmeta->numperim;
}

static inline void IterPerimeter(piter_t* iter, chunkmeta_t* chunkmeta)
{
  iter->chunkmeta = chunkmeta;
  iter->i = -1;
  IterPerimeterNext(iter);
}

void LoadSideGLNodes(void);
void ClearGLNodes(void);
void ResetGLNodes(void);

void AnnotateBleeds(void);
void ClearBleeds(void);

void AnnotateChunks(void);
void ClearChunks(void);
void ResetChunks(void);

void AnnotateRender(void);
void ClearRender(void);

#endif
