//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
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

#include "h2def.h"
#include "m_random.h"
#include "i_cdmus.h"
#include "i_sound.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_argv.h"
#include "m_misc.h"
#include "r_local.h"
#include "p_local.h"            // for P_AproxDistance
#include "sounds.h"
#include "s_sound.h"

//==========================================================================
//
// S_UpdateSounds
//
//==========================================================================

void S_UpdateSounds(mobj_t * listener)
{
    int i, dist, vol;
    int angle;
    int sep;
    int priority;
    int absx;
    int absy;

    I_UpdateSound();

    if (snd_MaxVolume == 0)
    {
        return;
    }

    // Update any Sequences
    SN_UpdateActiveSequences();

    for (i = 0; i < snd_Channels; i++)
    {
        if (!Channel[i].handle || S_sfx[Channel[i].sound_id].usefulness == -1)
        {
            continue;
        }
        if (!I_SoundIsPlaying(Channel[i].handle))
        {
            if (S_sfx[Channel[i].sound_id].usefulness > 0)
            {
                S_sfx[Channel[i].sound_id].usefulness--;
            }
            Channel[i].handle = 0;
            Channel[i].mo = NULL;
            Channel[i].sound_id = 0;
        }
        if (Channel[i].mo == NULL || Channel[i].sound_id == 0
         || Channel[i].mo == listener || listener == NULL)
        {
            continue;
        }
        else
        {
            absx = abs(Channel[i].mo->x - listener->x);
            absy = abs(Channel[i].mo->y - listener->y);
            dist = absx + absy - (absx > absy ? absy >> 1 : absx >> 1);
            dist >>= FRACBITS;

            if (dist >= MAX_SND_DIST)
            {
                S_StopSound(Channel[i].mo);
                continue;
            }
            if (dist < 0)
            {
                dist = 0;
            }
            vol =
                (SoundCurve[dist] * (snd_MaxVolume * 8) *
                 Channel[i].volume) >> 14;
            if (Channel[i].mo == listener)
            {
                sep = 128;
            }
            else
            {
                angle = R_PointToAngle2(listener->x, listener->y,
                                        Channel[i].mo->x, Channel[i].mo->y);
                angle = (angle - viewangle) >> 24;
                sep = angle * 2 - 128;
                if (sep < 64)
                    sep = -sep;
                if (sep > 192)
                    sep = 512 - sep;
            }
            I_UpdateSoundParams(i, vol, sep);
            priority = S_sfx[Channel[i].sound_id].priority;
            priority *= PRIORITY_MAX_ADJUST - (dist / DIST_ADJUST);
            Channel[i].priority = priority;
        }
    }
}

//==========================================================================
//
// S_GetChannelInfo
//
//==========================================================================

void S_GetChannelInfo(SoundInfo_t * s)
{
    int i;
    ChanInfo_t *c;

    s->channelCount = snd_Channels;
    s->musicVolume = snd_MusicVolume;
    s->soundVolume = snd_MaxVolume;
    for (i = 0; i < snd_Channels; i++)
    {
        c = &s->chan[i];
        c->id = Channel[i].sound_id;
        c->priority = Channel[i].priority;
        c->name = S_sfx[c->id].name;
        c->mo = Channel[i].mo;

        if (c->mo != NULL)
        {
            c->distance = P_AproxDistance(c->mo->x - viewx, c->mo->y - viewy)
                >> FRACBITS;
        }
        else
        {
            c->distance = 0;
        }
    }
}

//==========================================================================
//
// S_InitScript
//
//==========================================================================

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
            for (i = 0; i < NUMSFX; i++)
            {
                if (!strcmp(S_sfx[i].tagname, sc_String))
                {
                    SC_MustGetString();
                    if (*sc_String != '?')
                    {
                        M_StringCopy(S_sfx[i].name, sc_String,
                                     sizeof(S_sfx[i].name));
                    }
                    else
                    {
                        M_StringCopy(S_sfx[i].name, "default",
                                     sizeof(S_sfx[i].name));
                    }
                    break;
                }
            }
            if (i == NUMSFX)
            {
                SC_MustGetString();
            }
        }
    }
    SC_Close();

    for (i = 0; i < NUMSFX; i++)
    {
        if (!strcmp(S_sfx[i].name, ""))
        {
            M_StringCopy(S_sfx[i].name, "default", sizeof(S_sfx[i].name));
        }
    }
}
