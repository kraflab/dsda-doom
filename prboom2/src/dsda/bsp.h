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

#ifndef __DSDA_BSP__
#define __DSDA_BSP__

#include "r_defs.h"
#include "r_state.h"
#include "dgeom.h"

// Metadata structs separate from main structs for cache efficiency in render
// loop
typedef struct
{
  seg_t* partner;
  struct subsector_s* subsector;
  segflags_t flags;
} segmeta_t;

typedef enum subflags_e
{
  SUBF_NONE = 0x0,
  // Subsector participates in a rendering hack
  SUBF_HACKED = 0x1,
  // Subsector is degenerate (can't possibly form a triangle)
  SUBF_DEGENERATE = 0x2,
  // Subsector has more than one possible sector assignment (the canonical
  // assignment being the sector on the sidedef of its first seg)
  SUBF_MULTISECTOR = 0x4
} subflags_t;

typedef struct submeta_s
{
  struct submeta_s* q_next;
  int chunk_next;
  subflags_t flags;
} submeta_t;

typedef enum {
  CHUNKF_NONE = 0x0,
  // Chunk is degenerate (contains a single degenerate subsector)
  CHUNKF_DEGENERATE = 0x1,
  // Chunk is eligible for flat "bleed-through"
  CHUNKF_BLEED_THROUGH = 0x2,
  // Chunk is interior (has no segs on single-sided linedefs)
  CHUNKF_INTERIOR = 0x4
} chunkflags_t;

typedef struct
{
  // Linked list of subsectors (by `chunk_next`)
  int subsectors;
  // First and number of perimeter entries (stored in perims array)
  int firstperim;
  int numperim;
  // First and number of points in chunk polygon path
  int firstpoint;
  int numpoints;
  chunkflags_t flags;
} chunkmeta_t;

// What's data hiding?
typedef struct
{
  // Metadata
  segmeta_t* segmeta;
  submeta_t* submeta;
  chunkmeta_t* chunkmeta;

  // Chunk perimeters
  seg_t** perims;
  unsigned int numperims;

  // Polygon paths for chunks
  dpoint_t* paths;
  unsigned int numpaths;
} dsda_gl_rstate_t;

extern dsda_gl_rstate_t dsda_gl_rstate;

// Annotate map with GL nodes, chunks, bleeds, render data
void dsda_AnnotateBSP(void);
// Clear all annotations
void dsda_ClearBSP(dboolean samelevel);

// Test if a point in the `paths` array is an end-of-contour marker
dboolean dsda_PointIsEndOfContour(const dpoint_t* p);

#endif
