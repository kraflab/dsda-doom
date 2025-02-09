//
// Copyright(C) 2025 by Andrik Powell
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
//  DSDA GAMEINFO
//

#include <string.h>

extern "C" {
#include "d_main.h"
#include "w_wad.h"
#include "lprintf.h"
#include "z_zone.h"
}

#include "scanner.h"

#include "gameinfo.h"
    
void dsda_ParseGameInfoLine(Scanner &scanner) {

  if (!scanner.CheckString()) {
    scanner.GetNextToken();
    scanner.SkipLine();
    return;
  }

  if (!stricmp(scanner.string, "IWAD")) {
    scanner.MustGetToken('=');
    scanner.MustGetString();

    if (iwadlump)
      Z_Free(iwadlump);

    iwadlump = Z_Strdup(scanner.string);
  }
}

void dsda_LoadGameInfo(void) {
  int lump;

  lump = W_CheckNumForName("GAMEINFO");

  if (lump == LUMP_NOT_FOUND)
    return;

  Scanner scanner((const char*) W_LumpByNum(lump), W_LumpLength(lump));

  scanner.SetErrorCallback(I_Error);

  while (scanner.TokensLeft())
    dsda_ParseGameInfoLine(scanner);
}
