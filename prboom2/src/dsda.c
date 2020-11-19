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

#include <stdio.h>

#include "m_argv.h"
#include "doomstat.h"
#include "p_inter.h"
#include "p_tick.h"
#include "g_game.h"
#include "sounds.h"
#include "s_sound.h"

#include "dsda_mobj_extension.h"
#include "dsda.h"

#define TELEFRAG_DAMAGE 10000

// analysis variables
dboolean dsda_pacifist = true;
dboolean dsda_reality = true;
dboolean dsda_almost_reality = true;
int dsda_missed_monsters = 0;
int dsda_missed_secrets = 0;
dboolean dsda_tyson_weapons = true;
dboolean dsda_100k = true;
dboolean dsda_100s = true;
dboolean dsda_any_counted_monsters = false;
dboolean dsda_any_monsters = false;
dboolean dsda_any_secrets = false;
dboolean dsda_collector = false;
dboolean dsda_stroller = false;
dboolean dsda_nomo = false;
dboolean dsda_respawn = false;
dboolean dsda_fast = false;

// note-related
int dsda_kills_on_map = 0;
dboolean dsda_100k_on_map = false;
dboolean dsda_100k_note_shown = false;
dboolean dsda_pacifist_note_shown = false;

// command-line toggles
int dsda_analysis;
int dsda_track_pacifist;
int dsda_track_100k;

void dsda_DisplayNotification(const char* msg);
void dsda_ResetMapVariables(void);

void dsda_ReadCommandLine(void) {
  dsda_track_pacifist = M_CheckParm("-track_pacifist");
  dsda_track_100k = M_CheckParm("-track_100k");
  dsda_analysis = M_CheckParm("-analysis");
}

void dsda_DisplayNotifications(void) {
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
}

void dsda_WatchIconSpawn(mobj_t* spawned) {
  spawned->dsda_extension.spawned_by_icon = true;
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
  
  dsda_ResetMapVariables();
}

void dsda_ResetMapVariables(void) {
  dsda_100k_on_map = false;
  dsda_kills_on_map = 0;
  dsda_100k_note_shown = false;
}

void dsda_WatchWeaponFire(weapontype_t weapon) {
  if (weapon == wp_fist || weapon == wp_pistol || weapon == wp_chainsaw) return;
  
  dsda_tyson_weapons = false;
}

void dsda_WriteAnalysis(void) {
  FILE *fstream = NULL;
  
  if (!dsda_analysis) return;
  
  fstream = fopen("analysis.txt", "w");
  
  if (fstream == NULL) {
    fprintf(stderr, "Unable to open analysis.txt for writing!\n");
    return;
  }
  
  if (dsda_reality) dsda_almost_reality = false;
  
  dsda_nomo = nomonsters > 0;
  dsda_respawn = respawnparm > 0;
  dsda_fast = fastparm > 0;
  
  fprintf(fstream, "skill %d\n", gameskill + 1);
  fprintf(fstream, "nomonsters %d\n", dsda_nomo);
  fprintf(fstream, "respawn %d\n", dsda_respawn);
  fprintf(fstream, "fast %d\n", dsda_fast);
  fprintf(fstream, "pacifist %d\n", dsda_pacifist);
  fprintf(fstream, "reality %d\n", dsda_reality);
  fprintf(fstream, "almost_reality %d\n", dsda_almost_reality);
  fprintf(fstream, "100k %d\n", dsda_100k);
  fprintf(fstream, "100s %d\n", dsda_100s);
  fprintf(fstream, "missed_monsters %d\n", dsda_missed_monsters);
  fprintf(fstream, "missed_secrets %d\n", dsda_missed_secrets);
  fprintf(fstream, "tyson_weapons %d\n", dsda_tyson_weapons);
  
  fclose(fstream);
  
  return;
}

const char* dsda_DetectCategory(void) {
  dboolean satisfies_max;
  dboolean satisfies_respawn;
  dboolean satisfies_tyson;
  dboolean satisfies_100s;
  
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
    if (dsda_any_monsters && dsda_pacifist) {
      if (dsda_stroller) return "Stroller";
      
      return "Pacifist";
    }
    
    return "UV Speed";
  }
  else if (gameskill == sk_nightmare) {
    if (nomonsters) return "Other";
    if (satisfies_100s) return "NM 100S";
    
    return "NM Speed";
  }
  
  return "Other";
}
