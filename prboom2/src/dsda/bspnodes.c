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
//	GL nodes side loading
//

#include "lprintf.h"
#include "w_wad.h"
#include "p_setup.h"

#include "dsda/data_organizer.h"
#include "dsda/utility.h"
#include "dsda/ajbsp/glue.h"

#include "bspinternal.h"

static void LoadGLVertices(void* data, unsigned int size)
{
  typedef struct
  {
    int32_t x, y;
  } PACKEDATTR glvert5_t;

  glvert5_t* raw = (glvert5_t*) ((char*) data + 4);
  unsigned int count = (size > 4 ? size - 4 : 0) / sizeof(*raw);
  unsigned int i;

  gl_rstate.numvertexes = count;

  if (count == 0)
    return;

  gl_rstate.vertexes = Z_Calloc(sizeof(*gl_rstate.vertexes), count);
  if (!gl_rstate.vertexes)
    I_Error("Not enough memory for GL vertices");

  for (i = 0; i < count; ++i)
  {
    glvert5_t* vraw = &raw[i];
    vertex_t* v = &gl_rstate.vertexes[i];

    v->x = v->px = LittleLong(vraw->x);
    v->y = v->py = LittleLong(vraw->y);
  }
}

static vertex_t* DecodeVert(uint32_t v)
{
    v = LittleLong(v);
    if (v & 0x80000000)
    {
      v &= ~0x80000000;
      if (v >= gl_rstate.numvertexes)
        I_Error("LoadGLSegs: invalid new vertex %u", v);
      return &gl_rstate.vertexes[v];
    }
    if (v >= numvertexes)
      I_Error("LoadGLSegs: invalid original vertex %u", v);
    return &vertexes[v];
}

static void LoadGLSegs(void* data, unsigned int size)
{
  typedef struct
  {
    uint32_t start;
    uint32_t end;
    uint16_t linedef;
    uint16_t side;
    uint32_t partner;
  } PACKEDATTR glseg5_t;

  glseg5_t* raw = (glseg5_t*) data;
  unsigned int count = size / sizeof(*raw);
  unsigned int i;

  gl_rstate.numsegs = count;
  gl_rstate.segs = Z_Calloc(sizeof(*gl_rstate.segs), count);
  if (!gl_rstate.segs)
    I_Error("Not enough memory for GL segs");
  dsda_gl_rstate.segmeta = Z_Calloc(sizeof(*dsda_gl_rstate.segmeta), count);
  if (!dsda_gl_rstate.segmeta)
    I_Error("Not enough memory for seg metadata");
  gl_rstate.walls = Z_Calloc(sizeof(*gl_rstate.walls), count);
  if (!gl_rstate.walls)
    I_Error("Not enough memory for GL walls");

  for (i = 0; i < count; ++i)
  {
    glseg5_t* sraw = &raw[i];
    seg_t* s = &gl_rstate.segs[i];
    segmeta_t* m = &dsda_gl_rstate.segmeta[i];
    uint16_t linedef;
    uint32_t partner;

    s->v1 = DecodeVert(sraw->start);
    s->v2 = DecodeVert(sraw->end);

    linedef = LittleShort(sraw->linedef);
    if (linedef == 0xFFFF)
      s->linedef = NULL;
    else if (linedef >= numlines)
      I_Error("LoadGLSegs: seg %u had invalid linedef %u", i, linedef);
    else
      s->linedef = &lines[linedef];

    partner = LittleLong(sraw->partner);
    if (partner == 0xFFFFFFFF)
      m->partner = NULL;
    else if (partner >= gl_rstate.numsegs)
      I_Error("LoadGLSegs: seg %u has invalid partner %u", i, partner);
    else
      m->partner = &gl_rstate.segs[partner];

    if (s->linedef)
    {
      if (sraw->side)
      {
        s->frontsector = s->linedef->backsector;
        s->backsector = s->linedef->frontsector;
        s->sidedef = &sides[s->linedef->sidenum[1]];
      }
      else
      {
        s->frontsector = s->linedef->frontsector;
        s->backsector = s->linedef->backsector;
        s->sidedef = &sides[s->linedef->sidenum[0]];
      }
    }

    m->flags = SEGF_NONE;
    // FIXME: probably not needed for gl... right?
    s->offset = s->pangle = 0;
  }
}

static void LoadGLSubsectors(void* data, unsigned int size)
{
  typedef struct
  {
    uint32_t num;
    uint32_t first;
  } PACKEDATTR glssect5_t;

  glssect5_t* raw = (glssect5_t*) data;
  unsigned int count = size / sizeof(*raw);
  unsigned int i, j;

  gl_rstate.numsubsectors = count;
  gl_rstate.subsectors = Z_Calloc(sizeof(*gl_rstate.subsectors), count);
  if (!gl_rstate.subsectors)
    I_Error("Not enough memory for GL subsectors");
  dsda_gl_rstate.submeta = Z_Calloc(sizeof(*dsda_gl_rstate.submeta), count);
  if (!dsda_gl_rstate.submeta)
    I_Error("Not enough memory for GL subsectors");

  for (i = 0; i < count; ++i)
  {
    glssect5_t* sraw = &raw[i];
    subsector_t* s = &gl_rstate.subsectors[i];
    submeta_t* m = &dsda_gl_rstate.submeta[i];

    s->numlines = LittleLong(sraw->num);
    s->firstline = LittleLong(sraw->first);
    s->numwalls = 0;
    s->firstwall = -1;

    if (s->firstline >= gl_rstate.numsegs ||
        s->numlines > gl_rstate.numsegs || // Ensure following does not overflow
        s->firstline + s->numlines > gl_rstate.numsegs)
      I_Error("LoadGLSubsectors: subsector %u has out-of-bounds segment range", i);

    s->poly = NULL;
    s->chunk = NO_CHUNK;
    s->sector = gl_rstate.segs[s->firstline].frontsector;
    if (!s->sector)
      I_Error("GL subsector had miniseg as first seg");

    m->flags = SUBF_NONE;
    m->q_next = NULL;
    m->chunk_next = NO_CHUNK;

    for (j = 0; j < s->numlines; ++j)
    {
      seg_t* seg = &gl_rstate.segs[s->firstline + j];
      if (seg->frontsector && seg->frontsector != s->sector)
      {
        m->flags |= SUBF_MULTISECTOR;
        break;
      }
    }
  }

  gl_rstate.map_chunks = Z_Calloc(sizeof(*gl_rstate.map_chunks), count);
  if (!gl_rstate.map_chunks)
    I_Error("Not enough memory for GL map subsectors");

}

void LoadGLNodes(void* data, unsigned int size)
{
  typedef struct
  {
    int16_t x, y;
    int16_t dx, dy;
    int16_t boxes[2][4];
    uint32_t children[2];
  } PACKEDATTR glnode5_t;

  glnode5_t* raw = (glnode5_t*) data;
  unsigned int count = size / sizeof(*raw);
  unsigned int i;

  gl_rstate.numnodes = count;
  gl_rstate.nodes = Z_Calloc(sizeof(*gl_rstate.nodes), count);
  if (!gl_rstate.nodes)
    I_Error("Not enough memory for GL nodes");

  for (i = 0; i < count; ++i)
  {
    glnode5_t* nraw = &raw[i];
    node_t* n = &gl_rstate.nodes[i];
    int j, k;

    n->x = LittleShort(nraw->x) << FRACBITS;
    n->y = LittleShort(nraw->y) << FRACBITS;
    n->dx = LittleShort(nraw->dx) << FRACBITS;
    n->dy = LittleShort(nraw->dy) << FRACBITS;

    for (j = 0; j < 2; ++j)
      for (k = 0; k < 4; ++k)
        n->bbox[j][k] = LittleShort(nraw->boxes[j][k]) << FRACBITS;

    for (j = 0; j < 2; ++j)
    {
      uint32_t child = LittleLong(nraw->children[j]);

      if (child & NF_SUBSECTOR &&
          (child & ~NF_SUBSECTOR) >= gl_rstate.numsubsectors)
        I_Error("LoadGLNodes: node %u references invalid subsector %u", i,
                (child & ~NF_SUBSECTOR));
      else if (!(child & NF_SUBSECTOR) && child >= gl_rstate.numnodes)
        I_Error("LoadGLNodes: node %u references invalid child %u", i, child);
      n->children[j] = child;
    }
  }
}

void LoadSideGLNodes(void)
{
  dsda_ajbsp_glnodes_t nodes;
  int lumpnum;
  const lumpinfo_t* info;
  const char* inpath;
  const char* basename;
  dsda_string_t outpath;

  lumpnum = W_GetNumForName(current_map_lump);
  info = W_GetLumpInfoByNum(lumpnum);
  inpath = info->wadfile->name;
  basename = dsda_BaseName(inpath);

  dsda_StringPrintF(&outpath, "%s/gl.%s", dsda_DataDir(), basename);

  if (!dsda_ajbsp_LoadGLNodes(inpath, outpath.string, current_map_lump, &nodes))
    I_Error("Could not build GL nodes");

  dsda_FreeString(&outpath);

  LoadGLVertices(nodes.vertexes, nodes.vsize);
  LoadGLSegs(nodes.segs, nodes.ssize);
  LoadGLSubsectors(nodes.subsectors, nodes.sssize);
  LoadGLNodes(nodes.nodes, nodes.nsize);

  Z_Free(nodes.vertexes);
  Z_Free(nodes.segs);
  Z_Free(nodes.subsectors);
  Z_Free(nodes.nodes);
}

void ClearGLNodes(void)
{
  gl_rstate.numvertexes = 0;
  if (gl_rstate.vertexes && gl_rstate.vertexes != vertexes)
    Z_Free(gl_rstate.vertexes);
  gl_rstate.vertexes = NULL;

  gl_rstate.numsegs = 0;
  if (gl_rstate.segs && gl_rstate.segs != segs)
    Z_Free(gl_rstate.segs);
  gl_rstate.segs = NULL;
  if (dsda_gl_rstate.segmeta)
    Z_Free(dsda_gl_rstate.segmeta);
  dsda_gl_rstate.segmeta = NULL;

  gl_rstate.numsubsectors = 0;
  if (gl_rstate.subsectors && gl_rstate.subsectors != subsectors)
    Z_Free(gl_rstate.subsectors);
  gl_rstate.subsectors = NULL;
  if (gl_rstate.map_chunks)
    Z_Free(gl_rstate.map_chunks);
  gl_rstate.map_chunks = NULL;
  if (dsda_gl_rstate.submeta)
    Z_Free(dsda_gl_rstate.submeta);
  dsda_gl_rstate.submeta = NULL;

  gl_rstate.numnodes = 0;
  if (gl_rstate.nodes && gl_rstate.nodes != nodes)
    Z_Free(gl_rstate.nodes);
  gl_rstate.nodes = NULL;
}

void ResetGLNodes(void)
{
  int i;

  for (i = 0; i < gl_rstate.numchunks; ++i)
  {
    gl_chunk_t* chunk = &gl_rstate.chunks[i];
    chunk->numpolyobjs = 0;
  }
}
