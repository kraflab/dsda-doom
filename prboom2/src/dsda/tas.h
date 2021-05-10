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

#ifndef __DSDA_TAS__
#define __DSDA_TAS__

#include "d_ticcmd.h"

int dsda_UpdatePersistentCommand(const char* part, signed short value);
void dsda_DisablePersistentCommand(void);
void dsda_ApplyTasCommand(ticcmd_t* cmd);

#endif
