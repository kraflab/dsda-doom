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

#ifndef __DSDA_EXCMD__
#define __DSDA_EXCMD__

#include "d_ticcmd.h"

void dsda_EnableExCmd(void);
void dsda_DisableExCmd(void);
void dsda_EnableCasualExCmdFeatures(void);
void dsda_ResetCmd(ticcmd_t *cmd);
void dsda_ReadExCmd(ticcmd_t* cmd, const byte** p);
void dsda_WriteExCmd(char** p, ticcmd_t* cmd);
char* dsda_CmdBuffer(void);

#endif
