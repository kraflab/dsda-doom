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
