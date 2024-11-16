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
//	DSDA Analysis
//

#include "doomstat.h"
#include "m_file.h"

#include "dsda/excmd.h"
#include "dsda/exdemo.h"
#include "dsda/settings.h"

#include "analysis.h"

int dsda_analysis;

dboolean dsda_pacifist = true;
dboolean dsda_reality = true;
dboolean dsda_almost_reality = true;
dboolean dsda_reborn = false;
int dsda_missed_monsters = 0;
int dsda_missed_secrets = 0;
int dsda_missed_weapons = 0;
dboolean dsda_tyson_weapons = true;
dboolean dsda_100k = true;
dboolean dsda_100s = true;
dboolean dsda_any_counted_monsters = false;
dboolean dsda_any_monsters = false;
dboolean dsda_any_secrets = false;
dboolean dsda_any_weapons = false;
dboolean dsda_stroller = true;
dboolean dsda_nomo = false;
dboolean dsda_respawn = false;
dboolean dsda_fast = false;
dboolean dsda_turbo = false;
dboolean dsda_weapon_collector = true;

int dsda_kills_on_map = 0;
dboolean dsda_100k_on_map = false;
dboolean dsda_100k_note_shown = false;
dboolean dsda_pacifist_note_shown = false;
dboolean dsda_reality_note_shown = false;
dboolean dsda_almost_reality_note_shown = false;

void dsda_ResetAnalysis(void) {
  dsda_pacifist = true;
  dsda_reality = true;
  dsda_almost_reality = true;
  dsda_reborn = false;
  dsda_missed_monsters = 0;
  dsda_missed_secrets = 0;
  dsda_missed_weapons = 0;
  dsda_tyson_weapons = true;
  dsda_100k = true;
  dsda_100s = true;
  dsda_any_counted_monsters = false;
  dsda_any_monsters = false;
  dsda_any_secrets = false;
  dsda_any_weapons = false;
  dsda_stroller = true;
  dsda_turbo = false;
  dsda_weapon_collector = true;
}

void dsda_WriteAnalysis(void) {
  FILE *fstream = NULL;
  const char* category = NULL;
  int is_signed;

  if (!dsda_analysis) return;

  fstream = M_OpenFile("analysis.txt", "w");

  if (fstream == NULL) {
    fprintf(stderr, "Unable to open analysis.txt for writing!\n");
    return;
  }

  category = dsda_DetectCategory();
  is_signed = dsda_IsExDemoSigned();

  fprintf(fstream, "skill %d\n", gameskill + 1);
  fprintf(fstream, "nomonsters %d\n", dsda_nomo);
  fprintf(fstream, "respawn %d\n", dsda_respawn);
  fprintf(fstream, "fast %d\n", dsda_fast);
  fprintf(fstream, "pacifist %d\n", dsda_pacifist);
  fprintf(fstream, "stroller %d\n", dsda_stroller);
  fprintf(fstream, "reality %d\n", dsda_reality);
  fprintf(fstream, "almost_reality %d\n", dsda_almost_reality);
  fprintf(fstream, "reborn %d\n", dsda_reborn);
  fprintf(fstream, "100k %d\n", dsda_100k);
  fprintf(fstream, "100s %d\n", dsda_100s);
  fprintf(fstream, "missed_monsters %d\n", dsda_missed_monsters);
  fprintf(fstream, "missed_secrets %d\n", dsda_missed_secrets);
  fprintf(fstream, "weapon_collector %d\n", dsda_weapon_collector);
  fprintf(fstream, "tyson_weapons %d\n", dsda_tyson_weapons);
  fprintf(fstream, "turbo %d\n", dsda_turbo);
  fprintf(fstream, "solo_net %d\n", solo_net);
  fprintf(fstream, "coop_spawns %d\n", coop_spawns);
  fprintf(fstream, "category %s\n", category);
  fprintf(fstream, "signature %d\n", is_signed);

  fclose(fstream);

  return;
}

#define SKILL4 3
#define SKILL5 4

const char* dsda_DetectCategory(void) {
  dboolean satisfies_max;
  dboolean satisfies_respawn;
  dboolean satisfies_tyson;
  dboolean satisfies_100s;

  if (dsda_reality) dsda_almost_reality = false;
  if (!dsda_pacifist) dsda_stroller = false;
  if (!dsda_any_weapons) dsda_weapon_collector = false;

  dsda_nomo = nomonsters > 0;
  dsda_respawn = respawnparm > 0;
  dsda_fast = fastparm > 0;

  satisfies_max = (
    dsda_missed_monsters == 0 \
    && dsda_100s \
    && (dsda_any_secrets || dsda_any_counted_monsters)
  );
  satisfies_respawn = (
    dsda_100s \
    && dsda_100k \
    && dsda_any_monsters \
    && (dsda_any_secrets || dsda_any_counted_monsters)
  );
  satisfies_tyson = (
    dsda_missed_monsters == 0 \
    && dsda_tyson_weapons  \
    && dsda_any_counted_monsters
  );
  satisfies_100s = dsda_any_secrets && dsda_100s;

  if (dsda_ExCmdDemo()) return "Other";
  if (dsda_turbo) return "Other";
  if (coop_spawns) return "Other";
  if (solo_net) return "Other";
  if (dsda_reborn) return "Other";

  if (gameskill == SKILL4) {
    if (dsda_nomo && !dsda_respawn && !dsda_fast) {
      if (satisfies_100s) return "NoMo 100S";

      return "NoMo";
    }

    if (dsda_respawn && !dsda_nomo && !dsda_fast) {
      if (satisfies_respawn) return "UV Respawn";

      return "Other";
    }

    if (dsda_fast && !dsda_nomo && !dsda_respawn) {
      if (satisfies_max) return "UV Fast";

      return "Other";
    }

    if (dsda_nomo || dsda_respawn || dsda_fast) return "Other";

    if (satisfies_max) return "UV Max";
    if (satisfies_tyson) return "UV Tyson";
    if (dsda_any_monsters && dsda_stroller) return "Stroller";
    if (dsda_any_monsters && dsda_pacifist) return "Pacifist";

    return "UV Speed";
  }
  else if (gameskill == SKILL5) {
    if (nomonsters) return "Other";
    if (satisfies_100s) return "NM 100S";

    return "NM Speed";
  }

  return "Other";
}
