//
// Copyright(C) 2023 by Ryan Krafnick
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
//	DSDA ZMAPINFO Parser
//

#include <cstring>
#include <vector>

extern "C" {
char *Z_Strdup(const char *s);
void *Z_Malloc(size_t size);
}

#include "scanner.h"

#include "parser.h"

static std::vector<zmapinfo_map_t> zmapinfo_maps;

static void dsda_SkipValue(Scanner &scanner) {
  if (scanner.CheckToken('=')) {
    while (scanner.TokensLeft()) {
      if (scanner.CheckToken(';'))
        break;

      scanner.GetNextToken();
    }

    return;
  }

  scanner.MustGetToken('{');
  {
    int brace_count = 1;

    while (scanner.TokensLeft()) {
      if (scanner.CheckToken('}')) {
        --brace_count;
      }
      else if (scanner.CheckToken('{')) {
        ++brace_count;
      }

      if (!brace_count)
        break;

      scanner.GetNextToken();
    }

    return;
  }
}

// The scanner drops the sign when scanning, and we need it back
static char* dsda_FloatString(Scanner &scanner) {
  if (scanner.decimal >= 0)
    return Z_Strdup(scanner.string);

  char* buffer = (char*) Z_Malloc(strlen(scanner.string) + 2);
  buffer[0] = '-';
  buffer[1] = '\0';
  strcat(buffer, scanner.string);

  return buffer;
}

#define SCAN_INT(x)  { scanner.MustGetToken('='); \
                       scanner.MustGetInteger(); \
                       x = scanner.number; \
                       scanner.MustGetToken(';'); }

#define SCAN_FLOAT(x) { scanner.MustGetToken('='); \
                        scanner.MustGetFloat(); \
                        x = scanner.decimal; \
                        scanner.MustGetToken(';'); }

#define SCAN_FLAG(x, f) { scanner.MustGetToken('='); \
                          scanner.MustGetToken(TK_BoolConst); \
                          if (scanner.boolean) \
                            x |= f; \
                          scanner.MustGetToken(';'); }

#define SCAN_STRING_N(x, n) { scanner.MustGetToken('='); \
                              scanner.MustGetToken(TK_StringConst); \
                              strncpy(x, scanner.string, n); \
                              scanner.MustGetToken(';'); }

#define SCAN_STRING(x) { scanner.MustGetToken('='); \
                         scanner.MustGetToken(TK_StringConst); \
                         x = Z_Strdup(scanner.string); \
                         scanner.MustGetToken(';'); }

#define SCAN_FLOAT_STRING(x) { scanner.MustGetToken('='); \
                               scanner.MustGetFloat(); \
                               x = dsda_FloatString(scanner); \
                               scanner.MustGetToken(';'); }

static void dsda_ParseZMapInfoMap(Scanner &scanner) {
  zmapinfo_map_t map = { 0 };

  scanner.MustGetToken(TK_Identifier);
  map.lump_name = Z_Strdup(scanner.string);

  scanner.MustGetToken(TK_Identifier);

  // Lookup via a separate lump is not supported, so use the key instead
  if (!stricmp(scanner.string, "lookup"))
    scanner.MustGetToken(TK_Identifier);

  map.nice_name = Z_Strdup(scanner.string);

  // map.levelnum defaults from lump_name if it conforms

  scanner.MustGetToken('{');
  while (!scanner.CheckToken('}')) {
    scanner.MustGetToken(TK_Identifier);

    if (!stricmp(scanner.string, "levelnum")) {
      SCAN_INT(map.levelnum);
    }
    else {
      // known ignored fields:
      // SlideShow
      // DeathSequence
      dsda_SkipValue(scanner);
    }
  }

  zmapinfo_maps.push_back(map);
}

static void dsda_ParseZMapInfoIdentifier(Scanner &scanner) {
  scanner.MustGetToken(TK_Identifier);

  if (!stricmp(scanner.string, "map")) {
    dsda_ParseZMapInfoMap(scanner);
  }
  else {
    dsda_SkipValue(scanner);
  }
}

zmapinfo_t zmapinfo;

void dsda_ParseZMapInfo(const unsigned char* buffer, size_t length, zmapinfo_errorfunc err) {
  Scanner scanner((const char*) buffer, length);

  scanner.SetErrorCallback(err);

  zmapinfo_maps.clear();

  while (scanner.TokensLeft())
    dsda_ParseZMapInfoIdentifier(scanner);

  zmapinfo.maps = &zmapinfo_maps[0];
  zmapinfo.num_maps = zmapinfo_maps.size();
}
