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
//	DSDA Ambient
//

#include <string>
#include <cstring>
#include <unordered_map>

extern "C" {
#include "lprintf.h"
#include "sounds.h"
#include "w_wad.h"
#include "z_zone.h"

#include "dsda/sfx.h"
}

#include "scanner.h"

#include "ambient.h"

typedef struct {
  char* lump_name;
  int sfx_id;
} named_sfx_t;

std::unordered_map<std::string, named_sfx_t> name_to_sfx;
std::unordered_map<int, ambient_sfx_t> id_to_ambient_sfx;

static void dsda_ParseAmbient(Scanner &scanner) {
  ambient_sfx_t amb_sfx = { 0 };
  int id;

  scanner.MustGetInteger();
  id = scanner.number;

  scanner.MustGetString();
  amb_sfx.sound_name = Z_Strdup(scanner.string);

  scanner.MustGetString();
  if (!stricmp(scanner.string, "point")) {
    amb_sfx.attenuation = scanner.CheckFloat() ? scanner.decimal : 1.0;
    scanner.MustGetString();
  }
  else if (!stricmp(scanner.string, "world") || !stricmp(scanner.string, "surround")) {
    amb_sfx.attenuation = 0;
    scanner.MustGetString();
  }
  else {
    amb_sfx.attenuation = 0;
  }

  if (!stricmp(scanner.string, "continuous")) {
    amb_sfx.min_tics = -1;
    amb_sfx.max_tics = -1;
  }
  else if (!stricmp(scanner.string, "random")) {
    scanner.MustGetFloat();
    amb_sfx.min_tics = 35 * scanner.decimal;
    scanner.MustGetFloat();
    amb_sfx.max_tics = 35 * scanner.decimal;
  }
  else if (!stricmp(scanner.string, "periodic")) {
    scanner.MustGetFloat();
    amb_sfx.min_tics = 35 * scanner.decimal;
    amb_sfx.max_tics = amb_sfx.min_tics;
  }

  scanner.MustGetFloat();
  amb_sfx.volume = BETWEEN(0.0, 1.0, scanner.decimal);

  if (!amb_sfx.min_tics && !amb_sfx.max_tics) {
    lprintf(LO_WARN, "Ambient sound %d has invalid parameters\n", id);
    Z_Free(amb_sfx.sound_name);
    return;
  }

  if (id_to_ambient_sfx[id].sound_name)
    Z_Free(id_to_ambient_sfx[id].sound_name);

  id_to_ambient_sfx[id] = amb_sfx;
}

static void dsda_ParseSndInfoLine(Scanner &scanner) {
  if (!scanner.CheckString()) {
    scanner.GetNextToken();
    scanner.SkipLine();
    return;
  }

  if (!stricmp(scanner.string, "$ambient"))
    dsda_ParseAmbient(scanner);
  else if (scanner.string[0] == '$') {
    scanner.SkipLine();
    return;
  }
  else {
    std::string name(scanner.string);

    scanner.CheckToken('='); // Optional
    scanner.MustGetString();

    if (!W_LumpNameExists(scanner.string)) {
      lprintf(LO_WARN, "Sound lump \"%s\" does not exist\n", scanner.string);
      return;
    }

    if (name_to_sfx[name].lump_name)
      Z_Free(name_to_sfx[name].lump_name);

    name_to_sfx[name].lump_name = Z_Strdup(scanner.string);
  }
}

static void dsda_ResolveAmbientSounds(void) {
  for (auto &named_sfx : name_to_sfx) {
    sfxinfo_t* new_sfx;
    int id;

    new_sfx = dsda_NewSFX(&id);
    new_sfx->name = named_sfx.second.lump_name;
    named_sfx.second.sfx_id = id;

    lprintf(LO_DEBUG, "Named sound: %s -> %s %d\n", named_sfx.first.c_str(), new_sfx->name, id);
  }

  for (auto &amb_sfx : id_to_ambient_sfx) {
    amb_sfx.second.sfx_id = name_to_sfx[amb_sfx.second.sound_name].sfx_id;

    if (!amb_sfx.second.sfx_id)
      lprintf(LO_WARN, "Sound \"%s\" does not exist\n", amb_sfx.second.sound_name);

    lprintf(LO_DEBUG, "Ambient sound: %d att: %f, vol: %f, t: %d %d, sfx: %d\n",
                     amb_sfx.first,
                     amb_sfx.second.attenuation,
                     amb_sfx.second.volume,
                     amb_sfx.second.min_tics,
                     amb_sfx.second.max_tics,
                     amb_sfx.second.sfx_id);
  }
}

ambient_sfx_t* dsda_AmbientSFX(int id) {
  return id_to_ambient_sfx[id].sfx_id ? &id_to_ambient_sfx[id] : NULL;
}

void dsda_LoadAmbientSndInfo(void) {
  int lump;

  lump = W_CheckNumForName("SNDINFO");

  if (lump == LUMP_NOT_FOUND)
    return;

  Scanner scanner((const char*) W_LumpByNum(lump), W_LumpLength(lump));

  scanner.SetErrorCallback(I_Error);

  while (scanner.TokensLeft())
    dsda_ParseSndInfoLine(scanner);

  dsda_ResolveAmbientSounds();
}