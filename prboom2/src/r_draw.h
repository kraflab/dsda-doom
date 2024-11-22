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
 *      System specific interface stuff.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __R_DRAW__
#define __R_DRAW__

#include "r_defs.h"

enum column_pipeline_e {
  RDC_PIPELINE_STANDARD,
  RDC_PIPELINE_TRANSLUCENT,
  RDC_PIPELINE_TRANSLATED,
  RDC_PIPELINE_FUZZ,
  RDC_PIPELINE_MAXPIPELINES,
};

// Used to specify what kind of filering you want
enum draw_filter_type_e {
  RDRAW_FILTER_NONE,
  RDRAW_FILTER_POINT,
  RDRAW_FILTER_MAXFILTERS
};

typedef enum
{
  DRAW_COLUMN_ISPATCH = 0x00000001
} draw_column_flags_e;

typedef struct draw_column_vars_s* pdraw_column_vars_s;
typedef void (*R_DrawColumn_f)(pdraw_column_vars_s dcvars);

// Packaged into a struct - POPE
typedef struct draw_column_vars_s
{
  int                 x;
  int                 yl;
  int                 yh;
  int                 dy;
  fixed_t             iscale;
  fixed_t             texturemid;
  int                 texheight;    // killough
  const byte          *source; // first pixel in a column
  const byte          *prevsource; // first pixel in previous column
  const byte          *nextsource; // first pixel in next column
  const lighttable_t  *colormap;
  const byte          *translation;
  int                 edgeslope; // OR'ed RDRAW_EDGESLOPE_*
  // 1 if R_DrawColumn* is currently drawing a masked column, otherwise 0
  int                 drawingmasked;
  unsigned int        flags; //e6y: for detect patches ind colfunc()

  // heretic
  int baseclip;
} draw_column_vars_t;

void R_SetDefaultDrawColumnVars(draw_column_vars_t *dcvars);

typedef struct {
  int                 y;
  int                 x1;
  int                 x2;
  fixed_t             z; // the current span z coord
  fixed_t             xfrac;
  fixed_t             yfrac;
  fixed_t             xstep;
  fixed_t             ystep;
  const byte          *source; // start of a 64*64 tile image
  const lighttable_t  *colormap;

  fixed_t xoffs;
  fixed_t yoffs;
  fixed_t xscale;
  fixed_t yscale;
  fixed_t sine;
  fixed_t cosine;
  fixed_t planeheight;
  const lighttable_t **planezlight;
} draw_span_vars_t;

typedef struct {
  byte           *topleft;
  int   pitch;
} draw_vars_t;

extern draw_vars_t drawvars;

extern byte playernumtotrans[MAX_MAXPLAYERS]; // CPhipps - what translation table for what player
extern byte       *translationtables;

R_DrawColumn_f R_GetDrawColumnFunc(enum column_pipeline_e type, enum draw_filter_type_e filterz);

// Span blitting for rows, floor/ceiling. No Spectre effect needed.
void R_DrawSpan(draw_span_vars_t *dsvars);

void R_InitBuffer(int width, int height);

void R_InitBuffersRes(void);

// Initialize color translation tables, for player rendering etc.
void R_InitTranslationTables(void);

// Rendering function.
void R_FillBackScreen(void);

// If the view size is not full screen, draws a border around it.
void R_DrawViewBorder(void);

// haleyjd 09/13/04: new function to call from main rendering loop
// which gets rid of the unnecessary reset of various variables during
// column drawing.
void R_ResetColumnBuffer(void);

void R_SetFuzzPos(int fuzzpos);
int R_GetFuzzPos();

// height is the height of the last column, in pixels
void R_ResetFuzzCol(int height);

// Calls R_ResetFuzzCol if x is aligned to the fuzz cell grid
void R_CheckFuzzCol(int x, int height);

#endif
