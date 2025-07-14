//  Copyright (C) 2023 Fabian Greffrath
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
// DESCRIPTION:
//      Load sound lumps with libsndfile.

#include "SDL.h"
#include "sndfile.h"

#include "lprintf.h"
#include "memio.h"
#include "z_zone.h"

static sf_count_t sfvio_get_filelen(void *user_data)
{
  MEMFILE *fs = user_data;
  long pos;
  sf_count_t len;

  pos = mem_ftell(fs);
  mem_fseek(fs, 0, MEM_SEEK_END);
  len = mem_ftell(fs);
  mem_fseek(fs, pos, MEM_SEEK_SET);

  return len;
}

static sf_count_t sfvio_seek(sf_count_t offset, int whence, void *user_data)
{
  MEMFILE *fs = user_data;
  mem_fseek(fs, offset, whence);
  return mem_ftell(fs);
}

static sf_count_t sfvio_read(void *ptr, sf_count_t count, void *user_data)
{
  return mem_fread(ptr, 1, count, (MEMFILE *)user_data);
}

static sf_count_t sfvio_tell(void *user_data)
{
  return mem_ftell((MEMFILE *)user_data);
} 

void *Load_SNDFile(void *data, SDL_AudioSpec *sample, Uint8 **sampledata,
                   Uint32 *samplelen)
{
  SNDFILE *sndfile;
  SF_INFO sfinfo;
  SF_VIRTUAL_IO sfvio =
  {
    sfvio_get_filelen,
    sfvio_seek,
    sfvio_read,
    NULL,
    sfvio_tell
  };
  Uint32 local_samplelen;
  short *local_sampledata;

  MEMFILE *sfdata = mem_fopen_read(data, *samplelen);

  memset(&sfinfo, 0, sizeof(sfinfo));

  sndfile = sf_open_virtual(&sfvio, SFM_READ, &sfinfo, sfdata);

  if (!sndfile)
  {
    lprintf(LO_WARN, "sf_open_virtual: %s\n", sf_strerror(sndfile));
    mem_fclose(sfdata);
    return NULL;
  }

  if (sfinfo.frames <= 0 || sfinfo.channels <= 0)
  {
    sf_close(sndfile);
    mem_fclose(sfdata);
    return NULL;
  }

  local_samplelen = sfinfo.frames * sfinfo.channels * sizeof(short);
  local_sampledata = Z_Malloc(local_samplelen);

  if (!local_sampledata)
  {
    sf_close(sndfile);
    mem_fclose(sfdata);
    return NULL;
  }

  sf_command(sndfile, SFC_SET_SCALE_FLOAT_INT_READ, NULL, SF_TRUE);

  if (sf_readf_short(sndfile, local_sampledata, sfinfo.frames) < sfinfo.frames)
  {
    lprintf(LO_WARN, "sf_readf_short: %s\n", sf_strerror(sndfile));
    sf_close(sndfile);
    mem_fclose(sfdata);
    Z_Free(local_sampledata);
    return NULL;
  }

  sf_close(sndfile);
  mem_fclose(sfdata);

  sample->channels = sfinfo.channels;
  sample->freq = sfinfo.samplerate;
  sample->format = AUDIO_S16;

  *sampledata = (Uint8 *)local_sampledata;
  *samplelen = local_samplelen;

  return sample;
}
