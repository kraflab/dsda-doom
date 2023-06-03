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
//	BSP State Dumping
//

#include <assert.h>
#include <math.h>

#include "bspdump.h"
#include "bsp.h"
#include "dgeom.h"
#include "m_bbox.h"
#include "r_state.h"
#include "r_main.h"
#include "gl_intern.h"

//
// JSON dumping logic.  Extremely vital for debugging.
//

struct valueinfo
{
  unsigned long value;
  const char* name;
};

static void DumpComma(FILE* fp)
{
  fprintf(fp, ",");
}

static void DumpNull(FILE* fp)
{
  fprintf(fp, "null");
}

static void DumpStr(FILE* fp, const char* str)
{
  fprintf(fp, "\"%s\"", str);
}

static void DumpFixed(FILE* fp, fixed_t value)
{
  fprintf(fp, "%f", (double) value / FRACUNIT);
}

static void DumpPointInner(FILE* fp, fixed_t x, fixed_t y)
{
  fprintf(fp, "\"x\":%f,\"y\":%f", (double)x / FRACUNIT, (double)y / FRACUNIT);
}

static void DumpPoint(FILE* fp, fixed_t x, fixed_t y)
{
  fprintf(fp, "{");
  DumpPointInner(fp, x, y);
  fprintf(fp, "}");
}

static void DumpPointPairInner(FILE* fp, fixed_t x1, fixed_t y1, fixed_t x2,
                               fixed_t y2)
{
  fprintf(fp, "\"start\":{\"x\":%f,\"y\":%f},\"end\":{\"x\":%f,\"y\":%f}",
          (double)x1 / FRACUNIT, (double)y1 / FRACUNIT, (double)x2 / FRACUNIT,
          (double)y2 / FRACUNIT);
}

static void DumpPointPair(FILE* fp, fixed_t x1, fixed_t y1, fixed_t x2,
                          fixed_t y2)
{
  fprintf(fp, "{");
  DumpPointPairInner(fp, x1, y1, x2, y2);
  fprintf(fp, "}");
}

static void DumpDPoint(FILE* fp, const dpoint_t* point)
{
  fprintf(fp, "{\"x\":%f,\"y\":%f}", point->x / FRACUNIT, point->y / FRACUNIT);
}

static void DumpDLine(FILE* fp, const dline_t* line)
{
  fprintf(fp, "{\"start\":{\"x\":%f,\"y\":%f},\"end\":{\"x\":%f,\"y\":%f}}",
          line->start.x / FRACUNIT, line->start.y / FRACUNIT,
          line->end.x / FRACUNIT, line->end.y / FRACUNIT);
}

static void DumpBBox(FILE* fp, const fixed_t* value)
{
  fprintf(fp, "{\"bottom\":%f,\"left\":%f,\"right\":%f,\"top\":%f}",
      (double) value[BOXBOTTOM] / FRACUNIT,
      (double) value[BOXLEFT] / FRACUNIT,
      (double) value[BOXRIGHT] / FRACUNIT,
      (double) value[BOXTOP] / FRACUNIT);
}

static void DumpTriangle(FILE* fp, vbo_xyz_uv_t* p1, vbo_xyz_uv_t* p2, vbo_xyz_uv_t* p3)
{
  fprintf(fp, "[{\"x\":%f,\"y\":%f},{\"x\":%f,\"y\":%f},{\"x\":%f,\"y\":%f}]",
      -p1->x * MAP_COEFF,
      p1->z * MAP_COEFF,
      -p2->x * MAP_COEFF,
      p2->z * MAP_COEFF,
      -p3->x * MAP_COEFF,
      p3->z * MAP_COEFF);
}

static void DumpId(FILE* fp, unsigned long i)
{
  fprintf(fp, "\"%lu\"", i);
}

static void DumpKey(FILE* fp, const char* key)
{
  fprintf(fp, "\"%s\":", key);
}

static void DumpIdKey(FILE* fp, unsigned long i)
{
  DumpId(fp, i);
  fprintf(fp, ":");
}

static void DumpKeyId(FILE* fp, const char* key, int value)
{
  if (value < 0)
    fprintf(fp, "\"%s\":null", key);
  else
    fprintf(fp, "\"%s\":\"%i\"", key, value);
}

static void DumpKeyNull(FILE* fp, const char* key)
{
  fprintf(fp, "\"%s\":null", key);
}

static void DumpKeyFixed(FILE* fp, const char* key, fixed_t value)
{
  fprintf(fp, "\"%s\":%f", key, (double) value / FRACUNIT);
}

static void DumpFlagsInner(FILE* fp, const struct valueinfo* info,
                           unsigned long flags)
{
  dboolean comma = false;

  for (; info->name; info++)
  {
    if (comma)
      DumpComma(fp);
    DumpKey(fp, info->name);
    fprintf(fp, (flags & info->value) ? "true" : "false");
    comma = true;
  }
}

static void DumpFlags(FILE* fp, const struct valueinfo* info,
                      unsigned long flags)
{
  fprintf(fp, "{");
  DumpFlagsInner(fp, info, flags);
  fprintf(fp, "}");
}

static void DumpEnum(FILE* fp, const struct valueinfo* info,
                     unsigned long value)
{
  for (; info->name; info++)
  {
    if (value == info->value)
    {
      DumpStr(fp, info->name);
      return;
    }
  }
  DumpNull(fp);
}

static void DumpVertex(FILE* fp, const vertex_t* v)
{
  DumpPoint(fp, v->px, v->py);
}

static void DumpVertices(FILE* fp)
{
  int i;
  dboolean comma = false;

  fprintf(fp, "{");
  for (i = 0; i < numvertexes; ++i)
  {
    if (comma)
      DumpComma(fp);
    DumpIdKey(fp, i);
    DumpVertex(fp, &vertexes[i]);
    comma = true;
  }
  for (i = 0; i < gl_rstate.numvertexes; ++i)
  {
    if (comma)
      DumpComma(fp);
    DumpIdKey(fp, i + numvertexes);
    DumpVertex(fp, &gl_rstate.vertexes[i]);
    comma = true;
  }
  fprintf(fp, "}");
}

static void DumpLine(FILE* fp, const line_t* line)
{
  static const struct valueinfo linerflags[] =
  {
    {0, NULL}
  };

  fprintf(fp, "{");
  DumpKeyId(fp, "v1", line->v1 - vertexes);
  DumpComma(fp);
  DumpKeyId(fp, "v2", line->v2 - vertexes);
  DumpComma(fp);
  DumpKey(fp, "front");
  if (line->sidenum[0] != NO_INDEX)
    DumpId(fp, line->sidenum[0]);
  else
    DumpNull(fp);
  DumpComma(fp);
  DumpKey(fp, "back");
  if (line->sidenum[1] != NO_INDEX)
    DumpId(fp, line->sidenum[1]);
  else
    DumpNull(fp);
  DumpComma(fp);
  DumpKey(fp, "flags");
  DumpFlags(fp, linerflags, line->r_flags);
  fprintf(fp, "}");
}

static void DumpLines(FILE* fp)
{
  int i;

  fprintf(fp, "{");
  for (i = 0; i < numlines; ++i)
  {
    DumpIdKey(fp, i);
    DumpLine(fp, &lines[i]);
    if (i != numlines - 1)
      fprintf(fp, ",");
  }
  fprintf(fp, "}");
}

static void DumpSide(FILE* fp, const side_t* side)
{
  fprintf(fp, "{");
  DumpKeyId(fp, "sector", side->sector - sectors);
  fprintf(fp, "}");
}

static void DumpSides(FILE* fp)
{
  int i;

  fprintf(fp, "{");
  for (i = 0; i < numsides; ++i)
  {
    DumpIdKey(fp, i);
    DumpSide(fp, &sides[i]);
    if (i != numsides - 1)
      fprintf(fp, ",");
  }
  fprintf(fp, "}");
}

static int VertexId(vertex_t* v)
{
  if (v >= gl_rstate.vertexes && v < gl_rstate.vertexes + gl_rstate.numvertexes)
    return v - gl_rstate.vertexes + numvertexes;
  return v - vertexes;
}

static void DumpSeg(FILE* fp, const seg_t* seg)
{
  static const struct valueinfo segflags[] = {
    {SEGF_HACKED, "hacked"},
    {SEGF_ORPHAN, "orphan"},
    {0, NULL},
  };
  segmeta_t* meta = &dsda_gl_rstate.segmeta[seg - gl_rstate.segs];

  fprintf(fp, "{");
  DumpKey(fp, "line");
  if (seg->linedef)
    DumpId(fp, seg->linedef - lines);
  else
    DumpNull(fp);
  DumpComma(fp);
  DumpKey(fp, "side");
  if (seg->sidedef)
    DumpId(fp, seg->sidedef - sides);
  else
    DumpNull(fp);
  DumpComma(fp);
  DumpKeyId(fp, "subsector", meta->subsector - gl_rstate.subsectors);
  DumpComma(fp);
  DumpKey(fp, "partner");
  if (meta->partner)
    DumpId(fp, meta->partner - gl_rstate.segs);
  else
    DumpNull(fp);
  DumpComma(fp);
  DumpKeyId(fp, "v1", VertexId(seg->v1));
  DumpComma(fp);
  DumpKeyId(fp, "v2", VertexId(seg->v2));
  DumpComma(fp);
  DumpKey(fp, "flags");
  DumpFlags(fp, segflags, meta->flags);
  fprintf(fp, "}");
}

static void DumpSegs(FILE* fp)
{
  int i;
  fprintf(fp, "{");

  for (i = 0; i < gl_rstate.numsegs; ++i)
  {
    DumpIdKey(fp, i);
    DumpSeg(fp, &gl_rstate.segs[i]);
    if (i != gl_rstate.numsegs - 1)
      fprintf(fp, ",");
  }

  fprintf(fp, "}");
}

static void DumpSector(FILE* fp, const sector_t* sector)
{
  static const struct valueinfo sectorflags[] = {
    {0, NULL},
  };
  int i = 0;

  fprintf(fp, "{");
  DumpKeyFixed(fp, "floorheight", sector->floorheight);
  DumpComma(fp);
  DumpKeyFixed(fp, "ceilingheight", sector->ceilingheight);
  DumpComma(fp);
  DumpKey(fp, "flags");
  DumpFlags(fp, sectorflags, sector->flags);
  DumpComma(fp);
  DumpKey(fp, "lines");
  fprintf(fp, "[");
  for (i = 0; i < sector->linecount; ++i)
  {
    DumpId(fp, sector->lines[i] - lines);
    if (i != sector->linecount - 1)
      DumpComma(fp);
  }
  fprintf(fp, "]");
  fprintf(fp, "}");
}

static void DumpSectors(FILE* fp)
{
  int i;
  fprintf(fp, "{");

  for (i = 0; i < numsectors; ++i)
  {
    DumpIdKey(fp, i);
    DumpSector(fp, &sectors[i]);
    if (i != numsectors - 1)
      fprintf(fp, ",");
  }

  fprintf(fp, "}");
}
static void DumpSubsector(FILE* fp, const subsector_t* subsector, const submeta_t* submeta)
{
  static const struct valueinfo subflags[] = {
    {SUBF_HACKED, "hacked"},
    {SUBF_DEGENERATE, "degenerate"},
    {0, NULL},
  };
  int i = 0;

  fprintf(fp, "{");
  DumpKeyId(fp, "sector", subsector->sector - sectors);
  DumpComma(fp);
  DumpKeyId(fp, "chunk", subsector->chunk);
  DumpComma(fp);
  DumpKey(fp, "segs");
  fprintf(fp, "[");
  for (i = 0; i < subsector->numlines; ++i)
  {
    DumpId(fp, subsector->firstline + i);
    if (i != subsector->numlines - 1)
      DumpComma(fp);
  }
  fprintf(fp, "]");
  DumpComma(fp);
  DumpKey(fp, "flags");
  DumpFlags(fp, subflags, submeta->flags);
  fprintf(fp, "}");
}

static void DumpSubsectors(FILE* fp)
{
  int i;
  fprintf(fp, "{");

  for (i = 0; i < gl_rstate.numsubsectors; ++i)
  {
    DumpIdKey(fp, i);
    DumpSubsector(fp, &gl_rstate.subsectors[i], &dsda_gl_rstate.submeta[i]);
    if (i != gl_rstate.numsubsectors - 1)
      fprintf(fp, ",");
  }

  fprintf(fp, "}");
}

static void DumpNode(FILE* fp, const node_t* node, const fixed_t* bbox)
{
  int i;
  dline_t div = dgeom_DLineFromNode(node);

  dgeom_ClipLineByBox(&div, bbox, &div);

  if (node != &gl_rstate.nodes[gl_rstate.numnodes - 1])
    DumpComma(fp);

  DumpIdKey(fp, node - gl_rstate.nodes);
  fprintf(fp, "{");
  DumpKey(fp, "partition");
  DumpDLine(fp, &div);
  DumpComma(fp);
  DumpKey(fp, "children");
  fprintf(fp, "{");
  for (i = 0; i < 2; ++i)
  {
    int nodenum = node->children[i];
    DumpKey(fp, i ? "left" : "right");
    fprintf(fp, "{");
    if (nodenum & NF_SUBSECTOR)
      DumpKeyId(fp, "subsector", nodenum & ~NF_SUBSECTOR);
    else
      DumpKeyId(fp, "node", nodenum);
    fprintf(fp, "}");
    if (i == 0)
      DumpComma(fp);
  }
  fprintf(fp, "}");
  DumpComma(fp);
  DumpKey(fp, "bounds");
  fprintf(fp, "{");
  for (i = 0; i < 2; ++i)
  {
    DumpKey(fp, i ? "left" : "right");
    DumpBBox(fp, node->bbox[i]);
    if (i == 0)
      DumpComma(fp);
  }
  fprintf(fp, "}");
  fprintf(fp, "}");

  for (i = 0; i < 2; ++i)
  {
    if (!(node->children[i] & NF_SUBSECTOR))
      DumpNode(fp, &gl_rstate.nodes[node->children[i]], node->bbox[i]);
  }
}

static void DumpNodes(FILE* fp)
{
  fprintf(fp, "{");
  if (gl_rstate.numnodes)
  {
    fixed_t mapbox[4];
    node_t* node = &gl_rstate.nodes[gl_rstate.numnodes - 1];

    // Compute overall bounding box
    M_ClearBox(mapbox);
    M_AddToBox(mapbox, node->bbox[0][BOXLEFT], node->bbox[0][BOXBOTTOM]);
    M_AddToBox(mapbox, node->bbox[0][BOXRIGHT], node->bbox[0][BOXTOP]);
    M_AddToBox(mapbox, node->bbox[1][BOXLEFT], node->bbox[1][BOXBOTTOM]);
    M_AddToBox(mapbox, node->bbox[1][BOXRIGHT], node->bbox[1][BOXTOP]);

    // Recurse
    DumpNode(fp, &gl_rstate.nodes[gl_rstate.numnodes - 1], mapbox);
  }
  fprintf(fp, "}");
}

static void DumpChunk(FILE* fp, const gl_chunk_t* comp,
                          const chunkmeta_t* chunkmeta)
{
  static const struct valueinfo chunkflags[] = {
    {CHUNKF_DEGENERATE, "degenerate"},
    {CHUNKF_BLEED_THROUGH, "bleed_through"},
    {CHUNKF_INTERIOR, "interior"},
    {0, NULL},
  };
  subsector_t* cur;
  submeta_t* meta;
  int subnum;
  int i, j;

  fprintf(fp, "{");
  DumpKeyId(fp, "sector", comp->sector - sectors);
  DumpComma(fp);
  DumpKey(fp, "subsectors");
  fprintf(fp, "[");
  for (subnum = chunkmeta->subsectors; subnum != -1; subnum = meta->chunk_next)
  {
    cur = &gl_rstate.subsectors[subnum];
    meta = &dsda_gl_rstate.submeta[subnum];

    DumpId(fp, cur - subsectors);
    if (meta->chunk_next != -1)
      DumpComma(fp);
  }
  fprintf(fp, "]");
  DumpComma(fp);
  DumpKey(fp, "perimeter");
  fprintf(fp, "[");
  for (i = 0; i < chunkmeta->numperim; ++i)
  {
    DumpId(fp, dsda_gl_rstate.perims[chunkmeta->firstperim + i] - gl_rstate.segs);
    if (i != chunkmeta->numperim - 1)
      DumpComma(fp);
  }
  fprintf(fp, "]");
  DumpComma(fp);
  DumpKey(fp, "path");
  fprintf(fp, "[");
  for (i = 0; i < chunkmeta->numpoints; ++i)
  {
    DumpId(fp, chunkmeta->firstpoint + i);
    if (i != chunkmeta->numpoints - 1)
      DumpComma(fp);
  }
  fprintf(fp, "]");
  DumpComma(fp);
  DumpKey(fp, "triangles");
  fprintf(fp, "[");
  if (chunkloops && flats_vbo)
  {
    GLLoops* loops = &chunkloops[comp - gl_rstate.chunks];
    for (i = 0; i < loops->loopcount; ++i)
    {
      GLLoopDef* loop = &loops->loops[i];
      dboolean comma = false;
      switch (loop->mode)
      {
      case GL_TRIANGLES:
        for (j = loop->vertexindex; j < loop->vertexindex + loop->vertexcount; j += 3)
        {
          if (comma)
            DumpComma(fp);
          DumpTriangle(fp, &flats_vbo[j], &flats_vbo[j + 1], &flats_vbo[j + 2]);
          comma = true;
        }
        break;
      case GL_TRIANGLE_FAN:
        for (j = loop->vertexindex + 1; j < loop->vertexindex + loop->vertexcount - 1; ++j)
        {
          if (comma)
            DumpComma(fp);
          DumpTriangle(fp, &flats_vbo[loop->vertexindex], &flats_vbo[j], &flats_vbo[j + 1]);
          comma = true;
        }
        break;
      case GL_TRIANGLE_STRIP:
        for (j = loop->vertexindex; j < loop->vertexindex + loop->vertexcount - 2; ++j)
        {
          if (comma)
            DumpComma(fp);
          if (j % 1)
            DumpTriangle(fp, &flats_vbo[j], &flats_vbo[j + 2], &flats_vbo[j + 1]);
          else
            DumpTriangle(fp, &flats_vbo[j], &flats_vbo[j + 1], &flats_vbo[j + 2]);
          comma = true;
        }
        break;
      }
      if (i != loops->loopcount - 1)
        DumpComma(fp);
    }
  }
  fprintf(fp, "]");
  DumpComma(fp);
  DumpKey(fp, "bleeds");
  fprintf(fp, "[");
  for (i = 0; i < comp->numbleeds; ++i)
  {
    DumpId(fp, comp->firstbleed + i);
    if (i != comp->numbleeds - 1)
      DumpComma(fp);
  }
  fprintf(fp, "]");
  DumpComma(fp);
  DumpKey(fp, "flags");
  DumpFlags(fp, chunkflags, chunkmeta->flags);
  fprintf(fp, "}");
}

static void DumpChunks(FILE* fp)
{
  int i;

  fprintf(fp, "{");
  for (i = 0; i < gl_rstate.numchunks; ++i)
  {
    DumpIdKey(fp, i);
    DumpChunk(fp, &gl_rstate.chunks[i], &dsda_gl_rstate.chunkmeta[i]);
    if (i != gl_rstate.numchunks - 1)
      DumpComma(fp);
  }
  fprintf(fp, "}");
}

static void DumpPaths(FILE* fp)
{
  int i;

  fprintf(fp, "{");
  for (i = 0; i < dsda_gl_rstate.numpaths; ++i)
  {
    DumpIdKey(fp, i);
    if (dsda_PointIsEndOfContour(&dsda_gl_rstate.paths[i]))
      fprintf(fp, "null");
    else
      DumpDPoint(fp, &dsda_gl_rstate.paths[i]);
    if (i != dsda_gl_rstate.numpaths - 1)
      DumpComma(fp);
  }
  fprintf(fp, "}");
}

static void DumpBleed(FILE* fp, const bleed_t* bleed)
{
  static struct valueinfo typeinfo[] =
  {
    {BLEED_FLOOR_OVER, "floor_over"},
    {BLEED_FLOOR_UNDER, "floor_under"},
    {BLEED_FLOOR_THROUGH, "floor_through"},
    {BLEED_CEILING_OVER, "ceiling_over"},
    {BLEED_CEILING_UNDER, "ceiling_under"},
    {BLEED_CEILING_THROUGH, "ceiling_through"},
    {0, NULL}
  };

  fprintf(fp, "{");
  DumpKeyId(fp, "target", bleed->target);
  DumpComma(fp);
  DumpKey(fp, "type");
  DumpEnum(fp, typeinfo, bleed->type);
  DumpComma(fp);
  DumpKey(fp, "depth");
  fprintf(fp, "%d", bleed->depth);
  fprintf(fp, "}");
}

static void DumpBleeds(FILE* fp)
{
  int i;

  fprintf(fp, "{");
  for (i = 0; i < gl_rstate.numbleeds; ++i)
  {
    DumpIdKey(fp, i);
    DumpBleed(fp, &gl_rstate.bleeds[i]);
    if (i != gl_rstate.numbleeds - 1)
      DumpComma(fp);
  }
  fprintf(fp, "}");
}

//
// Public interface
//

void dsda_DumpBSP(FILE* fp)
{
  fprintf(fp, "{");
  DumpKey(fp, "vertices");
  DumpVertices(fp);
  DumpComma(fp);
  DumpKey(fp, "lines");
  DumpLines(fp);
  DumpComma(fp);
  DumpKey(fp, "sides");
  DumpSides(fp);
  DumpComma(fp);
  DumpKey(fp, "segs");
  DumpSegs(fp);
  DumpComma(fp);
  DumpKey(fp, "sectors");
  DumpSectors(fp);
  DumpComma(fp);
  DumpKey(fp, "subsectors");
  DumpSubsectors(fp);
  DumpComma(fp);
  DumpKey(fp, "nodes");
  DumpNodes(fp);
  DumpComma(fp);
  DumpKey(fp, "chunks");
  DumpChunks(fp);
  DumpComma(fp);
  DumpKey(fp, "paths");
  DumpPaths(fp);
  DumpComma(fp);
  DumpKey(fp, "bleeds");
  DumpBleeds(fp);
  DumpComma(fp);
  fprintf(fp, "}");
}
