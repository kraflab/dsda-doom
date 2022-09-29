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
#include "e6y.h"

#include "dsda/args.h"
#include "dsda/demo.h"
#include "dsda/playback.h"

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

#define PWAD_SIGNATURE "PWAD"

#define DEMOEX_PORTNAME_LUMPNAME "PORTNAME"
#define DEMOEX_PARAMS_LUMPNAME "CMDLINE"

// demo ex
char demoex_filename[PATH_MAX];

int AddString(char **str, const char *val);

static void R_DemoEx_AddParams(wadtbl_t *wadtbl);
static void R_DemoEx_GetParams(const byte *pwad_p, waddata_t *waddata);

static int G_ReadDemoFooter(const char *filename);

int AddString(char **str, const char *val)
{
  int size = 0;

  if (!str || !val)
    return 0;

  if (*str)
  {
    size = strlen(*str) + strlen(val) + 1;
    *str = Z_Realloc(*str, size);
    strcat(*str, val);
  }
  else
  {
    size = strlen(val) + 1;
    *str = Z_Malloc(size);
    strcpy(*str, val);
  }

  return size;
}

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

static void R_DemoEx_GetParams(const byte *pwad_p, waddata_t *waddata)
{
  int lump;
  size_t size;
  char *str;
  const char *data;
  char **params;
  int i, p, paramscount;

  lump = W_CheckNumForName(DEMOEX_PARAMS_LUMPNAME);
  if (lump == LUMP_NOT_FOUND)
    return;

  size = W_LumpLength(lump);
  if (size <= 0)
    return;

  str = Z_Calloc(size + 1, 1);
  if (!str)
    return;

  data = W_LumpByNum(lump);
  strncpy(str, data, size);

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
            WadDataAddItem(waddata, filename, files[i].source, 0);
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
  const char* fileext_p;

  char *files = NULL;
  char *iwad  = NULL;
  char *pwads = NULL;
  char *dehs  = NULL;
  char **item;

  //iwad and pwads
  for (i = 0; i < numwadfiles; i++)
  {
    filename_p = PathFindFileName(wadfiles[i].name);
    fileext_p = filename_p + strlen(filename_p) - 1;
    while (fileext_p != filename_p && *(fileext_p - 1) != '.')
      fileext_p--;
    if (fileext_p == filename_p)
      continue;

    item = NULL;

    if (wadfiles[i].src == source_iwad && !iwad && !strcasecmp(fileext_p, "wad"))
      item = &iwad;

    if (wadfiles[i].src == source_pwad && !strcasecmp(fileext_p, "wad"))
      item = &pwads;

    if (item)
    {
      AddString(item, "\"");
      AddString(item, filename_p);
      AddString(item, "\" ");
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
        AddString(&dehs, "\"");
        AddString(&dehs, filename_p);
        AddString(&dehs, "\" ");
        Z_Free(file);
      }
    }
  }

  if (iwad)
  {
    AddString(&files, "-iwad ");
    AddString(&files, iwad);
  }

  if (pwads)
  {
    AddString(&files, "-file ");
    AddString(&files, pwads);
  }

  if (dehs)
  {
    AddString(&files, "-deh ");
    AddString(&files, dehs);
  }

  //add complevel for formats which do not have it in header
  if (demo_compatibility)
  {
    sprintf(buf, "-complevel %d ", compatibility_level);
    AddString(&files, buf);
  }

  //for recording or playback using "single-player coop" mode
  if (dsda_Flag(dsda_arg_solo_net))
  {
    sprintf(buf, "-solo-net ");
    AddString(&files, buf);
  }

  //for recording or playback using "coop in single-player" mode
  if (dsda_Flag(dsda_arg_coop_spawns))
  {
    sprintf(buf, "-coop_spawns ");
    AddString(&files, buf);
  }

  arg = dsda_Arg(dsda_arg_emulate);
  if (arg->found)
  {
    sprintf(buf, "-emulate %s", arg->value.v_string);
    AddString(&files, buf);
  }

  // doom 1.2 does not store these params in header
  if (compatibility_level == doom_12_compatibility)
  {
    if (dsda_Flag(dsda_arg_respawn))
    {
      sprintf(buf, "-respawn ");
      AddString(&files, buf);
    }
    if (dsda_Flag(dsda_arg_fast))
    {
      sprintf(buf, "-fast ");
      AddString(&files, buf);
    }
    if (dsda_Flag(dsda_arg_nomonsters))
    {
      sprintf(buf, "-nomonsters ");
      AddString(&files, buf);
    }
  }

  if (spechit_baseaddr != 0 && spechit_baseaddr != DEFAULT_SPECHIT_MAGIC)
  {
    sprintf(buf, "-spechit %d ", spechit_baseaddr);
    AddString(&files, buf);
  }

  //overflows
  {
    overrun_list_t overflow;
    for (overflow = 0; overflow < OVERFLOW_MAX; overflow++)
    {
      if (overflows[overflow].shit_happens)
      {
        sprintf(buf, "-set %s=%d ", overflow_cfgname[overflow], overflows[overflow].emulate);
        AddString(&files, buf);
      }
    }
  }

  if (files)
  {
    W_AddLump(wadtbl, DEMOEX_PARAMS_LUMPNAME, (const byte*)files, strlen(files));
  }
}

void I_DemoExShutdown(void)
{
  W_ReleaseAllWads();

  if (demoex_filename[0])
  {
    lprintf(LO_DEBUG, "I_DemoExShutdown: removing %s\n", demoex_filename);
    if (unlink(demoex_filename) != 0)
    {
      lprintf(LO_DEBUG, "I_DemoExShutdown: %s\n", strerror(errno));
    }

    // remove original temp file too
    if (strlen(demoex_filename) > 4) // tempfile.wad -> tempfile
    {
      demoex_filename[strlen(demoex_filename) - 3] = 0;
      if (unlink(demoex_filename) != 0)
      {
        lprintf(LO_DEBUG, "I_DemoExShutdown: %s\n", strerror(errno));
      }
    }
  }
}

byte* G_GetDemoFooter(const char *filename, const byte **footer, size_t *size)
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
            *footer = p;
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

void G_SetDemoFooter(const char *filename, wadtbl_t *wadtbl)
{
  FILE *hfile;
  byte *buffer = NULL;
  const byte *demoex_p = NULL;
  size_t size;

  buffer = G_GetDemoFooter(filename, &demoex_p, &size);
  if (buffer)
  {
    char newfilename[PATH_MAX];

    strncpy(newfilename, filename, sizeof(newfilename) - 5);
    newfilename[sizeof(newfilename) - 5] = 0;
    strcat(newfilename, ".out");

    hfile = fopen(newfilename, "wb");
    if (hfile)
    {
      int demosize = (demoex_p - buffer);
      int headersize = sizeof(wadtbl->header);
      int datasize = wadtbl->datasize;
      int lumpssize = wadtbl->header.numlumps * sizeof(wadtbl->lumps[0]);

      //write pwad header, all data and lookup table to the end of a demo
      if (
        fwrite(buffer, demosize, 1, hfile) != 1 ||
        fwrite(&wadtbl->header, headersize, 1, hfile) != 1 ||
        fwrite(wadtbl->data, datasize, 1, hfile) != 1 ||
        fwrite(wadtbl->lumps, lumpssize, 1, hfile) != 1 ||
        false)
      {
        I_Error("G_SetDemoFooter: error writing");
      }

      fclose(hfile);
    }
    Z_Free(buffer);
  }
}

int CheckWadBufIntegrity(const char *buffer, size_t size)
{
  int i;
  unsigned int length;
  wadinfo_t *header;
  const filelump_t *fileinfo;
  int result = false;

  if (buffer && size > sizeof(*header))
  {
    // This is dirty, but essentially there is a part of the buffer that is editable
    // It would take too much work to take care of all the chained calls to fix this
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wcast-qual"
    header = (wadinfo_t*)buffer;
    #pragma GCC diagnostic pop
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
        result = (i == header->numlumps);
      }
    }
  }

  return result;
}

int CheckWadFileIntegrity(const char *filename)
{
  FILE *hfile;
  int i;
  unsigned int length;
  wadinfo_t header;
  filelump_t *fileinfo, *fileinfo2free = NULL;
  int result = false;

  hfile = fopen(filename, "rb");
  if (hfile)
  {
    if (fread(&header, sizeof(header), 1, hfile) == 1 &&
      (strncmp(header.identification, "IWAD", 4) == 0 ||
       strncmp(header.identification, "PWAD", 4) == 0))
    {
      header.numlumps = LittleLong(header.numlumps);
      header.infotableofs = LittleLong(header.infotableofs);
      length = header.numlumps * sizeof(filelump_t);

      fileinfo2free = fileinfo = Z_Malloc(length);
      if (fileinfo)
      {
        if (fseek(hfile, header.infotableofs, SEEK_SET) == 0 &&
          fread(fileinfo, length, 1, hfile) == 1)
        {
          for (i = 0; i < header.numlumps; i++, fileinfo++)
          {
            if (fileinfo->filepos < 0 ||
              fileinfo->filepos > header.infotableofs ||
              fileinfo->filepos + fileinfo->size > header.infotableofs)
            {
              break;
            }
          }
          result = (i == header.numlumps);
        }
        Z_Free(fileinfo2free);
      }
    }
    fclose(hfile);
  }

  return result;
}

static int G_ReadDemoFooter(const char *filename)
{
  int result = false;

  byte *buffer = NULL;
  const byte *demoex_p = NULL;
  size_t size;

  demoex_filename[0] = 0;

  buffer = G_GetDemoFooter(filename, &demoex_p, &size);

  if (buffer)
  {
    //the demo has an additional information itself
    size_t i;
    waddata_t waddata;
    int tmp_fd = -1;
    const char* tmp_dir;
    char* tmp_path = NULL;
    const char* template_format = "%sdsda-doom-demoex2-XXXXXX";

    tmp_dir = I_GetTempDir();
    if (tmp_dir && *tmp_dir != '\0')
    {
      tmp_path = Z_Malloc(strlen(tmp_dir) + 2);
      strcpy(tmp_path, tmp_dir);
      if (!HasTrailingSlash(tmp_dir))
      {
        strcat(tmp_path, "/");
      }

      snprintf(demoex_filename, sizeof(demoex_filename), template_format, tmp_path);
#ifdef HAVE_MKSTEMP
      if ((tmp_fd = mkstemp(demoex_filename)) == -1)
#else
      if (mktemp(demoex_filename) == NULL)
#endif
      {
        demoex_filename[0] = 0;
      }

      // don't leave file open
      if (tmp_fd >= 0)
      {
        close(tmp_fd);
      }

      Z_Free(tmp_path);
    }

    if (!demoex_filename[0])
    {
      lprintf(LO_ERROR, "G_ReadDemoFooter: failed to create demoex temp file");
    }
    else
    {
      AddDefaultExtension(demoex_filename, ".wad");

      if (!CheckWadBufIntegrity(demoex_p, size))
      {
        lprintf(LO_ERROR, "G_ReadDemoFooter: demo footer is corrupted\n");
      } // write an additional info from a demo to demoex.wad
      else if (!M_WriteFile(demoex_filename, demoex_p, size))
      {
        lprintf(LO_ERROR, "G_ReadDemoFooter: failed to create demoex temp file %s\n", demoex_filename);
      }
      else
      {
        //add demoex.wad to the wads list
        D_AddFile(demoex_filename, source_auto_load);

        //cache demoex.wad for immediately getting its data with W_LumpByName
        W_Init();

        WadDataInit(&waddata);

        //enumerate and save all auto-loaded files and demo for future use
        for (i = 0; i < numwadfiles; i++)
        {
          if (
            wadfiles[i].src == source_auto_load ||
            wadfiles[i].src == source_pre ||
            wadfiles[i].src == source_lmp)
          {
            WadDataAddItem(&waddata, wadfiles[i].name, wadfiles[i].src, 0);
          }
        }

        //get needed wads and dehs from demoex.wad
        //restore all critical params like -spechit x
        R_DemoEx_GetParams(buffer, &waddata);

        //replace old wadfiles with the new ones
        if (waddata.numwadfiles)
        {
          for (i = 0; (size_t)i < waddata.numwadfiles; i++)
          {
            if (waddata.wadfiles[i].src == source_iwad)
            {
              W_ReleaseAllWads();
              WadDataToWadFiles(&waddata);
              result = true;
              break;
            }
          }
        }
        WadDataFree(&waddata);
      }
    }
    Z_Free(buffer);
  }

  return result;
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

int WadDataInit(waddata_t *waddata)
{
  if (!waddata)
    return false;

  memset(waddata, 0, sizeof(*waddata));
  return true;
}

void WadDataFree(waddata_t *waddata)
{
  if (waddata)
  {
    if (waddata->wadfiles)
    {
      int i;
      for (i = 0; i < (int)waddata->numwadfiles; i++)
      {
        if (waddata->wadfiles[i].name)
        {
          Z_Free(waddata->wadfiles[i].name);
          waddata->wadfiles[i].name = NULL;
        }
      }
      Z_Free(waddata->wadfiles);
      waddata->wadfiles = NULL;
    }
  }
}

int WadDataAddItem(waddata_t *waddata, const char *filename, wad_source_t source, int handle)
{
  if (!waddata || !filename)
    return false;

  waddata->wadfiles = Z_Realloc(waddata->wadfiles, sizeof(*wadfiles) * (waddata->numwadfiles + 1));
  waddata->wadfiles[waddata->numwadfiles].name = Z_Strdup(filename);
  waddata->wadfiles[waddata->numwadfiles].src = source;
  waddata->wadfiles[waddata->numwadfiles].handle = handle;

  waddata->numwadfiles++;

  return true;
}

void WadDataToWadFiles(waddata_t *waddata)
{
  void ProcessDehFile(const char *filename, const char *outfilename, int lumpnum);
  const char *D_dehout(void);

  int i, iwadindex = -1;

  wadfile_info_t *old_wadfiles=NULL;
  size_t old_numwadfiles = numwadfiles;

  old_numwadfiles = numwadfiles;
  old_wadfiles = Z_Malloc(sizeof(*(wadfiles)) * numwadfiles);
  memcpy(old_wadfiles, wadfiles, sizeof(*(wadfiles)) * numwadfiles);

  Z_Free(wadfiles);
  wadfiles = NULL;
  numwadfiles = 0;

  for (i = 0; (size_t)i < waddata->numwadfiles; i++)
  {
    if (waddata->wadfiles[i].src == source_iwad)
    {
      AddIWAD(I_FindFile(waddata->wadfiles[i].name, ".wad"));
      iwadindex = i;
      break;
    }
  }

  if (iwadindex == -1)
  {
    I_Error("WadDataToWadFiles: IWAD not found\n");
  }

  for (i = 0; (size_t)i < old_numwadfiles; i++)
  {
    if (old_wadfiles[i].src == source_auto_load || old_wadfiles[i].src == source_pre)
    {
      wadfiles = Z_Realloc(wadfiles, sizeof(*wadfiles)*(numwadfiles+1));
      wadfiles[numwadfiles].name = Z_Strdup(old_wadfiles[i].name);
      wadfiles[numwadfiles].src = old_wadfiles[i].src;
      wadfiles[numwadfiles].handle = old_wadfiles[i].handle;
      numwadfiles++;
    }
  }

  for (i = 0; (size_t)i < waddata->numwadfiles; i++)
  {
    if (waddata->wadfiles[i].src == source_auto_load)
    {
      wadfiles = Z_Realloc(wadfiles, sizeof(*wadfiles)*(numwadfiles+1));
      wadfiles[numwadfiles].name = Z_Strdup(waddata->wadfiles[i].name);
      wadfiles[numwadfiles].src = waddata->wadfiles[i].src;
      wadfiles[numwadfiles].handle = waddata->wadfiles[i].handle;
      numwadfiles++;
    }
  }

  for (i = 0; (size_t)i < waddata->numwadfiles; i++)
  {
    if (waddata->wadfiles[i].src == source_iwad && i != iwadindex)
    {
      D_AddFile(waddata->wadfiles[i].name, source_pwad);
      modifiedgame = true;
    }
    if (waddata->wadfiles[i].src == source_pwad)
    {
      const char *file = I_FindFile2(waddata->wadfiles[i].name, ".wad");
      if (file)
      {
        D_AddFile(waddata->wadfiles[i].name, source_pwad);
        modifiedgame = true;
      }
    }
    if (waddata->wadfiles[i].src == source_deh)
    {
      ProcessDehFile(waddata->wadfiles[i].name, D_dehout(), 0);
    }
  }

  for (i = 0; (size_t)i < waddata->numwadfiles; i++)
  {
    if (waddata->wadfiles[i].src == source_lmp || waddata->wadfiles[i].src == source_net)
      D_AddFile(waddata->wadfiles[i].name, waddata->wadfiles[i].src);
  }

  Z_Free(old_wadfiles);
}

int CheckDemoExDemo(void)
{
  int result = false;
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
      result = G_ReadDemoFooter(demoname);
      Z_Free(demoname);
    }

    Z_Free(filename);
  }

  return result;
}
