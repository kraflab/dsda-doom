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

//
// JSON dumping logic.  Extremely vital for debugging.
//

struct flaginfo
{
  unsigned long flag;
  const char* name;
};

static void DumpComma(FILE* fp)
{
  fprintf(fp, ",");
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

static void DumpFlagsInner(FILE* fp, const struct flaginfo* info,
                           unsigned long flags)
{
  dboolean comma = false;

  for (info; info->name; info++)
  {
    if (comma)
      DumpComma(fp);
    DumpKey(fp, info->name);
    fprintf(fp, (flags & info->flag) ? "true" : "false");
    comma = true;
  }
}

static void DumpFlags(FILE* fp, const struct flaginfo* info,
                      unsigned long flags)
{
  fprintf(fp, "{");
  DumpFlagsInner(fp, info, flags);
  fprintf(fp, "}");
}

static void DumpVertex(FILE* fp, const vertex_t* v)
{
  DumpPoint(fp, v->px, v->py);
}

static void DumpVertices(FILE* fp)
{
  int i;

  fprintf(fp, "{");
  for (i = 0; i < numvertexes; ++i)
  {
    DumpIdKey(fp, i);
    DumpVertex(fp, &vertexes[i]);
    if (i != numvertexes - 1)
      fprintf(fp, ",");
  }
  fprintf(fp, "}");
}

static void DumpLine(FILE* fp, const line_t* line)
{
  static const struct flaginfo linerflags[] =
  {
    {RF_ISOLATED, "isolated"},
    {0, NULL}
  };

  fprintf(fp, "{");
  DumpKeyId(fp, "v1", line->v1 - vertexes);
  DumpComma(fp);
  DumpKeyId(fp, "v2", line->v2 - vertexes);
  DumpComma(fp);
  if (line->sidenum[0] != NO_INDEX)
    DumpKeyId(fp, "front", line->sidenum[0]);
  else
    DumpKeyNull(fp, "front");
  DumpComma(fp);
  if (line->sidenum[1] != NO_INDEX)
    DumpKeyId(fp, "back", line->sidenum[1]);
  else
    DumpKeyNull(fp, "front");
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

static const struct flaginfo segflags[] = {{SEGF_FAKE, "fake"},
                                           {SEGF_ORPHAN, "orphan"},
                                           {SEGF_CYCLIC, "cyclic"},
                                           {0, NULL}};

static void DumpSeg(FILE* fp, const seg_t* seg)
{
  int i;

  fprintf(fp, "{");
  DumpKeyId(fp, "line", seg->linedef - lines);
  DumpComma(fp);
  DumpKeyId(fp, "side", seg->sidedef - sides);
  DumpComma(fp);
  DumpKeyId(fp, "subsector", seg->subsector - subsectors);
  DumpComma(fp);
  DumpKey(fp, "adjacent");
  fprintf(fp, "[");
  for (i = 0; i < seg->numadjacent; ++i)
  {
    DumpId(fp, adjacency[seg->firstadjacent + i]);
    if (i != seg->numadjacent - 1)
      DumpComma(fp);
  }
  fprintf(fp, "]");
  DumpComma(fp);
  DumpKeyId(fp, "sector", seg->subsector->sector - sectors);
  DumpComma(fp);
  DumpKeyId(fp, "v1", seg->v1 - vertexes);
  DumpComma(fp);
  DumpKeyId(fp, "v2", seg->v2 - vertexes);
  DumpComma(fp);
  DumpKey(fp, "flags");
  DumpFlags(fp, segflags, seg->flags);

  fprintf(fp, "}");
}

static void DumpSegs(FILE* fp)
{
  int i;
  fprintf(fp, "{");

  for (i = 0; i < numsegs; ++i)
  {
    DumpIdKey(fp, i);
    DumpSeg(fp, &segs[i]);
    if (i != numsegs - 1)
      fprintf(fp, ",");
  }

  fprintf(fp, "}");
}

static void DumpISeg(FILE* fp, const iseg_t* seg)
{
  int i;

  fprintf(fp, "{");
  DumpKeyId(fp, "subsector", seg->subsector - subsectors);
  DumpComma(fp);
  DumpKey(fp, "adjacent");
  fprintf(fp, "[");
  for (i = 0; i < seg->numadjacent; ++i)
  {
    DumpId(fp, adjacency[seg->firstadjacent + i]);
    if (i != seg->numadjacent - 1)
      DumpComma(fp);
  }
  fprintf(fp, "]");
  DumpComma(fp);
  DumpKeyId(fp, "sector", seg->subsector->sector - sectors);
  DumpComma(fp);
  DumpKeyId(fp, "node", seg->nodenum);
  DumpComma(fp);
  DumpPointPairInner(fp, seg->x1, seg->y1, seg->x2, seg->y2);
  DumpComma(fp);
  DumpKey(fp, "flags");
  DumpFlags(fp, segflags, seg->flags);

  fprintf(fp, "}");
}

static void DumpISegs(FILE* fp)
{
  int i;
  dboolean comma = false;
  fprintf(fp, "{");

  for (i = 0; i < numisegs; ++i)
  {
    if (comma)
      DumpComma(fp);
    DumpIdKey(fp, i);
    DumpISeg(fp, &isegs[i]);
    comma = true;
  }

  fprintf(fp, "}");
}

static void DumpSector(FILE* fp, const sector_t* sector)
{
  static const struct flaginfo sectorflags[] = {
    {SECTOR_IS_CLOSED, "closed"},
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
static void DumpSubsector(FILE* fp, const subsector_t* subsector)
{
  static const struct flaginfo subsectorflags[] = {
    {SUBF_DEGENERATE, "degenerate"},
    {SUBF_FAKE, "fake"},
    {0, NULL},
  };
  int i = 0;

  fprintf(fp, "{");
  DumpKeyId(fp, "sector", subsector->sector - sectors);
  DumpComma(fp);
  DumpKey(fp, "flags");
  DumpFlags(fp, subsectorflags, subsector->flags);
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
  DumpKey(fp, "isegs");
  fprintf(fp, "[");
  for (i = 0; i < subsector->numisegs; ++i)
  {
    DumpId(fp, subsector->firstiseg + i);
    if (i != subsector->numisegs - 1)
      DumpComma(fp);
  }
  fprintf(fp, "]");
  fprintf(fp, "}");
}

static void DumpSubsectors(FILE* fp)
{
  int i;
  fprintf(fp, "{");

  for (i = 0; i < numsubsectors; ++i)
  {
    DumpIdKey(fp, i);
    DumpSubsector(fp, &subsectors[i]);
    if (i != numsubsectors - 1)
      fprintf(fp, ",");
  }

  fprintf(fp, "}");
}

static void DumpNode(FILE* fp, const node_t* node, const fixed_t* bbox)
{
  int i;
  iseg_t* cur;
  dline_t div = dgeom_DLineFromNode(node);

  dgeom_ClipLineByBox(&div, bbox, &div);

  if (node != &nodes[numnodes - 1])
    DumpComma(fp);

  DumpIdKey(fp, node - nodes);
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
    int nodenum = node->children[i];
    DumpKey(fp, i ? "left" : "right");
    DumpBBox(fp, node->bbox[i]);
    if (i == 0)
      DumpComma(fp);
  }
  fprintf(fp, "}");
  DumpComma(fp);
  DumpKey(fp, "isegs");
  fprintf(fp, "[");
  for (cur = dsda_ISeg(node->isegs); cur; cur = dsda_ISeg(cur->next))
  {
    DumpId(fp, cur - isegs);
    if (dsda_ISeg(cur->next))
      DumpComma(fp);
  }
  fprintf(fp, "]");
  fprintf(fp, "}");

  for (i = 0; i < 2; ++i)
  {
    if (!(node->children[i] & NF_SUBSECTOR))
      DumpNode(fp, &nodes[node->children[i]], node->bbox[i]);
  }
}

static void DumpNodes(FILE* fp)
{
  fprintf(fp, "{");
  if (numnodes)
    DumpNode(fp, &nodes[numnodes - 1], mapbox);
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
  DumpKey(fp, "isegs");
  DumpISegs(fp);
  DumpComma(fp);
  DumpKey(fp, "sectors");
  DumpSectors(fp);
  DumpComma(fp);
  DumpKey(fp, "subsectors");
  DumpSubsectors(fp);
  DumpComma(fp);
  DumpKey(fp, "nodes");
  DumpNodes(fp);
  fprintf(fp, "}");
}
