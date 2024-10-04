/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
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
 *  System interface for sound.
 *
 *-----------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <math.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "SDL.h"
#include "SDL_audio.h"
#include "SDL_mutex.h"

#include "SDL_endian.h"

#include "SDL_version.h"
#include "SDL_thread.h"
#define USE_RWOPS
#include "SDL_mixer.h"

#include "z_zone.h"

#include "m_swap.h"
#include "i_sound.h"
#include "m_misc.h"
#include "w_wad.h"
#include "lprintf.h"
#include "s_sound.h"

#include "doomdef.h"
#include "doomstat.h"
#include "doomtype.h"

#include "d_main.h"
#include "i_system.h"

//e6y
#include "e6y.h"

#include "dsda/settings.h"

static dboolean registered_non_rw = false;

// The number of internal mixing channels,
//  the samples calculated for each mixing step,
//  the size of the 16bit, 2 hardware channel (stereo)
//  mixing buffer, and the samplerate of the raw data.

// The actual output device.
int audio_fd;

typedef struct
{
  // SFX id of the playing sound effect.
  // Used to catch duplicates (like chainsaw).
  int id;
  // The channel step amount...
  unsigned int step;
  // ... and a 0.16 bit remainder of last step.
  unsigned int stepremainder;
  unsigned int samplerate;
  unsigned int bits;
  // The channel data pointers, start and end.
  const unsigned char *data;
  const unsigned char *startdata;
  const unsigned char *enddata;
  // Time/gametic that the channel started playing,
  //  used to determine oldest, which automatically
  //  has lowest priority.
  // In case number of active sounds exceeds
  //  available channels.
  int starttime;
  // left and right channel volume (0-127)
  int leftvol;
  int rightvol;
  dboolean loop;
} channel_info_t;

channel_info_t channelinfo[MAX_CHANNELS];

// Pitch to stepping lookup.
int   steptable[256];

// Volume lookups.
//int   vol_lookup[128 * 256];

// NSM
static int dumping_sound = 0;


// lock for updating any params related to sfx
SDL_mutex *sfxmutex;
// lock for updating any params related to music
SDL_mutex *musmutex;

static int pitched_sounds;
int snd_samplerate; // samples per second
static int snd_samplecount;

static const char *snd_midiplayer;

void I_InitSoundParams(void)
{
  pitched_sounds = dsda_IntConfig(dsda_config_pitched_sounds);

  // TODO: can we reinitialize sound with new sample rate / count?
  if (!snd_samplerate)
    snd_samplerate = dsda_IntConfig(dsda_config_snd_samplerate);
  if (!snd_samplecount)
    snd_samplecount = dsda_IntConfig(dsda_config_snd_samplecount);
}

/* cph
 * stopchan
 * Stops a sound, unlocks the data
 */

static void stopchan(int i)
{
  if (channelinfo[i].data) /* cph - prevent excess unlocks */
  {
    channelinfo[i].data = NULL;
  }
}

typedef struct wav_data_s
{
  int sfxid;
  const unsigned char *data;
  int samplelen;
  int samplerate;
  int bits;
  struct wav_data_s *next;
} wav_data_t;

#define WAV_DATA_HASH_SIZE 32
static wav_data_t *wav_data_hash[WAV_DATA_HASH_SIZE];

static wav_data_t *GetWavData(int sfxid, const unsigned char *data, size_t len)
{
  int key;
  wav_data_t *target = NULL;

  key = (sfxid % WAV_DATA_HASH_SIZE);

  if (wav_data_hash[key])
  {
    wav_data_t *rover = wav_data_hash[key];

    while (rover)
    {
      if (rover->sfxid == sfxid)
      {
        target = rover;
        break;
      }

      rover = rover->next;
    }
  }

  if (target == NULL &&
      len > 44 && !memcmp(data, "RIFF", 4) && !memcmp(data + 8, "WAVEfmt ", 8))
  {
    SDL_RWops *RWops;
    SDL_AudioSpec wav_spec;
    Uint8 *wav_buffer = NULL;
    int bits, samplelen;

    RWops = SDL_RWFromConstMem(data, len);

    if (SDL_LoadWAV_RW(RWops, 1, &wav_spec, &wav_buffer, &samplelen) == NULL)
    {
      lprintf(LO_WARN, "Could not open wav file: %s\n", SDL_GetError());
      return NULL;
    }

    if (wav_spec.channels != 1)
    {
      lprintf(LO_WARN, "Only mono WAV file is supported");
      SDL_FreeWAV(wav_buffer);
      return NULL;
    }

    if (!SDL_AUDIO_ISINT(wav_spec.format))
    {
      lprintf(LO_WARN, "WAV file in unsupported format");
      SDL_FreeWAV(wav_buffer);
      return NULL;
    }

    bits = SDL_AUDIO_BITSIZE(wav_spec.format);
    if (bits != 8 && bits != 16)
    {
      lprintf(LO_WARN, "Only 8 or 16 bit WAV files are supported");
      SDL_FreeWAV(wav_buffer);
      return NULL;
    }

    target = Z_Malloc(sizeof(*target));

    target->sfxid = sfxid;
    target->data = wav_buffer;
    target->samplelen = samplelen;
    target->samplerate = wav_spec.freq;
    target->bits = bits;

    // use head insertion
    target->next = wav_data_hash[key];
    wav_data_hash[key] = target;
  }

  return target;
}

//
// This function adds a sound to the
//  list of currently active sounds,
//  which is maintained as a given number
//  (eight, usually) of internal channels.
// Returns a handle.
//
static int addsfx(int sfxid, int channel, const unsigned char *data, size_t len)
{
  channel_info_t *ci = channelinfo + channel;
  wav_data_t *wav_data = GetWavData(sfxid, data, len);

  stopchan(channel);

  if (wav_data)
  {
    ci->data = wav_data->data;
    ci->enddata = ci->data + wav_data->samplelen - 1;
    ci->samplerate = wav_data->samplerate;
    ci->bits = wav_data->bits;
  }
  else
  {
    ci->data = data;
    /* Set pointer to end of raw data. */
    ci->enddata = ci->data + len - 1;
    ci->samplerate = (ci->data[3] << 8) + ci->data[2];
    ci->data += 8; /* Skip header */
    ci->bits = 8;
  }

  ci->stepremainder = 0;
  // Should be gametic, I presume.
  ci->starttime = gametic;

  ci->startdata = ci->data;

  // Preserve sound SFX id,
  //  e.g. for avoiding duplicates of chainsaw.
  ci->id = sfxid;

  return channel;
}

static int getSliceSize(void)
{
  int limit, n;

  if (snd_samplecount >= 32)
    return snd_samplecount * snd_samplerate / 11025;

  limit = snd_samplerate / TICRATE;

  // Try all powers of two, not exceeding the limit.

  for (n = 0; ; ++n)
  {
    // 2^n <= limit < 2^n+1 ?

    if ((1 << (n + 1)) > limit)
    {
      return (1 << n);
    }
  }

  // Should never happen?

  return 1024;
}

static void updateSoundParams(int handle, sfx_params_t *params)
{
  int slot = handle;
  int rightvol;
  int leftvol;

#ifdef RANGECHECK
  if ((handle < 0) || (handle >= MAX_CHANNELS))
    I_Error("I_UpdateSoundParams: handle out of range");
#endif

  channelinfo[slot].loop = params->loop;

  // Set stepping
  // MWM 2000-12-24: Calculates proportion of channel samplerate
  // to global samplerate for mixing purposes.
  // Patched to shift left *then* divide, to minimize roundoff errors
  // as well as to use SAMPLERATE as defined above, not to assume 11025 Hz
  if (pitched_sounds)
    channelinfo[slot].step = (unsigned int)(((uint64_t)channelinfo[slot].samplerate * steptable[params->pitch]) / snd_samplerate);
  else
    channelinfo[slot].step = ((channelinfo[slot].samplerate << 16) / snd_samplerate);

  // Separation, that is, orientation/stereo.
  //  range is: 1 - 256
  params->separation += 1;

  // Per left/right channel.
  //  x^2 separation,
  //  adjust volume properly.
  leftvol = params->volume - ((params->volume * params->separation * params->separation) >> 16);
  params->separation = params->separation - 257;
  rightvol = params->volume - ((params->volume * params->separation * params->separation) >> 16);

  // Sanity check, clamp volume.
  if (rightvol < 0 || rightvol > 127)
  {
    rightvol = rightvol < 0 ? 0 : 127;
    lprintf(LO_WARN, "rightvol out of bounds\n");
  }

  if (leftvol < 0 || leftvol > 127)
  {
    leftvol = leftvol < 0 ? 0 : 127;
    lprintf(LO_WARN, "leftvol out of bounds\n");
  }

  // Get the proper lookup table piece
  //  for this volume level???
  channelinfo[slot].leftvol = leftvol;
  channelinfo[slot].rightvol = rightvol;
}

void I_UpdateSoundParams(int handle, sfx_params_t *params)
{
  SDL_LockMutex (sfxmutex);
  updateSoundParams(handle, params);
  SDL_UnlockMutex (sfxmutex);
}

//
// SFX API
// Note: this was called by S_Init.
// However, whatever they did in the
// old DPMS based DOS version, this
// were simply dummies in the Linux
// version.
// See soundserver initdata().
//
void I_SetChannels(void)
{
  // Init internal lookups (raw data, mixing buffer, channels).
  // This function sets up internal lookups used during
  //  the mixing process.
  int   i;
  //int   j;

  int  *steptablemid = steptable + 128;

  // Okay, reset internal mixing channels to zero.
  for (i = 0; i < MAX_CHANNELS; i++)
  {
    memset(&channelinfo[i], 0, sizeof(channel_info_t));
  }

  // This table provides step widths for pitch parameters.
  for (i = -128 ; i < 128 ; i++)
    steptablemid[i] = (int)(pow(1.2, (double)i / 64.0) * 65536.0);


  // Generates volume lookup tables
  //  which also turn the unsigned samples
  //  into signed samples.
  /*
  for (i = 0 ; i < 128 ; i++)
    for (j = 0 ; j < 256 ; j++)
    {
      // proff - made this a little bit softer, because with
      // full volume the sound clipped badly
      vol_lookup[i * 256 + j] = (i * (j - 128) * 256) / 191;
      //vol_lookup[i*256+j] = (i*(j-128)*256)/127;
    }
  */
}

//
// Retrieve the raw data lump index
//  for a given SFX name.
//
int I_GetSfxLumpNum(sfxinfo_t *sfx)
{
  if (sfx->link)
    sfx = sfx->link;

  if (!sfx->name)
    return LUMP_NOT_FOUND;

  return W_CheckNumForName(sfx->name); //e6y: make missing sounds non-fatal
}

//
// Starting a sound means adding it
//  to the current list of active sounds
//  in the internal channels.
// As the SFX info struct contains
//  e.g. a pointer to the raw data,
//  it is ignored.
// As our sound handling does not handle
//  priority, it is ignored.
// Pitching (that is, increased speed of playback)
//  is set, but currently not used by mixing.
//
int I_StartSound(int id, int channel, sfx_params_t *params)
{
  const unsigned char *data;
  int lump;
  size_t len;

  if ((channel < 0) || (channel >= MAX_CHANNELS))
#ifdef RANGECHECK
    I_Error("I_StartSound: handle out of range");
#else
    return -1;
#endif

  lump = S_sfx[id].lumpnum;

  // We will handle the new SFX.
  // Set pointer to raw data.
  len = W_LumpLength(lump);

  // e6y: Crash with zero-length sounds.
  // Example wad: dakills (http://www.doomworld.com/idgames/index.php?id=2803)
  // The entries DSBSPWLK, DSBSPACT, DSSWTCHN and DSSWTCHX are all zero-length sounds
  if (len <= 8) return -1;

  /* Find padded length */
  len -= 8;
  // do the lump caching outside the SDL_LockAudio/SDL_UnlockAudio pair
  // use locking which makes sure the sound data is in a malloced area and
  // not in a memory mapped one
  data = (const unsigned char *)W_LockLumpNum(lump);

  SDL_LockMutex (sfxmutex);

  // Returns a handle (not used).
  addsfx(id, channel, data, len);
  updateSoundParams(channel, params);

  SDL_UnlockMutex (sfxmutex);


  return channel;
}



void I_StopSound (int handle)
{
#ifdef RANGECHECK
  if ((handle < 0) || (handle >= MAX_CHANNELS))
    I_Error("I_StopSound: handle out of range");
#endif

  SDL_LockMutex (sfxmutex);
  stopchan(handle);
  SDL_UnlockMutex (sfxmutex);
}


dboolean I_SoundIsPlaying(int handle)
{
#ifdef RANGECHECK
  if ((handle < 0) || (handle >= MAX_CHANNELS))
    I_Error("I_SoundIsPlaying: handle out of range");
#endif

  return channelinfo[handle].data != NULL;
}


dboolean I_AnySoundStillPlaying(void)
{
  dboolean result = false;
  int i;

  for (i = 0; i < MAX_CHANNELS; i++)
    result |= channelinfo[i].data != NULL;

  return result;
}


//
// This function loops all active (internal) sound
//  channels, retrieves a given number of samples
//  from the raw sound data, modifies it according
//  to the current (internal) channel parameters,
//  mixes the per channel samples into the given
//  mixing buffer, and clamping it to the allowed
//  range.
//
// This function currently supports only 16bit.
//

static void UpdateMusic (void *buff, unsigned nsamp);

static void I_UpdateSound(void *unused, Uint8 *stream, int len)
{
  // Mix current sound data.
  // Data, from raw sound, for right and left.
  // register unsigned char sample;
  register int    dl;
  register int    dr;

  // Pointers in audio stream, left, right, end.
  signed short   *leftout;
  signed short   *rightout;
  signed short   *leftend;
  // Step in stream, left and right, thus two.
  int       step;

  // Mixing channel index.
  int       chan;

  if (snd_midiplayer == NULL) // This is but a temporary fix. Please do remove after a more definitive one!
    memset(stream, 0, len);

  // NSM: when dumping sound, ignore the callback calls and only
  // service dumping calls
  if (dumping_sound && unused != (void *) 0xdeadbeef)
    return;

  // do music update
  if (registered_non_rw)
  {
    SDL_LockMutex (musmutex);
    UpdateMusic (stream, len / 4);
    SDL_UnlockMutex (musmutex);
  }

  SDL_LockMutex (sfxmutex);
  // Left and right channel
  //  are in audio stream, alternating.
  leftout = (signed short *)stream;
  rightout = ((signed short *)stream) + 1;
  step = 2;

  // Determine end, for left channel only
  //  (right channel is implicit).
  leftend = leftout + (len / 4) * step;

  // Mix sounds into the mixing buffer.
  // Loop over step*SAMPLECOUNT,
  //  that is 512 values for two channels.
  while (leftout != leftend)
  {
    // Reset left/right value.
    //dl = 0;
    //dr = 0;
    dl = *leftout;
    dr = *rightout;

    // Love thy L2 chache - made this a loop.
    // Now more channels could be set at compile time
    //  as well. Thus loop those  channels.
    for ( chan = 0; chan < numChannels; chan++ )
    {
      channel_info_t *ci = channelinfo + chan;

      // Check channel, if active.
      if (ci->data)
      {
        int s;
        // Get the raw data from the channel.
        // no filtering
        //int s = channelinfo[chan].data[0] * 0x10000 - 0x800000;

        // linear filtering
        // the old SRC did linear interpolation back into 8 bit, and then expanded to 16 bit.
        // this does interpolation and 8->16 at same time, allowing slightly higher quality
        if (ci->bits == 16)
        {
          s = (short)(ci->data[0] | (ci->data[1] << 8)) * (255 - (ci->stepremainder >> 8))
            + (short)(ci->data[2] | (ci->data[3] << 8)) * (ci->stepremainder >> 8);
        }
        else
        {
          s = ((unsigned int)ci->data[0] * (0x10000 - ci->stepremainder))
            + ((unsigned int)ci->data[1] * (ci->stepremainder))
            - 0x800000; // convert to signed
        }


        // Add left and right part
        //  for this channel (sound)
        //  to the current data.
        // Adjust volume accordingly.

        // full loudness (vol=127) is actually 127/191

        dl += ci->leftvol * s / 49152;  // >> 15;
        dr += ci->rightvol * s / 49152; // >> 15;

        // Increment index ???
        ci->stepremainder += ci->step;

        // MSB is next sample???
        if (ci->bits == 16)
          ci->data += (ci->stepremainder >> 16) * 2;
        else
          ci->data += ci->stepremainder >> 16;

        // Limit to LSB???
        ci->stepremainder &= 0xffff;

        // Check whether we are done.
        if (ci->data >= ci->enddata)
        {
          if (ci->loop)
            ci->data = ci->startdata;
          else
            stopchan(chan);
        }
      }
    }

    // Clamp to range. Left hardware channel.
    // Has been char instead of short.
    // if (dl > 127) *leftout = 127;
    // else if (dl < -128) *leftout = -128;
    // else *leftout = dl;

    if (dl > SHRT_MAX)
      *leftout = SHRT_MAX;
    else if (dl < SHRT_MIN)
      *leftout = SHRT_MIN;
    else
      *leftout = (signed short)dl;

    // Same for right hardware channel.
    if (dr > SHRT_MAX)
      *rightout = SHRT_MAX;
    else if (dr < SHRT_MIN)
      *rightout = SHRT_MIN;
    else
      *rightout = (signed short)dr;

    // Increment current pointers in stream
    leftout += step;
    rightout += step;
  }
  SDL_UnlockMutex (sfxmutex);
}

static dboolean sound_was_initialized;

void I_ShutdownSound(void)
{
  if (sound_was_initialized)
  {
    Mix_CloseAudio();
    SDL_CloseAudio();

    sound_was_initialized = false;

    if (sfxmutex)
    {
      SDL_DestroyMutex (sfxmutex);
      sfxmutex = NULL;
    }
  }
}

void I_InitSound(void)
{
  int audio_rate;
  int audio_channels;
  int audio_buffers;

  if (sound_was_initialized || (nomusicparm && nosfxparm))
    return;

  if (SDL_InitSubSystem(SDL_INIT_AUDIO))
  {
    lprintf(LO_WARN, "Couldn't initialize SDL audio (%s))\n", SDL_GetError());
    nosfxparm = true;
    nomusicparm = true;
    return;
  }

  // Secure and configure sound device first.
  lprintf(LO_DEBUG, "I_InitSound: ");

  I_InitSoundParams();

  audio_rate = snd_samplerate;
  audio_channels = 2;
  audio_buffers = getSliceSize();

  if (Mix_OpenAudioDevice(audio_rate, MIX_DEFAULT_FORMAT, audio_channels, audio_buffers,
                          NULL, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE) < 0)
  {
    lprintf(LO_DEBUG, "couldn't open audio with desired format (%s)\n", SDL_GetError());
    nosfxparm = true;
    nomusicparm = true;
    return;
  }

  // [FG] feed actual sample frequency back into config variable
  Mix_QuerySpec(&snd_samplerate, NULL, NULL);

  sound_was_initialized = true;

  Mix_SetPostMix(I_UpdateSound, NULL);

  lprintf(LO_DEBUG, " configured audio device with %d samples/slice\n", audio_buffers);

  I_AtExit(I_ShutdownSound, true, "I_ShutdownSound", exit_priority_normal);

  sfxmutex = SDL_CreateMutex ();

  if (!nomusicparm)
    I_InitMusic();

  lprintf(LO_DEBUG, "I_InitSound: sound module ready\n");
  SDL_PauseAudio(0);
}


// NSM sound capture routines

// silences sound output, and instead allows sound capture to work
// call this before sound startup
void I_SetSoundCap (void)
{
  dumping_sound = 1;
}

// grabs len samples of audio (16 bit interleaved)
unsigned char *I_GrabSound (int len)
{
  static unsigned char *buffer = NULL;
  static size_t buffer_size = 0;
  size_t size;

  if (!dumping_sound)
    return NULL;

  size = len * 4;
  if (!buffer || size > buffer_size)
  {
    buffer_size = size * 4;
    buffer = (unsigned char *)Z_Realloc (buffer, buffer_size);
  }

  if (buffer)
  {
    memset (buffer, 0, size);
    I_UpdateSound ((void *) 0xdeadbeef, buffer, size);
  }
  return buffer;
}




// NSM helper routine for some of the streaming audio
void I_ResampleStream (void *dest, unsigned nsamp, void (*proc) (void *dest, unsigned nsamp), unsigned sratein, unsigned srateout)
{ // assumes 16 bit signed interleaved stereo

  unsigned i;
  int j = 0;

  short *sout = (short*)dest;

  static short *sin = NULL;
  static unsigned sinsamp = 0;

  static unsigned remainder = 0;
  unsigned step = (sratein << 16) / (unsigned) srateout;

  unsigned nreq = (step * nsamp + remainder) >> 16;

  if (nreq > sinsamp)
  {
    sin = (short*)Z_Realloc (sin, (nreq + 1) * 4);
    if (!sinsamp) // avoid pop when first starting stream
      sin[0] = sin[1] = 0;
    sinsamp = nreq;
  }

  proc (sin + 2, nreq);

  for (i = 0; i < nsamp; i++)
  {
    *sout++ = ((unsigned) sin[j + 0] * (0x10000 - remainder) +
               (unsigned) sin[j + 2] * remainder) >> 16;
    *sout++ = ((unsigned) sin[j + 1] * (0x10000 - remainder) +
               (unsigned) sin[j + 3] * remainder) >> 16;
    remainder += step;
    j += remainder >> 16 << 1;
    remainder &= 0xffff;
  }
  sin[0] = sin[nreq * 2];
  sin[1] = sin[nreq * 2 + 1];
}

//
// MUSIC API.
//

static void UpdateMusic (void *buff, unsigned nsamp);
static int RegisterSong (const void *data, size_t len);
static int RegisterSongEx (const void *data, size_t len, int try_mus2mid);
static void UnRegisterSong(int handle);
static void StopSong(int handle);
static void ResumeSong (int handle);
static void PauseSong (int handle);
static void PlaySong(int handle, int looping);

#include "mus2mid.h"

#include "MUSIC/musicplayer.h"
#include "MUSIC/oplplayer.h"
#include "MUSIC/madplayer.h"
#include "MUSIC/dumbplayer.h"
#include "MUSIC/flplayer.h"
#include "MUSIC/vorbisplayer.h"
#include "MUSIC/portmidiplayer.h"

static Mix_Music *music[2] = { NULL, NULL };

// Some tracks are directly streamed from the RWops;
// we need to free them in the end
static SDL_RWops *rwops_stream = NULL;

// note that the "handle" passed around by s_sound is ignored
// however, a handle is maintained for the individual music players

static const music_player_t *music_players[] =
{ // until some ui work is done, the order these appear is the autodetect order.
  // of particular importance:  things that play mus have to be last, because
  // mus2midi very often succeeds even on garbage input
  &vorb_player, // vorbisplayer.h
  &mp_player, // madplayer.h
  &db_player, // dumbplayer.h
  &fl_player, // flplayer.h
  &opl_synth_player, // oplplayer.h
  &pm_player, // portmidiplayer.h
  NULL
};
#define NUM_MUS_PLAYERS ((int)(sizeof (music_players) / sizeof (music_player_t *) - 1))

static int music_player_was_init[NUM_MUS_PLAYERS];

#define PLAYER_VORBIS     "vorbis player"
#define PLAYER_MAD        "mad mp3 player"
#define PLAYER_DUMB       "dumb tracker player"
#define PLAYER_FLUIDSYNTH "fluidsynth midi player"
#define PLAYER_OPL        "opl synth player"
#define PLAYER_PORTMIDI   "portmidi midi player"

// order in which players are to be tried
char music_player_order[NUM_MUS_PLAYERS][200] =
{
  PLAYER_VORBIS,
  PLAYER_MAD,
  PLAYER_DUMB,
  PLAYER_FLUIDSYNTH,
  PLAYER_OPL,
  PLAYER_PORTMIDI,
};

const char *midiplayers[midi_player_last + 1] = {
  "fluidsynth", "opl", "portmidi", NULL };

static int current_player = -1;
static const void *music_handle = NULL;

static void *mus2mid_conversion_data = NULL;

void I_ShutdownMusic(void)
{
  int i;
  S_StopMusic ();

  for (i = 0; music_players[i]; i++)
  {
    if (music_player_was_init[i])
      music_players[i]->shutdown ();
  }

  if (musmutex)
  {
    SDL_DestroyMutex (musmutex);
    musmutex = NULL;
  }
}

void I_InitMusic(void)
{
  int i;
  musmutex = SDL_CreateMutex ();

  // todo not so greedy
  for (i = 0; music_players[i]; i++)
    music_player_was_init[i] = music_players[i]->init (snd_samplerate);

  I_AtExit(I_ShutdownMusic, true, "I_ShutdownMusic", exit_priority_normal);
}

// Derived value (not saved, accounts for muted music)
static int music_volume;

void I_ResetMusicVolume(void)
{
  snd_MusicVolume = dsda_IntConfig(dsda_config_music_volume);

  if (nomusicparm)
    return;

  if (dsda_MuteMusic())
    music_volume = 0;
  else
    music_volume = snd_MusicVolume;

  Mix_VolumeMusic(music_volume * 8);

  if (music_handle)
  {
    SDL_LockMutex(musmutex);
    music_players[current_player]->setvolume(music_volume);
    SDL_UnlockMutex(musmutex);
  }
}

void I_PlaySong(int handle, int looping)
{
  if (registered_non_rw)
  {
    PlaySong (handle, looping);
    return;
  }

  if ( music[handle] ) {
    //Mix_FadeInMusic(music[handle], looping ? -1 : 0, 500);
    Mix_PlayMusic(music[handle], looping ? -1 : 0);

    // haleyjd 10/28/05: make sure volume settings remain consistent
    I_ResetMusicVolume();
  }
}

void I_PauseSong (int handle)
{
  if (registered_non_rw)
  {
    PauseSong (handle);
    return;
  }

  switch (dsda_IntConfig(dsda_config_mus_pause_opt))
  {
    case 0:
        I_StopSong(handle);
      break;
    case 1:
      switch (Mix_GetMusicType(NULL))
      {
        case MUS_NONE:
          break;
        default:
          Mix_PauseMusic();
          break;
      }
      break;
  }
  // Default - let music continue
}

void I_ResumeSong (int handle)
{
  if (registered_non_rw)
  {
    ResumeSong (handle);
    return;
  }

  switch (dsda_IntConfig(dsda_config_mus_pause_opt)) {
    case 0:
        I_PlaySong(handle,1);
      break;
    case 1:
      switch (Mix_GetMusicType(NULL))
      {
        case MUS_NONE:
          break;
        default:
          Mix_ResumeMusic();
          break;
      }
      break;
  }
  /* Otherwise, music wasn't stopped */
}

void I_StopSong(int handle)
{
  if (registered_non_rw)
  {
    StopSong (handle);
    return;
  }

  // halt music playback
  Mix_HaltMusic();
}

void I_UnRegisterSong(int handle)
{
  if (registered_non_rw)
  {
    UnRegisterSong (handle);
    return;
  }

  if ( music[handle] ) {
    Mix_FreeMusic(music[handle]);
    music[handle] = NULL;

    // Free RWops
    if (rwops_stream != NULL)
    {
      //SDL_FreeRW(rwops_stream);
      rwops_stream = NULL;
    }
  }
}

int I_RegisterSong(const void *data, size_t len)
{
  registered_non_rw = false;

  if (RegisterSong(data, len))
    return 0;

  // e6y: new logic by me
  // Now you can hear title music in deca.wad
  // http://www.doomworld.com/idgames/index.php?id=8808
  // Ability to use mp3 and ogg as inwad lump

  music[0] = NULL;

  rwops_stream = SDL_RWFromConstMem(data, len);
  if (rwops_stream)
  {
    music[0] = Mix_LoadMUS_RW(rwops_stream, SDL_FALSE);
  }

  // Failed to load
  if (!music[0])
  {
    // Conversion failed, free everything
    if (rwops_stream != NULL)
    {
      //SDL_FreeRW(rwops_stream);
      rwops_stream = NULL;
    }

    lprintf(LO_ERROR, "Error loading song: %s\n", Mix_GetError());
  }

  return (0);
}

static void PlaySong(int handle, int looping)
{
  if (music_handle)
  {
    SDL_LockMutex (musmutex);
    music_players[current_player]->play (music_handle, looping);
    music_players[current_player]->setvolume (music_volume);
    SDL_UnlockMutex (musmutex);
  }
}

static void PauseSong (int handle)
{
  if (!music_handle)
    return;

  SDL_LockMutex (musmutex);
  switch (dsda_IntConfig(dsda_config_mus_pause_opt))
  {
    case 0:
      music_players[current_player]->stop ();
      break;
    case 1:
      music_players[current_player]->pause ();
      break;
    default: // Default - let music continue
      break;
  }
  SDL_UnlockMutex (musmutex);
}

static void ResumeSong (int handle)
{
  if (!music_handle)
    return;

  SDL_LockMutex (musmutex);
  switch (dsda_IntConfig(dsda_config_mus_pause_opt))
  {
    case 0: // i'm not sure why we can guarantee looping=true here,
            // but that's what the old code did
      music_players[current_player]->play (music_handle, 1);
      break;
    case 1:
      music_players[current_player]->resume ();
      break;
    default: // Default - music was never stopped
      break;
  }
  SDL_UnlockMutex (musmutex);
}

static void StopSong(int handle)
{
  if (music_handle)
  {
    SDL_LockMutex (musmutex);
    music_players[current_player]->stop ();
    SDL_UnlockMutex (musmutex);
  }
}

static void UnRegisterSong(int handle)
{
  if (music_handle)
  {
    SDL_LockMutex (musmutex);
    music_players[current_player]->unregistersong (music_handle);
    music_handle = NULL;
    if (mus2mid_conversion_data)
    {
      Z_Free (mus2mid_conversion_data);
      mus2mid_conversion_data = NULL;
    }
    SDL_UnlockMutex (musmutex);
  }
}

// returns 1 on success, 0 on failure
static int RegisterSongEx (const void *data, size_t len, int try_mus2mid)
{
  int i, j;

  MEMFILE *instream;
  MEMFILE *outstream;
  void *outbuf;
  size_t outbuf_len;
  int result;

  if (music_handle)
    UnRegisterSong (0);

  // e6y: new logic by me
  // Now you can hear title music in deca.wad
  // http://www.doomworld.com/idgames/index.php?id=8808
  // Ability to use mp3 and ogg as inwad lump

  if (len > 4 && memcmp(data, "MUS", 3) != 0)
  {
    // The header has no MUS signature
    // Let's try to load this song directly

    // go through music players in order
    int found = 0;

    for (j = 0; j < NUM_MUS_PLAYERS; j++)
    {
      found = 0;
      for (i = 0; music_players[i]; i++)
      {
        if (strcmp (music_players[i]->name (), music_player_order[j]) == 0)
        {
          found = 1;
          if (music_player_was_init[i])
          {
            const void *temp_handle = music_players[i]->registersong (data, len);
            if (temp_handle)
            {
              SDL_LockMutex (musmutex);
              current_player = i;
              music_handle = temp_handle;
              SDL_UnlockMutex (musmutex);
              lprintf(LO_DEBUG, "RegisterSongEx: Using player %s\n", music_players[i]->name ());
              return 1;
            }
          }
          else
            lprintf(LO_DEBUG, "RegisterSongEx: Music player %s on preferred list but it failed to init\n", music_players[i]-> name ());
        }
      }
      if (!found)
        lprintf(LO_DEBUG, "RegisterSongEx: Couldn't find preferred music player %s in list\n  (typo or support not included at compile time)\n", music_player_order[j]);
    }
    // load failed
  }

  // load failed? try mus2mid
  if (len > 4 && try_mus2mid)
  {

    instream = mem_fopen_read (data, len);
    outstream = mem_fopen_write ();

    // e6y: from chocolate-doom
    // New mus -> mid conversion code thanks to Ben Ryves <benryves@benryves.com>
    // This plays back a lot of music closer to Vanilla Doom - eg. tnt.wad map02
    result = mus2mid(instream, outstream);
    if (result != 0)
    {
      size_t muslen = len;
      const unsigned char *musptr = (const unsigned char*)data;

      // haleyjd 04/04/10: scan forward for a MUS header. Evidently DMX was
      // capable of doing this, and would skip over any intervening data. That,
      // or DMX doesn't use the MUS header at all somehow.
      while (musptr < (const unsigned char*)data + len - sizeof(musheader))
      {
        // if we found a likely header start, reset the mus pointer to that location,
        // otherwise just leave it alone and pray.
        if (!strncmp ((const char*) musptr, "MUS\x1a", 4))
        {
          mem_fclose (instream);
          instream = mem_fopen_read (musptr, muslen);
          result = mus2mid (instream, outstream);
          break;
        }

        musptr++;
        muslen--;
      }
    }
    if (result == 0)
    {
      mem_get_buf(outstream, &outbuf, &outbuf_len);

      // recopy so we can free the MEMFILE
      mus2mid_conversion_data = Z_Malloc (outbuf_len);
      if (mus2mid_conversion_data)
        memcpy (mus2mid_conversion_data, outbuf, outbuf_len);

      mem_fclose(instream);
      mem_fclose(outstream);

      if (mus2mid_conversion_data)
      {
        return RegisterSongEx (mus2mid_conversion_data, outbuf_len, 0);
      }
    }
  }

  lprintf(LO_ERROR, "RegisterSongEx: Failed\n");
  return 0;
}


static int RegisterSong (const void *data, size_t len)
{
  int result;

  result = RegisterSongEx (data, len, 1);

  if (result)
    registered_non_rw = true;

  return result;
}

static void UpdateMusic (void *buff, unsigned nsamp)
{
  if (!music_handle)
  {
    memset (buff, 0, nsamp * 4);
    return;
  }

  music_players[current_player]->render (buff, nsamp);
}

void M_ChangeMIDIPlayer(void)
{
  snd_midiplayer = dsda_StringConfig(dsda_config_snd_midiplayer);

  if (!strcasecmp(snd_midiplayer, midiplayers[midi_player_fluidsynth]))
  {
    strcpy(music_player_order[3], PLAYER_FLUIDSYNTH);
    strcpy(music_player_order[4], PLAYER_OPL);
    strcpy(music_player_order[5], PLAYER_PORTMIDI);
  }
  else if (!strcasecmp(snd_midiplayer, midiplayers[midi_player_opl]))
  {
    strcpy(music_player_order[3], PLAYER_OPL);
    strcpy(music_player_order[4], PLAYER_FLUIDSYNTH);
    strcpy(music_player_order[5], PLAYER_PORTMIDI);
  }
  else if (!strcasecmp(snd_midiplayer, midiplayers[midi_player_portmidi]))
  {
    strcpy(music_player_order[3], PLAYER_PORTMIDI);
    strcpy(music_player_order[4], PLAYER_FLUIDSYNTH);
    strcpy(music_player_order[5], PLAYER_OPL);
  }

  S_StopMusic();
  S_RestartMusic();
}
