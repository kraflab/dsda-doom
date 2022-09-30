/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze, Andrey Budko
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
 *      Demo stuff
 *
 *---------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef _WIN32
#include <io.h>
#include <process.h>
#else
#include <unistd.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "doomdef.h"
#include "doomtype.h"
#include "doomstat.h"
#include "r_demo.h"
#include "r_fps.h"
#include "lprintf.h"
#include "i_system.h"
#include "i_video.h"
#include "m_misc.h"
#include "m_argv.h"
#include "w_wad.h"
#include "d_main.h"
#include "d_deh.h"
#include "g_game.h"
#include "p_map.h"
#include "hu_stuff.h"
#include "g_overflow.h"
#include "w_wad.h"
#include "z_zone.h"
#include "e6y.h"

#include "dsda/args.h"
#include "dsda/demo.h"
#include "dsda/features.h"
#include "dsda/playback.h"
#include "dsda/utility.h"

int LoadDemo(const char *name, const byte **buffer, int *length)
{
  char basename[9];
  char *filename = NULL;
  int num = -1;
  int len = 0;
  const byte *buf = NULL;

  ExtractFileBase(name, basename);
  basename[8] = 0;

  // check ns_demos namespace first, then ns_global
  num = W_CheckNumForName2(basename, ns_demos);
  if (num == LUMP_NOT_FOUND)
  {
    num = W_CheckNumForName(basename);
  }

  if (num == LUMP_NOT_FOUND)
  {
    // Allow for demos not loaded as lumps
    static byte *sbuf = NULL;
    filename = I_FindFile(name, ".lmp");
    if (filename)
    {
      if (sbuf)
      {
        Z_Free(sbuf);
        sbuf = NULL;
      }

      len = M_ReadFile(filename, &sbuf);
      buf = (const byte *)sbuf;
      Z_Free(filename);
    }
  }
  else
  {
    buf = W_LumpByNum(num);
    len = W_LumpLength(num);
  }

  if (len < 0)
    len = 0;

  if (len > 0)
  {
    if (buffer)
      *buffer = buf;
    if (length)
      *length = len;
  }

  return (len > 0);
}

//
// Smooth playing stuff
//

int demo_smoothturns;
int demo_smoothturnsfactor = 6;

static int smooth_playing_turns[SMOOTH_PLAYING_MAXFACTOR];
static int_64_t smooth_playing_sum;
static int smooth_playing_index;
static angle_t smooth_playing_angle;

void R_SmoothPlaying_Reset(player_t *player)
{
  if (demo_smoothturns && demoplayback && !demorecording)
  {
    if (!player)
      player = &players[displayplayer];

    if (player==&players[displayplayer])
    {
      if (player->mo)
      {
        smooth_playing_angle = player->mo->angle;
        memset(smooth_playing_turns, 0, sizeof(smooth_playing_turns[0]) * SMOOTH_PLAYING_MAXFACTOR);
        smooth_playing_sum = 0;
        smooth_playing_index = 0;
      }
    }
  }
}

void R_SmoothPlaying_Add(int delta)
{
  if (demo_smoothturns && demoplayback && !demorecording)
  {
    smooth_playing_sum -= smooth_playing_turns[smooth_playing_index];
    smooth_playing_turns[smooth_playing_index] = delta;
    smooth_playing_index = (smooth_playing_index + 1)%(demo_smoothturnsfactor);
    smooth_playing_sum += delta;
    smooth_playing_angle += (int)(smooth_playing_sum/(demo_smoothturnsfactor));
  }
}

angle_t R_SmoothPlaying_Get(player_t *player)
{
  if (demo_smoothturns && demoplayback && !demorecording && player == &players[displayplayer])
    return smooth_playing_angle;
  else
    return player->mo->angle;
}

void R_ResetAfterTeleport(player_t *player)
{
  R_ResetViewInterpolation();
  R_SmoothPlaying_Reset(player);
}

//
// DemoEx stuff
//

typedef struct
{
  wadinfo_t header;
  filelump_t *lumps;
  char* data;
  int datasize;
} wadtbl_t;

#define PWAD_SIGNATURE "PWAD"

#define DEMOEX_PORTNAME_LUMPNAME "PORTNAME"
#define DEMOEX_PARAMS_LUMPNAME "CMDLINE"
#define DEMOEX_FEATURE_LUMPNAME "FEATURES"

void W_InitPWADTable(wadtbl_t *wadtbl)
{
  //init header signature and lookup table offset and size
  memcpy(wadtbl->header.identification, PWAD_SIGNATURE, 4);
  wadtbl->header.infotableofs = sizeof(wadtbl->header);
  wadtbl->header.numlumps = 0;

  //clear PWAD lookup table
  wadtbl->lumps = NULL;

  //clear PWAD data
  wadtbl->data = NULL;
  wadtbl->datasize = 0;
}

void W_FreePWADTable(wadtbl_t *wadtbl)
{
  //clear PWAD lookup table
  Z_Free(wadtbl->lumps);

  //clear PWAD data
  Z_Free(wadtbl->data);
}

void W_AddLump(wadtbl_t *wadtbl, const char *name, const byte* data, size_t size)
{
  int lumpnum;

  if (!wadtbl || (name && strlen(name) > 8))
  {
    I_Error("W_AddLump: wrong parameters.");
    return;
  }

  lumpnum = wadtbl->header.numlumps;

  if (name)
  {
    wadtbl->lumps = Z_Realloc(wadtbl->lumps, (lumpnum + 1) * sizeof(wadtbl->lumps[0]));

    memcpy(wadtbl->lumps[lumpnum].name, name, 8);
    wadtbl->lumps[lumpnum].size = size;
    wadtbl->lumps[lumpnum].filepos = wadtbl->header.infotableofs;

    wadtbl->header.numlumps++;
  }

  if (data && size > 0)
  {
    wadtbl->data = Z_Realloc(wadtbl->data, wadtbl->datasize + size);

    memcpy(wadtbl->data + wadtbl->datasize, data, size);
    wadtbl->datasize += size;

    wadtbl->header.infotableofs += size;
  }
}

static const filelump_t* R_DemoEx_LumpForName(const char* name, const wadinfo_t *header)
{
  int i;
  const filelump_t* lump_info;
  const byte* buffer;

  buffer = (const byte*) header;
  lump_info = (const filelump_t*)(buffer + header->infotableofs);
  for (i = 0; i < header->numlumps; i++, lump_info++)
    if (!strncmp(lump_info->name, name, 8))
      return lump_info;

  return NULL;
}

static char* R_DemoEx_LumpAsString(const char* name, const wadinfo_t *header)
{
  char *str;
  const char *lump_data;
  const byte* buffer;
  const filelump_t* lump_info;

  lump_info = R_DemoEx_LumpForName(name, header);
  if (!lump_info || !lump_info->size)
    return NULL;

  str = Z_Calloc(lump_info->size + 1, 1);

  buffer = (const byte*) header;
  lump_data = buffer + lump_info->filepos;
  strncpy(str, lump_data, lump_info->size);

  return str;
}

static void R_DemoEx_GetParams(const wadinfo_t *header)
{
  char *str;
  char **params;
  int i, p, paramscount;

  str = R_DemoEx_LumpAsString(DEMOEX_PARAMS_LUMPNAME, header);
  if (!str)
    return;

  M_ParseCmdLine(str, NULL, NULL, &paramscount, &i);

  params = Z_Malloc(paramscount * sizeof(char*) + i * sizeof(char) + 1);
  if (params)
  {
    struct {
      const char *param;
      wad_source_t source;
    } files[] = {
      {"-iwad" , source_iwad},
      {"-file" , source_pwad},
      {"-deh"  , source_deh},
      {NULL}
    };

    M_ParseCmdLine(str, params, ((char*)params) + sizeof(char*) * paramscount, &paramscount, &i);

    if (!dsda_Flag(dsda_arg_iwad) && !dsda_Flag(dsda_arg_file))
    {
      i = 0;
      while (files[i].param)
      {
        p = M_CheckParmEx(files[i].param, params, paramscount);
        if (p >= 0)
        {
          while (++p != paramscount && *params[p] != '-')
          {
            char *filename;
            //something is wrong here
            filename = I_FindFile(params[p], ".wad");
            if (!filename)
            {
              filename = Z_Strdup(params[p]);
            }

            if (files[i].source == source_iwad)
              AddIWAD(filename);
            else
              D_AddFile(filename, files[i].source);

            Z_Free(filename);
          }
        }
        i++;
      }
    }

    if (!dsda_Arg(dsda_arg_complevel)->found)
    {
      p = M_CheckParmEx("-complevel", params, paramscount);
      if (p >= 0 && p < (int)paramscount - 1)
      {
        dsda_UpdateIntArg(dsda_arg_complevel, params[p + 1]);
      }
    }

    //for recording or playback using "single-player coop" mode
    if (!dsda_Flag(dsda_arg_solo_net))
    {
      p = M_CheckParmEx("-solo-net", params, paramscount);
      if (p >= 0)
      {
        dsda_UpdateFlag(dsda_arg_solo_net, true);
      }
    }

    //for recording or playback using "coop in single-player" mode
    if (!dsda_Flag(dsda_arg_coop_spawns))
    {
      p = M_CheckParmEx("-coop_spawns", params, paramscount);
      if (p >= 0)
      {
        dsda_UpdateFlag(dsda_arg_coop_spawns, true);
      }
    }

    if (!dsda_Flag(dsda_arg_emulate))
    {
      p = M_CheckParmEx("-emulate", params, paramscount);
      if (p >= 0 && p < (int)paramscount - 1)
      {
        dsda_UpdateStringArg(dsda_arg_emulate, params[p + 1]);
      }
    }

    // for doom 1.2
    if (!dsda_Flag(dsda_arg_respawn))
    {
      p = M_CheckParmEx("-respawn", params, paramscount);
      if (p >= 0)
      {
        dsda_UpdateFlag(dsda_arg_respawn, true);
      }
    }

    // for doom 1.2
    if (!dsda_Flag(dsda_arg_fast))
    {
      p = M_CheckParmEx("-fast", params, paramscount);
      if (p >= 0)
      {
        dsda_UpdateFlag(dsda_arg_fast, true);
      }
    }

    // for doom 1.2
    if (!dsda_Flag(dsda_arg_nomonsters))
    {
      p = M_CheckParmEx("-nomonsters", params, paramscount);
      if (p >= 0)
      {
        dsda_UpdateFlag(dsda_arg_nomonsters, true);
      }
    }

    p = M_CheckParmEx("-spechit", params, paramscount);
    if (p >= 0 && p < (int)paramscount - 1)
    {
      spechit_baseaddr = atoi(params[p + 1]);
    }

    //overflows
    {
      overrun_list_t overflow;
      for (overflow = 0; overflow < OVERFLOW_MAX; overflow++)
      {
        int value;
        char *pstr, *mask;

        mask = Z_Malloc(strlen(overflow_cfgname[overflow]) + 16);
        if (mask)
        {
          sprintf(mask, "-set %s", overflow_cfgname[overflow]);
          pstr = strstr(str, mask);

          if (pstr)
          {
            strcat(mask, " = %d");
            if (sscanf(pstr, mask, &value) == 1)
            {
              overflows[overflow].footer = true;
              overflows[overflow].footer_emulate = value;
            }
          }
          Z_Free(mask);
        }
      }
    }

    Z_Free(params);
  }

  Z_Free(str);
}

static void R_DemoEx_AddParams(wadtbl_t *wadtbl)
{
  dsda_arg_t* arg;
  size_t i;
  int p;
  char buf[200];

  const char* filename_p;

  dsda_string_t files;
  dsda_string_t iwad;
  dsda_string_t pwads;
  dsda_string_t dehs;

  dsda_InitString(&files, NULL);
  dsda_InitString(&iwad, NULL);
  dsda_InitString(&pwads, NULL);
  dsda_InitString(&dehs, NULL);

  //iwad and pwads
  for (i = 0; i < numwadfiles; i++)
  {
    const char *fileext_p;
    dsda_string_t *item = NULL;

    filename_p = PathFindFileName(wadfiles[i].name);
    fileext_p = filename_p + strlen(filename_p) - 1;
    while (fileext_p != filename_p && *(fileext_p - 1) != '.')
      fileext_p--;
    if (fileext_p == filename_p)
      continue;

    if (wadfiles[i].src == source_iwad && !iwad.string && !strcasecmp(fileext_p, "wad"))
      item = &iwad;

    if (wadfiles[i].src == source_pwad && !strcasecmp(fileext_p, "wad"))
      item = &pwads;

    if (item)
    {
      dsda_StringCat(item, "\"");
      dsda_StringCat(item, filename_p);
      dsda_StringCat(item, "\" ");
    }
  }

  //dehs
  arg = dsda_Arg(dsda_arg_deh);
  if (arg->found)
  {
    for (i = 0; i < arg->count; ++i)
    {
      char *file = NULL;
      if ((file = I_FindFile(arg->value.v_string_array[i], ".bex")) ||
          (file = I_FindFile(arg->value.v_string_array[i], ".deh")))
      {
        filename_p = PathFindFileName(file);
        dsda_StringCat(&dehs, "\"");
        dsda_StringCat(&dehs, filename_p);
        dsda_StringCat(&dehs, "\" ");
        Z_Free(file);
      }
    }
  }

  if (iwad.string)
  {
    dsda_StringCat(&files, "-iwad ");
    dsda_StringCat(&files, iwad.string);
  }

  if (pwads.string)
  {
    dsda_StringCat(&files, "-file ");
    dsda_StringCat(&files, pwads.string);
  }

  if (dehs.string)
  {
    dsda_StringCat(&files, "-deh ");
    dsda_StringCat(&files, dehs.string);
  }

  //add complevel for formats which do not have it in header
  if (demo_compatibility)
  {
    sprintf(buf, "-complevel %d ", compatibility_level);
    dsda_StringCat(&files, buf);
  }

  //for recording or playback using "single-player coop" mode
  if (dsda_Flag(dsda_arg_solo_net))
  {
    sprintf(buf, "-solo-net ");
    dsda_StringCat(&files, buf);
  }

  //for recording or playback using "coop in single-player" mode
  if (dsda_Flag(dsda_arg_coop_spawns))
  {
    sprintf(buf, "-coop_spawns ");
    dsda_StringCat(&files, buf);
  }

  arg = dsda_Arg(dsda_arg_emulate);
  if (arg->found)
  {
    sprintf(buf, "-emulate %s", arg->value.v_string);
    dsda_StringCat(&files, buf);
  }

  // doom 1.2 does not store these params in header
  if (compatibility_level == doom_12_compatibility)
  {
    if (dsda_Flag(dsda_arg_respawn))
    {
      sprintf(buf, "-respawn ");
      dsda_StringCat(&files, buf);
    }
    if (dsda_Flag(dsda_arg_fast))
    {
      sprintf(buf, "-fast ");
      dsda_StringCat(&files, buf);
    }
    if (dsda_Flag(dsda_arg_nomonsters))
    {
      sprintf(buf, "-nomonsters ");
      dsda_StringCat(&files, buf);
    }
  }

  if (spechit_baseaddr != 0 && spechit_baseaddr != DEFAULT_SPECHIT_MAGIC)
  {
    sprintf(buf, "-spechit %d ", spechit_baseaddr);
    dsda_StringCat(&files, buf);
  }

  //overflows
  {
    overrun_list_t overflow;
    for (overflow = 0; overflow < OVERFLOW_MAX; overflow++)
    {
      if (overflows[overflow].shit_happens)
      {
        sprintf(buf, "-set %s=%d ", overflow_cfgname[overflow], overflows[overflow].emulate);
        dsda_StringCat(&files, buf);
      }
    }
  }

  if (files.string)
  {
    W_AddLump(wadtbl, DEMOEX_PARAMS_LUMPNAME, (const byte*) files.string, strlen(files.string));
  }

  dsda_FreeString(&files);
  dsda_FreeString(&iwad);
  dsda_FreeString(&pwads);
  dsda_FreeString(&dehs);
}

static void R_DemoEx_AddFeatures(wadtbl_t *wadtbl)
{
  dsda_cksum_t cksum;
  char* description;
  byte* buffer;
  size_t buffer_length;
  uint_64_t features;

  dsda_GetDemoRecordingCheckSum(&cksum);
  description = dsda_DescribeFeatures();
  features = dsda_UsedFeatures();

  // 18 for 64 bits in hex + \n + \0 + \- + extra space :^)
  buffer_length = strlen(cksum.string) + strlen(description) + 24;
  buffer = Z_Calloc(buffer_length, 1);

  snprintf(buffer, buffer_length, "%s\n0x%016llx-%s", description, features, cksum.string);

  W_AddLump(wadtbl, DEMOEX_FEATURE_LUMPNAME, buffer, buffer_length);

  Z_Free(buffer);
  Z_Free(description);
}

byte* G_GetDemoFooter(const char *filename, byte **footer, size_t *size)
{
  byte* result = NULL;

  FILE *hfile;
  byte *buffer = NULL;
  const byte* p;
  size_t file_size;

  hfile = fopen(filename, "rb");

  if (!hfile)
    return result;

  //get demo size in bytes
  fseek(hfile, 0, SEEK_END);
  file_size = ftell(hfile);
  fseek(hfile, 0, SEEK_SET);

  buffer = Z_Malloc(file_size);

  if (fread(buffer, file_size, 1, hfile) == 1)
  {
    p = dsda_DemoMarkerPosition(buffer, file_size);

    if (p)
    {
      //skip DEMOMARKER
      p++;

      //seach for the "PWAD" signature after ENDDEMOMARKER
      while (p - buffer + sizeof(wadinfo_t) < file_size)
      {
        if (!memcmp(p, PWAD_SIGNATURE, strlen(PWAD_SIGNATURE)))
        {
          //got it!
          //the demo has an additional information itself
          int demoex_size = file_size - (p - buffer);

          result = buffer;

          if (footer)
          {
            *footer = buffer + (p - buffer);
          }

          if (size)
          {
            *size = demoex_size;
          }

          break;
        }
        p++;
      }
    }
  }

  fclose(hfile);

  return result;
}

wadinfo_t *G_ReadDemoFooterHeader(char *buffer, size_t size)
{
  int i;
  unsigned int length;
  wadinfo_t *header;
  const filelump_t *fileinfo;

  if (buffer && size > sizeof(*header))
  {
    header = (wadinfo_t*)buffer;
    if (strncmp(header->identification, "IWAD", 4) == 0 ||
        strncmp(header->identification, "PWAD", 4) == 0)
    {
      header->numlumps = LittleLong(header->numlumps);
      header->infotableofs = LittleLong(header->infotableofs);
      length = header->numlumps * sizeof(filelump_t);

      if (header->infotableofs + length <= size)
      {
        fileinfo = (const filelump_t*)(buffer + header->infotableofs);
        for (i = 0; i < header->numlumps; i++, fileinfo++)
        {
          if (fileinfo->filepos < 0 ||
              fileinfo->filepos > header->infotableofs ||
              fileinfo->filepos + fileinfo->size > header->infotableofs)
          {
            break;
          }
        }
        if (i == header->numlumps)
          return header;
      }
    }
  }

  return NULL;
}

static void G_ReadDemoFooter(const char *filename)
{
  byte *buffer = NULL;
  byte *demoex_p = NULL;
  size_t size;

  buffer = G_GetDemoFooter(filename, &demoex_p, &size);

  if (buffer)
  {
    wadinfo_t *header;

    header = G_ReadDemoFooterHeader(demoex_p, size);

    if (!header)
    {
      lprintf(LO_ERROR, "G_ReadDemoFooter: demo footer is corrupted\n");
    }
    else
    {
      // get needed wads and dehs
      // restore all critical params like -spechit x
      R_DemoEx_GetParams(header);
    }

    Z_Free(buffer);
  }
}

static void R_DemoEx_NewLine(wadtbl_t *wadtbl)
{
  const char* const separator = "\n";

  W_AddLump(wadtbl, NULL, (const byte*) separator, strlen(separator));
}

void G_WriteDemoFooter(void)
{
  wadtbl_t demoex;

  W_InitPWADTable(&demoex);

  // separators for eye-friendly looking
  R_DemoEx_NewLine(&demoex);
  R_DemoEx_NewLine(&demoex);

  // R_DemoEx_AddFeatures(&demoex);
  // R_DemoEx_NewLine(&demoex);

  //process port name
  W_AddLump(&demoex, DEMOEX_PORTNAME_LUMPNAME,
    (const byte*)(PACKAGE_NAME" "PACKAGE_VERSION), strlen(PACKAGE_NAME" "PACKAGE_VERSION));
  R_DemoEx_NewLine(&demoex);

  //process iwad, pwads, dehs and critical for demos params like -spechit, etc
  R_DemoEx_AddParams(&demoex);
  R_DemoEx_NewLine(&demoex);

  //write pwad header, all data and lookup table to the end of a demo
  dsda_WriteToDemo(&demoex.header, sizeof(demoex.header));
  dsda_WriteToDemo(demoex.data, demoex.datasize);
  dsda_WriteToDemo(demoex.lumps, demoex.header.numlumps * sizeof(demoex.lumps[0]));

  W_FreePWADTable(&demoex);
}

void G_CheckDemoEx(void)
{
  const char* playback_name;

  playback_name = dsda_PlaybackName();

  if (playback_name)
  {
    char *demoname, *filename;

    filename = Z_Malloc(strlen(playback_name) + 16);
    strcpy(filename, playback_name);
    AddDefaultExtension(filename, ".lmp");

    demoname = I_FindFile(filename, NULL);
    if (demoname)
    {
      G_ReadDemoFooter(demoname);
      Z_Free(demoname);
    }

    Z_Free(filename);
  }
}
