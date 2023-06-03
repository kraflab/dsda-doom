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

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "p_spec.h"

#include "dsda/id_list.h"
#include "dsda/map_format.h"
#include "dsda/line_special.h"

#include "bspinternal.h"

dsda_gl_rstate_t dsda_gl_rstate = {0};

static dboolean annotated = false;

static void InitGLNodes()
{
#if 0
  // Not ready for this yet
  if (use_gl_rstate.nodes)
  {
    // We already have GL nodes
    gl_rstate.vertexes = vertexes;
    gl_rstate.numvertexes = numvertexes;
    gl_rstate.segs = segs;
    gl_rstate.numsegs = numsegs;
    gl_rstate.subsectors = subsectors;
    gl_rstate.numsubsectors = numsubsectors;
    gl_rstate.map_subsectors = map_subsectors;
    gl_rstate.nodes = nodes;
    gl_rstate.numnodes = numnodes;
    return;
  }
#endif

  LoadSideGLNodes();
}

static unsigned int SpecialSectorFlagsDoom(int special)
{
  switch (special)
  {
    case 2:
    case 3:
    case 4:
    case 6:
    case 16:
    case 25:
    case 29:
    case 41:
    case 42:
    case 43:
    case 44:
    case 46:
    case 49:
    case 50:
    case 57:
    case 61:
    case 63:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 86:
    case 90:
    case 99:
    case 103:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 133:
    case 134:
    case 135:
    case 136:
    case 137:
    case 141:
    case 145:
    case 150:
    case 152:
    case 164:
    case 165:
    case 167:
    case 168:
    case 175:
    case 183:
    case 184:
    case 185:
    case 187:
    case 188:
    case 196:
    case 199:
    case 200:
    case 201:
    case 202:
    case 203:
    case 204:
    case 205:
    case 206:
      return SECF_DYNAMIC_CEILING;
    case 5:
    case 7:
    case 8:
    case 9:
    case 10:
    case 14:
    case 15:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 30:
    case 36:
    case 37:
    case 38:
    case 45:
    case 47:
    case 53:
    case 54:
    case 55:
    case 56:
    case 58:
    case 59:
    case 60:
    case 62:
    case 64:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 82:
    case 83:
    case 84:
    case 87:
    case 88:
    case 89:
    case 91:
    case 92:
    case 93:
    case 94:
    case 95:
    case 96:
    case 98:
    case 100:
    case 101:
    case 102:
    case 119:
    case 120:
    case 121:
    case 122:
    case 123:
    case 127:
    case 128:
    case 129:
    case 130:
    case 131:
    case 132:
    case 140:
    case 142:
    case 143:
    case 144:
    case 146:
    case 147:
    case 148:
    case 149:
    case 155:
    case 158:
    case 159:
    case 160:
    case 161:
    case 162:
    case 163:
    case 176:
    case 177:
    case 178:
    case 179:
    case 180:
    case 181:
    case 182:
    case 191:
    case 211:
    case 212:
    case 219:
    case 220:
    case 221:
    case 222:
    case 227:
    case 228:
    case 229:
    case 230:
    case 231:
    case 232:
    case 233:
    case 234:
    case 235:
    case 236:
    case 237:
    case 238:
    case 256:
    case 257:
    case 258:
    case 259:
      return SECF_DYNAMIC_FLOOR;
    case 40:
    case 151:
    case 166:
    case 186:
      return SECF_DYNAMIC_FLOOR | SECF_DYNAMIC_CEILING;
  }
  return 0;
}

static unsigned int SpecialSectorFlagsHeretic(int special)
{
  switch (special)
  {
  case 2:
  case 3:
  case 4:
  case 6:
  case 16:
  case 25:
  case 29:
  case 42:
  case 43:
  case 44:
  case 49:
  case 50:
  case 57:
  case 61:
  case 63:
  case 72:
  case 73:
  case 74:
  case 75:
  case 76:
  case 77:
  case 86:
  case 90:
  case 100:
  case 103:
    return SECF_DYNAMIC_CEILING;
  case 5:
  case 7:
  case 8:
  case 9:
  case 10:
  case 14:
  case 15:
  case 18:
  case 19:
  case 20:
  case 21:
  case 22:
  case 23:
  case 30:
  case 36:
  case 37:
  case 38:
  case 41:
  case 45:
  case 53:
  case 54:
  case 55:
  case 56:
  case 58:
  case 59:
  case 60:
  case 62:
  case 64:
  case 65:
  case 66:
  case 67:
  case 68:
  case 69:
  case 70:
  case 71:
  case 82:
  case 83:
  case 84:
  case 87:
  case 88:
  case 89:
  case 91:
  case 92:
  case 93:
  case 94:
  case 95:
  case 96:
  case 98:
  case 101:
  case 102:
  case 106:
  case 107:
    return SECF_DYNAMIC_FLOOR;
  case 40:
    return SECF_DYNAMIC_FLOOR | SECF_DYNAMIC_CEILING;
  }
  return 0;
}

static unsigned int SpecialTouchFlagsDoom(int special)
{
  switch (special)
  {
  case 1:
  case 26:
  case 27:
  case 28:
  case 31:
  case 32:
  case 33:
  case 34:
  case 117:
  case 118:
    return SECF_DYNAMIC_CEILING;
  }
  return 0;
}

static unsigned int SpecialTouchFlagsHeretic(int special)
{
  switch (special)
  {
  case 1:
  case 26:
  case 27:
  case 28:
  case 31:
  case 32:
  case 33:
  case 34:
    return SECF_DYNAMIC_CEILING;
  }
  return 0;
}

static unsigned int SpecialTouchFlagsHexen(int special)
{
  switch (special)
  {
  case 11:
  case 12:
  case 13:
    return SECF_DYNAMIC_CEILING;
  }
  return 0;
}

static unsigned int SpecialTouchFlagsZDoom(const line_t* line)
{
  switch (line->special)
  {
    case zl_door_close:
    case zl_door_open:
    case zl_door_raise:
    case zl_door_locked_raise:
    case zl_door_close_wait_open:
    case zl_door_wait_raise:
    case zl_door_wait_close:
      if (line->special_args[0] == 0)
        return SECF_DYNAMIC_CEILING;
      break;
    case zl_generic_door:
      if (line->special_args[2] & 128 ||
          line->special_args[0] == 0)
        return SECF_DYNAMIC_CEILING;
      break;
  }
  return 0;
}

static unsigned int SpecialTouchFlags(const line_t* line)
{
  if (map_format.zdoom)
    return SpecialTouchFlagsZDoom(line);
  if (map_format.hexen)
    return SpecialTouchFlagsHexen(line->special);
  if (heretic)
    return SpecialTouchFlagsHeretic(line->special);
  return SpecialTouchFlagsDoom(line->special);
}

static unsigned int SpecialSectorFlags(int special)
{
  if ((unsigned)special >= GenEnd)
    return 0;
  else if ((unsigned)special >= GenFloorBase)
    return SECF_DYNAMIC_FLOOR;
  else if ((unsigned)special >= GenCeilingBase)
    return SECF_DYNAMIC_CEILING;
  else if ((unsigned)special >= GenDoorBase)
    return SECF_DYNAMIC_CEILING;
  else if ((unsigned)special >= GenLockedBase)
    return 0;
  else if ((unsigned)special >= GenLiftBase)
    return SECF_DYNAMIC_FLOOR;
  else if ((unsigned)special >= GenStairsBase)
    return SECF_DYNAMIC_FLOOR;
  else if ((unsigned)special >= GenCrusherBase)
    return SECF_DYNAMIC_CEILING;
  if (heretic)
    return SpecialSectorFlagsHeretic(special);
  return SpecialSectorFlagsDoom(special);
}

static void AnnotateDynamicSectors(void)
{
  int i;

  for (i = 0; i < numsectors; ++i)
  {
    sector_t* sector = &sectors[i];
    const int* lid;

    if (!sector->tag)
      continue;

    if (map_format.zdoom || map_format.hexen)
    {
      // ACS means that all tagged sectors can potentially be dynamic,
      // so absent some sort of static analysis, we have to be pessimistic
      sector->flags |= SECF_DYNAMIC_CEILING | SECF_DYNAMIC_FLOOR;
      continue;
    }

    for (lid = dsda_FindLinesFromID(sector->tag); *lid != -1; ++lid)
    {
      line_t* line = &lines[*lid];

      if (!line->special)
        continue;

      sector->flags |= SpecialSectorFlags(line->special);
    }
  }

  for (i = 0; i < numlines; ++i)
  {
    line_t* line = &lines[i];
    sector_t* sector;

    if (!line->special)
      continue;

    if (map_format.hexen && line->activation & SPAC_USEBACK)
      sector = line->frontsector;
    else
      sector = line->backsector;

    if (!sector)
      continue;

    sector->flags |= SpecialTouchFlags(line);
  }
}

static void AnnotateInit(void)
{
  int i, j;
  subsector_t* sub;
  segmeta_t* segmeta;

  for (i = 0; i < gl_rstate.numsubsectors; ++i)
  {
    sub = &gl_rstate.subsectors[i];

    for (j = 0; j < sub->numlines; ++j)
    {
      segmeta = &dsda_gl_rstate.segmeta[sub->firstline + j];
      segmeta->subsector = sub;
    }
  }
}

// Determine if subsector is degenerate (has fewer than 3 distinct segments)
// FIXME: does ajbsp even generate these?
static dboolean SubsectorIsDegenerate(subsector_t* sub)
{
  int i;
  seg_t* first = &gl_rstate.segs[sub->firstline];
  dboolean have_second = false;
  dline_t dfirst = dgeom_DLineFromSeg(first);
  dline_t dsecond;

  for (i = 1; i < sub->numlines; ++i)
  {
    seg_t* seg = &gl_rstate.segs[sub->firstline + i];
    dline_t dline = dgeom_DLineFromSeg(seg);

    if (!dgeom_LinesCoincide(&dfirst, &dline, DGEOM_EPSILONR2))
    {
      if (!have_second)
      {
        dsecond = dline;
        have_second = true;
      }
      else if (!dgeom_LinesCoincide(&dsecond, &dline, DGEOM_EPSILONR2))
        return false;
    }
  }

  return true;
}

static void AnnotateDegenerateSubsectors(void)
{
  int i;

  // Now we can mark degenerate subsectors
  for (i = 0; i < gl_rstate.numsubsectors; ++i)
  {
    subsector_t* sub = &gl_rstate.subsectors[i];
    submeta_t* submeta = &dsda_gl_rstate.submeta[i];

    if (SubsectorIsDegenerate(sub))
      submeta->flags |= SUBF_DEGENERATE;
  }
}

// Detect segs involved in rendering hacks
static void AnnotateHackedSegs(void)
{
  int i;

  for (i = 0; i < gl_rstate.numsegs; ++i)
  {
    seg_t* seg = &gl_rstate.segs[i];
    segmeta_t* segmeta = &dsda_gl_rstate.segmeta[i];
    seg_t* partner = segmeta->partner;
    segmeta_t* psegmeta;
    dboolean mismatch_case, miniseg_case;

    if (!partner)
      continue;

    psegmeta = &dsda_gl_rstate.segmeta[partner - gl_rstate.segs];

    // Case 1: seg's linedef has the same sector on both sides, but partner
    // seg's sector doesn't match
    mismatch_case = seg->linedef &&
                    seg->linedef->frontsector == seg->linedef->backsector &&
                    psegmeta->subsector->sector != seg->linedef->backsector;

    // Case 2: minisegs separating different sectors.
    // Minisegs ordinarly divide subsectors in the same sector, but self-referencing
    // sector tricks can sometimes give rise to this case.
    miniseg_case = (!seg->linedef || !partner->linedef) &&
                   segmeta->subsector->sector != psegmeta->subsector->sector;

    if (mismatch_case || miniseg_case)
    {
      submeta_t* submeta =
          &dsda_gl_rstate.submeta[segmeta->subsector - gl_rstate.subsectors];
      submeta_t* psubmeta =
          &dsda_gl_rstate.submeta[psegmeta->subsector - gl_rstate.subsectors];

      segmeta->flags |= SEGF_HACKED;
      psegmeta->flags |= SEGF_HACKED;
      submeta->flags |= SUBF_HACKED;
      psubmeta->flags |= SUBF_HACKED;
    }
  }
}

//
// Public interface
//

void dsda_AnnotateBSP(void)
{
  if (!annotated)
  {
    InitGLNodes();
    AnnotateInit();
    AnnotateDynamicSectors();
    AnnotateDegenerateSubsectors();
    AnnotateHackedSegs();
    AnnotateChunks();
    AnnotateBleeds();
    AnnotateRender();
    annotated = true;
  }
}

void dsda_ClearBSP(dboolean samelevel)
{
  if (samelevel)
  {
    ResetChunks();
    ResetGLNodes();
  }
  else
  {
    ClearRender();
    ClearBleeds();
    ClearChunks();
    ClearGLNodes();

    annotated = false;
  }
}
