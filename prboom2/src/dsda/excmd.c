//
// Copyright(C) 2021 by Ryan Krafnick
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
//	DSDA Extended Cmd
//

#include <stdlib.h>
#include <string.h>

#include "excmd.h"

static dboolean excmd_enabled;
static dboolean casual_excmd_features;

void dsda_EnableExCmd(void) {
  excmd_enabled = true;
}

void dsda_DisableExCmd(void) {
  excmd_enabled = false;
}

void dsda_EnableCasualExCmdFeatures(void) {
  casual_excmd_features = true;
}

dboolean dsda_AllowCasualExCmdFeatures(void) {
  return casual_excmd_features;
}

void dsda_ReadExCmd(ticcmd_t* cmd, const byte** p) {
  const byte* demo_p = *p;

  if (!excmd_enabled) return;

  cmd->ex.actions = *demo_p++;
  if (cmd->ex.actions & XC_SAVE)
    cmd->ex.save_slot = *demo_p++;
  if (cmd->ex.actions & XC_LOAD)
    cmd->ex.load_slot = *demo_p++;

  *p = demo_p;
}

void dsda_WriteExCmd(char** p, ticcmd_t* cmd) {
  char* demo_p = *p;

  if (!excmd_enabled) return;

  *demo_p++ = cmd->ex.actions;
  if (cmd->ex.actions & XC_SAVE)
    *demo_p++ = cmd->ex.save_slot;
  if (cmd->ex.actions & XC_LOAD)
    *demo_p++ = cmd->ex.load_slot;

  *p = demo_p;
}
