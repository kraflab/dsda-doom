/* Emacs style mode select   -*- C++ -*-
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
 * DESCRIPTION:  Platform-independent sound code
 *
 *-----------------------------------------------------------------------------*/

// killough 3/7/98: modified to allow arbitrary listeners in spy mode
// killough 5/2/98: reindented, removed useless code, beautified

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doomstat.h"
#include "s_sound.h"
#include "s_advsound.h"
#include "i_sound.h"
#include "i_system.h"
#include "d_main.h"
#include "r_main.h"
#include "m_random.h"
#include "w_wad.h"
#include "lprintf.h"
#include "sc_man.h"
#include "p_setup.h"
#include "e6y.h"

#include "hexen/sn_sonix.h"

#include "dsda/memory.h"

// when to clip out sounds
// Does not fit the large outdoor areas.
#define S_CLIPPING_DIST (1200<<FRACBITS)

// Distance tp origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).
// Originally: (200*0x10000).

#define S_CLOSE_DIST (160<<FRACBITS)
#define S_ATTENUATOR ((S_CLIPPING_DIST-S_CLOSE_DIST)>>FRACBITS)

// Adjustable by menu.
#define NORM_PITCH 128
#define NORM_PRIORITY 64
#define NORM_SEP 128
#define S_STEREO_SWING (96<<FRACBITS)

typedef struct
{
  sfxinfo_t *sfxinfo;  // sound information (if null, channel avail.)
  void *origin;        // origin of sound
  int handle;          // handle of the sound being played
  int is_pickup;       // killough 4/25/98: whether sound is a player's weapon
  int pitch;

  // heretic
  int priority;

  // hexen
  int volume;
} channel_t;

// the set of channels available
static channel_t *channels;
static degenmobj_t *sobjs;

// These are not used, but should be (menu).
// Maximum volume of a sound effect.
// Internal default is max out of 0-15.
int snd_SfxVolume = 15;

// Maximum volume of music. Useless so far.
int snd_MusicVolume = 15;

// whether songs are mus_paused
static dboolean mus_paused;

// music currently being played
musicinfo_t *mus_playing;

// music currently should play
static int musicnum_current;

// following is set
//  by the defaults code in M_misc:
// number of channels available
int default_numChannels;
int numChannels;

//jff 3/17/98 to keep track of last IDMUS specified music num
int idmusnum;

//
// Internals.
//

void S_StopChannel(int cnum);

int S_AdjustSoundParams(mobj_t *listener, mobj_t *source,
                        int *vol, int *sep, int *pitch);

static int S_getChannel(void *origin, sfxinfo_t *sfxinfo, int is_pickup);


// heretic
int max_snd_dist = 1600;
int dist_adjust = 160;

static byte* soundCurve;
static int AmbChan = -1;

static void Heretic_S_StopSound(void *_origin);
static void Heretic_S_UpdateSounds(mobj_t *listener);
static void Heretic_S_StartSoundAtVolume(void *_origin, int sound_id, int volume);
static void Hexen_S_StartSoundAtVolume(void *_origin, int sound_id, int volume);

// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//

void S_Init(int sfxVolume, int musicVolume)
{
  idmusnum = -1; //jff 3/17/98 insure idmus number is blank

  //jff 1/22/98 skip sound init if sound not enabled
  numChannels = default_numChannels;
  if (snd_card && !nosfxparm)
  {
    int i;

    lprintf(LO_INFO, "S_Init: default sfx volume %d\n", sfxVolume);

    // Whatever these did with DMX, these are rather dummies now.
    I_SetChannels();

    S_SetSfxVolume(sfxVolume);

    // Allocating the internal channels for mixing
    // (the maximum numer of sounds rendered
    // simultaneously) within zone memory.
    // CPhipps - calloc
    channels =
      (channel_t *) calloc(numChannels,sizeof(channel_t));
    sobjs =
      (degenmobj_t *) calloc(numChannels, sizeof(degenmobj_t));

    // Note that sounds have not been cached (yet).
    for (i=1 ; i<num_sfx ; i++)
      S_sfx[i].lumpnum = -1;

    dsda_CacheSoundLumps();
  }

  // CPhipps - music init reformatted
  if (mus_card && !nomusicparm) {
    S_SetMusicVolume(musicVolume);

    // no sounds are playing, and they are not mus_paused
    mus_paused = 0;
  }

  if (raven)
  {
    int lump;
    int length;

    lump = W_GetNumForName("SNDCURVE");
    length = W_LumpLength(lump);

    max_snd_dist = length;
    dist_adjust = max_snd_dist / 10;

    soundCurve = Z_Malloc(max_snd_dist, PU_STATIC, NULL);
    memcpy(soundCurve, (const byte *) W_CacheLumpNum(lump), max_snd_dist);
    W_UnlockLumpNum(lump);
  }
}

void S_Stop(void)
{
  int cnum;

  // heretic
  AmbChan = -1;

  //jff 1/22/98 skip sound init if sound not enabled
  if (snd_card && !nosfxparm)
    for (cnum=0 ; cnum<numChannels ; cnum++)
      if (channels[cnum].sfxinfo)
        S_StopChannel(cnum);
}

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//

static inline int WRAP(int i, int w)
{
  while (i < 0)
    i += w;

  return i % w;
}

void S_Start(void)
{
  int mnum;

  // kill all playing sounds at start of level
  //  (trust me - a good idea)

  S_Stop();

  // start new music for the level
  mus_paused = 0;

  if (hexen)
  {
    mnum = gamemap;
  }
  else
  {
    if (gamemapinfo && gamemapinfo->music[0])
    {
  	  int muslump = W_CheckNumForName(gamemapinfo->music);
  	  if (muslump >= 0)
  	  {
  		  musinfo.items[0] = muslump;
  		  S_ChangeMusInfoMusic(muslump, true);
  		  return;
  	  }
  	  // If the mapinfo defined music cannot be found, try the default for the given map.
    }

    if (idmusnum!=-1)
      mnum = idmusnum; //jff 3/17/98 reload IDMUS music if not -1
    else
    {
      if (gamemode == commercial)
        mnum = mus_runnin + WRAP(gamemap - 1, DOOM_NUMMUSIC - mus_runnin);
      else
      {
        static const int spmus[] =     // Song - Who? - Where?
        {
          mus_e3m4,     // American     e4m1
          mus_e3m2,     // Romero       e4m2
          mus_e3m3,     // Shawn        e4m3
          mus_e1m5,     // American     e4m4
          mus_e2m7,     // Tim  e4m5
          mus_e2m4,     // Romero       e4m6
          mus_e2m6,     // J.Anderson   e4m7 CHIRON.WAD
          mus_e2m5,     // Shawn        e4m8
          mus_e1m9      // Tim          e4m9
        };

        if (heretic)
          mnum = heretic_mus_e1m1 +
                 WRAP((gameepisode - 1) * 9 + gamemap - 1, HERETIC_NUMMUSIC - heretic_mus_e1m1);
        else if (gameepisode < 4)
          mnum = mus_e1m1 + WRAP((gameepisode - 1) * 9 + gamemap - 1, mus_runnin - mus_e1m1);
        else
          mnum = spmus[WRAP(gamemap - 1, 9)];
      }
    }
  }

  memset(&musinfo, 0, sizeof(musinfo));
  musinfo.items[0] = -1;

  S_ChangeMusic(mnum, true);
}

void S_StartSoundAtVolume(void *origin_p, int sfx_id, int volume)
{
  int sep, pitch, priority, cnum, is_pickup;
  sfxinfo_t *sfx;
  mobj_t *origin;

  if (heretic) return Heretic_S_StartSoundAtVolume(origin_p, sfx_id, volume);
  if (hexen) return Hexen_S_StartSoundAtVolume(origin_p, sfx_id, volume);

  origin = (mobj_t *) origin_p;

  //jff 1/22/98 return if sound is not enabled
  if (!snd_card || nosfxparm)
    return;

  is_pickup = sfx_id & PICKUP_SOUND || sfx_id == sfx_oof || (compatibility_level >= prboom_2_compatibility && sfx_id == sfx_noway); // killough 4/25/98
  sfx_id &= ~PICKUP_SOUND;

  if (sfx_id == sfx_None)
    return;

  // check for bogus sound #
  if (sfx_id < 1 || sfx_id > num_sfx)
    I_Error("S_StartSoundAtVolume: Bad sfx #: %d", sfx_id);

  sfx = &S_sfx[sfx_id];

  // Initialize sound parameters
  if (sfx->link)
    {
      pitch = sfx->pitch;
      priority = sfx->priority;
      volume += sfx->volume;

      if (volume < 1)
        return;

      if (volume > snd_SfxVolume)
        volume = snd_SfxVolume;
    }
  else
    {
      pitch = NORM_PITCH;
      priority = NORM_PRIORITY;
    }

  // Check to see if it is audible, modify the params
  // killough 3/7/98, 4/25/98: code rearranged slightly

  if (!origin || (origin == players[displayplayer].mo && walkcamera.type < 2)) {
    sep = NORM_SEP;
    volume *= 8;
  } else
    if (!S_AdjustSoundParams(players[displayplayer].mo, origin, &volume,
                             &sep, &pitch))
      return;
    else
      if ( origin->x == players[displayplayer].mo->x &&
           origin->y == players[displayplayer].mo->y)
        sep = NORM_SEP;

  // hacks to vary the sfx pitches
  if (sfx_id >= sfx_sawup && sfx_id <= sfx_sawhit)
    pitch += 8 - (M_Random()&15);
  else
    if (sfx_id != sfx_itemup && sfx_id != sfx_tink)
      pitch += 16 - (M_Random()&31);

  if (pitch<0)
    pitch = 0;

  if (pitch>255)
    pitch = 255;

  // kill old sound
  for (cnum=0 ; cnum<numChannels ; cnum++)
    if (channels[cnum].sfxinfo && channels[cnum].origin == origin &&
        (comp[comp_sound] || channels[cnum].is_pickup == is_pickup))
      {
        S_StopChannel(cnum);
        break;
      }

  // try to find a channel
  cnum = S_getChannel(origin, sfx, is_pickup);

  if (cnum<0)
    return;

  // get lumpnum if necessary
  // killough 2/28/98: make missing sounds non-fatal
  if (sfx->lumpnum < 0 && (sfx->lumpnum = I_GetSfxLumpNum(sfx)) < 0)
    return;

  // Assigns the handle to one of the channels in the mix/output buffer.
  { // e6y: [Fix] Crash with zero-length sounds.
    int h = I_StartSound(sfx_id, cnum, volume, sep, pitch, priority);
    if (h != -1)
    {
      channels[cnum].handle = h;
      channels[cnum].pitch = pitch;
    }
  }
}

void S_StartSound(void *origin, int sfx_id)
{
  if (raven) return Hexen_S_StartSoundAtVolume(origin, sfx_id, 127);

  S_StartSoundAtVolume(origin, sfx_id, snd_SfxVolume);
}

void S_StopSound(void *origin)
{
  int cnum;

  if (raven) return Heretic_S_StopSound(origin);

  //jff 1/22/98 return if sound is not enabled
  if (!snd_card || nosfxparm)
    return;

  for (cnum=0 ; cnum<numChannels ; cnum++)
    if (channels[cnum].sfxinfo && channels[cnum].origin == origin)
      {
        S_StopChannel(cnum);
        break;
      }
}

// [FG] disable sound cutoffs
int full_sounds;

void S_UnlinkSound(void *origin)
{
  int cnum;

  //jff 1/22/98 return if sound is not enabled
  if (!snd_card || nosfxparm)
    return;

  if (origin)
  {
    for (cnum = 0; cnum < numChannels; cnum++)
    {
      if (channels[cnum].sfxinfo && channels[cnum].origin == origin)
      {
        degenmobj_t *const sobj = &sobjs[cnum];
        const mobj_t *const mobj = (mobj_t *) origin;
        sobj->x = mobj->x;
        sobj->y = mobj->y;
        sobj->z = mobj->z;
        channels[cnum].origin = (mobj_t *) sobj;
        break;
      }
    }
  }
}

//
// Stop and resume music, during game PAUSE.
//
void S_PauseSound(void)
{
  //jff 1/22/98 return if music is not enabled
  if (!mus_card || nomusicparm)
    return;

  if (mus_playing && !mus_paused)
    {
      I_PauseSong(mus_playing->handle);
      mus_paused = true;
    }
}

void S_ResumeSound(void)
{
  //jff 1/22/98 return if music is not enabled
  if (!mus_card || nomusicparm)
    return;

  if (mus_playing && mus_paused)
    {
      I_ResumeSong(mus_playing->handle);
      mus_paused = false;
    }
}


//
// Updates music & sounds
//
void S_UpdateSounds(void* listener_p)
{
  mobj_t *listener;
  int cnum;

  if (raven) return Heretic_S_UpdateSounds(listener_p);

  listener = (mobj_t*) listener_p;

  //jff 1/22/98 return if sound is not enabled
  if (!snd_card || nosfxparm)
    return;

#ifdef UPDATE_MUSIC
  I_UpdateMusic();
#endif

  for (cnum=0 ; cnum<numChannels ; cnum++)
  {
    sfxinfo_t *sfx;
    channel_t *c = &channels[cnum];
    if ((sfx = c->sfxinfo))
    {
      if (I_SoundIsPlaying(c->handle))
      {
        // initialize parameters
        int volume = snd_SfxVolume;
        int pitch = c->pitch; // use channel's pitch!
        int sep = NORM_SEP;

        if (sfx->link)
        {
          pitch = sfx->pitch;
          volume += sfx->volume;
          if (volume < 1)
          {
            S_StopChannel(cnum);
            continue;
          }
          else
            if (volume > snd_SfxVolume)
              volume = snd_SfxVolume;
        }

        // check non-local sounds for distance clipping
        // or modify their params
        if (c->origin && listener_p != c->origin) // killough 3/20/98
        {
          if (!S_AdjustSoundParams(listener, c->origin, &volume, &sep, &pitch))
            S_StopChannel(cnum);
          else
            I_UpdateSoundParams(c->handle, volume, sep, pitch);
        }
      }
      else   // if channel is allocated but sound has stopped, free it
        S_StopChannel(cnum);
    }
  }
}



void S_SetMusicVolume(int volume)
{
  //jff 1/22/98 return if music is not enabled
  if (!mus_card || nomusicparm)
    return;
  if (volume < 0 || volume > 15)
    I_Error("S_SetMusicVolume: Attempt to set music volume at %d", volume);
  I_SetMusicVolume(volume);
  snd_MusicVolume = volume;
}



void S_SetSfxVolume(int volume)
{
  //jff 1/22/98 return if sound is not enabled
  if (!snd_card || nosfxparm)
    return;
  if (volume < 0 || volume > 127)
    I_Error("S_SetSfxVolume: Attempt to set sfx volume at %d", volume);
  snd_SfxVolume = volume;
}



// Starts some music with the music id found in sounds.h.
//
void S_StartMusic(int m_id)
{
  S_ChangeMusic(m_id, false);
}



void S_ChangeMusic(int musicnum, int looping)
{
  musicinfo_t *music;

  // current music which should play
  musicnum_current = musicnum;
  musinfo.current_item = -1;
  S_music[mus_musinfo].lumpnum = -1;

  //jff 1/22/98 return if music is not enabled
  if (!mus_card || nomusicparm)
    return;

  if (musicnum <= mus_None || musicnum >= num_music)
    I_Error("S_ChangeMusic: Bad music number %d", musicnum);

  music = &S_music[musicnum];

  if (mus_playing == music)
    return;

  // shutdown old music
  S_StopMusic();

  // get lumpnum if neccessary
  if (!music->lumpnum)
  {
    if (hexen && musicnum < hexen_mus_hub)
    {
      const char* songLump;

      songLump = P_GetMapSongLump(musicnum);
      if (!songLump)
      {
        return;
      }

      music->lumpnum = W_GetNumForName(songLump);
    }
    else
    {
      char namebuf[9];
      const char* format;

      format = raven ? "%s" : "d_%s";

      sprintf(namebuf, format, music->name);
      music->lumpnum = W_GetNumForName(namebuf);
    }
  }

  // load & register it
  music->data = W_CacheLumpNum(music->lumpnum);
  music->handle = I_RegisterSong(music->data, W_LumpLength(music->lumpnum));

  // play it
  I_PlaySong(music->handle, looping);

  mus_playing = music;

  musinfo.current_item = -1;

  // [crispy] MUSINFO value 0 is reserved for the map's default music
  if (musinfo.items[0] == -1)
  {
     musinfo.items[0] = music->lumpnum;
     S_music[mus_musinfo].lumpnum = -1;
  }
}

void S_RestartMusic(void)
{
  if (musinfo.current_item != -1)
  {
    S_ChangeMusInfoMusic(musinfo.current_item, true);
  }
  else
  {
    if (musicnum_current > mus_None && musicnum_current < num_music)
    {
      S_ChangeMusic(musicnum_current, true);
    }
  }
}

void S_ChangeMusInfoMusic(int lumpnum, int looping)
{
  musicinfo_t *music;

  if (doSkip)
  {
    musinfo.current_item = lumpnum;
    return;
  }

  //jff 1/22/98 return if music is not enabled
  if (!mus_card || nomusicparm)
    return;

  if (mus_playing && mus_playing->lumpnum == lumpnum)
    return;

  music = &S_music[mus_musinfo];

  if (music->lumpnum == lumpnum)
    return;

  // shutdown old music
  S_StopMusic();

  // save lumpnum
  music->lumpnum = lumpnum;

  // load & register it
  music->data = W_CacheLumpNum(music->lumpnum);
  music->handle = I_RegisterSong(music->data, W_LumpLength(music->lumpnum));

  // play it
  I_PlaySong(music->handle, looping);

  mus_playing = music;

  musinfo.current_item = lumpnum;
}

void S_StopMusic(void)
{
  //jff 1/22/98 return if music is not enabled
  if (!mus_card || nomusicparm)
    return;

  if (mus_playing)
    {
      if (mus_paused)
        I_ResumeSong(mus_playing->handle);

      I_StopSong(mus_playing->handle);
      I_UnRegisterSong(mus_playing->handle);
      if (mus_playing->lumpnum >= 0)
  W_UnlockLumpNum(mus_playing->lumpnum); // cph - release the music data

      mus_playing->data = 0;
      mus_playing = 0;
    }
}



void S_StopChannel(int cnum)
{
  int i;
  channel_t *c = &channels[cnum];

  //jff 1/22/98 return if sound is not enabled
  if (!snd_card || nosfxparm)
    return;

  if (c->sfxinfo)
    {
      // stop the sound playing
      if (I_SoundIsPlaying(c->handle))
        I_StopSound(c->handle);

      // check to see
      //  if other channels are playing the sound
      for (i=0 ; i<numChannels ; i++)
        if (cnum != i && c->sfxinfo == channels[i].sfxinfo)
          break;

      c->sfxinfo = 0;

      // heretic_note: do this for doom too?
      if (raven) c->handle = 0;
    }
}

//
// Changes volume, stereo-separation, and pitch variables
//  from the norm of a sound effect to be played.
// If the sound is not audible, returns a 0.
// Otherwise, modifies parameters and returns 1.
//

int S_AdjustSoundParams(mobj_t *listener, mobj_t *source,
                        int *vol, int *sep, int *pitch)
{
  fixed_t adx, ady,approx_dist;
  angle_t angle;

  //jff 1/22/98 return if sound is not enabled
  if (!snd_card || nosfxparm)
    return 0;

  // e6y
  // Fix crash when the program wants to S_AdjustSoundParams() for player
  // which is not displayplayer and displayplayer was not spawned at the moment.
  // It happens in multiplayer demos only.
  //
  // Stack trace is:
  // P_SetupLevel() - P_LoadThings() - P_SpawnMapThing() \ P_SpawnPlayer(players[0]) -
  // P_SetupPsprites() - P_BringUpWeapon() - S_StartSound(players[0]->mo, sfx_sawup) -
  // S_StartSoundAtVolume() - S_AdjustSoundParams(players[displayplayer]->mo, ...);
  // players[displayplayer]->mo is NULL
  //
  // There is no more crash on e1cmnet3.lmp between e1m2 and e1m3
  // http://competn.doom2.net/pub/compet-n/doom/coop/movies/e1cmnet3.zip
  if (!listener)
    return 0;

  // calculate the distance to sound origin
  //  and clip it if necessary
  if (walkcamera.type > 1)
  {
    adx = D_abs(walkcamera.x - source->x);
    ady = D_abs(walkcamera.y - source->y);
  }
  else
  {
    adx = D_abs(listener->x - source->x);
    ady = D_abs(listener->y - source->y);
  }

  // From _GG1_ p.428. Appox. eucledian distance fast.
  approx_dist = adx + ady - ((adx < ady ? adx : ady)>>1);

  if (!approx_dist)  // killough 11/98: handle zero-distance as special case
    {
      *sep = NORM_SEP;
      *vol = snd_SfxVolume;
      return *vol > 0;
    }

  if (approx_dist > S_CLIPPING_DIST)
    return 0;

  // angle of source to listener
  angle = R_PointToAngle2(listener->x, listener->y, source->x, source->y);

  if (angle <= listener->angle)
    angle += 0xffffffff;
  angle -= listener->angle;
  angle >>= ANGLETOFINESHIFT;

  // stereo separation
  *sep = 128 - (FixedMul(S_STEREO_SWING,finesine[angle])>>FRACBITS);

  // volume calculation
  if (approx_dist < S_CLOSE_DIST)
    *vol = snd_SfxVolume*8;
  else
    // distance effect
    *vol = (snd_SfxVolume * ((S_CLIPPING_DIST-approx_dist)>>FRACBITS) * 8)
      / S_ATTENUATOR;

  return (*vol > 0);
}

//
// S_getChannel :
//   If none available, return -1.  Otherwise channel #.
//
// killough 4/25/98: made static, added is_pickup argument

static int S_getChannel(void *origin, sfxinfo_t *sfxinfo, int is_pickup)
{
  // channel number to use
  int cnum;
  channel_t *c;

  //jff 1/22/98 return if sound is not enabled
  if (!snd_card || nosfxparm)
    return -1;

  // Find an open channel
  for (cnum=0; cnum<numChannels && channels[cnum].sfxinfo; cnum++)
    if (origin && channels[cnum].origin == origin &&
        channels[cnum].is_pickup == is_pickup)
      {
        S_StopChannel(cnum);
        break;
      }

    // None available
  if (cnum == numChannels)
    {      // Look for lower priority
      for (cnum=0 ; cnum<numChannels ; cnum++)
        if (channels[cnum].sfxinfo->priority >= sfxinfo->priority)
          break;
      if (cnum == numChannels)
        return -1;                  // No lower priority.  Sorry, Charlie.
      else
        S_StopChannel(cnum);        // Otherwise, kick out lower priority.
    }

  c = &channels[cnum];              // channel is decided to be cnum.
  c->sfxinfo = sfxinfo;
  c->origin = origin;
  c->is_pickup = is_pickup;         // killough 4/25/98
  return cnum;
}

// heretic

static mobj_t* GetSoundListener(void)
{
  static degenmobj_t dummy_listener;

  // If we are at the title screen, the display player doesn't have an
  // object yet, so return a pointer to a static dummy listener instead.

  if (players[displayplayer].mo != NULL)
  {
    return players[displayplayer].mo;
  }
  else
  {
    dummy_listener.x = 0;
    dummy_listener.y = 0;
    dummy_listener.z = 0;

    return (mobj_t *) &dummy_listener;
  }
}

static dboolean S_StopSoundInfo(sfxinfo_t* sfx, int priority)
{
  int i;
  int least_priority;
  int found;

  if (sfx->numchannels == -1)
    return true;

  least_priority = -1;
  found = 0;

  for (i = 0; i < numChannels; i++)
  {
    if (channels[i].sfxinfo == sfx && channels[i].origin)
    {
      found++;            //found one.  Now, should we replace it??
      if (priority >= channels[i].priority)
      {                   // if we're gonna kill one, then this'll be it
        least_priority = i;
        priority = channels[i].priority;
      }
    }
  }

  if (found < sfx->numchannels)
    return true;

  if (least_priority >= 0)
  {
    channel_t* channel = &channels[least_priority];

    if (channel->handle)
    {
      if (I_SoundIsPlaying(channel->handle))
        I_StopSound(channel->handle);

      channel->origin = NULL;
    }

    return true;
  }

  return false; // don't replace any sounds
}

// This is essentially Heretic's S_StartSound AND Hexen's S_StartSoundAtVolume
static void Hexen_S_StartSoundAtVolume(void *_origin, int sound_id, int volume)
{
  sfxinfo_t *sfx;
  mobj_t *origin;
  mobj_t *listener;
  int dist, vol;
  int i;
  int priority;
  int sep;
  angle_t angle;
  fixed_t absx;
  fixed_t absy;
  int chan;

  static int sndcount = 0;

  origin = (mobj_t *)_origin;
  listener = GetSoundListener();

  //jff 1/22/98 return if sound is not enabled
  if (!snd_card || nosfxparm)
    return;

  if (sound_id == heretic_sfx_None)
    return;

  if (origin == NULL)
    origin = listener;

  sfx = &S_sfx[sound_id];

  // calculate the distance before other stuff so that we can throw out
  // sounds that are beyond the hearing range.
  absx = abs(origin->x - listener->x);
  absy = abs(origin->y - listener->y);
  dist = absx + absy - (absx > absy ? absy >> 1 : absx >> 1);
  dist >>= FRACBITS;

  if (dist >= max_snd_dist)
    return; //sound is beyond the hearing range...
  if (dist < 0)
    dist = 0;

  priority = sfx->priority;
  priority *= (10 - (dist / dist_adjust));

  if (!S_StopSoundInfo(sfx, priority))
    return; // other sounds have greater priority

  for (i = 0; i < numChannels; i++)
  {
    if (origin->player)
    {
      i = numChannels;
      break;              // let the player have more than one sound.
    }
    if (origin == channels[i].origin)
    {                       // only allow other mobjs one sound
      S_StopSound(channels[i].origin);
      break;
    }
  }

  if (i >= numChannels)
  {
    if (sound_id >= heretic_sfx_wind)
    {
      if (AmbChan != -1 && sfx->priority <= channels[AmbChan].sfxinfo->priority)
        return;         //ambient channel already in use

      AmbChan = -1;
    }

    for (i = 0; i < numChannels; i++)
      if (channels[i].origin == NULL)
        break;

    if (i >= numChannels)
    {
      //look for a lower priority sound to replace.
      sndcount++;
      if (sndcount >= numChannels)
        sndcount = 0;

      for (chan = 0; chan < numChannels; chan++)
      {
        i = (sndcount + chan) % numChannels;
        if (priority >= channels[i].priority)
        {
          chan = -1;  //denote that sound should be replaced.
          break;
        }
      }

      if (chan != -1)
        return;         //no free channels.

      if (channels[i].handle)
      {
        if (I_SoundIsPlaying(channels[i].handle))
          I_StopSound(channels[i].handle);

        if (AmbChan == i)
          AmbChan = -1;
      }
    }
  }

  if (sfx->lumpnum <= 0)
    sfx->lumpnum = I_GetSfxLumpNum(sfx);

  vol = (soundCurve[dist] * volume * snd_SfxVolume * 8) >> 14;

  if (origin == listener)
    sep = 128;
  else
  {
    angle = R_PointToAngle2(listener->x, listener->y, origin->x, origin->y);
    if (angle <= listener->angle)
      angle += 0xffffffff;
    angle -= listener->angle;
    angle >>= ANGLETOFINESHIFT;

    // stereo separation
    sep = 128 - (FixedMul(S_STEREO_SWING,finesine[angle])>>FRACBITS);
  }

  if (!hexen || sfx->pitch)
  {
    channels[i].pitch = (byte) (NORM_PITCH + (M_Random() & 7) - (M_Random() & 7));
  }
  else
  {
    channels[i].pitch = NORM_PITCH;
  }
  channels[i].handle = I_StartSound(sound_id, i, vol, sep, channels[i].pitch, priority);
  channels[i].origin = origin;
  channels[i].sfxinfo = sfx;
  channels[i].priority = priority;
  channels[i].volume = volume;
  if (heretic && sound_id >= heretic_sfx_wind)
    AmbChan = i;
}

static void Heretic_S_StartSoundAtVolume(void *_origin, int sound_id, int volume)
{
  sfxinfo_t *sfx;
  mobj_t *origin;
  mobj_t *listener;
  int i;

  origin = (mobj_t *)_origin;
  listener = GetSoundListener();

  if (!snd_card || nosfxparm)
    return;

  if (sound_id == heretic_sfx_None || volume == 0)
    return;

  if (origin == NULL)
    origin = listener;

  sfx = &S_sfx[sound_id];

  volume = (volume * (snd_SfxVolume + 1) * 8) >> 7;

  // no priority checking, as ambient sounds would be the LOWEST.
  for (i = 0; i < numChannels; i++)
    if (channels[i].origin == NULL)
      break;

  if (i >= numChannels)
    return;

  if (sfx->lumpnum <= 0)
    sfx->lumpnum = I_GetSfxLumpNum(sfx);

  channels[i].pitch = (byte) (NORM_PITCH - (M_Random() & 3) + (M_Random() & 3));
  channels[i].handle = I_StartSound(sound_id, i, volume, 128, channels[i].pitch, 1);
  channels[i].origin = origin;
  channels[i].sfxinfo = sfx;
  channels[i].priority = 1;    //super low priority.
}

static void Heretic_S_StopSound(void *_origin)
{
  mobj_t *origin = _origin;
  int i;

  //jff 1/22/98 return if sound is not enabled
  if (!snd_card || nosfxparm)
    return;

  for (i = 0; i < numChannels; i++)
  {
    if (channels[i].origin == origin)
    {
      I_StopSound(channels[i].handle);

      channels[i].handle = 0;
      channels[i].origin = NULL;

      if (AmbChan == i)
        AmbChan = -1;
    }
  }
}

void Heretic_S_UpdateSounds(mobj_t *listener)
{
  int i, dist, vol;
  int sep;
  int priority;
  angle_t angle;
  fixed_t absx;
  fixed_t absy;

  //jff 1/22/98 return if sound is not enabled
  if (!snd_card || nosfxparm)
    return;

#ifdef UPDATE_MUSIC
  I_UpdateMusic();
#endif

  // I_UpdateSound();

  listener = GetSoundListener();
  if (snd_SfxVolume == 0)
    return;

  if (hexen)
  {
    // Update any Sequences
    SN_UpdateActiveSequences();
  }

  for (i = 0; i < numChannels; i++)
  {
    mobj_t* origin;

    if (!channels[i].handle)
      continue;

    if (!I_SoundIsPlaying(channels[i].handle))
    {
      channels[i].handle = 0;
      channels[i].origin = NULL;
      channels[i].sfxinfo = NULL;
      if (AmbChan == i)
        AmbChan = -1;
    }

    if (
      channels[i].origin == NULL ||
      channels[i].sfxinfo == NULL ||
      channels[i].origin == listener ||
      listener == NULL
    )
      continue;

    origin = (mobj_t*)channels[i].origin;

    absx = abs(origin->x - listener->x);
    absy = abs(origin->y - listener->y);
    dist = absx + absy - (absx > absy ? absy >> 1 : absx >> 1);
    dist >>= FRACBITS;

    if (dist >= max_snd_dist)
    {
      S_StopSound(origin);
      continue;
    }
    if (dist < 0)
      dist = 0;

    // calculate the volume based upon the distance from the sound origin.
    vol = (soundCurve[dist] * snd_SfxVolume * 8 * channels[i].volume) >> 14;

    angle = R_PointToAngle2(listener->x, listener->y, origin->x, origin->y);
    if (angle <= listener->angle)
      angle += 0xffffffff;
    angle -= listener->angle;
    angle >>= ANGLETOFINESHIFT;

    // stereo separation
    sep = 128 - (FixedMul(S_STEREO_SWING,finesine[angle])>>FRACBITS);

    // TODO: Pitch shifting.
    I_UpdateSoundParams(channels[i].handle, vol, sep, channels[i].pitch);
    priority = channels[i].sfxinfo->priority;
    // heretic_note: divides by 256 instead of the dist_adjust
    priority *= (10 - (dist / dist_adjust));
    channels[i].priority = priority;
  }
}

// hexen

dboolean S_GetSoundPlayingInfo(void * origin, int sound_id)
{
    int i;
    sfxinfo_t *sfx;

    //jff 1/22/98 return if sound is not enabled
    if (!snd_card || nosfxparm)
        return false;

    sfx = &S_sfx[sound_id];

    for (i = 0; i < numChannels; i++)
    {
        if (channels[i].sfxinfo == sfx && channels[i].origin == origin)
        {
            if (I_SoundIsPlaying(channels[i].handle))
            {
                return true;
            }
        }
    }
    return false;
}

int S_GetSoundID(const char *name)
{
    int i;

    for (i = 0; i < HEXEN_NUMSFX; i++)
    {
        if (!strcmp(S_sfx[i].tagname, name))
        {
            return i;
        }
    }
    return 0;
}

void S_StartSongName(const char *songLump, dboolean loop)
{
    int musicnum;

    // lazy shortcut hack - this is a unique character
    switch (songLump[1])
    {
      case 'e':
        musicnum = hexen_mus_hexen;
        break;
      case 'u':
        musicnum = hexen_mus_hub;
        break;
      case 'a':
        musicnum = hexen_mus_hall;
        break;
      case 'r':
        musicnum = hexen_mus_orb;
        break;
      case 'h':
        musicnum = hexen_mus_chess;
        break;
      default:
        musicnum = hexen_mus_hub;
        break;
    }

    S_ChangeMusic(musicnum, loop);
}

void S_InitScript(void)
{
    int i;

    SC_OpenLump("sndinfo");

    while (SC_GetString())
    {
        if (*sc_String == '$')
        {
            if (!strcasecmp(sc_String, "$ARCHIVEPATH"))
            {
                SC_MustGetString();
            }
            else if (!strcasecmp(sc_String, "$MAP"))
            {
                SC_MustGetNumber();
                SC_MustGetString();
                if (sc_Number)
                {
                    P_PutMapSongLump(sc_Number, sc_String);
                }
            }
            continue;
        }
        else
        {
            for (i = 0; i < HEXEN_NUMSFX; i++)
            {
                if (!strcmp(S_sfx[i].tagname, sc_String))
                {
                    SC_MustGetString();
                    if (*sc_String != '?')
                    {
                        S_sfx[i].name = strdup(sc_String);
                    }
                    else
                    {
                        S_sfx[i].name = strdup("default");
                    }
                    break;
                }
            }
            if (i == HEXEN_NUMSFX)
            {
                SC_MustGetString();
            }
        }
    }
    SC_Close();

    for (i = 0; i < HEXEN_NUMSFX; i++)
    {
        if (!strcmp(S_sfx[i].name, ""))
        {
            S_sfx[i].name = strdup("default");
        }
    }
}
