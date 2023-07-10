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
//	DSDA MAPINFO Doom Parser
//

#include <cstring>
#include <cstdio>
#include <vector>

extern "C" {
#include "z_zone.h"

#include "dsda/name.h"
#include "dsda/utility.h"
}

#include "scanner.h"

#include "parser.h"

static std::vector<doom_mapinfo_map_t> doom_mapinfo_maps;
static doom_mapinfo_map_t default_map;

static void dsda_SkipValue(Scanner &scanner) {
  if (scanner.CheckToken('=')) {
    do {
      scanner.GetNextToken();
    } while (scanner.CheckToken(','));

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

#define STR_DUP(x) { Z_Free(x); x = Z_Strdup(scanner.string); }

// The scanner drops the sign when scanning, and we need it back
static void dsda_FloatString(Scanner &scanner, char* &str) {
  if (scanner.decimal >= 0) {
    STR_DUP(str);
  }
  else {
    str = (char*) Z_Realloc(str, strlen(scanner.string) + 2);
    str[0] = '-';
    str[1] = '\0';
    strcat(str, scanner.string);
  }
}

#define SCAN_INT(x)  { scanner.MustGetToken('='); \
                       scanner.MustGetInteger(); \
                       x = scanner.number; }

#define SCAN_FLOAT(x) { scanner.MustGetToken('='); \
                        scanner.MustGetFloat(); \
                        x = scanner.decimal; }

#define SCAN_STRING(x) { scanner.MustGetToken('='); \
                         scanner.MustGetToken(TK_StringConst); \
                         STR_DUP(x); }

#define SCAN_FLOAT_STRING(x) { scanner.MustGetToken('='); \
                               scanner.MustGetFloat(); \
                               dsda_FloatString(scanner, x); }

static void dsda_FreeMap(doom_mapinfo_map_t &map) {
  Z_Free(map.lump_name);
  Z_Free(map.nice_name);
  Z_Free(map.title_patch);
  Z_Free(map.music);
  Z_Free(map.inter_music);
  Z_Free(map.exit_pic);
  Z_Free(map.enter_pic);
  Z_Free(map.border_texture);
  Z_Free(map.gravity);
  Z_Free(map.air_control);
  Z_Free(map.author);
  Z_Free(map.special_actions);

  Z_Free(map.next.map);
  Z_Free(map.next.endpic);
  Z_Free(map.secret_next.map);
  Z_Free(map.secret_next.endpic);

  Z_Free(map.sky1.lump);
}

#define REPLACE_WITH_COPY(x) { if (x) x = Z_Strdup(x); }

static void dsda_CopyMap(doom_mapinfo_map_t &dest, doom_mapinfo_map_t &source) {
  dest = source;

  REPLACE_WITH_COPY(dest.lump_name);
  REPLACE_WITH_COPY(dest.nice_name);
  REPLACE_WITH_COPY(dest.title_patch);
  REPLACE_WITH_COPY(dest.music);
  REPLACE_WITH_COPY(dest.inter_music);
  REPLACE_WITH_COPY(dest.exit_pic);
  REPLACE_WITH_COPY(dest.enter_pic);
  REPLACE_WITH_COPY(dest.border_texture);
  REPLACE_WITH_COPY(dest.gravity);
  REPLACE_WITH_COPY(dest.air_control);
  REPLACE_WITH_COPY(dest.author);

  REPLACE_WITH_COPY(dest.next.map);
  REPLACE_WITH_COPY(dest.next.endpic);
  REPLACE_WITH_COPY(dest.secret_next.map);
  REPLACE_WITH_COPY(dest.secret_next.endpic);

  REPLACE_WITH_COPY(dest.sky1.lump);

  dest.num_special_actions = 0;
  dest.special_actions = NULL;
}

static const char* end_names[dmi_end_count] = {
  NULL,
  "EndGame1",
  "EndGame2",
  "EndGame3",
  "EndGame4",
  "EndGameC",
  "EndTitle",
};

static void dsda_ParseDoomMapInfoMapNext(Scanner &scanner, doom_mapinfo_map_next_t &next) {
  scanner.MustGetToken('=');

  next.end = dmi_end_null;

  if (scanner.CheckToken(TK_StringConst)) {
    for (int i = 1; i < dmi_end_count; ++i)
      if (!stricmp(scanner.string, end_names[i])) {
        next.end = (dmi_end_t) i;
        return;
      }

    STR_DUP(next.map);
  }
  else if (scanner.CheckToken(TK_Identifier)) {
    if (!stricmp(scanner.string, "EndPic")) {
      scanner.MustGetToken(',');
      scanner.MustGetToken(TK_StringConst);
      STR_DUP(next.endpic);
    }
    else if (!stricmp(scanner.string, "endgame")) {
      // TODO: endgame block
      dsda_SkipValue(scanner);
    }
    else {
      dsda_SkipValue(scanner);
    }
  }
}

static void dsda_ParseDoomMapInfoMapSky(Scanner &scanner, doom_mapinfo_sky_t &sky) {
  scanner.MustGetToken('=');
  scanner.MustGetToken(TK_StringConst);

  STR_DUP(sky.lump);
  if (scanner.CheckToken(',')) {
    scanner.MustGetFloat();
    sky.scrollspeed = scanner.decimal;
  }
  else {
    sky.scrollspeed = 0.0;
  }
}

static void dsda_ParseDoomMapInfoTitlePatch(Scanner &scanner, doom_mapinfo_map_t &map) {
  scanner.MustGetToken('=');
  scanner.MustGetToken(TK_StringConst);

  STR_DUP(map.title_patch);
  if (scanner.CheckToken(',')) {
    scanner.MustGetInteger();
    if (scanner.number)
      map.flags &= ~DMI_SHOW_AUTHOR;
  }
}

static void dsda_ParseDoomMapInfoMusic(Scanner &scanner, doom_mapinfo_map_t &map) {
  scanner.MustGetToken('=');
  scanner.MustGetToken(TK_StringConst);

  if (strchr(scanner.string, ':'))
    return;

  if (scanner.CheckToken(',')) {
    scanner.MustGetInteger();
    return;
  }

  STR_DUP(map.music);
}

static void dsda_ParseDoomMapInfoIntermissionPic(Scanner &scanner, char* &pic) {
  scanner.MustGetToken('=');
  scanner.MustGetToken(TK_StringConst);

  if (scanner.string[0] == '$')
    return;

  STR_DUP(pic);
}

static void dsda_ParseDoomMapInfoMapSpecialAction(Scanner &scanner,
                                               std::vector<doom_mapinfo_special_action_t> &special_actions) {
  doom_mapinfo_special_action_t special_action = { 0 };

  scanner.MustGetToken('=');

  scanner.MustGetToken(TK_StringConst);
  special_action.monster_type = dsda_ActorNameToType(scanner.string);

  scanner.MustGetToken(',');

  scanner.MustGetToken(TK_StringConst);
  special_action.action_special = dsda_ActionNameToNumber(scanner.string);

  for (int i = 0; i < 5; ++i) {
    if (!scanner.CheckToken(','))
      break;

    scanner.MustGetInteger();
    special_action.special_args[i] = scanner.number;
  }

  if (special_action.monster_type == NAME_NOT_FOUND ||
      special_action.action_special == NAME_NOT_FOUND)
    return;

  special_actions.push_back(special_action);
}

static void dsda_GuessLevelNum(doom_mapinfo_map_t &map) {
  int level, episode;

  dsda_UppercaseString(map.lump_name);

  if (sscanf(map.lump_name, "MAP%d", &level) == 1)
    map.level_num = level;
  else if (sscanf(map.lump_name, "E%dM%d", &episode, &level) == 2)
    map.level_num = (episode - 1) * 10 + level;
  else
    map.level_num = -1;
}

static void dsda_ParseDoomMapInfoMapBlock(Scanner &scanner, doom_mapinfo_map_t &map,
                                       std::vector<doom_mapinfo_special_action_t> &special_actions) {
  scanner.MustGetToken('{');
  while (!scanner.CheckToken('}')) {
    scanner.MustGetToken(TK_Identifier);

    if (!stricmp(scanner.string, "LevelNum")) {
      SCAN_INT(map.level_num);
    }
    else if (!stricmp(scanner.string, "Next")) {
      dsda_ParseDoomMapInfoMapNext(scanner, map.next);
    }
    else if (!stricmp(scanner.string, "SecretNext")) {
      dsda_ParseDoomMapInfoMapNext(scanner, map.secret_next);
    }
    else if (!stricmp(scanner.string, "Cluster")) {
      SCAN_INT(map.cluster);
    }
    else if (!stricmp(scanner.string, "Sky1")) {
      dsda_ParseDoomMapInfoMapSky(scanner, map.sky1);
    }
    else if (!stricmp(scanner.string, "TitlePatch")) {
      dsda_ParseDoomMapInfoTitlePatch(scanner, map);
    }
    else if (!stricmp(scanner.string, "Par")) {
      SCAN_INT(map.par);
    }
    else if (!stricmp(scanner.string, "NoIntermission")) {
      map.flags &= ~DMI_INTERMISSION;
    }
    else if (!stricmp(scanner.string, "Intermission")) {
      map.flags |= DMI_INTERMISSION;
    }
    else if (!stricmp(scanner.string, "Music")) {
      dsda_ParseDoomMapInfoMusic(scanner, map);
    }
    else if (!stricmp(scanner.string, "ExitPic")) {
      dsda_ParseDoomMapInfoIntermissionPic(scanner, map.exit_pic);
    }
    else if (!stricmp(scanner.string, "EnterPic")) {
      dsda_ParseDoomMapInfoIntermissionPic(scanner, map.enter_pic);
    }
    else if (!stricmp(scanner.string, "InterMusic")) {
      SCAN_STRING(map.inter_music);
    }
    else if (!stricmp(scanner.string, "BorderTexture")) {
      SCAN_STRING(map.border_texture);
    }
    else if (!stricmp(scanner.string, "EvenLighting")) {
      map.lighting = dmi_lighting_even;
    }
    else if (!stricmp(scanner.string, "SmoothLighting")) {
      map.lighting = dmi_lighting_smooth;
    }
    else if (!stricmp(scanner.string, "Gravity")) {
      SCAN_FLOAT_STRING(map.gravity);
    }
    else if (!stricmp(scanner.string, "AirControl")) {
      SCAN_FLOAT_STRING(map.air_control);
    }
    else if (!stricmp(scanner.string, "AllowMonsterTelefrags")) {
      map.flags |= DMI_ALLOW_MONSTER_TELEFRAGS;
    }
    else if (!stricmp(scanner.string, "KillerActivatesDeathSpecials")) {
      map.flags &= ~DMI_ACTIVATE_OWN_DEATH_SPECIALS;
    }
    else if (!stricmp(scanner.string, "ActivateOwnDeathSpecials")) {
      map.flags |= DMI_ACTIVATE_OWN_DEATH_SPECIALS;
    }
    else if (!stricmp(scanner.string, "SpecialAction")) {
      dsda_ParseDoomMapInfoMapSpecialAction(scanner, special_actions);
    }
    else if (!stricmp(scanner.string, "StrictMonsterActivation")) {
      map.flags &= ~DMI_LAX_MONSTER_ACTIVATION;
    }
    else if (!stricmp(scanner.string, "LaxMonsterActivation")) {
      map.flags |= DMI_LAX_MONSTER_ACTIVATION;
    }
    else if (!stricmp(scanner.string, "MissileShootersActivateImpactLines")) {
      map.flags &= ~DMI_MISSILES_ACTIVATE_IMPACT_LINES;
    }
    else if (!stricmp(scanner.string, "MissilesActivateImpactLines")) {
      map.flags |= DMI_MISSILES_ACTIVATE_IMPACT_LINES;
    }
    else if (!stricmp(scanner.string, "FilterStarts")) {
      map.flags |= DMI_FILTER_STARTS;
    }
    else if (!stricmp(scanner.string, "AllowRespawn")) {
      map.flags |= DMI_ALLOW_RESPAWN;
    }
    else if (!stricmp(scanner.string, "NoJump")) {
      map.flags &= ~DMI_ALLOW_JUMP;
    }
    else if (!stricmp(scanner.string, "AllowJump")) {
      map.flags |= DMI_ALLOW_JUMP;
    }
    else if (!stricmp(scanner.string, "NoFreelook")) {
      map.flags &= ~DMI_ALLOW_FREE_LOOK;
    }
    else if (!stricmp(scanner.string, "AllowFreelook")) {
      map.flags |= DMI_ALLOW_FREE_LOOK;
    }
    else if (!stricmp(scanner.string, "NoCheckSwitchRange")) {
      map.flags &= ~DMI_CHECK_SWITCH_RANGE;
    }
    else if (!stricmp(scanner.string, "CheckSwitchRange")) {
      map.flags |= DMI_CHECK_SWITCH_RANGE;
    }
    else if (!stricmp(scanner.string, "ResetHealth")) {
      map.flags |= DMI_RESET_HEALTH;
    }
    else if (!stricmp(scanner.string, "ResetInventory")) {
      map.flags |= DMI_RESET_INVENTORY;
    }
    else if (!stricmp(scanner.string, "NoPassover")) {
      map.flags &= ~DMI_PASSOVER;
    }
    else if (!stricmp(scanner.string, "Passover")) {
      map.flags |= DMI_PASSOVER;
    }
    else if (!stricmp(scanner.string, "UsePlayerStartZ")) {
      map.flags |= DMI_USE_PLAYER_START_Z;
    }
    else if (!stricmp(scanner.string, "RandomPlayerStarts")) {
      map.flags |= DMI_RANDOM_PLAYER_STARTS;
    }
    else if (!stricmp(scanner.string, "ForgetState")) {
      map.flags &= ~DMI_REMEMBER_STATE;
    }
    else if (!stricmp(scanner.string, "RememberState")) {
      map.flags |= DMI_REMEMBER_STATE;
    }
    else if (!stricmp(scanner.string, "Author")) {
      SCAN_STRING(map.author);
    }
    else {
      dsda_SkipValue(scanner);
    }
  }

  map.num_special_actions = special_actions.size();
  map.special_actions = (doom_mapinfo_special_action_t *) Z_Realloc(
    map.special_actions, map.num_special_actions * sizeof(*map.special_actions)
  );
  memcpy(map.special_actions, &special_actions[0],
         map.num_special_actions * sizeof(*map.special_actions));
}

static void dsda_ParseDoomMapInfoMap(Scanner &scanner) {
  doom_mapinfo_map_t map;
  std::vector<doom_mapinfo_special_action_t> special_actions;

  // Create a copy of the existing special actions from the default map
  for (int i = 0; i < default_map.num_special_actions; ++i)
    special_actions.push_back(default_map.special_actions[i]);

  dsda_CopyMap(map, default_map);

  scanner.MustGetToken(TK_Identifier);
  STR_DUP(map.lump_name);

  // Lookup via a separate lump is not supported, so use the key instead
  if (scanner.CheckToken(TK_Identifier)) {
    if (!stricmp(scanner.string, "lookup"))
      scanner.MustGetToken(TK_Identifier);
    else
      scanner.ErrorF("Unknown map declaration %s", scanner.string);
  }
  else
    scanner.MustGetToken(TK_StringConst);

  STR_DUP(map.nice_name);

  dsda_GuessLevelNum(map);

  dsda_ParseDoomMapInfoMapBlock(scanner, map, special_actions);

  for (auto &old_map : doom_mapinfo_maps)
    if (old_map.level_num == map.level_num)
      old_map.level_num = 0;

  doom_mapinfo_maps.push_back(map);
}

static void dsda_InitDefaultMap(void) {
  dsda_FreeMap(default_map);

  memset(&default_map, 0, sizeof(default_map));

  default_map.gravity = Z_Strdup("800");
  default_map.flags = DMI_INTERMISSION |
                      DMI_ACTIVATE_OWN_DEATH_SPECIALS |
                      DMI_LAX_MONSTER_ACTIVATION |
                      DMI_MISSILES_ACTIVATE_IMPACT_LINES |
                      DMI_REMEMBER_STATE |
                      DMI_SHOW_AUTHOR;
}

static void dsda_ParseDoomMapInfoDefaultMap(Scanner &scanner) {
  std::vector<doom_mapinfo_special_action_t> special_actions;

  dsda_InitDefaultMap();

  dsda_ParseDoomMapInfoMapBlock(scanner, default_map, special_actions);
}

static void dsda_ParseDoomMapInfoAddDefaultMap(Scanner &scanner) {
  std::vector<doom_mapinfo_special_action_t> special_actions;

  // Create a copy of the existing special actions from the default map
  for (int i = 0; i < default_map.num_special_actions; ++i)
    special_actions.push_back(default_map.special_actions[i]);

  Z_Free(default_map.special_actions);
  default_map.num_special_actions = 0;

  dsda_ParseDoomMapInfoMapBlock(scanner, default_map, special_actions);
}

static void dsda_ParseDoomMapInfoIdentifier(Scanner &scanner) {
  scanner.MustGetToken(TK_Identifier);

  if (!stricmp(scanner.string, "map")) {
    dsda_ParseDoomMapInfoMap(scanner);
  }
  else if (!stricmp(scanner.string, "defaultmap")) {
    dsda_ParseDoomMapInfoDefaultMap(scanner);
  }
  else if (!stricmp(scanner.string, "adddefaultmap")) {
    dsda_ParseDoomMapInfoAddDefaultMap(scanner);
  }
  else {
    dsda_SkipValue(scanner);
  }
}

doom_mapinfo_t doom_mapinfo;

void dsda_ParseDoomMapInfo(const unsigned char* buffer, size_t length, doom_mapinfo_errorfunc err) {
  Scanner scanner((const char*) buffer, length);

  scanner.SetErrorCallback(err);

  DO_ONCE
    dsda_InitDefaultMap();
  END_ONCE

  while (scanner.TokensLeft())
    dsda_ParseDoomMapInfoIdentifier(scanner);

  doom_mapinfo.maps = &doom_mapinfo_maps[0];
  doom_mapinfo.num_maps = doom_mapinfo_maps.size();

  doom_mapinfo.loaded = true;
}
