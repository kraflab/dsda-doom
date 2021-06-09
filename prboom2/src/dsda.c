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
//	DSDA Tools
//

#include <stdlib.h>
#include <stdio.h>

#include "m_argv.h"
#include "doomstat.h"
#include "p_inter.h"
#include "p_tick.h"
#include "g_game.h"
#include "sounds.h"
#include "s_sound.h"
#include "am_map.h"

#include "dsda/mobj_extension.h"
#include "dsda/ghost.h"
#include "dsda/hud.h"
#include "dsda/command_display.h"
#include "dsda/key_frame.h"
#include "dsda/settings.h"
#include "dsda/split_tracker.h"
#include "dsda.h"

#define TELEFRAG_DAMAGE 10000
#define STROLLER_THRESHOLD 25
#define TURBO_THRESHOLD 50

// analysis variables
dboolean dsda_pacifist = true;
dboolean dsda_reality = true;
dboolean dsda_almost_reality = true;
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

// note-related
int dsda_kills_on_map = 0;
dboolean dsda_100k_on_map = false;
dboolean dsda_100k_note_shown = false;
dboolean dsda_pacifist_note_shown = false;
dboolean dsda_time_keys = false;
dboolean dsda_time_use = false;
dboolean dsda_time_secrets = false;
dboolean dsda_time_all = false;

// command-line toggles
int dsda_analysis;
int dsda_track_pacifist;
int dsda_track_100k;

int dsda_last_leveltime;
int dsda_last_gamemap;

// other
static char* dsda_demo_name_base;
int dsda_max_kill_requirement;
int dsda_session_attempts = 1;

dboolean dsda_IsWeapon(mobj_t* thing);
void dsda_DisplayNotification(const char* msg);
void dsda_ResetMapVariables(void);

void dsda_ReadCommandLine(void) {
  int p;

  dsda_track_pacifist = M_CheckParm("-track_pacifist");
  dsda_track_100k = M_CheckParm("-track_100k");
  dsda_analysis = M_CheckParm("-analysis");
  dsda_time_keys = M_CheckParm("-time_keys");
  dsda_time_use = M_CheckParm("-time_use");
  dsda_time_secrets = M_CheckParm("-time_secrets");
  dsda_time_all = M_CheckParm("-time_all");

  if (dsda_time_all) {
    dsda_time_keys = true;
    dsda_time_use = true;
    dsda_time_secrets = true;
  }

  if ((p = M_CheckParm("-export_ghost")) && ++p < myargc)
    dsda_InitGhostExport(myargv[p]);

  if ((p = M_CheckParm("-import_ghost"))) dsda_InitGhostImport(p);

  if (M_CheckParm("-tas")) dsda_SetTas();

  dsda_InitKeyFrame();
  dsda_InitCommandHistory();
}

static int dsda_shown_attempt = 0;

void dsda_DisplayNotifications(void) {
  if (dsda_ShowDemoAttempts() && dsda_session_attempts > dsda_shown_attempt) {
    doom_printf("Attempt %d / %d", dsda_session_attempts, dsda_DemoAttempts());

    dsda_shown_attempt = dsda_session_attempts;
  }

  if (!dsda_pacifist && dsda_track_pacifist && !dsda_pacifist_note_shown) {
    dsda_pacifist_note_shown = true;
    dsda_DisplayNotification("Not pacifist!");
  }

  if (dsda_100k_on_map && dsda_track_100k && !dsda_100k_note_shown) {
    dsda_100k_note_shown = true;
    dsda_DisplayNotification("100K achieved!");
  }
}

void dsda_DisplayNotification(const char* msg) {
  S_StartSound(0, sfx_radio);
  doom_printf(msg);
}

void dsda_WatchCard(card_t card) {
  if (dsda_time_keys)
    switch (card) {
      case it_bluecard:
      case it_blueskull:
        dsda_AddSplit(DSDA_SPLIT_BLUE_KEY);
        break;
      case it_yellowcard:
      case it_yellowskull:
        dsda_AddSplit(DSDA_SPLIT_YELLOW_KEY);
        break;
      case it_redcard:
      case it_redskull:
        dsda_AddSplit(DSDA_SPLIT_RED_KEY);
        break;
      default:
        break;
    }
}

void dsda_WatchDamage(mobj_t* target, mobj_t* inflictor, mobj_t* source, int damage) {
  if (
    ((source && source->player) || (inflictor && inflictor->dsda_extension.player_damaged_barrel)) \
    && damage != TELEFRAG_DAMAGE
  ) {
    if (target->type == MT_BARREL)
      target->dsda_extension.player_damaged_barrel = true;
    else if (!target->player)
      dsda_pacifist = false;
  }

  if (target->player) {
    dsda_reality = false;

    // "almost reality" means allowing nukage damage
    // we cannot differentiate between crushers and nukage in this scope
    // we account for crushers in dsda_WatchCrush instead
    if (inflictor) dsda_almost_reality = false;
  }
}

void dsda_WatchDeath(mobj_t* thing) {
  if (thing->flags & MF_COUNTKILL) {
    ++dsda_kills_on_map;

    if (dsda_kills_on_map >= totalkills) dsda_100k_on_map = true;
  }
}

void dsda_WatchKill(player_t* player, mobj_t* target) {
  player->killcount++;
  if (target->dsda_extension.spawned_by_icon) player->maxkilldiscount++;
}

void dsda_WatchResurrection(mobj_t* target) {
  int i;

  if (
    (
      (target->flags ^ MF_COUNTKILL) &
      (MF_FRIEND | MF_COUNTKILL)
    ) || target->dsda_extension.spawned_by_icon
  ) return;

  for (i = 0; i < MAXPLAYERS; ++i) {
    if (!playeringame[i] || players[i].killcount == 0) continue;

    if (players[i].killcount > 0) {
      players[i].maxkilldiscount++;
      return;
    }
  }
}

void dsda_WatchCrush(mobj_t* thing, int damage) {
  player_t *player;

  player = thing->player;
  if (!player) return;

  // invincible
  if (
    (damage < 1000 || (!comp[comp_god] && (player->cheats&CF_GODMODE))) \
    && (player->cheats&CF_GODMODE || player->powers[pw_invulnerability])
  ) return;

  dsda_almost_reality = false;
}

void dsda_WatchSpawn(mobj_t* spawned) {
  if (
    (spawned->flags & MF_COUNTKILL) \
    || spawned->type == MT_SKULL \
    || spawned->type == MT_BOSSBRAIN
  ) dsda_any_monsters = true;

  if (!dsda_any_weapons) dsda_any_weapons = dsda_IsWeapon(spawned);

  if (!((spawned->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)))
    ++dsda_max_kill_requirement;
}

void dsda_WatchIconSpawn(mobj_t* spawned) {
  spawned->dsda_extension.spawned_by_icon = true;

  // Fix count from dsda_WatchSpawn
  // We can't know inside P_SpawnMobj what the source is
  // This is less invasive than introducing a spawn source concept
  if (!((spawned->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)))
    --dsda_max_kill_requirement;
}

int dsda_MaxKillRequirement() {
  return dsda_max_kill_requirement;
}

void dsda_WatchCommand(void) {
  int i;
  ticcmd_t* cmd;

  for (i = 0; i < MAXPLAYERS; ++i) {
    if (!playeringame[i]) continue;

    cmd = &players[i].cmd;

    if (cmd->buttons & BT_USE && dsda_time_use)
      dsda_AddSplit(DSDA_SPLIT_USE);

    if (cmd->sidemove != 0 || abs(cmd->forwardmove) > STROLLER_THRESHOLD)
      dsda_stroller = false;

    if (abs(cmd->sidemove) > TURBO_THRESHOLD || abs(cmd->forwardmove) > TURBO_THRESHOLD)
      dsda_turbo = true;
  }

  dsda_AddCommandToCommandDisplay(&players[displayplayer].cmd);

  dsda_ExportGhostFrame();
}

void dsda_WatchBeforeLevelSetup(void) {
  dsda_100k_on_map = false;
  dsda_kills_on_map = 0;
  dsda_100k_note_shown = false;
  dsda_max_kill_requirement = 0;
}

void dsda_WatchAfterLevelSetup(void) {
  dsda_SpawnGhost();
}

void dsda_WatchLevelCompletion(void) {
  thinker_t *th;
  mobj_t *mobj;
  int i;
  int secret_count = 0;
  int kill_count = 0;

  for (th = thinkercap.next; th != &thinkercap; th = th->next) {
    if (th->function != P_MobjThinker) continue;

    mobj = (mobj_t *)th;

    // max rules: everything dead that affects kill counter except icon spawns
    if (
      !((mobj->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)) \
      && !mobj->dsda_extension.spawned_by_icon \
      && mobj->health > 0
    ) {
      ++dsda_missed_monsters;
    }

    if (dsda_IsWeapon(mobj)) {
      ++dsda_missed_weapons;
      dsda_weapon_collector = false;
    }
  }

  for (i = 0; i < MAXPLAYERS; ++i) {
    if (!playeringame[i]) continue;

    kill_count += players[i].killcount;
    secret_count += players[i].secretcount;
  }

  dsda_missed_secrets += (totalsecret - secret_count);

  if (kill_count < totalkills) dsda_100k = false;
  if (secret_count < totalsecret) dsda_100s = false;
  if (totalkills > 0) dsda_any_counted_monsters = true;
  if (totalsecret > 0) dsda_any_secrets = true;

  dsda_last_leveltime = leveltime;
  dsda_last_gamemap = gamemap;

  dsda_RecordSplit();
}

dboolean dsda_IsWeapon(mobj_t* thing) {
  switch (thing->sprite) {
    case SPR_BFUG:
    case SPR_MGUN:
    case SPR_CSAW:
    case SPR_LAUN:
    case SPR_PLAS:
    case SPR_SHOT:
    case SPR_SGN2:
      return true;
    default:
      return false;
  }
}

void dsda_WatchWeaponFire(weapontype_t weapon) {
  if (weapon == wp_fist || weapon == wp_pistol || weapon == wp_chainsaw) return;

  dsda_tyson_weapons = false;
}

void dsda_WatchSecret(void) {
  if (dsda_time_secrets) dsda_AddSplit(DSDA_SPLIT_SECRET);
}

char* dsda_DemoNameBase(void) {
  return dsda_demo_name_base;
}

// from crispy - incrementing demo file names
char* dsda_NewDemoName(void) {
  char* demo_name;
  size_t demo_name_size;
  FILE* fp = NULL;
  static unsigned int j = 2;

  demo_name_size = strlen(dsda_demo_name_base) + 11; // 11 = -12345.lmp\0
  demo_name = malloc(demo_name_size);
  snprintf(demo_name, demo_name_size, "%s.lmp", dsda_demo_name_base);

  for (; j <= 99999 && (fp = fopen(demo_name, "rb")) != NULL; j++) {
    snprintf(demo_name, demo_name_size, "%s-%05d.lmp", dsda_demo_name_base, j);
    fclose (fp);
  }

  return demo_name;
}

static void dsda_ResetTracking(void) {
  dsda_pacifist = true;
  dsda_reality = true;
  dsda_almost_reality = true;
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

  dsda_pacifist_note_shown = false;
}

void dsda_WatchDeferredInitNew(skill_t skill, int episode, int map) {
  char* demo_name;

  if (!demorecording) return;

  ++dsda_session_attempts;

  dsda_ResetTracking();

  AM_ResetIDDTcheat();
  G_CheckDemoStatus();

  demo_name = dsda_NewDemoName();

  G_RecordDemo(demo_name);

  basetic = gametic;

  free(demo_name);
}

void dsda_WatchNewGame(void) {
  if (!demorecording) return;

  G_BeginRecording();
}

void dsda_WatchLevelReload(int* reloaded) {
  if (!demorecording || *reloaded) return;

  G_DeferedInitNew(gameskill, gameepisode, startmap);
  *reloaded = 1;
}

void dsda_WatchRecordDemo(const char* name) {
  size_t base_size;
  if (dsda_demo_name_base != NULL) return;

  base_size = strlen(name) - 3;
  dsda_demo_name_base = malloc(base_size);
  strncpy(dsda_demo_name_base, name, base_size);
  dsda_demo_name_base[base_size - 1] = '\0';

  // demorecording is set after prboom+ has already cached its settings
  // we need to reset things here to satisfy strict mode
  dsda_InitSettings();
  dsda_InitKeyFrame();
}

void dsda_WriteAnalysis(void) {
  FILE *fstream = NULL;
  const char* category = NULL;

  if (!dsda_analysis) return;

  fstream = fopen("analysis.txt", "w");

  if (fstream == NULL) {
    fprintf(stderr, "Unable to open analysis.txt for writing!\n");
    return;
  }

  category = dsda_DetectCategory();

  fprintf(fstream, "skill %d\n", gameskill + 1);
  fprintf(fstream, "nomonsters %d\n", dsda_nomo);
  fprintf(fstream, "respawn %d\n", dsda_respawn);
  fprintf(fstream, "fast %d\n", dsda_fast);
  fprintf(fstream, "pacifist %d\n", dsda_pacifist);
  fprintf(fstream, "stroller %d\n", dsda_stroller);
  fprintf(fstream, "reality %d\n", dsda_reality);
  fprintf(fstream, "almost_reality %d\n", dsda_almost_reality);
  fprintf(fstream, "100k %d\n", dsda_100k);
  fprintf(fstream, "100s %d\n", dsda_100s);
  fprintf(fstream, "missed_monsters %d\n", dsda_missed_monsters);
  fprintf(fstream, "missed_secrets %d\n", dsda_missed_secrets);
  fprintf(fstream, "weapon_collector %d\n", dsda_weapon_collector);
  fprintf(fstream, "tyson_weapons %d\n", dsda_tyson_weapons);
  fprintf(fstream, "turbo %d\n", dsda_turbo);
  fprintf(fstream, "category %s\n", category);

  fclose(fstream);

  return;
}

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

  if (dsda_turbo) return "Other";
  if (coop_spawns) return "Other";

  if (gameskill == sk_hard) {
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
  else if (gameskill == sk_nightmare) {
    if (nomonsters) return "Other";
    if (satisfies_100s) return "NM 100S";

    return "NM Speed";
  }

  return "Other";
}
