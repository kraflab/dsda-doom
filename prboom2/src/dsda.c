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

#include "dsda/analysis.h"
#include "dsda/ghost.h"
#include "dsda/hud.h"
#include "dsda/command_display.h"
#include "dsda/key_frame.h"
#include "dsda/mouse.h"
#include "dsda/settings.h"
#include "dsda/split_tracker.h"
#include "dsda.h"

#define TELEFRAG_DAMAGE 10000

// command-line toggles
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
  S_StartSound(0, gamemode == commercial ? sfx_radio : sfx_itmbk);
  doom_printf("%s", msg);
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
    ((source && source->player) || (inflictor && inflictor->intflags & MIF_PLAYER_DAMAGED_BARREL)) \
    && damage != TELEFRAG_DAMAGE
  ) {
    if (target->type == MT_BARREL)
      target->intflags |= MIF_PLAYER_DAMAGED_BARREL;
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
  if (target->intflags & MIF_SPAWNED_BY_ICON) player->maxkilldiscount++;
}

void dsda_WatchResurrection(mobj_t* target) {
  int i;

  if (
    (
      (target->flags ^ MF_COUNTKILL) &
      (MF_FRIEND | MF_COUNTKILL)
    ) || target->intflags & MIF_SPAWNED_BY_ICON
  ) return;

  for (i = 0; i < g_maxplayers; ++i) {
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

void dsda_WatchMorph(mobj_t* morphed) {
  // Fix count from dsda_WatchSpawn
  if (!((morphed->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)))
    --dsda_max_kill_requirement;
}

void dsda_WatchUnMorph(mobj_t* morphed) {
  // Fix count from dsda_WatchSpawn
  if (!((morphed->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)))
    --dsda_max_kill_requirement;
}

void dsda_WatchIconSpawn(mobj_t* spawned) {
  spawned->intflags |= MIF_SPAWNED_BY_ICON;

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
  dsda_pclass_t *player_class;

  for (i = 0; i < g_maxplayers; ++i) {
    if (!playeringame[i]) continue;

    cmd = &players[i].cmd;
    player_class = &pclass[players[i].pclass];

    if (cmd->buttons & BT_USE && dsda_time_use)
      dsda_AddSplit(DSDA_SPLIT_USE);

    if (cmd->sidemove != 0 || abs(cmd->forwardmove) > player_class->stroller_threshold)
      dsda_stroller = false;

    if (
      abs(cmd->sidemove) > player_class->turbo_threshold ||
      abs(cmd->forwardmove) > player_class->turbo_threshold
    )
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

void dsda_WatchNewLevel(void) {
  dsda_ResetAutoKeyFrameTimeout();
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
      && !(mobj->intflags & MIF_SPAWNED_BY_ICON) \
      && mobj->health > 0
    ) {
      ++dsda_missed_monsters;
    }

    if (dsda_IsWeapon(mobj)) {
      ++dsda_missed_weapons;
      dsda_weapon_collector = false;
    }
  }

  for (i = 0; i < g_maxplayers; ++i) {
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
  dsda_ResetAnalysis();

  dsda_pacifist_note_shown = false;
}

void dsda_WatchDeferredInitNew(skill_t skill, int episode, int map) {
  char* demo_name;

  if (!demorecording) return;

  ++dsda_session_attempts;

  dsda_ResetTracking();
  dsda_QueueQuickstart();

  dsda_ResetRevealMap();
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

  if (dsda_demo_name_base != NULL) {
    dsda_InitSettings();
    return;
  }

  base_size = strlen(name) - 3;
  dsda_demo_name_base = malloc(base_size);
  strncpy(dsda_demo_name_base, name, base_size);
  dsda_demo_name_base[base_size - 1] = '\0';

  // demorecording is set after prboom+ has already cached its settings
  // we need to reset things here to satisfy strict mode
  dsda_InitSettings();
  dsda_InitKeyFrame();
}
