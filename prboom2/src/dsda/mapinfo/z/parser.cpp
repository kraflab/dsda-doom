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
void *Z_Realloc(void *p, size_t n);
}

#include "scanner.h"

#include "parser.h"

static std::vector<zmapinfo_map_t> zmapinfo_maps;
static zmapinfo_map_t default_map;

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

// The scanner drops the sign when scanning, and we need it back
static void dsda_FloatString(Scanner &scanner, const char* &str) {
  if (scanner.decimal >= 0)
    DUP_STR(str);
  else {
    str = (char*) Z_Realloc(strlen(scanner.string) + 2);
    str[0] = '-';
    str[1] = '\0';
    strcat(str, scanner.string);
  }
}

#define STR_DUP(x) { Z_Free(x); x = Z_Strdup(scanner.string); }

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
                               Z_Free(x); \
                               dsda_FloatString(scanner, x); }

static const char* end_names[zmn_end_count] = {
  [zmn_end_game_1] = "EndGame1",
  [zmn_end_game_2] = "EndGame2",
  [zmn_end_game_3] = "EndGame3",
  [zmn_end_game_4] = "EndGame4",
  [zmn_end_game_d2] = "EndGameC",
  [zmn_end_title] = "EndTitle",
};

static void dsda_ParseZMapInfoMapNext(Scanner &scanner, zmapinfo_map_next_t &next) {
  scanner.MustGetToken('=');

  next.end = zmn_end_null;

  if (scanner.CheckToken(TK_StringConst)) {
    for (int i = 1; i < zmn_end_count; ++i)
      if (!stricmp(scanner.string, end_names[i])) {
        next.end = i;
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

static void dsda_ParseZMapInfoMapSky(Scanner &scanner, zmapinfo_sky_t &sky) {
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

static void dsda_ParseZMapInfoTitlePatch(Scanner &scanner, zmapinfo_map_t &map) {
  scanner.MustGetToken('=');
  scanner.MustGetToken(TK_StringConst);

  STR_DUP(map.title_patch);
  if (scanner.CheckToken(',')) {
    scanner.MustGetInteger();
    if (scanner.number)
      map.flags &= ~ZM_SHOW_AUTHOR;
  }
}

static void dsda_ParseZMapInfoMusic(Scanner &scanner, zmapinfo_map_t &map) {
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

static void dsda_ParseZMapInfoIntermissionPic(Scanner &scanner, const char* &pic) {
  scanner.MustGetToken('=');
  scanner.MustGetToken(TK_StringConst);

  if (scanner.string[0] == '$')
    return;

  STR_DUP(pic);
}

// TODO: type lookup
static int dsda_ThingNameToType(const char* name) {
  return 0;
}

 // TODO: action lookup
static int dsda_ActionNameToNumber(const char* name) {
  return 0;
}

static void dsda_ParseZMapInfoMapSpecialAction(Scanner &scanner,
                                               std::vector<zmapinfo_special_action_t> &special_actions) {
  zmapinfo_special_action_t special_action = { 0 };

  scanner.MustGetToken('=');

  scanner.MustGetToken(TK_StringConst);
  special_action.monster_type = dsda_ThingNameToType(scanner.string);

  scanner.MustGetToken(TK_StringConst);
  special_action.action_special = dsda_ActionNameToNumber(scanner.string);

  for (int i = 0; i < 5; ++i) {
    if (!scanner.CheckToken(','))
      break;

    scanner.MustGetInteger();
    special_action.special_args[i] = scanner.number;
  }

  special_actions.push_back(special_action);
}

static void dsda_GuessLevelNum(zmapinfo_map_t &map) {
  int map, episode;

  if (sscanf(map.lump_name, "%1[mM]%1[aA]%1[pP]%d", &map) == 1)
    map.levelnum = map;
  else if (sscanf(map.lump_name, "%1[eE]%d%1[mM]%d", &episode, &map) == 2)
    map.levelnum = (episode - 1) * 10 + map;
  else
    map.levelnum = -1;
}

static void dsda_ParseZMapInfoMapBlock(Scanner &scanner, zmapinfo_map_t &map,
                                       std::vector<zmapinfo_special_action_t> &special_actions) {
  scanner.MustGetToken('{');
  while (!scanner.CheckToken('}')) {
    scanner.MustGetToken(TK_Identifier);

    if (!stricmp(scanner.string, "LevelNum")) {
      SCAN_INT(map.level_num);
    }
    else if (!stricmp(scanner.string, "Next")) {
      dsda_ParseZMapInfoMapNext(scanner, map.next);
    }
    else if (!stricmp(scanner.string, "SecretNext")) {
      dsda_ParseZMapInfoMapNext(scanner, map.secret_next);
    }
    else if (!stricmp(scanner.string, "Cluster")) {
      SCAN_INT(map.cluster);
    }
    else if (!stricmp(scanner.string, "Sky1")) {
      dsda_ParseZMapInfoMapSky(scanner, map.sky1);
    }
    else if (!stricmp(scanner.string, "Sky2")) {
      dsda_ParseZMapInfoMapSky(scanner, map.sky2);
    }
    else if (!stricmp(scanner.string, "DoubleSky")) {
      map.flags |= ZM_DOUBLE_SKY;
    }
    else if (!stricmp(scanner.string, "ForceNoSkyStretch")) {
      map.flags &= ~ZM_SKY_STRETCH;
    }
    else if (!stricmp(scanner.string, "SkyStretch")) {
      map.flags |= ZM_SKY_STRETCH;
    }
    else if (!stricmp(scanner.string, "FadeTable")) {
      SCAN_STRING(map.fade_table);
    }
    else if (!stricmp(scanner.string, "TitlePatch")) {
      dsda_ParseZMapInfoTitlePatch(scanner, map);
    }
    else if (!stricmp(scanner.string, "Par")) {
      SCAN_INT(map.par);
    }
    else if (!stricmp(scanner.string, "NoIntermission")) {
      map.flags &= ~ZM_INTERMISSION;
    }
    else if (!stricmp(scanner.string, "Intermission")) {
      map.flags |= ZM_INTERMISSION;
    }
    else if (!stricmp(scanner.string, "Music")) {
      dsda_ParseZMapInfoMusic(scanner, map);
    }
    else if (!stricmp(scanner.string, "ExitPic")) {
      dsda_ParseZMapInfoIntermissionPic(scanner, map.exit_pic);
    }
    else if (!stricmp(scanner.string, "EnterPic")) {
      dsda_ParseZMapInfoIntermissionPic(scanner, map.enter_pic);
    }
    else if (!stricmp(scanner.string, "InterMusic")) {
      SCAN_STRING(map.inter_music);
    }
    else if (!stricmp(scanner.string, "BorderTexture")) {
      SCAN_STRING(map.border_texture);
    }
    else if (!stricmp(scanner.string, "Lightning")) {
      map.flags |= ZM_LIGHTNING;
    }
    else if (!stricmp(scanner.string, "EvenLighting")) {
      map.lighting = zm_lighting_even;
    }
    else if (!stricmp(scanner.string, "SmoothLighting")) {
      map.lighting = zm_lighting_smooth;
    }
    else if (!stricmp(scanner.string, "Gravity")) {
      SCAN_INT(map.gravity);
    }
    else if (!stricmp(scanner.string, "AirControl")) {
      SCAN_FLOAT_STRING(map.air_control);
    }
    else if (!stricmp(scanner.string, "AllowMonsterTelefrags")) {
      map.flags |= ZM_ALLOW_MONSTER_TELEFRAGS;
    }
    else if (!stricmp(scanner.string, "KillerActivatesDeathSpecials")) {
      map.flags &= ~ZM_ACTIVATE_OWN_DEATH_SPECIALS;
    }
    else if (!stricmp(scanner.string, "ActivateOwnDeathSpecials")) {
      map.flags |= ZM_ACTIVATE_OWN_DEATH_SPECIALS;
    }
    else if (!stricmp(scanner.string, "SpecialAction")) {
      dsda_ParseZMapInfoMapSpecialAction(scanner, special_actions);
    }
    else if (!stricmp(scanner.string, "ClipMidTextures")) {
      map.flags |= ZM_CLIP_MID_TEXTURES;
    }
    else if (!stricmp(scanner.string, "WrapMidTextures")) {
      map.flags |= ZM_WRAP_MID_TEXTURES;
    }
    else if (!stricmp(scanner.string, "StrictMonsterActivation")) {
      map.flags &= ~ZM_LAX_MONSTER_ACTIVATION;
    }
    else if (!stricmp(scanner.string, "LaxMonsterActivation")) {
      map.flags |= ZM_LAX_MONSTER_ACTIVATION;
    }
    else if (!stricmp(scanner.string, "MissileShootersActivateImpactLines")) {
      map.flags &= ~ZM_MISSILES_ACTIVATE_IMPACT_LINES;
    }
    else if (!stricmp(scanner.string, "MissilesActivateImpactLines")) {
      map.flags |= ZM_MISSILES_ACTIVATE_IMPACT_LINES;
    }
    else if (!stricmp(scanner.string, "AvoidMelee")) {
      map.flags |= ZM_AVOID_MELEE;
    }
    else if (!stricmp(scanner.string, "FilterStarts")) {
      map.flags |= ZM_FILTER_STARTS;
    }
    else if (!stricmp(scanner.string, "AllowRespawn")) {
      map.flags |= ZM_ALLOW_RESPAWN;
    }
    else if (!stricmp(scanner.string, "NoJump")) {
      map.flags &= ~ZM_ALLOW_JUMP;
    }
    else if (!stricmp(scanner.string, "AllowJump")) {
      map.flags |= ZM_ALLOW_JUMP;
    }
    else if (!stricmp(scanner.string, "NoFreelook")) {
      map.flags &= ~ZM_ALLOW_FREE_LOOK;
    }
    else if (!stricmp(scanner.string, "AllowFreelook")) {
      map.flags |= ZM_ALLOW_FREE_LOOK;
    }
    else if (!stricmp(scanner.string, "NoCheckSwitchRange")) {
      map.flags &= ~ZM_CHECK_SWITCH_RANGE;
    }
    else if (!stricmp(scanner.string, "CheckSwitchRange")) {
      map.flags |= ZM_CHECK_SWITCH_RANGE;
    }
    else if (!stricmp(scanner.string, "ResetHealth")) {
      map.flags |= ZM_RESET_HEALTH;
    }
    else if (!stricmp(scanner.string, "ResetInventory")) {
      map.flags |= ZM_RESET_INVENTORY;
    }
    else if (!stricmp(scanner.string, "NoPassover")) {
      map.flags &= ~ZM_PASSOVER;
    }
    else if (!stricmp(scanner.string, "Passover")) {
      map.flags |= ZM_PASSOVER;
    }
    else if (!stricmp(scanner.string, "UsePlayerStartZ")) {
      map.flags |= ZM_USE_PLAYER_START_Z;
    }
    else if (!stricmp(scanner.string, "RandomPlayerStarts")) {
      map.flags |= ZM_RANDOM_PLAYER_STARTS;
    }
    else if (!stricmp(scanner.string, "ForgetState")) {
      map.flags &= ~ZM_REMEMBER_STATE;
    }
    else if (!stricmp(scanner.string, "RememberState")) {
      map.flags |= ZM_REMEMBER_STATE;
    }
    else if (!stricmp(scanner.string, "Author")) {
      SCAN_STRING(map.author);
    }
    else {
      dsda_SkipValue(scanner);
    }
  }

  map.num_special_actions = special_actions.size();
  map.special_actions = Z_Realloc(map.num_special_actions * sizeof(*map.special_actions));
  memcpy(map.special_actions, &special_actions[0],
         map.num_special_actions * sizeof(*map.special_actions));

  zmapinfo_maps.push_back(map);
}

static void dsda_ParseZMapInfoMap(Scanner &scanner) {
  zmapinfo_map_t map = default_map;
  std::vector<zmapinfo_special_action_t> special_actions;

  // Create a copy of the existing special actions from the default map
  for (int i = 0; i < map.num_special_actions; ++i)
    special_actions.push_back(map.special_actions[i]);

  scanner.MustGetToken(TK_Identifier);
  STR_DUP(map.lump_name);

  scanner.MustGetToken(TK_Identifier);

  // Lookup via a separate lump is not supported, so use the key instead
  if (!stricmp(scanner.string, "lookup"))
    scanner.MustGetToken(TK_Identifier);

  STR_DUP(map.nice_name);

  dsda_GuessLevelNum(map);

  dsda_ParseZMapInfoMapBlock(scanner, map, special_actions);
}

static void dsda_FreeMapNext(zmapinfo_map_next_t &next) {
  Z_Free(next.map);
  Z_Free(next.endpic);
}

static void dsda_FreeMapSky(zmapinfo_sky_t &sky) {
  Z_Free(sky.lump);
}

static void dsda_FreeMap(zmapinfo_map_t &map) {
  Z_Free(map.lump_name);
  Z_Free(map.nice_name);
  Z_Free(map.fade_table);
  Z_Free(map.title_patch);
  Z_Free(map.music);
  Z_Free(map.inter_music);
  Z_Free(map.exit_pic);
  Z_Free(map.enter_pic);
  Z_Free(map.border_texture);
  Z_Free(map.air_control);
  Z_Free(map.author);
  Z_Free(map.special_actions);
  dsda_FreeMapNext(map.next);
  dsda_FreeMapNext(map.secret_next);
  dsda_FreeMapSky(map.sky1);
  dsda_FreeMapSky(map.sky2);
}

static void dsda_InitDefaultMap(void) {
  dsda_FreeMap(default_map);

  memset(&default_map, 0, sizeof(default_map));

  default_map.flags = ZM_SKY_STRETCH |
                      ZM_INTERMISSION |
                      ZM_ACTIVATE_OWN_DEATH_SPECIALS |
                      ZM_LAX_MONSTER_ACTIVATION |
                      ZM_MISSILES_ACTIVATE_IMPACT_LINES |
                      ZM_REMEMBER_STATE |
                      ZM_SHOW_AUTHOR;
}

static void dsda_ParseZMapInfoDefaultMap(Scanner &scanner) {
  std::vector<zmapinfo_special_action_t> special_actions;

  dsda_InitDefaultMap();

  dsda_ParseZMapInfoMapBlock(scanner, default_map, special_actions);
}

static void dsda_ParseZMapInfoAddDefaultMap(Scanner &scanner) {
  std::vector<zmapinfo_special_action_t> special_actions;

  // Create a copy of the existing special actions from the default map
  for (int i = 0; i < default_map.num_special_actions; ++i)
    special_actions.push_back(default_map.special_actions[i]);

  Z_Free(default_map.special_actions);
  default_map.num_special_actions = 0;

  dsda_ParseZMapInfoMapBlock(scanner, default_map, special_actions);
}

static void dsda_ParseZMapInfoIdentifier(Scanner &scanner) {
  scanner.MustGetToken(TK_Identifier);

  if (!stricmp(scanner.string, "map")) {
    dsda_ParseZMapInfoMap(scanner);
  }
  else if (!stricmp(scanner.string, "defaultmap")) {
    dsda_ParseZMapInfoDefaultMap(scanner);
  }
  else if (!stricmp(scanner.string, "adddefaultmap")) {
    dsda_ParseZMapInfoAddDefaultMap(scanner);
  }
  else {
    dsda_SkipValue(scanner);
  }
}

zmapinfo_t zmapinfo;

void dsda_ParseZMapInfo(const unsigned char* buffer, size_t length, zmapinfo_errorfunc err) {
  Scanner scanner((const char*) buffer, length);

  scanner.SetErrorCallback(err);

  dsda_InitDefaultMap();

  while (scanner.TokensLeft())
    dsda_ParseZMapInfoIdentifier(scanner);

  zmapinfo.maps = &zmapinfo_maps[0];
  zmapinfo.num_maps = zmapinfo_maps.size();
}
