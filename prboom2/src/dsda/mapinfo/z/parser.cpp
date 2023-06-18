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
                       x = scanner.number; }

#define SCAN_FLOAT(x) { scanner.MustGetToken('='); \
                        scanner.MustGetFloat(); \
                        x = scanner.decimal; }

#define SCAN_FLAG(x, f) { scanner.MustGetToken('='); \
                          scanner.MustGetToken(TK_BoolConst); \
                          if (scanner.boolean) \
                            x |= f; }

#define SCAN_STRING_N(x, n) { scanner.MustGetToken('='); \
                              scanner.MustGetToken(TK_StringConst); \
                              strncpy(x, scanner.string, n); }

#define SCAN_STRING(x) { scanner.MustGetToken('='); \
                         scanner.MustGetToken(TK_StringConst); \
                         x = Z_Strdup(scanner.string); }

#define SCAN_FLOAT_STRING(x) { scanner.MustGetToken('='); \
                               scanner.MustGetFloat(); \
                               x = dsda_FloatString(scanner); }

static const char* end_names[zmn_end_count] = {
  [zmn_endgame1] = "EndGame1",
  [zmn_endgame2] = "EndGame2",
  [zmn_endgamew] = "EndGameW",
  [zmn_endgame4] = "EndGame4",
  [zmn_endgamec] = "EndGameC",
  [zmn_endgame3] = "EndGame3",
  [zmn_enddemon] = "EndDemon",
  [zmn_endgames] = "EndGameS",
  [zmn_endchess] = "EndChess",
  [zmn_endtitle] = "EndTitle",
};

static void dsda_ParseZMapInfoMapNext(Scanner &scanner, zmapinfo_map_next_t &next) {
  scanner.MustGetToken('=');

  if (scanner.CheckToken(TK_StringConst)) {
    for (int i = zmn_endgame1; i < zmn_end_count; ++i)
      if (!stricmp(scanner.string, end_names[i])) {
        next.end = i;
        return;
      }

    next.map = Z_Strdup(scanner.string);
  }
  else if (scanner.CheckToken(TK_Identifier)) {
    if (!stricmp(scanner.string, "EndPic")) {
      scanner.MustGetToken(',');
      scanner.MustGetToken(TK_StringConst);
      next.endpic = Z_Strdup(scanner.string);
    }
    else if (!stricmp(scanner.string, "EndSequence")) {
      scanner.MustGetToken(',');
      scanner.MustGetToken(TK_StringConst);
      next.intermission = Z_Strdup(scanner.string);
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

static const char* item_names[zmr_item_count] = { };

static void dsda_ParseZMapInfoMapRedirect(Scanner &scanner, zmapinfo_map_redirect_t &redirect) {
  scanner.MustGetToken('=');
  scanner.MustGetToken(TK_StringConst);

  for (int i = 1; i < zmr_item_count; ++i)
    if (!stricmp(scanner.string, item_names[i]))
      redirect.item = i;

  scanner.MustGetToken(',');
  scanner.MustGetToken(TK_StringConst);

  redirect.map = Z_Strdup(scanner.string);
}

static void dsda_ParseZMapInfoMapSky(Scanner &scanner, zmapinfo_sky_t &sky) {
  scanner.MustGetToken('=');
  scanner.MustGetToken(TK_StringConst);

  sky.lump = Z_Strdup(scanner.string);
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

  map.title_patch = Z_Strdup(scanner.string);
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

  map.music = Z_Strdup(scanner.stirng);
}

static void dsda_ParseZMapInfoIntermissionPic(Scanner &scanner, const char* &pic) {
  scanner.MustGetToken('=');
  scanner.MustGetToken(TK_StringConst);

  if (scanner.string[0] == '$')
    return;

  pic = Z_Strdup(scanner.stirng);
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

static void dsda_InitDefaultMap(void) {
  default_map.flags = ZM_SKY_STRETCH |
                      ZM_INTERMISSION |
                      ZM_LAX_MONSTER_ACTIVATION |
                      ZM_ALLOW_JUMP |
                      ZM_REMEMBER_STATE |
                      ZM_SHOW_AUTHOR;
}

static void dsda_ParseZMapInfoMap(Scanner &scanner) {
  zmapinfo_map_t map = default_map;
  std::vector<zmapinfo_special_action_t> special_actions;

  // Create a copy of the existing special actions from the default map
  for (int i = 0; i < map.num_special_actions; ++i)
    special_actions.push_back(map.special_actions[i]);

  scanner.MustGetToken(TK_Identifier);
  map.lump_name = Z_Strdup(scanner.string);

  scanner.MustGetToken(TK_Identifier);

  // Lookup via a separate lump is not supported, so use the key instead
  if (!stricmp(scanner.string, "lookup"))
    scanner.MustGetToken(TK_Identifier);

  map.nice_name = Z_Strdup(scanner.string);

  dsda_GuessLevelNum(map);

  scanner.MustGetToken('{');
  while (!scanner.CheckToken('}')) {
    scanner.MustGetToken(TK_Identifier);

    if (!stricmp(scanner.string, "levelnum")) {
      SCAN_INT(map.levelnum);
    }
    else if (!stricmp(scanner.string, "next")) {
      dsda_ParseZMapInfoMapNext(scanner, map.next);
    }
    else if (!stricmp(scanner.string, "secret") ||
             !stricmp(scanner.string, "secretnext")) {
      dsda_ParseZMapInfoMapNext(scanner, map.secretnext);
    }
    else if (!stricmp(scanner.string, "redirect")) {
      dsda_ParseZMapInfoMapRedirect(scanner, map.redirect);
    }
    else if (!stricmp(scanner.string, "cluster")) {
      SCAN_INT(map.cluster);
    }
    else if (!stricmp(scanner.string, "sky1") ||
             !stricmp(scanner.string, "skybox")) {
      dsda_ParseZMapInfoMapSky(scanner, map.sky1);
    }
    else if (!stricmp(scanner.string, "sky2")) {
      dsda_ParseZMapInfoMapSky(scanner, map.sky2);
    }
    else if (!stricmp(scanner.string, "doublesky")) {
      map.flags |= ZM_DOUBLE_SKY;
    }
    else if (!stricmp(scanner.string, "forcenoskystretch")) {
      map.flags &= ~ZM_SKY_STRETCH;
    }
    else if (!stricmp(scanner.string, "skystretch")) {
      map.flags |= ZM_SKY_STRETCH;
    }
    else if (!stricmp(scanner.string, "fadetable")) {
      SCAN_STRING(map.fade_table);
    }
    else if (!stricmp(scanner.string, "titlepatch")) {
      dsda_ParseZMapInfoTitlePatch(scanner, map);
    }
    else if (!stricmp(scanner.string, "par")) {
      SCAN_INT(map.par);
    }
    else if (!stricmp(scanner.string, "sucktime")) {
      SCAN_INT(map.suck_time);
    }
    else if (!stricmp(scanner.string, "nointermission")) {
      map.flags &= ~ZM_INTERMISSION;
    }
    else if (!stricmp(scanner.string, "intermission")) {
      map.flags |= ZM_INTERMISSION;
    }
    else if (!stricmp(scanner.string, "music")) {
      dsda_ParseZMapInfoMusic(scanner, map);
    }
    else if (!stricmp(scanner.string, "exitpic")) {
      dsda_ParseZMapInfoIntermissionPic(scanner, map.exit_pic);
    }
    else if (!stricmp(scanner.string, "enterpic")) {
      dsda_ParseZMapInfoIntermissionPic(scanner, map.enter_pic);
    }
    else if (!stricmp(scanner.string, "intermusic")) {
      SCAN_STRING(map.inter_music);
    }
    else if (!stricmp(scanner.string, "bordertexture")) {
      SCAN_STRING(map.border_texture);
    }
    else if (!stricmp(scanner.string, "lightning")) {
      map.flags |= ZM_LIGHTNING;
    }
    else if (!stricmp(scanner.string, "evenlighting")) {
      map.lighting = zm_lighting_even;
    }
    else if (!stricmp(scanner.string, "smoothlighting")) {
      map.lighting = zm_lighting_smooth;
    }
    else if (!stricmp(scanner.string, "gravity")) {
      SCAN_INT(map.gravity);
    }
    else if (!stricmp(scanner.string, "aircontrol")) {
      SCAN_FLOAT_STRING(map.air_control);
    }
    else if (!stricmp(scanner.string, "allowmonstertelefrags")) {
      map.flags |= ZM_ALLOW_MONSTER_TELEFRAGS;
    }
    else if (!stricmp(scanner.string, "activateowndeathspecials")) {
      map.flags |= ZM_ACTIVATE_OWN_DEATH_SPECIALS;
    }
    else if (!stricmp(scanner.string, "specialaction")) {
      dsda_ParseZMapInfoMapSpecialAction(scanner, special_actions);
    }
    else {
      // known ignored fields:
      // SlideShow
      // DeathSequence
      // Fade
      // OutsideFog
      // CDTrack
      // CDId
      // VertWallShade
      // HorizWallShade
      // TeamDamage
      // AirSupply
      // F1
      // MapBackground
      // Translator
      // IronlichSpecial
      // MinotaurSpecial
      // DSparilSpecial
      // NoAutoSequences
      // AutoSequences
      // FallingDamage
      // OldFallingDamage
      // ForceFallingDamage
      // StrifeFallingDamage
      // NoFallingDamage
      // MonsterFallingDamage
      // ProperMonsterFallingDamage
      // TeamPlayOn
      // TeamPlayOff
      // NoInventoryBar
      // KeepFullInventory
      // InfiniteFlightPowerup
      // NoCrouch
      // AllowCrouch
      // UnFreezeSinglePlayerConversations
      // ResetItems
      // NoAutosaveHint
      // ForceFakeContrast
      // ForceWorldPanning
      // NeedClusterText
      // NoClusterText
      // EnableSkyboxAO
      // DisableSkyboxAO
      // EnableShadowmap
      // DisableShadowmap
      // AttenuateLights
      dsda_SkipValue(scanner);
    }
  }

  map.num_special_actions = special_actions.size();
  map.special_actions = Z_Malloc(map.num_special_actions * sizeof(*map.special_actions));
  memcpy(map.special_actions, &special_actions[0],
         map.num_special_actions * sizeof(*map.special_actions));

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

  dsda_InitDefaultMap();

  while (scanner.TokensLeft())
    dsda_ParseZMapInfoIdentifier(scanner);

  zmapinfo.maps = &zmapinfo_maps[0];
  zmapinfo.num_maps = zmapinfo_maps.size();
}
