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

//==========================================================================
//
// InitMapInfo
//
//==========================================================================

static void InitMapInfo(void)
{
    int map;
    int mapMax;
    int mcmdValue;
    mapInfo_t *info;
    char songMulch[10];
    const char *default_sky_name = DEFAULT_SKY_NAME;

    mapMax = 1;

    if (gamemode == shareware)
    {
	default_sky_name = "SKY2";
    }

    // Put defaults into MapInfo[0]
    info = MapInfo;
    info->cluster = 0;
    info->warpTrans = 0;
    info->nextMap = 1;          // Always go to map 1 if not specified
    info->cdTrack = 1;
    info->sky1Texture = R_TextureNumForName(default_sky_name);
    info->sky2Texture = info->sky1Texture;
    info->sky1ScrollDelta = 0;
    info->sky2ScrollDelta = 0;
    info->doubleSky = false;
    info->lightning = false;
    info->fadetable = W_GetNumForName(DEFAULT_FADE_TABLE);
    M_StringCopy(info->name, UNKNOWN_MAP_NAME, sizeof(info->name));

//    M_StringCopy(info->songLump, DEFAULT_SONG_LUMP, sizeof(info->songLump));
    SC_Open(MAPINFO_SCRIPT_NAME);
    while (SC_GetString())
    {
        if (SC_Compare("MAP") == false)
        {
            SC_ScriptError(NULL);
        }
        SC_MustGetNumber();
        if (sc_Number < 1 || sc_Number > 99)
        {                       //
            SC_ScriptError(NULL);
        }
        map = sc_Number;

        info = &MapInfo[map];

        // Save song lump name
        M_StringCopy(songMulch, info->songLump, sizeof(songMulch));

        // Copy defaults to current map definition
        memcpy(info, &MapInfo[0], sizeof(*info));

        // Restore song lump name
        M_StringCopy(info->songLump, songMulch, sizeof(info->songLump));

        // The warp translation defaults to the map number
        info->warpTrans = map;

        // Map name must follow the number
        SC_MustGetString();
        M_StringCopy(info->name, sc_String, sizeof(info->name));

        // Process optional tokens
        while (SC_GetString())
        {
            if (SC_Compare("MAP"))
            {                   // Start next map definition
                SC_UnGet();
                break;
            }
            mcmdValue = MapCmdIDs[SC_MustMatchString(MapCmdNames)];
            switch (mcmdValue)
            {
                case MCMD_CLUSTER:
                    SC_MustGetNumber();
                    info->cluster = sc_Number;
                    break;
                case MCMD_WARPTRANS:
                    SC_MustGetNumber();
                    info->warpTrans = sc_Number;
                    break;
                case MCMD_NEXT:
                    SC_MustGetNumber();
                    info->nextMap = sc_Number;
                    break;
                case MCMD_CDTRACK:
                    SC_MustGetNumber();
                    info->cdTrack = sc_Number;
                    break;
                case MCMD_SKY1:
                    SC_MustGetString();
                    info->sky1Texture = R_TextureNumForName(sc_String);
                    SC_MustGetNumber();
                    info->sky1ScrollDelta = sc_Number << 8;
                    break;
                case MCMD_SKY2:
                    SC_MustGetString();
                    info->sky2Texture = R_TextureNumForName(sc_String);
                    SC_MustGetNumber();
                    info->sky2ScrollDelta = sc_Number << 8;
                    break;
                case MCMD_DOUBLESKY:
                    info->doubleSky = true;
                    break;
                case MCMD_LIGHTNING:
                    info->lightning = true;
                    break;
                case MCMD_FADETABLE:
                    SC_MustGetString();
                    info->fadetable = W_GetNumForName(sc_String);
                    break;
                case MCMD_CD_STARTTRACK:
                case MCMD_CD_END1TRACK:
                case MCMD_CD_END2TRACK:
                case MCMD_CD_END3TRACK:
                case MCMD_CD_INTERTRACK:
                case MCMD_CD_TITLETRACK:
                    SC_MustGetNumber();
                    cd_NonLevelTracks[mcmdValue - MCMD_CD_STARTTRACK] =
                        sc_Number;
                    break;
            }
        }
        mapMax = map > mapMax ? map : mapMax;
    }
    SC_Close();
    MapCount = mapMax;
}

//==========================================================================
//
// P_GetMapCluster
//
//==========================================================================

int P_GetMapCluster(int map)
{
    return MapInfo[QualifyMap(map)].cluster;
}

//==========================================================================
//
// P_GetMapCDTrack
//
//==========================================================================

int P_GetMapCDTrack(int map)
{
    return MapInfo[QualifyMap(map)].cdTrack;
}

//==========================================================================
//
// P_GetMapWarpTrans
//
//==========================================================================

int P_GetMapWarpTrans(int map)
{
    return MapInfo[QualifyMap(map)].warpTrans;
}

//==========================================================================
//
// P_GetMapNextMap
//
//==========================================================================

int P_GetMapNextMap(int map)
{
    return MapInfo[QualifyMap(map)].nextMap;
}

//==========================================================================
//
// P_TranslateMap
//
// Returns the actual map number given a warp map number.
//
//==========================================================================

int P_TranslateMap(int map)
{
    int i;

    for (i = 1; i < 99; i++)    // Make this a macro
    {
        if (MapInfo[i].warpTrans == map)
        {
            return i;
        }
    }
    // Not found
    return -1;
}

//==========================================================================
//
// P_GetMapSky1Texture
//
//==========================================================================

int P_GetMapSky1Texture(int map)
{
    return MapInfo[QualifyMap(map)].sky1Texture;
}

//==========================================================================
//
// P_GetMapSky2Texture
//
//==========================================================================

int P_GetMapSky2Texture(int map)
{
    return MapInfo[QualifyMap(map)].sky2Texture;
}

//==========================================================================
//
// P_GetMapName
//
//==========================================================================

char *P_GetMapName(int map)
{
    return MapInfo[QualifyMap(map)].name;
}

//==========================================================================
//
// P_GetMapSky1ScrollDelta
//
//==========================================================================

fixed_t P_GetMapSky1ScrollDelta(int map)
{
    return MapInfo[QualifyMap(map)].sky1ScrollDelta;
}

//==========================================================================
//
// P_GetMapSky2ScrollDelta
//
//==========================================================================

fixed_t P_GetMapSky2ScrollDelta(int map)
{
    return MapInfo[QualifyMap(map)].sky2ScrollDelta;
}

//==========================================================================
//
// P_GetMapDoubleSky
//
//==========================================================================

dboolean P_GetMapDoubleSky(int map)
{
    return MapInfo[QualifyMap(map)].doubleSky;
}

//==========================================================================
//
// P_GetMapLightning
//
//==========================================================================

dboolean P_GetMapLightning(int map)
{
    return MapInfo[QualifyMap(map)].lightning;
}

//==========================================================================
//
// P_GetMapFadeTable
//
//==========================================================================

dboolean P_GetMapFadeTable(int map)
{
    return MapInfo[QualifyMap(map)].fadetable;
}

//==========================================================================
//
// P_GetMapSongLump
//
//==========================================================================

char *P_GetMapSongLump(int map)
{
    if (!strcasecmp(MapInfo[QualifyMap(map)].songLump, DEFAULT_SONG_LUMP))
    {
        return NULL;
    }
    else
    {
        return MapInfo[QualifyMap(map)].songLump;
    }
}

//==========================================================================
//
// P_PutMapSongLump
//
//==========================================================================

void P_PutMapSongLump(int map, char *lumpName)
{
    if (map < 1 || map > MapCount)
    {
        return;
    }
    M_StringCopy(MapInfo[map].songLump, lumpName,
                 sizeof(MapInfo[map].songLump));
}

//==========================================================================
//
// P_GetCDStartTrack
//
//==========================================================================

int P_GetCDStartTrack(void)
{
    return cd_NonLevelTracks[MCMD_CD_STARTTRACK - MCMD_CD_STARTTRACK];
}

//==========================================================================
//
// P_GetCDEnd1Track
//
//==========================================================================

int P_GetCDEnd1Track(void)
{
    return cd_NonLevelTracks[MCMD_CD_END1TRACK - MCMD_CD_STARTTRACK];
}

//==========================================================================
//
// P_GetCDEnd2Track
//
//==========================================================================

int P_GetCDEnd2Track(void)
{
    return cd_NonLevelTracks[MCMD_CD_END2TRACK - MCMD_CD_STARTTRACK];
}

//==========================================================================
//
// P_GetCDEnd3Track
//
//==========================================================================

int P_GetCDEnd3Track(void)
{
    return cd_NonLevelTracks[MCMD_CD_END3TRACK - MCMD_CD_STARTTRACK];
}

//==========================================================================
//
// P_GetCDIntermissionTrack
//
//==========================================================================

int P_GetCDIntermissionTrack(void)
{
    return cd_NonLevelTracks[MCMD_CD_INTERTRACK - MCMD_CD_STARTTRACK];
}

//==========================================================================
//
// P_GetCDTitleTrack
//
//==========================================================================

int P_GetCDTitleTrack(void)
{
    return cd_NonLevelTracks[MCMD_CD_TITLETRACK - MCMD_CD_STARTTRACK];
}

//==========================================================================
//
// QualifyMap
//
//==========================================================================

static int QualifyMap(int map)
{
    return (map < 1 || map > MapCount) ? 0 : map;
}

//==========================================================================
//
// P_Init
//
//==========================================================================

void P_Init(void)
{
    InitMapInfo();
    P_InitSwitchList();
    P_InitFTAnims();            // Init flat and texture animations
    P_InitTerrainTypes();
    P_InitLava();
    R_InitSprites(sprnames);
}


// Special early initializer needed to start sound before R_Init()
void InitMapMusicInfo(void)
{
    int i;

    for (i = 0; i < 99; i++)
    {
        M_StringCopy(MapInfo[i].songLump, DEFAULT_SONG_LUMP,
                     sizeof(MapInfo[i].songLump));
    }
    MapCount = 98;
}

/*
void My_Debug(void)
{
	int i;

	printf("My debug stuff ----------------------\n");
	printf("gamemap=%d\n",gamemap);
	for (i=0; i<10; i++)
	{
		printf("i=%d  songlump=%s\n",i,MapInfo[i].songLump);
	}
}
*/
