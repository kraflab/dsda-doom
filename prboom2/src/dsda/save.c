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

#include "lprintf.h"
#include "z_zone.h"

#include "dsda/data_organizer.h"
#include "save.h"

int dsda_organized_saves;
static char* dsda_base_save_dir;
static char* dsda_wad_save_dir;

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

char* dsda_SaveGameName(int slot, int demo_save) {
  int length;
  char* name;
  const char* save_dir;
  const char* save_type;

  if (slot > 9999 || slot < 0)
    I_Error("dsda_SaveGameName: bad save slot %d", slot);

  save_dir = dsda_SaveDir();

  save_type = demo_save ? "demosav" : savegamename;

  length = strlen(save_type) + strlen(save_dir) + 10; // "/" + "9999.dsg\0"

  name = malloc(length);

  snprintf(name, length, "%s/%s%d.dsg", save_dir, save_type, slot);

  return name;
}
