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
// SV_SaveMap
//
//==========================================================================

void SV_SaveMap(void)
{
    char fileName[100];

    // Open the output file
    doom_snprintf(fileName, sizeof(fileName), "%shex6%02d.hxs", SavePath, gamemap);
    SV_OpenWrite(fileName);

    // Place a header marker
    SV_WriteLong(ASEG_MAP_HEADER);

    // Write the level timer
    SV_WriteLong(leveltime);

    // Set the mobj archive numbers
    SetMobjArchiveNums();

    ArchiveWorld();
    ArchivePolyobjs();
    ArchiveMobjs();
    ArchiveThinkers();
    ArchiveScripts();
    ArchiveSounds();
    ArchiveMisc();

    // Place a termination marker
    SV_WriteLong(ASEG_END);

    // Close the output file
    SV_Close();
}

//==========================================================================
//
// SV_LoadMap
//
//==========================================================================

void SV_LoadMap(void)
{
    char fileName[100];

    // Load a base level
    G_InitNew(gameskill, gameepisode, gamemap);

    // Remove all thinkers
    RemoveAllThinkers();

    // Create the name
    doom_snprintf(fileName, sizeof(fileName), "%shex6%02d.hxs", SavePath, gamemap);

    // Load the file
    SV_OpenRead(fileName);

    AssertSegment(ASEG_MAP_HEADER);

    // Read the level timer
    leveltime = SV_ReadLong();

    UnarchiveWorld();
    UnarchivePolyobjs();
    UnarchiveMobjs();
    UnarchiveThinkers();
    UnarchiveScripts();
    UnarchiveSounds();
    UnarchiveMisc();

    AssertSegment(ASEG_END);

    // Free mobj list and save buffer
    Z_Free(MobjList);
    SV_Close();
}

//==========================================================================
//
// SV_InitBaseSlot
//
//==========================================================================

void SV_InitBaseSlot(void)
{
    ClearSaveSlot(BASE_SLOT);
}

//==========================================================================
//
// ArchiveWorld
//
//==========================================================================

static void ArchiveWorld(void)
{
    int i;
    int j;
    sector_t *sec;
    line_t *li;
    side_t *si;

    SV_WriteLong(ASEG_WORLD);
    for (i = 0, sec = sectors; i < numsectors; i++, sec++)
    {
        SV_WriteWord(sec->floorheight >> FRACBITS);
        SV_WriteWord(sec->ceilingheight >> FRACBITS);
        SV_WriteWord(sec->floorpic);
        SV_WriteWord(sec->ceilingpic);
        SV_WriteWord(sec->lightlevel);
        SV_WriteWord(sec->special);
        SV_WriteWord(sec->tag);
        SV_WriteWord(sec->seqType);
    }
    for (i = 0, li = lines; i < numlines; i++, li++)
    {
        SV_WriteWord(li->flags);
        SV_WriteByte(li->special);
        SV_WriteByte(li->arg1);
        SV_WriteByte(li->arg2);
        SV_WriteByte(li->arg3);
        SV_WriteByte(li->arg4);
        SV_WriteByte(li->arg5);
        for (j = 0; j < 2; j++)
        {
            if (li->sidenum[j] == -1)
            {
                continue;
            }
            si = &sides[li->sidenum[j]];
            SV_WriteWord(si->textureoffset >> FRACBITS);
            SV_WriteWord(si->rowoffset >> FRACBITS);
            SV_WriteWord(si->toptexture);
            SV_WriteWord(si->bottomtexture);
            SV_WriteWord(si->midtexture);
        }
    }
}

//==========================================================================
//
// UnarchiveWorld
//
//==========================================================================

static void UnarchiveWorld(void)
{
    int i;
    int j;
    sector_t *sec;
    line_t *li;
    side_t *si;

    AssertSegment(ASEG_WORLD);
    for (i = 0, sec = sectors; i < numsectors; i++, sec++)
    {
        sec->floorheight = SV_ReadWord() << FRACBITS;
        sec->ceilingheight = SV_ReadWord() << FRACBITS;
        sec->floorpic = SV_ReadWord();
        sec->ceilingpic = SV_ReadWord();
        sec->lightlevel = SV_ReadWord();
        sec->special = SV_ReadWord();
        sec->tag = SV_ReadWord();
        sec->seqType = SV_ReadWord();
        sec->specialdata = 0;
        sec->soundtarget = 0;
    }
    for (i = 0, li = lines; i < numlines; i++, li++)
    {
        li->flags = SV_ReadWord();
        li->special = SV_ReadByte();
        li->arg1 = SV_ReadByte();
        li->arg2 = SV_ReadByte();
        li->arg3 = SV_ReadByte();
        li->arg4 = SV_ReadByte();
        li->arg5 = SV_ReadByte();
        for (j = 0; j < 2; j++)
        {
            if (li->sidenum[j] == -1)
            {
                continue;
            }
            si = &sides[li->sidenum[j]];
            si->textureoffset = SV_ReadWord() << FRACBITS;
            si->rowoffset = SV_ReadWord() << FRACBITS;
            si->toptexture = SV_ReadWord();
            si->bottomtexture = SV_ReadWord();
            si->midtexture = SV_ReadWord();
        }
    }
}

//==========================================================================
//
// SetMobjArchiveNums
//
// Sets the archive numbers in all mobj structs.  Also sets the MobjCount
// global.  Ignores player mobjs.
//
//==========================================================================

static void SetMobjArchiveNums(void)
{
    mobj_t *mobj;
    thinker_t *thinker;

    MobjCount = 0;
    for (thinker = thinkercap.next; thinker != &thinkercap;
         thinker = thinker->next)
    {
        if (thinker->function == P_MobjThinker)
        {
            mobj = (mobj_t *) thinker;
            if (mobj->player)
            {                   // Skipping player mobjs
                continue;
            }
            mobj->archiveNum = MobjCount++;
        }
    }
}

//==========================================================================
//
// ArchiveMobjs
//
//==========================================================================

static void ArchiveMobjs(void)
{
    int count;
    thinker_t *thinker;

    SV_WriteLong(ASEG_MOBJS);
    SV_WriteLong(MobjCount);
    count = 0;
    for (thinker = thinkercap.next; thinker != &thinkercap;
         thinker = thinker->next)
    {
        if (thinker->function != P_MobjThinker)
        {                       // Not a mobj thinker
            continue;
        }
        if (((mobj_t *) thinker)->player)
        {                       // Skipping player mobjs
            continue;
        }
        count++;
        StreamOut_mobj_t((mobj_t *) thinker);
    }
    if (count != MobjCount)
    {
        I_Error("ArchiveMobjs: bad mobj count");
    }
}

//==========================================================================
//
// UnarchiveMobjs
//
//==========================================================================

static void UnarchiveMobjs(void)
{
    int i;
    mobj_t *mobj;

    AssertSegment(ASEG_MOBJS);
    TargetPlayerAddrs = Z_Malloc(MAX_TARGET_PLAYERS * sizeof(mobj_t **),
                                 PU_STATIC, NULL);
    TargetPlayerCount = 0;
    MobjCount = SV_ReadLong();
    MobjList = Z_Malloc(MobjCount * sizeof(mobj_t *), PU_STATIC, NULL);
    for (i = 0; i < MobjCount; i++)
    {
        MobjList[i] = Z_Malloc(sizeof(mobj_t), PU_LEVEL, NULL);
    }
    for (i = 0; i < MobjCount; i++)
    {
        mobj = MobjList[i];
        StreamIn_mobj_t(mobj);

        // Restore broken pointers.
        mobj->info = &mobjinfo[mobj->type];
        P_SetThingPosition(mobj);
        mobj->floorz = mobj->subsector->sector->floorheight;
        mobj->ceilingz = mobj->subsector->sector->ceilingheight;

        mobj->thinker.function = P_MobjThinker;
        P_AddThinker(&mobj->thinker);
    }
    P_CreateTIDList();
    P_InitCreatureCorpseQueue(true);    // true = scan for corpses
}

//==========================================================================
//
// GetMobjNum
//
//==========================================================================

static int GetMobjNum(mobj_t * mobj)
{
    if (mobj == NULL)
    {
        return MOBJ_NULL;
    }
    if (mobj->player)
    {
        return MOBJ_XX_PLAYER;
    }
    return mobj->archiveNum;
}

//==========================================================================
//
// SetMobjPtr
//
//==========================================================================

static void SetMobjPtr(mobj_t **ptr, unsigned int archiveNum)
{
    if (archiveNum == MOBJ_NULL)
    {
        *ptr = NULL;
    }
    else if (archiveNum == MOBJ_XX_PLAYER)
    {
        if (TargetPlayerCount == MAX_TARGET_PLAYERS)
        {
            I_Error("RestoreMobj: exceeded MAX_TARGET_PLAYERS");
        }
        TargetPlayerAddrs[TargetPlayerCount++] = ptr;
        *ptr = NULL;
    }
    else
    {
        *ptr = MobjList[archiveNum];
    }
}

//==========================================================================
//
// Thinker types list.
//
// This is used by ArchiveThinkers and UnarchiveThinkers, below.
//
// Original comment:
// "This list has been prioritized using frequency estimates"
//
//==========================================================================

static thinkInfo_t ThinkerInfo[] = {
    {
     TC_MOVE_FLOOR,
     T_MoveFloor,
     StreamOut_floormove_t,
     StreamIn_floormove_t,
     RestoreSSThinker,
     sizeof(floormove_t)
    },
    {
     TC_PLAT_RAISE,
     T_PlatRaise,
     StreamOut_plat_t,
     StreamIn_plat_t,
     RestorePlatRaise,
     sizeof(plat_t)
    },
    {
     TC_MOVE_CEILING,
     T_MoveCeiling,
     StreamOut_ceiling_t,
     StreamIn_ceiling_t,
     RestoreMoveCeiling,
     sizeof(ceiling_t)
    },
    {
     TC_LIGHT,
     T_Light,
     StreamOut_light_t,
     StreamIn_light_t,
     NULL,
     sizeof(light_t)
    },
    {
     TC_VERTICAL_DOOR,
     T_VerticalDoor,
     StreamOut_vldoor_t,
     StreamIn_vldoor_t,
     RestoreSSThinker,
     sizeof(vldoor_t)
    },
    {
     TC_PHASE,
     T_Phase,
     StreamOut_phase_t,
     StreamIn_phase_t,
     NULL,
     sizeof(phase_t)
    },
    {
     TC_INTERPRET_ACS,
     T_InterpretACS,
     StreamOut_acs_t,
     StreamIn_acs_t,
     NULL,
     sizeof(acs_t)
    },
    {
     TC_ROTATE_POLY,
     T_RotatePoly,
     StreamOut_polyevent_t,
     StreamIn_polyevent_t,
     NULL,
     sizeof(polyevent_t)
    },
    {
     TC_BUILD_PILLAR,
     T_BuildPillar,
     StreamOut_pillar_t,
     StreamIn_pillar_t,
     RestoreSSThinker,
     sizeof(pillar_t)
    },
    {
     TC_MOVE_POLY,
     T_MovePoly,
     StreamOut_polyevent_t,
     StreamIn_polyevent_t,
     NULL,
     sizeof(polyevent_t)
    },
    {
     TC_POLY_DOOR,
     T_PolyDoor,
     StreamOut_polydoor_t,
     StreamIn_polydoor_t,
     NULL,
     sizeof(polydoor_t)
    },
    {
     TC_FLOOR_WAGGLE,
     T_FloorWaggle,
     StreamOut_floorWaggle_t,
     StreamIn_floorWaggle_t,
     RestoreSSThinker,
     sizeof(floorWaggle_t)
    },
    { TC_NULL, NULL, NULL, NULL, NULL, 0},
};

//==========================================================================
//
// ArchiveThinkers
//
//==========================================================================

static void ArchiveThinkers(void)
{
    thinker_t *thinker;
    thinkInfo_t *info;

    SV_WriteLong(ASEG_THINKERS);
    for (thinker = thinkercap.next; thinker != &thinkercap;
         thinker = thinker->next)
    {
        for (info = ThinkerInfo; info->tClass != TC_NULL; info++)
        {
            if (thinker->function == info->thinkerFunc)
            {
                SV_WriteByte(info->tClass);
                info->writeFunc(thinker);
                break;
            }
        }
    }
    // Add a termination marker
    SV_WriteByte(TC_NULL);
}

//==========================================================================
//
// UnarchiveThinkers
//
//==========================================================================

static void UnarchiveThinkers(void)
{
    int tClass;
    thinker_t *thinker;
    thinkInfo_t *info;

    AssertSegment(ASEG_THINKERS);
    while ((tClass = SV_ReadByte()) != TC_NULL)
    {
        for (info = ThinkerInfo; info->tClass != TC_NULL; info++)
        {
            if (tClass == info->tClass)
            {
                thinker = Z_Malloc(info->size, PU_LEVEL, NULL);
                info->readFunc(thinker);
                thinker->function = info->thinkerFunc;
                if (info->restoreFunc)
                {
                    info->restoreFunc(thinker);
                }
                P_AddThinker(thinker);
                break;
            }
        }
        if (info->tClass == TC_NULL)
        {
            I_Error("UnarchiveThinkers: Unknown tClass %d in "
                    "savegame", tClass);
        }
    }
}

//==========================================================================
//
// RestoreSSThinker
//
//==========================================================================

static void RestoreSSThinker(ssthinker_t *sst)
{
    sst->sector->specialdata = sst->thinker.function;
}

//==========================================================================
//
// RestorePlatRaise
//
//==========================================================================

static void RestorePlatRaise(plat_t *plat)
{
    plat->sector->specialdata = T_PlatRaise;
    P_AddActivePlat(plat);
}

//==========================================================================
//
// RestoreMoveCeiling
//
//==========================================================================

static void RestoreMoveCeiling(ceiling_t *ceiling)
{
    ceiling->sector->specialdata = T_MoveCeiling;
    P_AddActiveCeiling(ceiling);
}

//==========================================================================
//
// ArchiveScripts
//
//==========================================================================

static void ArchiveScripts(void)
{
    int i;

    SV_WriteLong(ASEG_SCRIPTS);
    for (i = 0; i < ACScriptCount; i++)
    {
        SV_WriteWord(ACSInfo[i].state);
        SV_WriteWord(ACSInfo[i].waitValue);
    }

    for (i = 0; i< MAX_ACS_MAP_VARS; ++i)
    {
        SV_WriteLong(MapVars[i]);
    }
}

//==========================================================================
//
// UnarchiveScripts
//
//==========================================================================

static void UnarchiveScripts(void)
{
    int i;

    AssertSegment(ASEG_SCRIPTS);
    for (i = 0; i < ACScriptCount; i++)
    {
        ACSInfo[i].state = SV_ReadWord();
        ACSInfo[i].waitValue = SV_ReadWord();
    }

    for (i = 0; i < MAX_ACS_MAP_VARS; ++i)
    {
        MapVars[i] = SV_ReadLong();
    }
}

//==========================================================================
//
// ArchiveMisc
//
//==========================================================================

static void ArchiveMisc(void)
{
    int ix;

    SV_WriteLong(ASEG_MISC);
    for (ix = 0; ix < MAXPLAYERS; ix++)
    {
        SV_WriteLong(localQuakeHappening[ix]);
    }
}

//==========================================================================
//
// UnarchiveMisc
//
//==========================================================================

static void UnarchiveMisc(void)
{
    int ix;

    AssertSegment(ASEG_MISC);
    for (ix = 0; ix < MAXPLAYERS; ix++)
    {
        localQuakeHappening[ix] = SV_ReadLong();
    }
}

//==========================================================================
//
// RemoveAllThinkers
//
//==========================================================================

static void RemoveAllThinkers(void)
{
    thinker_t *thinker;
    thinker_t *nextThinker;

    thinker = thinkercap.next;
    while (thinker != &thinkercap)
    {
        nextThinker = thinker->next;
        if (thinker->function == P_MobjThinker)
        {
            P_RemoveMobj((mobj_t *) thinker);
        }
        else
        {
            Z_Free(thinker);
        }
        thinker = nextThinker;
    }
    P_InitThinkers();
}

//==========================================================================
//
// ArchiveSounds
//
//==========================================================================

static void ArchiveSounds(void)
{
    seqnode_t *node;
    sector_t *sec;
    int difference;
    int i;

    SV_WriteLong(ASEG_SOUNDS);

    // Save the sound sequences
    SV_WriteLong(ActiveSequences);
    for (node = SequenceListHead; node; node = node->next)
    {
        SV_WriteLong(node->sequence);
        SV_WriteLong(node->delayTics);
        SV_WriteLong(node->volume);
        SV_WriteLong(SN_GetSequenceOffset(node->sequence,
                                           node->sequencePtr));
        SV_WriteLong(node->currentSoundID);
        for (i = 0; i < po_NumPolyobjs; i++)
        {
            if (node->mobj == (mobj_t *) & polyobjs[i].startSpot)
            {
                break;
            }
        }
        if (i == po_NumPolyobjs)
        {                       // Sound is attached to a sector, not a polyobj
            sec = R_PointInSubsector(node->mobj->x, node->mobj->y)->sector;
            difference = (int) ((byte *) sec
                                - (byte *) & sectors[0]) / sizeof(sector_t);
            SV_WriteLong(0);   // 0 -- sector sound origin
        }
        else
        {
            SV_WriteLong(1);   // 1 -- polyobj sound origin
            difference = i;
        }
        SV_WriteLong(difference);
    }
}

//==========================================================================
//
// UnarchiveSounds
//
//==========================================================================

static void UnarchiveSounds(void)
{
    int i;
    int numSequences;
    int sequence;
    int delayTics;
    int volume;
    int seqOffset;
    int soundID;
    int polySnd;
    int secNum;
    mobj_t *sndMobj;

    AssertSegment(ASEG_SOUNDS);

    // Reload and restart all sound sequences
    numSequences = SV_ReadLong();
    i = 0;
    while (i < numSequences)
    {
        sequence = SV_ReadLong();
        delayTics = SV_ReadLong();
        volume = SV_ReadLong();
        seqOffset = SV_ReadLong();

        soundID = SV_ReadLong();
        polySnd = SV_ReadLong();
        secNum = SV_ReadLong();
        if (!polySnd)
        {
            sndMobj = (mobj_t *) & sectors[secNum].soundorg;
        }
        else
        {
            sndMobj = (mobj_t *) & polyobjs[secNum].startSpot;
        }
        SN_StartSequence(sndMobj, sequence);
        SN_ChangeNodeData(i, seqOffset, delayTics, volume, soundID);
        i++;
    }
}

//==========================================================================
//
// ArchivePolyobjs
//
//==========================================================================

static void ArchivePolyobjs(void)
{
    int i;

    SV_WriteLong(ASEG_POLYOBJS);
    SV_WriteLong(po_NumPolyobjs);
    for (i = 0; i < po_NumPolyobjs; i++)
    {
        SV_WriteLong(polyobjs[i].tag);
        SV_WriteLong(polyobjs[i].angle);
        SV_WriteLong(polyobjs[i].startSpot.x);
        SV_WriteLong(polyobjs[i].startSpot.y);
    }
}

//==========================================================================
//
// UnarchivePolyobjs
//
//==========================================================================

static void UnarchivePolyobjs(void)
{
    int i;
    fixed_t deltaX;
    fixed_t deltaY;

    AssertSegment(ASEG_POLYOBJS);
    if (SV_ReadLong() != po_NumPolyobjs)
    {
        I_Error("UnarchivePolyobjs: Bad polyobj count");
    }
    for (i = 0; i < po_NumPolyobjs; i++)
    {
        if (SV_ReadLong() != polyobjs[i].tag)
        {
            I_Error("UnarchivePolyobjs: Invalid polyobj tag");
        }
        PO_RotatePolyobj(polyobjs[i].tag, (angle_t) SV_ReadLong());
        deltaX = SV_ReadLong() - polyobjs[i].startSpot.x;
        deltaY = SV_ReadLong() - polyobjs[i].startSpot.y;
        PO_MovePolyobj(polyobjs[i].tag, deltaX, deltaY);
    }
}

//==========================================================================
//
// AssertSegment
//
//==========================================================================

static void AssertSegment(gameArchiveSegment_t segType)
{
    if (SV_ReadLong() != segType)
    {
        I_Error("Corrupt save game: Segment [%d] failed alignment check",
                segType);
    }
}

//==========================================================================
//
// ClearSaveSlot
//
// Deletes all save game files associated with a slot number.
//
//==========================================================================

static void ClearSaveSlot(int slot)
{
    int i;
    char fileName[100];

    for (i = 0; i < MAX_MAPS; i++)
    {
        doom_snprintf(fileName, sizeof(fileName),
                   "%shex%d%02d.hxs", SavePath, slot, i);
        remove(fileName);
    }
    doom_snprintf(fileName, sizeof(fileName), "%shex%d.hxs", SavePath, slot);
    remove(fileName);
}

//==========================================================================
//
// CopySaveSlot
//
// Copies all the save game files from one slot to another.
//
//==========================================================================

static void CopySaveSlot(int sourceSlot, int destSlot)
{
    int i;
    char sourceName[100];
    char destName[100];

    for (i = 0; i < MAX_MAPS; i++)
    {
        doom_snprintf(sourceName, sizeof(sourceName),
                   "%shex%d%02d.hxs", SavePath, sourceSlot, i);
        if (ExistingFile(sourceName))
        {
            doom_snprintf(destName, sizeof(destName),
                       "%shex%d%02d.hxs", SavePath, destSlot, i);
            CopyFile(sourceName, destName);
        }
    }
    doom_snprintf(sourceName, sizeof(sourceName),
               "%shex%d.hxs", SavePath, sourceSlot);
    if (ExistingFile(sourceName))
    {
        doom_snprintf(destName, sizeof(destName),
                   "%shex%d.hxs", SavePath, destSlot);
        CopyFile(sourceName, destName);
    }
    else
    {
        I_Error("Could not load savegame %s", sourceName);
    }
}

//==========================================================================
//
// CopyFile
//
// This function was rewritten to copy files with minimal strain on zone
// allocation and allow for big maps that technically work in vanilla to
// save without error.
//==========================================================================

static void CopyFile(char *source_name, char *dest_name)
{
    const int BUFFER_CHUNK_SIZE = 0x10000;

    byte *buffer;
    int file_length, file_remaining;
    FILE *read_handle, *write_handle;
    int buf_count, read_count, write_count;

    read_handle = fopen(source_name, "rb");
    if (read_handle == NULL)
    {
        I_Error ("Couldn't read file %s", source_name);
    }
    file_length = file_remaining = M_FileLength(read_handle);

    // Vanilla savegame emulation.
    //
    // CopyFile() typically calls M_ReadFile() which stores the entire file
    // in memory: Chocolate Hexen should force an allocation error here
    // whenever it's appropriate.

    if (vanilla_savegame_limit)
    {
        buffer = Z_Malloc(file_length, PU_STATIC, NULL);
        Z_Free(buffer);
    }

    write_handle = fopen(dest_name, "wb");
    if (write_handle == NULL)
    {
        I_Error ("Couldn't read file %s", dest_name);
    }

    buffer = Z_Malloc (BUFFER_CHUNK_SIZE, PU_STATIC, NULL);

    do
    {
        buf_count = BUFFER_CHUNK_SIZE;
        if( file_remaining < BUFFER_CHUNK_SIZE)
        {
            buf_count = file_remaining;
        }

        read_count = fread(buffer, 1, buf_count, read_handle);
        if (read_count < buf_count)
        {
            I_Error ("Couldn't read file %s", source_name);
        }

        write_count = fwrite(buffer, 1, buf_count, write_handle);
        if (write_count < buf_count)
        {
            I_Error ("Couldn't write to file %s", dest_name);
        }

        file_remaining -= buf_count;
    } while (file_remaining > 0);

    Z_Free(buffer);
    fclose(read_handle);
    fclose(write_handle);
}

//==========================================================================
//
// ExistingFile
//
//==========================================================================

static dboolean ExistingFile(char *name)
{
    FILE *fp;

    if ((fp = fopen(name, "rb")) != NULL)
    {
        fclose(fp);
        return true;
    }
    else
    {
        return false;
    }
}

//==========================================================================
//
// SV_Open
//
//==========================================================================

static void SV_OpenRead(char *fileName)
{
    SavingFP = fopen(fileName, "rb");

    // Should never happen, only if hex6.hxs cannot ever be created.
    if (SavingFP == NULL)
    {
        I_Error("Could not load savegame %s", fileName);
    }
}

static void SV_OpenWrite(char *fileName)
{
    SavingFP = fopen(fileName, "wb");
}

//==========================================================================
//
// SV_Close
//
//==========================================================================

static void SV_Close(void)
{
    if (SavingFP)
    {
        fclose(SavingFP);
    }
}

//==========================================================================
//
// SV_Read
//
//==========================================================================

static void SV_Read(void *buffer, int size)
{
    int retval = fread(buffer, 1, size, SavingFP);
    if (retval != size)
    {
        I_Error("Incomplete read in SV_Read: Expected %d, got %d bytes",
            size, retval);
    }
}

static byte SV_ReadByte(void)
{
    byte result;
    SV_Read(&result, sizeof(byte));
    return result;
}

static uint16_t SV_ReadWord(void)
{
    uint16_t result;
    SV_Read(&result, sizeof(unsigned short));
    return SHORT(result);
}

static uint32_t SV_ReadLong(void)
{
    uint32_t result;
    SV_Read(&result, sizeof(int));
    return LONG(result);
}

static void *SV_ReadPtr(void)
{
    return (void *) (intptr_t) SV_ReadLong();
}

//==========================================================================
//
// SV_Write
//
//==========================================================================

static void SV_Write(const void *buffer, int size)
{
    fwrite(buffer, size, 1, SavingFP);
}

static void SV_WriteByte(byte val)
{
    fwrite(&val, sizeof(byte), 1, SavingFP);
}

static void SV_WriteWord(unsigned short val)
{
    val = SHORT(val);
    fwrite(&val, sizeof(unsigned short), 1, SavingFP);
}

static void SV_WriteLong(unsigned int val)
{
    val = LONG(val);
    fwrite(&val, sizeof(int), 1, SavingFP);
}

static void SV_WritePtr(void *val)
{
    long ptr;

    // Write a pointer value. In Vanilla Hexen pointers are 32-bit but
    // nowadays they might be larger. Whatever value we write here isn't
    // going to be much use when we reload the game.

    ptr = (long)(intptr_t) val;
    SV_WriteLong((unsigned int) (ptr & 0xffffffff));
}
