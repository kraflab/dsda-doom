/* Emacs style mode select   -*- C++ -*-
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

// TODO: some duplicated code with this and the fluidplayer should be
// split off or something

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "musicplayer.h"

#ifndef HAVE_LIBPORTMIDI
#include <string.h>

static const char *pm_name (void)
{
  return "portmidi midi player (DISABLED)";
}

static int pm_init (int samplerate)
{
  return 0;
}

const music_player_t pm_player =
{
  pm_name,
  pm_init,
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

#else // HAVE_LIBPORTMIDI

#include <math.h>
#include <portmidi.h>
#include <porttime.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lprintf.h"
#include "midifile.h"

#include "dsda/configuration.h"

static midi_event_t **events;
static int eventpos;
static midi_file_t *midifile;

static int pm_playing;
static int pm_paused;
static int pm_looping;
static int pm_volume;
static double spmc;
static double pm_delta;

static unsigned long trackstart;

static PortMidiStream *pm_stream;

#define SYSEX_BUFF_SIZE 1024
static unsigned char sysexbuff[SYSEX_BUFF_SIZE];
static int sysexbufflen;

static const char *mus_portmidi_reset_type; // portmidi reset type
static int mus_portmidi_reset_delay; // portmidi delay after reset

// latency: we're generally writing timestamps slightly in the past (from when the last time
// render was called to this time.  portmidi latency instruction must be larger than that window
// so the messages appear in the future.  ~46-47ms is the nominal length if i_sound.c gets its way
#define DRIVER_LATENCY 80 // ms
// driver event buffer needs to be big enough to hold however many events occur in latency time
#define DRIVER_BUFFER 100 // events

static const char *pm_name (void)
{
  return "portmidi midi player";
}

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <delayimp.h>
#endif

static dboolean use_reset_delay;
static unsigned char gs_reset[] = {0xf0, 0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7f, 0x00, 0x41, 0xf7};
static unsigned char gm_system_on[] = {0xf0, 0x7e, 0x7f, 0x09, 0x01, 0xf7};
static unsigned char gm2_system_on[] = {0xf0, 0x7e, 0x7f, 0x09, 0x03, 0xf7};
static unsigned char xg_system_on[] = {0xf0, 0x43, 0x10, 0x4c, 0x00, 0x00, 0x7e, 0x00, 0xf7};
static PmEvent event_buffer[13 * 16];

static void reset_device (unsigned long when)
{
  int i;

  // non-sysex resets for compatibility with MS GS Wavetable Synth
  for (i = 0; i < 13 * 16; ++i)
  {
    event_buffer[i].timestamp = when;
  }
  Pm_Write(pm_stream, event_buffer, 13 * 16);

  // sysex reset
  if (!strcasecmp(mus_portmidi_reset_type, "gm"))
    Pm_WriteSysEx(pm_stream, when, gm_system_on);
  else if (!strcasecmp(mus_portmidi_reset_type, "gm2"))
    Pm_WriteSysEx(pm_stream, when, gm2_system_on);
  else if (!strcasecmp(mus_portmidi_reset_type, "xg"))
    Pm_WriteSysEx(pm_stream, when, xg_system_on);
  else // default to "gs"
    Pm_WriteSysEx(pm_stream, when, gs_reset);

  use_reset_delay = mus_portmidi_reset_delay > 0;
}

static void init_reset_buffer (void)
{
  int i;
  PmEvent *event = event_buffer;
  for (i = 0; i < 16; ++i)
  {
    // all notes off
    event[0].message = Pm_Message(MIDI_EVENT_CONTROLLER | i, 0x7b, 0x00);
    // all sound off
    event[1].message = Pm_Message(MIDI_EVENT_CONTROLLER | i, 0x78, 0x00);
    // reset all controllers
    event[2].message = Pm_Message(MIDI_EVENT_CONTROLLER | i, 0x79, 0x00);
    // reset channel pressure
    event[3].message = Pm_Message(MIDI_EVENT_CHAN_AFTERTOUCH | i, 0x00, 0x00);
    // reset expression
    event[4].message = Pm_Message(MIDI_EVENT_CONTROLLER | i, 0x0b, 0x7f);
    // reset pitch bend
    event[5].message = Pm_Message(MIDI_EVENT_PITCH_BEND | i, 0x00, 0x40);
    // reset pitch bend sensitivity
    event[6].message = Pm_Message(MIDI_EVENT_CONTROLLER | i, 0x65, 0x00);
    event[7].message = Pm_Message(MIDI_EVENT_CONTROLLER | i, 0x64, 0x00);
    event[8].message = Pm_Message(MIDI_EVENT_CONTROLLER | i, 0x06, 0x02);
    event[9].message = Pm_Message(MIDI_EVENT_CONTROLLER | i, 0x26, 0x00);
    event[10].message = Pm_Message(MIDI_EVENT_CONTROLLER | i, 0x64, 0x7f);
    event[11].message = Pm_Message(MIDI_EVENT_CONTROLLER | i, 0x65, 0x7f);
    // default program change
    event[12].message = Pm_Message(MIDI_EVENT_PROGRAM_CHANGE | i, 0x00, 0x00);
    event += 13;
  }
}

static int pm_init (int samplerate)
{
  PmDeviceID outputdevice;
  const PmDeviceInfo *oinfo;
  int i;
  char devname[64];
  const char* snd_mididev;

  mus_portmidi_reset_type = dsda_StringConfig(dsda_config_mus_portmidi_reset_type);
  mus_portmidi_reset_delay = dsda_IntConfig(dsda_config_mus_portmidi_reset_delay);

  if (Pm_Initialize () != pmNoError)
  {
    lprintf (LO_WARN, "portmidiplayer: Pm_Initialize () failed\n");
    return 0;
  }

  outputdevice = Pm_GetDefaultOutputDeviceID ();

  if (outputdevice == pmNoDevice)
  {
    lprintf (LO_WARN, "portmidiplayer: No output devices available\n");
    Pm_Terminate ();
    return 0;
  }

  // look for a device that matches the user preference

  snd_mididev = dsda_StringConfig(dsda_config_snd_mididev);

  lprintf (LO_DEBUG, "portmidiplayer device list:\n");
  for (i = 0; i < Pm_CountDevices (); i++)
  {
    oinfo = Pm_GetDeviceInfo (i);
    if (!oinfo || !oinfo->output)
      continue;
    snprintf (devname, sizeof(devname), "%s:%s", oinfo->interf, oinfo->name);
    if (strlen (snd_mididev) && strstr (devname, snd_mididev))
    {
      outputdevice = i;
      lprintf (LO_DEBUG, ">>%s\n", devname);
    }
    else
    {
      lprintf (LO_DEBUG, "  %s\n", devname);
    }
  }


  oinfo = Pm_GetDeviceInfo (outputdevice);

  lprintf (LO_DEBUG, "portmidiplayer: Opening device %s:%s for output\n", oinfo->interf, oinfo->name);

  if (Pm_OpenOutput(&pm_stream, outputdevice, NULL, DRIVER_BUFFER, NULL, NULL, DRIVER_LATENCY) != pmNoError)
  {
    lprintf (LO_WARN, "portmidiplayer: Pm_OpenOutput () failed\n");
    Pm_Terminate ();
    return 0;
  }

  init_reset_buffer();
  reset_device(0);
  return 1;
}

static void pm_stop (void);

static void pm_shutdown (void)
{
  if (pm_stream)
  {
    // stop all sound, in case of hanging notes
    if (pm_playing)
      pm_stop();

    /* ugly deadlock in portmidi win32 implementation:

    main thread gets stuck in Pm_Close
    midi thread (started by windows) gets stuck in winmm_streamout_callback

    winapi ref says:
    "Applications should not call any multimedia functions from inside the callback function,
     as doing so can cause a deadlock. Other system functions can safely be called from the callback."

    winmm_streamout_callback calls midiOutUnprepareHeader.  oops?


    since timestamps are slightly in the future, it's very possible to have some messages still in
    the windows midi queue when Pm_Close is called.  this is normally no problem, but if one so happens
    to dequeue and call winmm_streamout_callback at the exact right moment...

    fix: at this point, we've stopped generating midi messages.  sleep for more than DRIVER_LATENCY to ensure
    all messages are flushed.

    not a fix: calling Pm_Abort(); then midiStreamStop deadlocks instead of midiStreamClose.
    */
    Pt_Sleep (DRIVER_LATENCY * 2);

    Pm_Close (pm_stream);
    Pm_Terminate ();
    pm_stream = NULL;
  }
}

static const void *pm_registersong (const void *data, unsigned len)
{
  midimem_t mf;

  mf.len = len;
  mf.pos = 0;
  mf.data = data;

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
  //spmc = compute_spmc (MIDI_GetFileTimeDivision (midifile), 500000, 1000);
  spmc = MIDI_spmc (midifile, NULL, 1000);

  // handle not used
  return data;
}

static void writeevent (unsigned long when, int eve, int channel, int v1, int v2)
{
  PmMessage m;

  m = Pm_Message (eve | channel, v1, v2);
  Pm_WriteShort (pm_stream, when, m);
}

static int mastervol;

static void set_mastervol (unsigned long when)
{
  int vol = mastervol * sqrt((float) pm_volume / 15);
  unsigned char data[] = {0xf0, 0x7f, 0x7f, 0x04, 0x01, vol & 0x7f, vol >> 7, 0xf7};
  Pm_WriteSysEx(pm_stream, when, data);
}

static void refresh_mastervol (void)
{
  unsigned long when = Pt_Time ();
  set_mastervol(when);
}

static void clear_mastervol (void)
{
  mastervol = 16383; // default: max, 14-bit
}

static int firsttime = 1;

static void pm_setvolume (int v)
{
  if (pm_volume == v && !firsttime)
    return;
  firsttime = 0;

  pm_volume = v;

  refresh_mastervol ();
}

static void pm_unregistersong (const void *handle)
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

static void pm_pause (void)
{
  int i;
  unsigned long when = Pt_Time ();
  pm_paused = 1;
  for (i = 0; i < 16; i++)
  {
    writeevent (when, MIDI_EVENT_CONTROLLER, i, 123, 0); // all notes off
  }
}

static void pm_resume (void)
{
  pm_paused = 0;
  trackstart = Pt_Time ();
}

static void pm_play (const void *handle, int looping)
{
  eventpos = 0;
  pm_looping = looping;
  pm_playing = 1;
  //pm_paused = 0;
  pm_delta = 0.0;
  clear_mastervol();
  if (!firsttime) // set pm_volume first, see pm_setvolume()
  {
    refresh_mastervol();
  }
  trackstart = Pt_Time ();
}

static dboolean is_mastervol (unsigned char *data, int len)
{
  unsigned char msg[] = {0xf0, 0x7f, 0x7f, 0x04, 0x01, 0x00, 0x00, 0xf7};
  return (len == 8 && !memcmp(data, msg, 5));
}

static dboolean is_sysex_reset (unsigned char *data)
{
  return (!memcmp(data, gs_reset, sizeof(gs_reset))
          || !memcmp(data, gm_system_on, sizeof(gm_system_on))
          || !memcmp(data, gm2_system_on, sizeof(gm2_system_on))
          || !memcmp(data, xg_system_on, sizeof(xg_system_on)));
}

static void writesysex (unsigned long when, int etype, unsigned char *data, int len)
{
  // sysex code is untested
  // it's possible to use an auto-resizing buffer here, but a malformed
  // midi file could make it grow arbitrarily large (since it must grow
  // until it hits an 0xf7 terminator)
  if (len + sysexbufflen > SYSEX_BUFF_SIZE - 1)
  {
    lprintf (LO_WARN, "portmidiplayer: ignoring large or malformed sysex message\n");
    sysexbufflen = 0;
    return;
  }
  memcpy (sysexbuff + sysexbufflen, data, len);
  sysexbufflen += len;
  if (sysexbuff[sysexbufflen - 1] == 0xf7) // terminator
  {
    memmove(&sysexbuff[1], &sysexbuff[0], sysexbufflen * sizeof(*sysexbuff));
    sysexbuff[0] = 0xf0; // start of exclusive (SOX) in front
    sysexbufflen++;

    if (is_mastervol(sysexbuff, sysexbufflen))
    {
      // master volume message from midi file, scale by volume slider
      mastervol = sysexbuff[6] << 7 | sysexbuff[5]; // back to 14-bit
      set_mastervol(when);
      sysexbufflen = 0;
      return;
    }

    Pm_WriteSysEx (pm_stream, when, sysexbuff);

    if (is_sysex_reset(sysexbuff))
    {
      use_reset_delay = mus_portmidi_reset_delay > 0;

      // sysex reset from midi file, reapply master volume
      clear_mastervol();
      set_mastervol(when);
    }

    sysexbufflen = 0;
  }
}

static void pm_stop (void)
{
  unsigned long when = Pt_Time ();
  pm_playing = 0;

  // songs can be stopped at any time, so reset everything
  reset_device(when);
  // abort any partial sysex
  sysexbufflen = 0;
}

static void pm_render (void *vdest, unsigned bufflen)
{
  // wherever you see samples in here, think milliseconds
  unsigned long newtime = Pt_Time ();
  unsigned long length = newtime - trackstart;

  //timerpos = newtime;
  unsigned long when;

  midi_event_t *currevent;

  unsigned sampleswritten = 0;
  unsigned samples;

  memset (vdest, 0, bufflen * 4);

  if (!pm_playing || pm_paused)
    return;

  while (1)
  {
    double eventdelta;
    currevent = events[eventpos];

    if (use_reset_delay)
    {
      // delay after reset, for real devices only (e.g. roland sc-55)
      currevent->delta_time += mus_portmidi_reset_delay / spmc;
      use_reset_delay = false;
    }

    // how many samples away event is
    eventdelta = currevent->delta_time * spmc;

    // how many we will render (rounding down); include delta offset
    samples = (unsigned) (eventdelta + pm_delta);

    if (samples + sampleswritten > length)
    { // overshoot; render some samples without processing an event
      break;
    }

    sampleswritten += samples;
    pm_delta -= samples;

    // process event
    when = trackstart + sampleswritten;
    switch (currevent->event_type)
    {
      case MIDI_EVENT_SYSEX:
      case MIDI_EVENT_SYSEX_SPLIT:
        writesysex (when, currevent->event_type, currevent->data.sysex.data, currevent->data.sysex.length);
        break;
      case MIDI_EVENT_META: // tempo is the only meta message we're interested in
        if (currevent->data.meta.type == MIDI_META_SET_TEMPO)
          spmc = MIDI_spmc (midifile, currevent, 1000);
        else if (currevent->data.meta.type == MIDI_META_END_OF_TRACK)
        {
          if (pm_looping)
          {
            int i;
            eventpos = 0;
            pm_delta += eventdelta;
            // fix buggy songs that forget to terminate notes held over loop point
            // sdl_mixer does this as well
            for (i = 0; i < 16; i++)
              writeevent (when, MIDI_EVENT_CONTROLLER, i, 123, 0); // all notes off
            continue;
          }
          // stop
          pm_stop ();
          return;
        }
        break; // not interested in most metas
      default:
        writeevent (when, currevent->event_type, currevent->data.channel.channel, currevent->data.channel.param1, currevent->data.channel.param2);
        break;

    }

    // event processed so advance midiclock
    pm_delta += eventdelta;
    eventpos++;

  }

  if (samples + sampleswritten > length)
  { // broke due to next event being past the end of current render buffer
    // finish buffer, return
    samples = length - sampleswritten;
    pm_delta -= samples; // save offset
  }

  trackstart = newtime;
}

const music_player_t pm_player =
{
  pm_name,
  pm_init,
  pm_shutdown,
  pm_setvolume,
  pm_pause,
  pm_resume,
  pm_registersong,
  pm_unregistersong,
  pm_play,
  pm_stop,
  pm_render
};

#endif // HAVE_LIBPORTMIDI
