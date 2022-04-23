//
// Copyright(C) 2020 by Ryan Krafnick
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
// DESCRIPTION:
//	DSDA Save
//

#include <stdlib.h>

#include "doomstat.h"
#include "lprintf.h"
#include "z_zone.h"
#include "p_saveg.h"
#include "p_map.h"

#include "dsda/data_organizer.h"
#include "dsda/excmd.h"
#include "save.h"

int dsda_organized_saves;
static char* dsda_base_save_dir;
static char* dsda_wad_save_dir;

#define TRACKING_SIZE sizeof(int)

static void dsda_ArchiveInternal(void) {
  extern int dsda_max_kill_requirement;
  int internal_size = sizeof(dsda_max_kill_requirement);

  CheckSaveGame(sizeof(internal_size));
  memcpy(save_p, &internal_size, sizeof(internal_size));
  save_p += sizeof(internal_size);

  CheckSaveGame(sizeof(dsda_max_kill_requirement));
  memcpy(save_p, &dsda_max_kill_requirement, sizeof(dsda_max_kill_requirement));
  save_p += sizeof(dsda_max_kill_requirement);
}

static void dsda_UnArchiveInternal(void) {
  extern int dsda_max_kill_requirement;
  int internal_size;

  memcpy(&internal_size, save_p, sizeof(internal_size));
  save_p += sizeof(internal_size);

  if (internal_size > 0)
  {
    memcpy(&dsda_max_kill_requirement, save_p, sizeof(dsda_max_kill_requirement));
    save_p += sizeof(dsda_max_kill_requirement);
  }

  if (internal_size > TRACKING_SIZE)
  {
    save_p += internal_size - TRACKING_SIZE;
  }
}

void dsda_ArchiveAll(void) {
  P_ArchiveACS();
  P_ArchivePlayers();
  P_ThinkerToIndex();
  P_ArchiveWorld();
  P_ArchivePolyobjs();
  P_TrueArchiveThinkers();
  P_ArchiveScripts();
  P_ArchiveSounds();
  P_ArchiveMisc();
  P_IndexToThinker();
  P_ArchiveRNG();
  P_ArchiveMap();

  dsda_ArchiveInternal();
}

void dsda_UnArchiveAll(void) {
  P_MapStart();
  P_UnArchiveACS();
  P_UnArchivePlayers();
  P_UnArchiveWorld();
  P_UnArchivePolyobjs();
  P_TrueUnArchiveThinkers();
  P_UnArchiveScripts();
  P_UnArchiveSounds();
  P_UnArchiveMisc();
  P_UnArchiveRNG();
  P_UnArchiveMap();
  P_MapEnd();

  dsda_UnArchiveInternal();
}

void dsda_InitSaveDir(void) {
  dsda_base_save_dir = dsda_DetectDirectory("DOOMSAVEDIR", "-save");
}

static char* dsda_SaveDir(void) {
  if (dsda_organized_saves) {
    if (!dsda_wad_save_dir)
      dsda_wad_save_dir = dsda_DataDir();

    return dsda_wad_save_dir;
  }

  return dsda_base_save_dir;
}

extern const char* savegamename;

char* dsda_SaveGameName(int slot, dboolean via_cmd) {
  int length;
  char* name;
  const char* save_dir;
  const char* save_type;

  if (slot > 9999 || slot < 0)
    I_Error("dsda_SaveGameName: bad save slot %d", slot);

  save_dir = dsda_SaveDir();

  save_type = (via_cmd && demoplayback) ? "demosav" : savegamename;

  length = strlen(save_type) + strlen(save_dir) + 10; // "/" + "9999.dsg\0"

  name = malloc(length);

  snprintf(name, length, "%s/%s%d.dsg", save_dir, save_type, slot);

  return name;
}

static int* demo_save_slots;
static int allocated_save_slot_count;
static int demo_save_slot_count;

void dsda_ResetDemoSaveSlots(void) {
  demo_save_slot_count = 0;
}

static void dsda_MarkSaveSlotUsed(int slot) {
  int i;

  if (!demorecording) return;

  for (i = 0; i < demo_save_slot_count; ++i)
    if (demo_save_slots[i] == slot)
      return;

  ++demo_save_slot_count;
  if (demo_save_slot_count > allocated_save_slot_count) {
    allocated_save_slot_count = allocated_save_slot_count ? allocated_save_slot_count * 2 : 8;
    demo_save_slots =
      realloc(demo_save_slots, allocated_save_slot_count * sizeof(*demo_save_slots));
  }

  demo_save_slots[demo_save_slot_count - 1] = slot;
}

int dsda_AllowMenuLoad(int slot) {
  int i;

  if (!demorecording) return true;
  if (!dsda_AllowCasualExCmdFeatures()) return false;

  for (i = 0; i < demo_save_slot_count; ++i)
    if (demo_save_slots[i] == slot)
      return true;

  return false;
}

int dsda_AllowAnyMenuLoad(void) {
  return !demorecording || dsda_AllowCasualExCmdFeatures();
}

static int last_save_file_slot = -1;

void dsda_SetLastLoadSlot(int slot) {
  last_save_file_slot = slot;
}

void dsda_SetLastSaveSlot(int slot) {
  dsda_MarkSaveSlotUsed(slot);
  last_save_file_slot = slot;
}

int dsda_LastSaveSlot(void) {
  return last_save_file_slot;
}

void dsda_ResetLastSaveSlot(void) {
  last_save_file_slot = -1;
}
