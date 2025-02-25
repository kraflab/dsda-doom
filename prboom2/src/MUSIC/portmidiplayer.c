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
static int pm_volume = -1;
static double spmc;
static double pm_delta;

static unsigned long trackstart;

static PortMidiStream *pm_stream;

#define SYSEX_BUFF_SIZE 1024
static byte sysexbuff[SYSEX_BUFF_SIZE];
static int sysexbufflen;

static const char *mus_portmidi_reset_type; // portmidi reset type
static int mus_portmidi_reset_delay; // portmidi delay after reset
static int mus_portmidi_filter_sysex; // portmidi block sysex from midi files
static int mus_portmidi_reverb_level; // portmidi reverb send level
static int mus_portmidi_chorus_level; // portmidi chorus send level

// latency: we're generally writing timestamps slightly in the past (from when the last time
// render was called to this time.  portmidi latency instruction must be larger than that window
// so the messages appear in the future.  ~46-47ms is the nominal length if i_sound.c gets its way
#define DRIVER_LATENCY 80 // ms
// driver event buffer needs to be big enough to hold however many events occur in latency time
#define DRIVER_BUFFER 1024 // events

static const char *pm_name (void)
{
  return "portmidi midi player";
}

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <delayimp.h>
#endif

static dboolean channel_used[16];

#define DEFAULT_VOLUME 100
static int channel_volume[16];
static float volume_scale;

static dboolean use_reset_delay;
static byte *sysex_reset;
static byte gs_reset[] = {0xf0, 0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7f, 0x00, 0x41, 0xf7};
static byte gm_system_on[] = {0xf0, 0x7e, 0x7f, 0x09, 0x01, 0xf7};
static byte gm2_system_on[] = {0xf0, 0x7e, 0x7f, 0x09, 0x03, 0xf7};
static byte xg_system_on[] = {0xf0, 0x43, 0x10, 0x4c, 0x00, 0x00, 0x7e, 0x00, 0xf7};
static PmEvent event_notes_off[16];
static PmEvent event_sound_off[16];
static PmEvent event_reset[16 * 6];
static PmEvent event_pbs[16 * 6];
static PmEvent event_reverb[16];
static PmEvent event_chorus[16];

static void reset_device (void)
{
  Pm_Write(pm_stream, event_notes_off, 16);
  Pm_Write(pm_stream, event_sound_off, 16);

  if (sysex_reset == NULL)
    Pm_Write(pm_stream, event_reset, 16 * 6);
  else
    Pm_WriteSysEx(pm_stream, 0, sysex_reset);

  Pm_Write(pm_stream, event_pbs, 16 * 6);

  if (mus_portmidi_reverb_level > -1 || sysex_reset == NULL)
    Pm_Write(pm_stream, event_reverb, 16);

  if (mus_portmidi_chorus_level > -1 || sysex_reset == NULL)
    Pm_Write(pm_stream, event_chorus, 16);

  use_reset_delay = mus_portmidi_reset_delay > 0;
  memset(channel_used, 0, sizeof(channel_used));
}

static void init_reset_buffer (void)
{
  int i;
  PmEvent *reset = event_reset;
  PmEvent *pbs = event_pbs;
  int reverb = mus_portmidi_reverb_level;
  int chorus = mus_portmidi_chorus_level;

  for (i = 0; i < 16; ++i)
  {
    event_notes_off[i].message = Pm_Message(0xB0 | i, 0x7B, 0x00);
    event_sound_off[i].message = Pm_Message(0xB0 | i, 0x78, 0x00);

    reset[0].message = Pm_Message(0xB0 | i, 0x79, 0x00); // reset all controllers
    reset[1].message = Pm_Message(0xB0 | i, 0x07, 0x64); // channel volume
    reset[2].message = Pm_Message(0xB0 | i, 0x0A, 0x40); // pan
    reset[3].message = Pm_Message(0xB0 | i, 0x00, 0x00); // bank select msb
    reset[4].message = Pm_Message(0xB0 | i, 0x20, 0x00); // bank select lsb
    reset[5].message = Pm_Message(0xC0 | i, 0x00, 0x00); // program change
    reset += 6;

    pbs[0].message = Pm_Message(0xB0 | i, 0x64, 0x00); // pitch bend sens RPN LSB
    pbs[1].message = Pm_Message(0xB0 | i, 0x65, 0x00); // pitch bend sens RPN MSB
    pbs[2].message = Pm_Message(0xB0 | i, 0x06, 0x02); // data entry MSB
    pbs[3].message = Pm_Message(0xB0 | i, 0x26, 0x00); // data entry LSB
    pbs[4].message = Pm_Message(0xB0 | i, 0x64, 0x7F); // null RPN LSB
    pbs[5].message = Pm_Message(0xB0 | i, 0x65, 0x7F); // null RPN MSB
    pbs += 6;
  }

  if (!strcasecmp(mus_portmidi_reset_type, "gs"))
    sysex_reset = gs_reset;
  else if (!strcasecmp(mus_portmidi_reset_type, "gm"))
    sysex_reset = gm_system_on;
  else if (!strcasecmp(mus_portmidi_reset_type, "gm2"))
    sysex_reset = gm2_system_on;
  else if (!strcasecmp(mus_portmidi_reset_type, "xg"))
    sysex_reset = xg_system_on;
  else
    sysex_reset = NULL;

  // if no reverb specified and no SysEx reset selected, then use GM default
  if (reverb == -1 && sysex_reset == NULL)
    reverb = 40;

  if (reverb > -1)
  {
    for (i = 0; i < 16; ++i)
      event_reverb[i].message = Pm_Message(0xB0 | i, 0x5B, reverb);
  }

  // if no chorus specified and no SysEx reset selected, then use GM default
  if (chorus == -1 && sysex_reset == NULL)
    chorus = 0;

  if (chorus > -1)
  {
    for (i = 0; i < 16; ++i)
      event_chorus[i].message = Pm_Message(0xB0 | i, 0x5D, chorus);
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
  mus_portmidi_filter_sysex = dsda_IntConfig(dsda_config_mus_portmidi_filter_sysex);
  mus_portmidi_reverb_level = dsda_IntConfig(dsda_config_mus_portmidi_reverb_level);
  mus_portmidi_chorus_level = dsda_IntConfig(dsda_config_mus_portmidi_chorus_level);

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
  reset_device();

  for (int i = 0; i < 16; i++)
    channel_volume[i] = DEFAULT_VOLUME;

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

static void write_volume (unsigned long when, int channel, int volume)
{
  int vol = volume * volume_scale + 0.5f;
  writeevent (when, MIDI_EVENT_CONTROLLER, channel, MIDI_CONTROLLER_MAIN_VOLUME, vol);
  channel_volume[channel] = volume;
}

static void update_volume (void)
{
  for (int i = 0; i < 16; i++)
    write_volume (0, i, channel_volume[i]);
}

static void reset_volume (void)
{
  for (int i = 0; i < 16; i++)
    write_volume (0, i, DEFAULT_VOLUME);
}

static void pm_setvolume (int v)
{
  if (pm_volume == v)
    return;

  pm_volume = v;
  volume_scale = pm_volume / 15.0f;
  update_volume();
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
  pm_paused = 1;
  Pm_Write(pm_stream, event_notes_off, 16);
  Pm_Write(pm_stream, event_sound_off, 16);
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
  pm_delta = 0.0;
  if (pm_volume != -1) // set pm_volume first, see pm_setvolume()
    reset_volume();
  trackstart = Pt_Time ();
}

static dboolean is_sysex_reset (byte *msg, int len)
{
  if (len < 6)
    return false;

  switch (msg[1])
  {
    case 0x41: // roland
      switch (msg[3])
      {
        case 0x42: // gs
          switch (msg[4])
          {
            case 0x12: // dt1
              if (len == 11 &&
                  msg[5] == 0x00 &&  // address msb
                  msg[6] == 0x00 &&  // address
                  msg[7] == 0x7F &&  // address lsb
                ((msg[8] == 0x00 &&  // data     (mode-1)
                  msg[9] == 0x01) || // checksum (mode-1)
                 (msg[8] == 0x01 &&  // data     (mode-2)
                  msg[9] == 0x00)))  // checksum (mode-2)
              {
                // sc-88 system mode set
                // F0 41 <dev> 42 12 00 00 7F 00 01 F7 (mode-1)
                // F0 41 <dev> 42 12 00 00 7F 01 00 F7 (mode-2)
                return true;
              }
              else if (len == 11 &&
                       msg[5] == 0x40 && // address msb
                       msg[6] == 0x00 && // address
                       msg[7] == 0x7F && // address lsb
                       msg[8] == 0x00 && // data (gs reset)
                       msg[9] == 0x41)   // checksum
              {
                // gs reset
                // F0 41 <dev> 42 12 40 00 7F 00 41 F7
                return true;
              }
              break;
          }
          break;
      }
      break;

    case 0x43: // yamaha
      switch (msg[3])
      {
        case 0x2B: // tg300
          if (len == 10 &&
              msg[4] == 0x00 && // start address b20 - b14
              msg[5] == 0x00 && // start address b13 - b7
              msg[6] == 0x7F && // start address b6 - b0
              msg[7] == 0x00 && // data
              msg[8] == 0x01)   // checksum
          {
            // tg300 all parameter reset
            // F0 43 <dev> 2B 00 00 7F 00 01 F7
            return true;
          }
          break;

        case 0x4C: // xg
          if (len == 9 &&
              msg[4] == 0x00 &&  // address high
              msg[5] == 0x00 &&  // address mid
             (msg[6] == 0x7E ||  // address low (xg system on)
              msg[6] == 0x7F) && // address low (xg all parameter reset)
              msg[7] == 0x00)    // data
          {
            // xg system on, xg all parameter reset
            // F0 43 <dev> 4C 00 00 7E 00 F7
            // F0 43 <dev> 4C 00 00 7F 00 F7
            return true;
          }
          break;
      }
      break;

    case 0x7E: // universal non-real time
      switch (msg[3])
      {
        case 0x09: // general midi
          if (len == 6 &&
             (msg[4] == 0x01 || // gm system on
              msg[4] == 0x02 || // gm system off
              msg[4] == 0x03))  // gm2 system on
          {
            // gm system on/off, gm2 system on
            // F0 7E <dev> 09 01 F7
            // F0 7E <dev> 09 02 F7
            // F0 7E <dev> 09 03 F7
            return true;
          }
          break;
      }
      break;
  }

  return false;
}

static void writesysex (unsigned long when, int etype, byte *data, int len)
{
  // sysex messages in midi files (smf 1.0 pages 6-7):
  // complete:        (F0 ... F7)
  // multi-packet:    (F0 ...) + (F7 ...) + ... + (F7 ... F7)
  // escape sequence: (F7 ...)

  if (len + sysexbufflen > SYSEX_BUFF_SIZE - 1)
  {
    // ignore messages that are too long
    sysexbufflen = 0;
    return;
  }

  if (etype == MIDI_EVENT_SYSEX_SPLIT && sysexbufflen == 0)
  {
    // ignore escape sequence
    return;
  }

  if (etype == MIDI_EVENT_SYSEX)
  {
    // start a new message (discards any previous incomplete message)
    sysexbuff[0] = MIDI_EVENT_SYSEX;
    sysexbufflen = 1;
  }

  memcpy (sysexbuff + sysexbufflen, data, len);
  sysexbufflen += len;

  // process message if it's complete, otherwise do nothing yet
  if (sysexbuff[sysexbufflen - 1] == MIDI_EVENT_SYSEX_SPLIT)
  {
    Pm_WriteSysEx (pm_stream, when, sysexbuff);

    if (is_sysex_reset(sysexbuff, sysexbufflen))
    {
      reset_volume();
      memset(channel_used, 0, sizeof(channel_used));
    }

    sysexbufflen = 0;
  }
}

static void pm_stop (void)
{
  pm_playing = 0;

  // songs can be stopped at any time, so reset everything
  reset_device();

  // abort any partial sysex
  sysexbufflen = 0;
}

static void pm_render (void *vdest, unsigned bufflen)
{
  // wherever you see samples in here, think milliseconds
  unsigned long when = trackstart;
  unsigned long newtime = Pt_Time ();
  unsigned int samples;

  memset (vdest, 0, bufflen * 4);

  if (!pm_playing || pm_paused)
    return;

  while (1)
  {
    midi_event_t *currevent = events[eventpos];

    // how many samples away event is
    double eventdelta = currevent->delta_time * spmc;

    // delay after reset, for real devices only (e.g. roland sc-55)
    if (use_reset_delay)
      eventdelta += mus_portmidi_reset_delay;

    // how many we will render (rounding down); include delta offset
    samples = eventdelta + pm_delta;

    if (when + samples > newtime)
    {
      // overshoot; render some samples without processing an event
      pm_delta -= (newtime - when); // save offset
      trackstart = newtime;
      return;
    }

    use_reset_delay = false;
    pm_delta += eventdelta - samples;
    when += samples;

    switch (currevent->event_type)
    {
      case MIDI_EVENT_SYSEX:
      case MIDI_EVENT_SYSEX_SPLIT:
        if (!mus_portmidi_filter_sysex)
          writesysex (when, currevent->event_type, currevent->data.sysex.data, currevent->data.sysex.length);
        break;
      case MIDI_EVENT_META:
        switch (currevent->data.meta.type)
        {
          case MIDI_META_SET_TEMPO:
            spmc = MIDI_spmc (midifile, currevent, 1000);
            break;
          case MIDI_META_END_OF_TRACK:
            if (pm_looping)
            {
              eventpos = 0;
              // prevent hanging notes (doom2.wad MAP14, MAP22)
              for (int i = 0; i < 16; i++)
              {
                if (channel_used[i])
                {
                  writeevent(when, 0xB0, i, 0x79, 0x00); // reset all controllers
                  write_volume(when, i, DEFAULT_VOLUME); // reset volume
                  channel_used[i] = false;
                }
              }
              continue;
            }
            pm_stop();
            return;
        }
        break; // not interested in most metas
      case MIDI_EVENT_CONTROLLER:
        if (currevent->data.channel.param1 == MIDI_CONTROLLER_MAIN_VOLUME)
        {
          write_volume (when, currevent->data.channel.channel, currevent->data.channel.param2);
          channel_used[currevent->data.channel.channel] = true;
          break;
        }
        else if (currevent->data.channel.param1 == 0x79)
        {
          // ms gs synth resets volume if "reset all controllers" value isn't zero
          writeevent (when, 0xB0, currevent->data.channel.channel, 0x79, 0x00);
          channel_used[currevent->data.channel.channel] = true;
          break;
        }
        // fall through
      default:
        writeevent (when, currevent->event_type, currevent->data.channel.channel, currevent->data.channel.param1, currevent->data.channel.param2);
        channel_used[currevent->data.channel.channel] = true;
        break;
    }

    eventpos++;
  }
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
