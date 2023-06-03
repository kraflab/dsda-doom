/* Emacs style mode select   -*- C -*-
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
#include "m_bbox.h"
#include "hexen/po_man.h"

#include "dsda/bsp.h"

static int gld_max_vertexes = 0;
static int gld_num_vertexes = 0;

// this is the list for all subsectors to the loops
// uses by textured automap
GLLoops *chunkloops;

static void gld_AddGlobalVertexes(int count)
{
  if ((gld_num_vertexes+count)>=gld_max_vertexes)
  {
    gld_max_vertexes+=count+1024;
    flats_vbo = Z_Realloc(flats_vbo, gld_max_vertexes * sizeof(flats_vbo[0]));
  }
}

static void gld_SetupSubsectorLoop(chunkmeta_t* chunkmeta, subsector_t *ssec)
{
  GLLoopDef **loop;
  int *loopcount;
  int chunknum = chunkmeta - dsda_gl_rstate.chunkmeta;

  loop = &chunkloops[chunknum].loops;
  loopcount = &chunkloops[chunknum].loopcount;

  (*loopcount)++;
  (*loop) = Z_Realloc((*loop), sizeof(GLLoopDef)*(*loopcount));
  ((*loop)[(*loopcount) - 1]).index = ssec - gl_rstate.subsectors;
  ((*loop)[(*loopcount) - 1]).mode = GL_TRIANGLE_FAN;
  ((*loop)[(*loopcount) - 1]).vertexindex = gld_num_vertexes;
}

static void gld_FinishSubsectorLoop(chunkmeta_t* chunkmeta, subsector_t* ssec, int numedgepoints)
{
  GLLoopDef **loop;
  int *loopcount;
  int chunknum = chunkmeta - dsda_gl_rstate.chunkmeta;
  loop = &chunkloops[chunknum].loops;
  loopcount = &chunkloops[chunknum].loopcount;

  ((*loop)[(*loopcount) - 1]).vertexcount = numedgepoints;
}

// Chunk tessellation
//
// This (hopefully) generates more efficient vertex arrays than
// concatenating all the subsector vertices

typedef struct tessctx_s
{
  int chunk;
  int vertices;
} tessctx_t;

static void CALLBACK TessBegin(GLenum type, void* data)
{
  tessctx_t* ctx = (tessctx_t*) data;
  GLLoops* comp = &chunkloops[ctx->chunk];
  GLLoopDef* loop;

  comp->loopcount++;
  comp->loops = Z_Realloc(comp->loops, sizeof(GLLoopDef) * comp->loopcount);
  loop = &comp->loops[comp->loopcount - 1];
  loop->index = -1;
  loop->mode = type;
  loop->vertexcount = 0;
  loop->vertexindex = gld_num_vertexes;
}

static void CALLBACK TessVertex(void* vert, void* data)
{
  tessctx_t* ctx = (tessctx_t*) data;
  GLLoops* comp = &chunkloops[ctx->chunk];
  GLLoopDef* loop = &comp->loops[comp->loopcount - 1];
  dpoint_t* v = (dpoint_t*) vert;
  vbo_xyz_uv_t *vbo;

  loop->vertexcount++;

  gld_AddGlobalVertexes(1);
  vbo = &flats_vbo[gld_num_vertexes++];
  vbo->u = (v->x / FRACUNIT) / 64.0f;
  vbo->v = (-v->y / FRACUNIT) / 64.0f;
  vbo->x = -v->x / MAP_SCALE;
  vbo->y = 0.0f;
  vbo->z = v->y / MAP_SCALE;

  ctx->vertices++;
}

static dboolean gld_TesselateChunk(chunkmeta_t* chunkmeta)
{
  GLUtesselator* tess;
  tessctx_t ctx;
  int i;

  ctx.chunk = chunkmeta - dsda_gl_rstate.chunkmeta;
  ctx.vertices = 0;

  tess = gluNewTess();
  if (!tess)
    I_Error("Could not allocate GLU tesselator");

  gluTessCallback(tess, GLU_TESS_BEGIN_DATA, (void(CALLBACK*)()) TessBegin);
  gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (void(CALLBACK*)()) TessVertex);

  gluTessBeginPolygon(tess, &ctx);
  gluTessBeginContour(tess);
  for (i = 0; i < chunkmeta->numpoints; ++i)
  {
    dpoint_t* p = &dsda_gl_rstate.paths[chunkmeta->firstpoint + i];
    GLdouble coords[3];

    if (dsda_PointIsEndOfContour(p))
    {
      gluTessEndContour(tess);
      if (i != chunkmeta->numpoints - 1)
        gluTessBeginContour(tess);
      continue;
    }

    coords[0] = -p->x / MAP_SCALE;
    coords[1] = 0.0;
    coords[2] = p->y / MAP_SCALE;

    gluTessVertex(tess, coords, p);
  }
  gluTessEndPolygon(tess);
  gluDeleteTess(tess);

  return ctx.vertices != 0;
}

/********************************************
 * Name     : gld_GetSubSectorVertices      *
 * created  : 08/13/00                      *
 * modified : 09/18/00, adapted for PrBoom  *
 * author   : figgi                         *
 * what     : prepares subsectorvertices    *
 *            (glnodes only)                *
 ********************************************/

static void gld_GetSubSectorVertices(chunkmeta_t* chunkmeta, subsector_t* ssector)
{
  int i;
  int numedgepoints = 0;

  gld_SetupSubsectorLoop(chunkmeta, ssector);

  for (i = 0; i < ssector->numlines; i++)
  {
    seg_t* seg = &gl_rstate.segs[ssector->firstline + i];
    vbo_xyz_uv_t* vbo;

    gld_AddGlobalVertexes(1);
    vbo = &flats_vbo[gld_num_vertexes++];
    numedgepoints++;

    vbo->u = ((float)(seg->v1->x) / FRACUNIT) / 64.0f;
    vbo->v = (-(float)(seg->v1->y) / FRACUNIT) / 64.0f;
    vbo->x = -(float)(seg->v1->x) / MAP_SCALE;
    vbo->y = 0.0f;
    vbo->z = (float)(seg->v1->y) / MAP_SCALE;
  }

  gld_FinishSubsectorLoop(chunkmeta, ssector, numedgepoints);
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

static void gld_MarkLoopsForClamp(GLLoops* loopsarray, int count)
{
  int i;

  for (i = 0; i < count; i++)
  {
    GLLoops* loops = &loopsarray[i];
    int loopnum; // current loop number
    GLLoopDef *currentloop; // the current loop
    GLfloat minu, maxu, minv, maxv;
    dboolean fail;

    minu = minv = 65535;
    maxu = maxv = -65535;
    fail = false;

    for (loopnum = 0; !fail && loopnum < loops->loopcount; loopnum++)
    {
      int vertexnum;
      // set the current loop
      currentloop = &loops->loops[loopnum];
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
      loops->flags = SECTOR_CLAMPXY;

      for (loopnum=0; loopnum<loops->loopcount; loopnum++)
      {
        int vertexnum;
        // set the current loop
        currentloop = &loops->loops[loopnum];
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
  int i;

  if (gl_rstate.numchunks)
  {
    chunkloops=Z_Malloc(sizeof(*chunkloops)*gl_rstate.numchunks);
    if (!chunkloops)
      I_Error("gld_PreprocessSectors: Not enough memory for array chunkloops");
    memset(chunkloops, 0, sizeof(*chunkloops)*gl_rstate.numchunks);
  }

  if (numlines)
  {
    linerendered[0]=Z_Calloc(numlines, sizeof(*linerendered[0]));
    linerendered[1]=Z_Calloc(numlines, sizeof(*linerendered[1]));
    if (!linerendered[0] || !linerendered[1])
      I_Error("gld_PreprocessSectors: Not enough memory for array linerendered");
  }

  flats_vbo = NULL;
  gld_max_vertexes=0;
  gld_num_vertexes=0;

  // Generate chunk loops
  for (i = 0; i < gl_rstate.numchunks; ++i)
  {
    chunkmeta_t* chunkmeta = &dsda_gl_rstate.chunkmeta[i];

    if (!chunkmeta->numpoints || !gld_TesselateChunk(chunkmeta))
    {
      // Fall back on concatenating all the subsector loops if path generation
      // or tesselation failed
      subsector_t* sub;
      submeta_t* meta;
      int subnum;

      for (subnum = chunkmeta->subsectors; subnum != -1; subnum = meta->chunk_next)
      {
        sub = &gl_rstate.subsectors[subnum];
        meta = &dsda_gl_rstate.submeta[subnum];

        gld_GetSubSectorVertices(chunkmeta, sub);
      }
    }
  }

  //e6y: for seamless rendering
  gld_MarkLoopsForClamp(chunkloops, gl_rstate.numchunks);
}

static void gld_PreprocessSegs(void)
{
  int i;

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
    static int numchunks_prev = 0;

    Z_Free(gl_lines);

    Z_Free(flats_vbo);
    flats_vbo = NULL;

    Z_Free(linerendered[0]);
    Z_Free(linerendered[1]);

    for (i = 0; i < numchunks_prev; i++)
      Z_Free(chunkloops[i].loops);
    Z_Free(chunkloops);

    dsda_AnnotateBSP();

    gld_Precache();
    gld_PreprocessSectors();
    gld_PreprocessSegs();

    // Move poly objs into GL subsectors
    PO_ChangeRenderMode();

    numchunks_prev = gl_rstate.numchunks;
  }
  else
  {
    memset(linerendered[0], 0, numlines*sizeof(linerendered[0][0]));
    memset(linerendered[1], 0, numlines*sizeof(linerendered[1][0]));
  }

  rendermarker = 0;

  gld_ResetLastTexture();
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

#if 0
      // These are technically no longer required, but having them
      // around for dumping/debugging is useful
      Z_Free(flats_vbo);
      flats_vbo = NULL;
#endif
    }
  }

  //e6y
  gld_InitVertexData();

  gl_preprocessed = true;
}
