/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *
 *  Copyright (C) 2011 by
 *  Nicholai Main
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
 *
 *---------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "musicplayer.h"

#ifndef HAVE_LIBFLUIDSYNTH
#include <string.h>

static const char *fl_name (void)
{
  return "fluidsynth midi player (DISABLED)";
}


static int fl_init (int samplerate)
{
  return 0;
}

const music_player_t fl_player =
{
  fl_name,
  fl_init,
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

#else // HAVE_LIBFLUIDSYNTH

#include <fluidsynth.h>
#include <stdlib.h>
#include <string.h>
#include "i_system.h" // for I_FindFile()
#include "lprintf.h"
#include "midifile.h"
#include "memio.h"
#include "w_wad.h"

#include "dsda/args.h"
#include "dsda/configuration.h"

#if (FLUIDSYNTH_VERSION_MAJOR < 2 || (FLUIDSYNTH_VERSION_MAJOR == 2 && FLUIDSYNTH_VERSION_MINOR < 2))
  typedef int fl_sfread_count_t;
  typedef long fl_sfseek_offset_t;
  typedef long fl_sftell_t;
#else
  typedef fluid_long_long_t fl_sfread_count_t;
  typedef fluid_long_long_t fl_sfseek_offset_t;
  typedef fluid_long_long_t fl_sftell_t;
#endif

static fluid_settings_t *f_set;
static fluid_synth_t *f_syn;
static int f_font;
static midi_event_t **events;
static int eventpos;
static midi_file_t *midifile;

static int f_playing;
static int f_paused;
static int f_looping;
static int f_volume;
static double spmc;
static double f_delta;
static int f_soundrate;

static const char *fl_name (void)
{
  return "fluidsynth midi player";
}

#ifdef _MSC_VER
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#include <delayimp.h>
#endif

static void *fl_sfopen(const char *lumpname)
{
  MEMFILE *instream;
  int lumpnum = W_GetNumForName(lumpname);
  int len = W_LumpLength(lumpnum);
  const void *data = W_LumpByNum(lumpnum);

  instream = mem_fopen_read(data, len);

  return instream;
}

static int fl_sfread(void *buf, fl_sfread_count_t count, void *handle)
{
  if (mem_fread(buf, sizeof(byte), count, (MEMFILE *)handle) == count)
  {
    return FLUID_OK;
  }
  return FLUID_FAILED;
}

static int fl_sfseek(void *handle, fl_sfseek_offset_t offset, int origin)
{
  if (mem_fseek((MEMFILE *)handle, offset, origin) < 0)
  {
    return FLUID_FAILED;
  }
  return FLUID_OK;
}

static int fl_sfclose(void *handle)
{
  mem_fclose((MEMFILE *)handle);
  return FLUID_OK;
}

static fl_sftell_t fl_sftell(void *handle)
{
  return mem_ftell((MEMFILE *)handle);
}

static void fl_null_logger(int level, const char *message, void *data) {
  // no op
}

static int fl_init (int samplerate)
{
  int mus_fluidsynth_chorus;
  int mus_fluidsynth_reverb;
  int mus_fluidsynth_gain;
  int mus_fluidsynth_chorus_depth;
  int mus_fluidsynth_chorus_level;
  int mus_fluidsynth_reverb_damp;
  int mus_fluidsynth_reverb_level;
  int mus_fluidsynth_reverb_width;
  int mus_fluidsynth_reverb_room_size;
  const char *filename;

  if (!dsda_Flag(dsda_arg_verbose) || dsda_Flag(dsda_arg_quiet))
    fluid_set_log_function(FLUID_WARN, fl_null_logger, NULL);

  mus_fluidsynth_chorus = dsda_IntConfig(dsda_config_mus_fluidsynth_chorus);
  mus_fluidsynth_reverb = dsda_IntConfig(dsda_config_mus_fluidsynth_reverb);
  mus_fluidsynth_gain = dsda_IntConfig(dsda_config_mus_fluidsynth_gain);
  mus_fluidsynth_chorus_depth = dsda_IntConfig(dsda_config_mus_fluidsynth_chorus_depth);
  mus_fluidsynth_chorus_level = dsda_IntConfig(dsda_config_mus_fluidsynth_chorus_level);
  mus_fluidsynth_reverb_damp = dsda_IntConfig(dsda_config_mus_fluidsynth_reverb_damp);
  mus_fluidsynth_reverb_level = dsda_IntConfig(dsda_config_mus_fluidsynth_reverb_level);
  mus_fluidsynth_reverb_width = dsda_IntConfig(dsda_config_mus_fluidsynth_reverb_width);
  mus_fluidsynth_reverb_room_size = dsda_IntConfig(dsda_config_mus_fluidsynth_reverb_room_size);

  f_soundrate = samplerate;
  // fluidsynth 1.1.4 supports sample rates as low as 8000hz.  earlier versions only go down to 22050hz
  // since the versions are ABI compatible, detect at runtime, not compile time
  {
    int sratemin;
    int major;
    int minor;
    int micro;
    fluid_version (&major, &minor, &micro);
    lprintf (LO_DEBUG, "Fluidplayer: Fluidsynth version %i.%i.%i\n", major, minor, micro);
    if (major >= 2 || (minor >=1 && micro >= 4))
      sratemin = 8000;
    else
      sratemin = 22050;
    if (f_soundrate < sratemin)
    {
      lprintf (LO_WARN, "Fluidplayer: samplerates under %i are not supported\n", sratemin);
      return 0;
    }
  }


  f_set = new_fluid_settings ();

#if FLUIDSYNTH_VERSION_MAJOR == 1
  #define FSET(a,b,c) if (!fluid_settings_set##a(f_set,b,c))\
    lprintf (LO_WARN, "fl_init: Couldn't set " b "\n")
#else
  #define FSET(a,b,c) if (fluid_settings_set##a(f_set,b,c) == FLUID_FAILED)\
    lprintf (LO_WARN, "fl_init: Couldn't set " b "\n")
#endif

  FSET (num, "synth.sample-rate", f_soundrate);

  FSET (int, "synth.chorus.active", mus_fluidsynth_chorus);
  FSET (int, "synth.reverb.active", mus_fluidsynth_reverb);

  if (mus_fluidsynth_chorus)
  {
    FSET (num, "synth.chorus.depth", mus_fluidsynth_chorus_depth / 100.0);
    FSET (num, "synth.chorus.level", mus_fluidsynth_chorus_level / 100.0);
  }

  if (mus_fluidsynth_reverb)
  {
    FSET (num, "synth.reverb.damp", mus_fluidsynth_reverb_damp / 100.0);
    FSET (num, "synth.reverb.level", mus_fluidsynth_reverb_level / 100.0);
    FSET (num, "synth.reverb.width", mus_fluidsynth_reverb_width / 100.0);
    FSET (num, "synth.reverb.room-size", mus_fluidsynth_reverb_room_size / 100.0);
  }

  // gain control
  FSET (num, "synth.gain", mus_fluidsynth_gain / 100.0); // 0.0 - 0.2 - 10.0
  // behavior wrt bank select messages
  FSET (str, "synth.midi-bank-select", "gs"); // fluidsynth default
  // general midi spec says 24 voices, but modern midi songs use more
  FSET (int, "synth.polyphony", 256); // fluidsynth default

  // we're not using the builtin shell or builtin midiplayer,
  // and our own access to the synth is protected by mutex in i_sound.c
  FSET (int, "synth.threadsafe-api", 0);
#if FLUIDSYNTH_VERSION_MAJOR == 1
  FSET (int, "synth.parallel-render", 0);
#endif

  // prints debugging information to STDOUT
  //FSET (int, "synth.verbose", 1);

  #undef FSET

  f_syn = new_fluid_synth (f_set);
  if (!f_syn)
  {
    lprintf (LO_WARN, "fl_init: error creating fluidsynth object\n");
    delete_fluid_settings (f_set);
    return 0;
  }

  {
    int lumpnum;
    int checked_file = false;
    dboolean replaced_soundfont = false;
    const char *checked_f_font = NULL;
    const char *snd_soundfont;

    lumpnum = W_CheckNumForName("SNDFONT");

    if (lumpnum != LUMP_NOT_FOUND)
    {
      replaced_soundfont = !W_LumpNumInPortWad(lumpnum);
    }

    snd_soundfont = dsda_StringConfig(dsda_config_snd_soundfont);

    if (!replaced_soundfont && snd_soundfont && snd_soundfont[0])
    {
      checked_file = true;
      checked_f_font = snd_soundfont;
      filename = I_FindFile2(snd_soundfont, ".sf2");
      f_font = fluid_synth_sfload (f_syn, filename, 1);
    }

    if ((!checked_file || f_font == FLUID_FAILED) && lumpnum >= 0)
    {
      fluid_sfloader_t *sfloader;

      checked_f_font = "SNDFONT";
      sfloader = new_fluid_defsfloader(f_set);
      fluid_sfloader_set_callbacks(sfloader, fl_sfopen, fl_sfread, fl_sfseek,
                                   fl_sftell, fl_sfclose);
      fluid_synth_add_sfloader(f_syn, sfloader);
      f_font = fluid_synth_sfload(f_syn, "SNDFONT", 1);
    }

    if (!checked_f_font)
    {
      lprintf(LO_WARN, "fl_init: no soundfont detected!\n");
      delete_fluid_synth (f_syn);
      delete_fluid_settings (f_set);
      return 0;
    }

    if (f_font == FLUID_FAILED)
    {
      lprintf (LO_WARN, "fl_init: error loading soundfont %s\n", checked_f_font);
      delete_fluid_synth (f_syn);
      delete_fluid_settings (f_set);
      return 0;
    }
  }

  return 1;
}

static void fl_shutdown (void)
{
  if (f_syn)
  {
    delete_fluid_synth (f_syn);
    f_syn = NULL;
    f_font = 0;
  }

  if (f_set)
  {
    delete_fluid_settings (f_set);
    f_set = NULL;
  }
}





static const void *fl_registersong (const void *data, unsigned len)
{
  midimem_t mf;

  mf.len = len;
  mf.pos = 0;
  mf.data = (const byte*)data;

  midifile = MIDI_LoadFile (&mf);

  if (!midifile)
  {
    return NULL;
  }

  events = MIDI_GenerateFlatList (midifile);
  if (!events)
  {
    MIDI_FreeFile (midifile);
    return NULL;
  }
  eventpos = 0;

  // implicit 120BPM (this is correct to spec)
  //spmc = compute_spmc (MIDI_GetFileTimeDivision (midifile), 500000, f_soundrate);
  spmc = MIDI_spmc (midifile, NULL, f_soundrate);

  // handle not used
  return data;
}

static void fl_unregistersong (const void *handle)
{
  if (events)
  {
    MIDI_DestroyFlatList (events);
    events = NULL;
  }
  if (midifile)
  {
    MIDI_FreeFile (midifile);
    midifile = NULL;
  }
}

static void fl_pause (void)
{
  //int i;
  f_paused = 1;
  // instead of cutting notes, pause the synth so they can resume seamlessly
  //for (i = 0; i < 16; i++)
  //  fluid_synth_cc (f_syn, i, 123, 0); // ALL NOTES OFF
}
static void fl_resume (void)
{
  f_paused = 0;
}
static void fl_play (const void *handle, int looping)
{
  eventpos = 0;
  f_looping = looping;
  f_playing = 1;
  //f_paused = 0;
  f_delta = 0.0;
  fluid_synth_program_reset (f_syn);
  fluid_synth_system_reset (f_syn);
}

static void fl_stop (void)
{
  int i;
  f_playing = 0;

  for (i = 0; i < 16; i++)
  {
    fluid_synth_cc (f_syn, i, 123, 0); // ALL NOTES OFF
    fluid_synth_cc (f_syn, i, 121, 0); // RESET ALL CONTROLLERS
  }
}

static void fl_setvolume (int v)
{
  f_volume = v;
}


static void fl_writesamples_ex (short *dest, int nsamp)
{ // does volume conversion and then writes samples
  int i;
  float multiplier = 16384.0f / 15.0f * f_volume;

  static float *fbuff = NULL;
  static int fbuff_siz = 0;

  if (nsamp * 2 > fbuff_siz)
  {
    float *newfbuff = (float*)realloc (fbuff, nsamp * 2 * sizeof (float));
	if (!newfbuff) return;
	fbuff = newfbuff;
    fbuff_siz = nsamp * 2;
  }

  fluid_synth_write_float (f_syn, nsamp, fbuff, 0, 2, fbuff, 1, 2);

  for (i = 0; i < nsamp * 2; i++)
  {
    // data is NOT already clipped
	  float f = fbuff[i];
    if (f > 1.0f)
      f = 1.0f;
    if (f < -1.0f)
      f = -1.0f;
    dest[i] = (short) (f * multiplier);
  }
}

static void fl_render (void *vdest, unsigned length)
{
  short *dest = (short*)vdest;

  unsigned sampleswritten = 0;
  unsigned samples;

  midi_event_t *currevent;

  if (!f_playing || f_paused)
  {
    // save CPU time and allow for seamless resume after pause
    memset (vdest, 0, length * 4);
    //fl_writesamples_ex (vdest, length);
    return;
  }


  while (1)
  {
    double eventdelta;
    currevent = events[eventpos];

    // how many samples away event is
    eventdelta = currevent->delta_time * spmc;


    // how many we will render (rounding down); include delta offset
    samples = (unsigned) (eventdelta + f_delta);


    if (samples + sampleswritten > length)
    { // overshoot; render some samples without processing an event
      break;
    }


    if (samples)
    {
      fl_writesamples_ex (dest, samples);
      sampleswritten += samples;
      f_delta -= samples;
      dest += samples * 2;
    }

    // process event
    switch (currevent->event_type)
    {
      case MIDI_EVENT_NOTE_OFF:
        fluid_synth_noteoff (f_syn, currevent->data.channel.channel, currevent->data.channel.param1);
        break;
      case MIDI_EVENT_NOTE_ON:
        fluid_synth_noteon (f_syn, currevent->data.channel.channel, currevent->data.channel.param1, currevent->data.channel.param2);
        break;
      case MIDI_EVENT_AFTERTOUCH:
        // not suipported?
        break;
      case MIDI_EVENT_CONTROLLER:
        fluid_synth_cc (f_syn, currevent->data.channel.channel, currevent->data.channel.param1, currevent->data.channel.param2);
        break;
      case MIDI_EVENT_PROGRAM_CHANGE:
        fluid_synth_program_change (f_syn, currevent->data.channel.channel, currevent->data.channel.param1);
        break;
      case MIDI_EVENT_CHAN_AFTERTOUCH:
        fluid_synth_channel_pressure (f_syn, currevent->data.channel.channel, currevent->data.channel.param1);
        break;
      case MIDI_EVENT_PITCH_BEND:
        fluid_synth_pitch_bend (f_syn, currevent->data.channel.channel, currevent->data.channel.param1 | currevent->data.channel.param2 << 7);
        break;
      case MIDI_EVENT_META:
        if (currevent->data.meta.type == MIDI_META_SET_TEMPO)
          spmc = MIDI_spmc (midifile, currevent, f_soundrate);
        else if (currevent->data.meta.type == MIDI_META_END_OF_TRACK)
        {
          if (f_looping)
          {
            int i;
            eventpos = 0;
            f_delta += eventdelta;
            // fix buggy songs that forget to terminate notes held over loop point
            // sdl_mixer does this as well
            for (i = 0; i < 16; i++)
            {
              fluid_synth_cc (f_syn, i, 123, 0); // ALL NOTES OFF
              fluid_synth_cc (f_syn, i, 121, 0); // RESET ALL CONTROLLERS
              fluid_synth_cc (f_syn, i, 7, 100); // reset volume
            }
            continue;
          }
          // stop, write leadout
          fl_stop ();
          samples = length - sampleswritten;
          if (samples)
          {
            fl_writesamples_ex (dest, samples);
            sampleswritten += samples;
            // timecodes no longer relevant
            dest += samples * 2;

          }
          return;
        }
        break; // not interested in most metas
      default: //uhh
        break;

    }
    // event processed so advance midiclock
    f_delta += eventdelta;
    eventpos++;

  }




  if (samples + sampleswritten > length)
  { // broke due to next event being past the end of current render buffer
    // finish buffer, return
    samples = length - sampleswritten;
    if (samples)
    {
      fl_writesamples_ex (dest, samples);
      sampleswritten += samples;
      f_delta -= samples; // save offset
      dest += samples * 2;
    }
  }
  else
  { // huh?
    return;
  }


}


const music_player_t fl_player =
{
  fl_name,
  fl_init,
  fl_shutdown,
  fl_setvolume,
  fl_pause,
  fl_resume,
  fl_registersong,
  fl_unregistersong,
  fl_play,
  fl_stop,
  fl_render
};


#endif // HAVE_LIBFLUIDSYNTH
