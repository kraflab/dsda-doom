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
//	DSDA/ajbsp glue interface
//

#include <cstring>
#include <cstdio>
#include <vector>

#include "bsp.h"
#include "wad.h"
#include "glue.h"

extern "C"
{
#include "lprintf.h"
#include "z_zone.h"
}

class DsdaInfo: public buildinfo_t
{
public:
  DsdaInfo(void): buildinfo_t()
  {
    this->fast = true;
    this->gl_nodes = true;
    this->do_blockmap = false;
    this->do_reject = false;
    this->force_v5 = true;
  }

protected:
  void Print(int level, const char *msg, ...)
  {
    va_list ap;
    va_start(ap, msg);
    vlprintf(LO_INFO, msg, ap);
    va_end(ap);
  }

  void Debug(const char *msg, ...)
  {
    va_list ap;
    va_start(ap, msg);
    vlprintf(LO_DEBUG, msg, ap);
    va_end(ap);
  }

  void ShowMap(const char *name)
  {
    lprintf(LO_INFO, "ajbsp: building GL nodes for %s\n", name);
  }

  void FatalError(const char *fmt, ...)
  {
    va_list ap;
    va_start(ap, fmt);
    I_VError(fmt, ap);
    va_end(ap);
  }
};

static void FillLump(ajbsp::Wad_file* wad, int level_index, const char* name,
                     void*& data, unsigned int& size)
{
  ajbsp::Lump_c* lump;

  lump = wad->GetLump(wad->LevelLookupLump(level_index, name));

  size = lump->Length();
  data = Z_Malloc(size);

  lump->Seek(0);
  lump->Read(data, size);
}

static void FillNodes(const char* outwad, int level_index, dsda_ajbsp_glnodes_t* nodes)
{
  ajbsp::Wad_file* wad = ajbsp::Wad_file::Open(outwad, 'r');

  FillLump(wad, level_index, "GL_VERT", nodes->vertexes, nodes->vsize);
  FillLump(wad, level_index, "GL_SEGS", nodes->segs, nodes->ssize);
  FillLump(wad, level_index, "GL_SSECT", nodes->subsectors, nodes->sssize);
  FillLump(wad, level_index, "GL_NODES", nodes->nodes, nodes->nsize);

  delete wad;
}

static int FindOutputLevel(const char* map)
{
  int index;

  for (index = 0; index < ajbsp::LevelsInOutputWad(); ++index)
  {
    if (!strcmp(ajbsp::GetOutputLevelName(index), map))
    {
      // Already built
      return index;
    }
  }

  return -1;
}

extern "C" bool dsda_ajbsp_LoadGLNodes(const char* inwad, const char* outwad,
                                       const char* map,
                                       dsda_ajbsp_glnodes_t* nodes)
{
  DsdaInfo info;
  build_result_e res = BUILD_Cancelled;
  int index;

  ajbsp::SetInfo(&info);

  ajbsp::OpenOutputWad(outwad);

  index = FindOutputLevel(map);
  if (index < 0)
  {
    ajbsp::OpenInputWad(inwad);
    for (index = 0; index < ajbsp::LevelsInWad(); ++index)
    {
      if (!strcmp(ajbsp::GetLevelName(index), map))
        break;
    }
    res = ajbsp::BuildGLNodes(index);
    if (res == BUILD_OK)
      index = FindOutputLevel(map);
  }
  else
    res = BUILD_OK;

  ajbsp::CloseWad();
  ajbsp::SetInfo(NULL);

  if (res == BUILD_OK)
  {
    ::FillNodes(outwad, index, nodes);
    return true;
  }

  return false;
}
