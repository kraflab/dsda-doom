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
 *      Refresh/render internal state variables (global).
 *
 *-----------------------------------------------------------------------------*/


#ifndef __R_STATE__
#define __R_STATE__

// Need data structure definitions.
#include "d_player.h"
#include "r_data.h"

//
// Refresh internal data structures,
//  for rendering.
//

// needed for texture pegging
extern fixed_t *textureheight;

extern int firstflat, numflats;

// for global animation
extern int *flattranslation;
extern int *texturetranslation;

// Sprite....
extern int firstspritelump;
extern int lastspritelump;
extern int numspritelumps;

//
// Lookup tables for map data.
//
extern spritedef_t      *sprites;

extern int              numvertexes;
extern vertex_t         *vertexes;

extern int              numsegs;
extern seg_t            *segs;

extern int              numsectors;
extern sector_t         *sectors;

extern int              numsubsectors;
extern subsector_t      *subsectors;

extern int              numnodes;
extern node_t           *nodes;

extern int              numlines;
extern line_t           *lines;

extern int              numsides;
extern side_t           *sides;

extern int              *sslines_indexes;
extern ssline_t         *sslines;

extern byte             *map_subsectors;

//
// POV data.
//
extern fixed_t          viewx;
extern fixed_t          viewy;
extern fixed_t          viewz;
extern angle_t          viewangle;
extern player_t         *viewplayer;
extern angle_t          clipangle;
extern int              viewangletox[FINEANGLES/2];

// e6y: resolution limitation is removed
extern angle_t          *xtoviewangle;  // killough 2/8/98

extern int              FieldOfView;

extern fixed_t          rw_distance;
extern angle_t          rw_normalangle;

// angle to line origin
extern int              rw_angle1;

extern visplane_t       *floorplane;
extern visplane_t       *ceilingplane;

// GL-only render state
typedef struct
{
  // GL vertices
  vertex_t* vertexes;
  int numvertexes;

  // GL segs
  seg_t* segs;
  int numsegs;

  // GL walls (no minisegs, cache-efficient)
  gl_wall_t* walls;

  // GL subsectors
  subsector_t* subsectors;
  int numsubsectors;

  // Chunk seen flags
  char* map_chunks;
  char* map_lines_seen;

  // GL nodes
  node_t* nodes;
  int numnodes;

  // GL chunks, connected group of subsectors in a sector
  gl_chunk_t* chunks;
  unsigned int numchunks;

  // Bleed lists for chunks
  bleed_t* bleeds;
  unsigned int numbleeds;

  // GL polyobjs
  gl_polyobj_t* polyobjs;

  // Deferred chunks (used by gl_bsp.c)
  gl_chunk_t** deferred;

  // Visible chunks queue (used by gl_bsp.c)
  gl_chunk_t** visible;
} gl_rstate_t;

extern gl_rstate_t gl_rstate;

// Resolve chunk id to pointer
static inline gl_chunk_t* GL_Chunk(int id)
{
  return id == NO_CHUNK ? NULL : &gl_rstate.chunks[id];
}

#endif
