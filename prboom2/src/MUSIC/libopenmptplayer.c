/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
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
 *  Tracker music player based on libopenmpt.
 *---------------------------------------------------------------------
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "musicplayer.h"

#ifndef HAVE_LIBOPENMPT
#include <stddef.h>

static const char *mpt_name (void)
{
  return "libopenmpt tracker player (DISABLED)";
}


static int mpt_init (int samplerate)
{
  return 0;
}

const music_player_t mpt_player =
{
  mpt_name,
  mpt_init,
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

#else // HAVE_LIBOPENMPT

#include <libopenmpt/libopenmpt.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "lprintf.h"


static int mpt_samplerate;
static int mpt_paused;
static int mpt_gain;
static openmpt_module *mpt_cur_module;

static const char *mpt_name (void)
{
  return "libopenmpt tracker player";
}


static int mpt_init (int samplerate)
{
  mpt_samplerate = samplerate;

  return 1;
}

static void mpt_shutdown (void)
{
}

static void mpt_setvolume (int v)
{
  int ret = 1;

  // For 0% volume, log10f(0.f) yields -inf, which after converting to
  // int yields INT_MIN gain.
  mpt_gain = 2000.f * log10f(v / 15.f);

  if (mpt_cur_module)
    ret = openmpt_module_set_render_param(mpt_cur_module,
                    OPENMPT_MODULE_RENDER_MASTERGAIN_MILLIBEL,
                    mpt_gain);

  if (!ret)
    lprintf(LO_WARN, "mpt_setvolume: failed to set current module master gain\n");
}

static void mpt_log_func (const char *message, void *user)
{
  (void) user;
  lprintf(LO_DEBUG, "libopenmpt log: %s\n", message);
}

static const void* mpt_registersong (const void *data, unsigned len)
{
  openmpt_module *mod = NULL;

  mod = openmpt_module_create_from_memory2(data, len,
                  mpt_log_func, NULL,
                  NULL, NULL,
                  NULL, NULL, NULL);
  if (!mod)
    lprintf(LO_DEBUG, "mpt_registersong: failed to open song\n");

  return mod;
}

static void mpt_unregistersong (const void *handle)
{
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wcast-qual"
  openmpt_module_destroy((openmpt_module *) handle);
  #pragma GCC diagnostic pop
}

static void mpt_play (const void *handle, int looping)
{
  int ret;

  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wcast-qual"
  mpt_cur_module = (openmpt_module *) handle;
  #pragma GCC diagnostic pop
  openmpt_module_set_repeat_count(mpt_cur_module, looping ? -1 : 0);

  ret = openmpt_module_set_render_param(mpt_cur_module,
                  OPENMPT_MODULE_RENDER_MASTERGAIN_MILLIBEL,
                  mpt_gain);

  if (!ret)
    lprintf(LO_WARN, "mpt_play: failed to set current module master gain\n");
}

static void mpt_stop (void)
{
  mpt_cur_module = NULL;
}

static void mpt_pause (void)
{
  mpt_paused = 1;
}

static void mpt_resume (void)
{
  mpt_paused = 0;
}

static void mpt_render (void *dest, unsigned nsamp)
{
  short *sdest = (short *)dest;

  if (mpt_cur_module && !mpt_paused)
  {
    size_t nout;

    nout = openmpt_module_read_interleaved_stereo(mpt_cur_module,
                    mpt_samplerate, nsamp, sdest);

    if (nout != nsamp)
      memset(sdest + nout * 2, 0, (nsamp - nout) * 4);
  }
  else
    memset (dest, 0, nsamp * 4);
}

const music_player_t mpt_player =
{
  mpt_name,
  mpt_init,
  mpt_shutdown,
  mpt_setvolume,
  mpt_pause,
  mpt_resume,
  mpt_registersong,
  mpt_unregistersong,
  mpt_play,
  mpt_stop,
  mpt_render
};

#endif // HAVE_LIBOPENMPT
