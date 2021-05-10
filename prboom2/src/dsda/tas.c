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
//	DSDA TAS Interface
//

#include <string.h>

#include "tas.h"

typedef struct {
  char forwardmove_enabled;
  char sidemove_enabled;
  ticcmd_t cmd;
} dsda_tascmd_t;

static dsda_tascmd_t persistent_command;

static int dsda_UpdateCommand(const char* part, signed short value, dsda_tascmd_t* tascmd) {
  if (!stricmp(part, "MF")) {
    tascmd->cmd.forwardmove = (signed char) value;
    tascmd->forwardmove_enabled = true;
  }
  else if (!stricmp(part, "MB")) {
    tascmd->cmd.forwardmove = (signed char) -value;
    tascmd->forwardmove_enabled = true;
  }
  else if (!stricmp(part, "SR")) {
    tascmd->cmd.sidemove = (signed char) value;
    tascmd->sidemove_enabled = true;
  }
  else if (!stricmp(part, "SL")) {
    tascmd->cmd.sidemove = (signed char) -value;
    tascmd->sidemove_enabled = true;
  }
  else
    return false;

  return true;
}

static void dsda_DisableCommand(dsda_tascmd_t* tascmd) {
  tascmd->forwardmove_enabled = false;
  tascmd->sidemove_enabled = false;
}

int dsda_UpdatePersistentCommand(const char* part, signed short value) {
  return dsda_UpdateCommand(part, value, &persistent_command);
}

void dsda_DisablePersistentCommand(void) {
  dsda_DisableCommand(&persistent_command);
}

void dsda_ApplyTasCommand(ticcmd_t* cmd) {
  if (persistent_command.forwardmove_enabled)
    cmd->forwardmove = persistent_command.cmd.forwardmove;
  if (persistent_command.sidemove_enabled)
    cmd->sidemove = persistent_command.cmd.sidemove;
}
