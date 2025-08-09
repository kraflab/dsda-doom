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
//	DSDA Skill Info
//

#include "doomstat.h"

#include "dsda/args.h"
#include "dsda/configuration.h"
#include "dsda/mapinfo/doom/parser.h"
#include "dsda/text_color.h"
#include "dsda/utility.h"

#include "skill_info.h"

skill_info_t skill_info;

const skill_info_t doom_skill_infos[5] = {
  {
    .ammo_factor = FRACUNIT * 2,
    .damage_factor = FRACUNIT / 2,
    .spawn_filter = 1,
    .key = 'i',
    .name = "I'm too young to die.",
    .pic_name = "M_JKILL",
    .flags = SI_EASY_BOSS_BRAIN
  },
  {
    .spawn_filter = 2,
    .key = 'h',
    .name = "Hey, not too rough.",
    .pic_name = "M_ROUGH",
    .flags = SI_EASY_BOSS_BRAIN
  },
  {
    .spawn_filter = 3,
    .key = 'h',
    .name = "Hurt me plenty.",
    .pic_name = "M_HURT",
    .flags = 0
  },
  {
    .spawn_filter = 4,
    .key = 'u',
    .name = "Ultra-Violence.",
    .pic_name = "M_ULTRA",
    .flags = 0
  },
  {
    .ammo_factor = FRACUNIT * 2,
    .spawn_filter = 5,
    .key = 'n',
    .name = "Nightmare!",
    .pic_name = "M_NMARE",
    .respawn_time = 12,
    .flags = SI_FAST_MONSTERS | SI_INSTANT_REACTION | SI_MUST_CONFIRM
  },
};

const skill_info_t heretic_skill_infos[5] = {
  {
    .ammo_factor = FRACUNIT * 3 / 2,
    .damage_factor = FRACUNIT / 2,
    .spawn_filter = 1,
    .name = "THOU NEEDETH A WET-NURSE",
    .flags = SI_AUTO_USE_HEALTH
  },
  {
    .spawn_filter = 2,
    .name = "YELLOWBELLIES-R-US",
    .flags = 0
  },
  {
    .spawn_filter = 3,
    .name = "BRINGEST THEM ONETH",
    .flags = 0
  },
  {
    .spawn_filter = 4,
    .name = "THOU ART A SMITE-MEISTER",
    .flags = 0
  },
  {
    .ammo_factor = FRACUNIT * 3 / 2,
    .spawn_filter = 5,
    .name = "BLACK PLAGUE POSSESSES THEE",
    .flags = SI_FAST_MONSTERS | SI_INSTANT_REACTION
  },
};

const skill_info_t hexen_skill_infos[5] = {
  {
    .ammo_factor = FRACUNIT * 3 / 2,
    .damage_factor = FRACUNIT / 2,
    .spawn_filter = 1,
    .flags = SI_AUTO_USE_HEALTH
  },
  {
    .spawn_filter = 2,
    .flags = 0
  },
  {
    .spawn_filter = 3,
    .flags = 0
  },
  {
    .spawn_filter = 4,
    .flags = 0
  },
  {
    .ammo_factor = FRACUNIT * 3 / 2,
    .spawn_filter = 5,
    .flags = SI_FAST_MONSTERS | SI_INSTANT_REACTION
  },
};

int num_skills;
int num_og_skills;
skill_info_t* skill_infos;

static void dsda_CopyFactor(fixed_t* dest, const char* source) {
  // We will compute integers with these,
  // and common human values (i.e., not powers of 2) are rounded down.
  // Add 1 (= 1/2^16) to yield expected results.
  // Otherwise, 0.2 * 15 would be 2 instead of 3.

  if (source)
    *dest = dsda_StringToFixed(source) + 1;
}

static void dsda_CopySkillInfo(int i, const doom_mapinfo_skill_t* info) {
  memset(&skill_infos[i], 0, sizeof(skill_infos[i]));

  dsda_CopyFactor(&skill_infos[i].ammo_factor, info->ammo_factor);
  dsda_CopyFactor(&skill_infos[i].damage_factor, info->damage_factor);
  dsda_CopyFactor(&skill_infos[i].armor_factor, info->armor_factor);
  dsda_CopyFactor(&skill_infos[i].health_factor, info->health_factor);
  dsda_CopyFactor(&skill_infos[i].monster_health_factor, info->monster_health_factor);
  dsda_CopyFactor(&skill_infos[i].friend_health_factor, info->friend_health_factor);

  skill_infos[i].respawn_time = info->respawn_time;
  skill_infos[i].spawn_filter = info->spawn_filter;
  skill_infos[i].key = info->key;

  if (info->must_confirm)
    skill_infos[i].must_confirm = Z_Strdup(info->must_confirm);

  if (info->name)
    skill_infos[i].name = Z_Strdup(info->name);

  if (info->pic_name)
    skill_infos[i].pic_name = Z_Strdup(info->pic_name);

  if (info->text_color)
    skill_infos[i].text_color = dsda_ColorNameToIndex(info->text_color);

  skill_infos[i].flags = info->flags;
}

void dsda_InitSkills(void) {
  int i = 0;
  int j;
  int original_skills;
  dboolean clear_skills;

  // Check for new skills
  dsda_CheckCustomSkill();

  clear_skills = (doom_mapinfo.num_skills && doom_mapinfo.skills_cleared);

  num_skills = (clear_skills ? 0 : 5) + (int)doom_mapinfo.num_skills + customskill;
  num_og_skills = num_skills - customskill;

  original_skills = 5;

  skill_infos = Z_Calloc(num_skills, sizeof(*skill_infos));

  if (!clear_skills) {
    const skill_info_t* original_skill_infos;

    original_skill_infos = hexen   ? hexen_skill_infos   :
                           heretic ? heretic_skill_infos :
                                     doom_skill_infos;

    for (i = 0; i < original_skills; ++i)
      skill_infos[i] = original_skill_infos[i];
  }

  for (j = 0; j < doom_mapinfo.num_skills; ++j) {
    if (!stricmp(doom_mapinfo.skills[j].unique_id, "baby")) {
      dsda_CopySkillInfo(0, &doom_mapinfo.skills[j]);
      --i;
      --num_skills;
    }
    else if (!stricmp(doom_mapinfo.skills[j].unique_id, "easy")) {
      dsda_CopySkillInfo(1, &doom_mapinfo.skills[j]);
      --i;
      --num_skills;
    }
    else if (!stricmp(doom_mapinfo.skills[j].unique_id, "normal")) {
      dsda_CopySkillInfo(2, &doom_mapinfo.skills[j]);
      --i;
      --num_skills;
    }
    else if (!stricmp(doom_mapinfo.skills[j].unique_id, "hard")) {
      dsda_CopySkillInfo(3, &doom_mapinfo.skills[j]);
      --i;
      --num_skills;
    }
    else if (!stricmp(doom_mapinfo.skills[j].unique_id, "nightmare")) {
      dsda_CopySkillInfo(4, &doom_mapinfo.skills[j]);
      --i;
      --num_skills;
    }
    else
      dsda_CopySkillInfo(i + j, &doom_mapinfo.skills[j]);
  }
}

/////////////////////////////////////////
//
// Add Custom Skill Properties
//
//

// Custom Skill variables
int cskill_spawn_filter;
int cskill_ammo_factor;
int cskill_damage_factor;
int cskill_armor_factor;
int cskill_health_factor;
int cskill_monster_hp_factor;
int cskill_friend_hp_factor;
int cskill_respawn;
int cskill_respawn_time;
int cskill_coop_spawns;
int cskill_no_monsters;
int cskill_fast_monsters;
int cskill_aggressive;
int cskill_no_pain;
int cskill_easy_brain;
int cskill_auto_use_hp;

static int dsda_GetCustomSpawnFilter(int config) {
  switch (config)
  {
    case 0: return 1; // if "easy",   skill 1
    case 1: return 3; // if "medium", skill 3
    case 2: return 4; // if "hard",   skill 4
    default: return false;
  }
}

static int dsda_GetCustomFactor(int config) {
  switch (config)
  {
    case 0: return FRACUNIT / 2;      // Half
    case 1: return FRACUNIT;          // Default
    case 2: return FRACUNIT * 3 / 2;  // 1.5x - Raven games use this
    case 3: return FRACUNIT * 2;      // Double
    case 4: return FRACUNIT * 4;      // Quad
    default: return false;
  }
}

void dsda_UpdateCustomSkill(int custom_skill_num) {
  // Add label for skill cheat
  skill_infos[custom_skill_num].name = "Custom Skill";

  // Reset custom skill properties
  skill_infos[custom_skill_num].flags = 0;
  skill_infos[custom_skill_num].respawn_time = 0;

  // Get spawn filter value
  skill_infos[custom_skill_num].spawn_filter = dsda_GetCustomSpawnFilter(cskill_spawn_filter);;

  // Get multiplier factors
  skill_infos[custom_skill_num].ammo_factor             = dsda_GetCustomFactor(cskill_ammo_factor);
  skill_infos[custom_skill_num].damage_factor           = dsda_GetCustomFactor(cskill_damage_factor);
  skill_infos[custom_skill_num].armor_factor            = dsda_GetCustomFactor(cskill_armor_factor);
  skill_infos[custom_skill_num].health_factor           = dsda_GetCustomFactor(cskill_health_factor);
  skill_infos[custom_skill_num].monster_health_factor   = dsda_GetCustomFactor(cskill_monster_hp_factor);
  skill_infos[custom_skill_num].friend_health_factor    = dsda_GetCustomFactor(cskill_friend_hp_factor);

  // Get respawn time (if respawn is enabled)
  if (cskill_respawn) skill_infos[custom_skill_num].respawn_time = cskill_respawn_time;

  // Add remaining flags
  if (cskill_coop_spawns)            skill_infos[custom_skill_num].flags |= SI_SPAWN_MULTI;
  if (cskill_no_monsters)            skill_infos[custom_skill_num].flags |= SI_NO_MONSTERS;
  if (cskill_fast_monsters)          skill_infos[custom_skill_num].flags |= SI_FAST_MONSTERS;
  if (cskill_aggressive)             skill_infos[custom_skill_num].flags |= SI_INSTANT_REACTION;
  if (cskill_no_pain)                skill_infos[custom_skill_num].flags |= SI_NO_PAIN;
  if (cskill_easy_brain && !raven)   skill_infos[custom_skill_num].flags |= SI_EASY_BOSS_BRAIN;
  if (cskill_auto_use_hp && raven)   skill_infos[custom_skill_num].flags |= SI_AUTO_USE_HEALTH;

  // Update the skill
  dsda_UpdateGameSkill(custom_skill_num);
}

// At startup, set-up temp game modifier configs based off args / persistent cfgs
// Only set once, as modifiers can break away from args
//
// "Always Pistol Start" is the only persistent cfg (saved in cfg file)
//

void dsda_InitGameModifiers(void)
{
  if (dsda_Flag(dsda_arg_pistol_start) || dsda_IntConfig(dsda_config_always_pistol_start))
      dsda_UpdateIntConfig(dsda_config_pistol_start, true, true);
  if (dsda_Flag(dsda_arg_respawn))
      dsda_UpdateIntConfig(dsda_config_respawn_monsters, true, true);
  if (dsda_Flag(dsda_arg_fast))
      dsda_UpdateIntConfig(dsda_config_fast_monsters, true, true);
  if (dsda_Flag(dsda_arg_nomonsters))
      dsda_UpdateIntConfig(dsda_config_no_monsters, true, true);
  if (dsda_Flag(dsda_arg_coop_spawns))
      dsda_UpdateIntConfig(dsda_config_coop_spawns, true, true);
}

// if "Pistol Start" is disabled, disable "Always Pistol Start" (avoid impossible condition)
void dsda_RefreshPistolStart(void)
{
  dboolean pistol_start_conflict = dsda_IntConfig(dsda_config_always_pistol_start) && !dsda_IntConfig(dsda_config_pistol_start);

  if (allow_incompatibility || in_game)
    if (pistol_start_conflict)
    {
      dsda_UpdateIntConfig(dsda_config_always_pistol_start, false, true);
      dsda_ResetGameModifiers();
    }
}

// if "Always Pistol Start" is enabled, enable "Pistol Start" (avoid impossible condition)
void dsda_RefreshAlwaysPistolStart(void)
{
  dboolean pistol_start_conflict = dsda_IntConfig(dsda_config_always_pistol_start) && !dsda_IntConfig(dsda_config_pistol_start);

  if (allow_incompatibility || in_game)
    if (pistol_start_conflict)
    {
      dsda_UpdateIntConfig(dsda_config_pistol_start, true, true);
      dsda_ResetGameModifiers();
    }
}

// During demo recording/playback only use args, else use cfgs
void dsda_ResetGameModifiers(void)
{
  pistolstart = (allow_incompatibility ? dsda_IntConfig(dsda_config_pistol_start)     : false);   // pistolstart not allowed in demos
  respawnparm = (allow_incompatibility ? dsda_IntConfig(dsda_config_respawn_monsters) : dsda_Flag(dsda_arg_respawn));
  fastparm    = (allow_incompatibility ? dsda_IntConfig(dsda_config_fast_monsters)    : dsda_Flag(dsda_arg_fast));
  nomonsters  = (allow_incompatibility ? dsda_IntConfig(dsda_config_no_monsters)      : dsda_Flag(dsda_arg_nomonsters));
  coop_spawns = (allow_incompatibility ? dsda_IntConfig(dsda_config_coop_spawns)      : dsda_Flag(dsda_arg_coop_spawns));
}

void dsda_RefreshGameSkill(void) {
  void G_RefreshFastMonsters(void);

  if (allow_incompatibility)
    dsda_ResetGameModifiers();

  skill_info = skill_infos[gameskill];

  if (respawnparm && !skill_info.respawn_time)
    skill_info.respawn_time = 12;

  if (nomonsters)
    skill_info.flags |= SI_NO_MONSTERS;

  if (fastparm)
    skill_info.flags |= SI_FAST_MONSTERS;

  if (coop_spawns)
    skill_info.flags |= SI_SPAWN_MULTI;

  G_RefreshFastMonsters();
}

void dsda_UpdateGameSkill(int skill) {
  if (skill > num_skills - 1)
    skill = num_skills - 1;

  gameskill = skill;
  dsda_RefreshGameSkill();
}

void dsda_AlterGameFlags(void)
{
  if (!allow_incompatibility || !in_game)
    return;

  dsda_RefreshGameSkill();
}

void dsda_CheckCustomSkill(void) {
  //if (started_demo)
    //return;

  if (!allow_incompatibility || netgame)
    return;

  customskill = true;
}
