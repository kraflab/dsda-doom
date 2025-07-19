//
// Copyright(C) 2025 ceski
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
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "musicplayer.h"

#ifndef HAVE_LIBXMP

#include <stddef.h>

static const char *xmp_name(void)
{
  return "libxmp tracker player (DISABLED)";
}

static int xmp_init(int samplerate)
{
  return 0;
}

const music_player_t xmp_player =
{
  xmp_name,
  xmp_init,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

#else // HAVE_LIBXMP

#include <xmp.h>
#include <string.h>
#include "lprintf.h"

static xmp_context context;
static int xmp_samplerate;
static float xmp_volume;
static int xmp_looping;
static int xmp_playing;
static int xmp_paused;

static const char *xmp_name(void)
{
  return "libxmp tracker player";
}

static int xmp_init(int samplerate)
{
  xmp_samplerate = samplerate;
  context = xmp_create_context();

  if (!context)
  {
    lprintf(LO_WARN, "xmp_init: failed to create context.\n");
    return 0;
  }

  return 1;
}

static void xmp_shutdown(void)
{
  xmp_free_context(context);
  context = NULL;
}

static void xmp_setvolume(int v)
{
  xmp_volume = (float)v / 15.0f;
}

static const void *xmp_registersong(const void *data, unsigned len)
{
  if (xmp_load_module_from_memory(context, data, len) < 0)
  {
    return NULL;
  }

  return data;
}

static void xmp_unregistersong(const void *handle)
{
  xmp_stop_module(context);
  xmp_end_player(context);
  xmp_release_module(context);
}

static void xmp_play(const void *handle, int looping)
{
  if (xmp_start_player(context, xmp_samplerate, 0) < 0)
  {
    lprintf(LO_WARN, "xmp_play: failed to start player.\n");
    xmp_playing = 0;
    return;
  }

  xmp_set_player(context, XMP_PLAYER_VOLUME, 100);
  xmp_looping = looping;
  xmp_playing = 1;
}

static void xmp_stop(void)
{
  xmp_playing = 0;
}

static void xmp_pause(void)
{
  xmp_paused = 1;
}

static void xmp_resume(void)
{
  xmp_paused = 0;
}

static void xmp_render(void *dest, unsigned nsamp)
{
  if (xmp_playing && !xmp_paused)
  {
    if (xmp_play_buffer(context, dest, nsamp * 4, xmp_looping ? 0 : 1) == 0)
    {
      short *sdest = (short *)dest;

      for (int i = 0; i < nsamp * 2; i++)
      {
        sdest[i] = (short)(sdest[i] * xmp_volume);
      }
    }
    else
    {
      if (xmp_looping)
      {
        xmp_restart_module(context);
        xmp_set_position(context, 0);
      }
      else
      {
        xmp_stop();
      }

      memset(dest, 0, nsamp * 4);
    }
  }
  else
  {
    memset(dest, 0, nsamp * 4);
  }
}

const music_player_t xmp_player =
{
  xmp_name,
  xmp_init,
  xmp_shutdown,
  xmp_setvolume,
  xmp_pause,
  xmp_resume,
  xmp_registersong,
  xmp_unregistersong,
  xmp_play,
  xmp_stop,
  xmp_render
};

#endif // HAVE_LIBXMP
