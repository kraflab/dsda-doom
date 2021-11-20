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

static char *tic_buf;
static byte tic_buf_size;
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

void dsda_ResetCmd(ticcmd_t* cmd) {
  excmd_t excmd;

  excmd = cmd->ex;

  memset(cmd, 0, sizeof(*cmd));

  if (excmd.data) {
    cmd->ex = excmd;
    memset(cmd->ex.data, 0, cmd->ex.allocated_size);
    cmd->ex.tic_size = 0;
  }
}

static void dsda_AppendAction(ticcmd_t* cmd, byte value, byte size) {
  if (cmd->ex.allocated_size < size)
    cmd->ex.data = realloc(cmd->ex.data, size);

  cmd->ex.data[size - 1] = value;
}

void dsda_ReadExCmd(ticcmd_t* cmd, const byte** p) {
  byte action = 0;
  byte action_count = 0;
  const byte* demo_p = *p;

  if (!excmd_enabled) return;

  while ((action = *demo_p++) != 0)
    dsda_AppendAction(cmd, action, ++action_count);

  *p = demo_p;
}

void dsda_WriteExCmd(char** p, ticcmd_t* cmd) {
  char* demo_p = *p;

  if (!excmd_enabled) return;

  if (1 + cmd->ex.tic_size + (demo_p - tic_buf) > tic_buf_size) {
    tic_buf_size = 1 + cmd->ex.tic_size + (demo_p - tic_buf);
    tic_buf = realloc(tic_buf, tic_buf_size);
  }

  if (cmd->ex.tic_size) {
    memcpy(demo_p, cmd->ex.data, cmd->ex.tic_size);

    demo_p += cmd->ex.tic_size;
  }

  // signifies end of extended command
  *demo_p++ = 0;

  *p = demo_p;
}

char* dsda_CmdBuffer(void) {
  if (!tic_buf) {
    tic_buf_size = 7;
    tic_buf = malloc(tic_buf_size);
  }

  memset(tic_buf, 0, tic_buf_size);

  return tic_buf;
}
