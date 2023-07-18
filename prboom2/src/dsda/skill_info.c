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

#include "skill_info.h"

skill_info_t skill_info;

static const skill_info_t doom_sk1_info = {
  .ammo_factor = FRACUNIT * 2,
  .damage_factor = FRACUNIT / 2,
  .spawn_filter = 1,
  .key = 'i',
  .name = "I'm too young to die.",
  .pic_name = "M_JKILL",
  .flags = SI_EASY_BOSS_BRAIN
};

static const skill_info_t doom_sk2_info = {
  .spawn_filter = 2,
  .key = 'h',
  .name = "Hey, not too rough.",
  .pic_name = "M_ROUGH",
  .flags = SI_EASY_BOSS_BRAIN
};

static const skill_info_t doom_sk3_info = {
  .spawn_filter = 3,
  .key = 'h',
  .name = "Hurt me plenty.",
  .pic_name = "M_HURT",
  .flags = SI_DEFAULT_SKILL
};

static const skill_info_t doom_sk4_info = {
  .spawn_filter = 4,
  .key = 'u',
  .name = "Ultra-Violence.",
  .pic_name = "M_ULTRA",
  .flags = 0
};

static const skill_info_t doom_sk5_info = {
  .ammo_factor = FRACUNIT * 2,
  .spawn_filter = 5,
  .key = 'n',
  .name = "Nightmare!",
  .pic_name = "M_NMARE",
  .respawn_time = 12,
  .flags = SI_FAST_MONSTERS | SI_INSTANT_REACTION | SI_MUST_CONFIRM
};

static const skill_info_t heretic_sk1_info = {
  .ammo_factor = FRACUNIT * 3 / 2,
  .damage_factor = FRACUNIT / 2,
  .spawn_filter = 1,
  .flags = SI_AUTO_USE_HEALTH
};

static const skill_info_t heretic_sk2_info = {
  .spawn_filter = 2,
  .flags = 0
};

static const skill_info_t heretic_sk3_info = {
  .spawn_filter = 3,
  .flags = SI_DEFAULT_SKILL
};

static const skill_info_t heretic_sk4_info = {
  .spawn_filter = 4,
  .flags = 0
};

static const skill_info_t heretic_sk5_info = {
  .ammo_factor = FRACUNIT * 3 / 2,
  .spawn_filter = 5,
  .flags = SI_FAST_MONSTERS | SI_INSTANT_REACTION
};

static const skill_info_t hexen_sk1_info = {
  .ammo_factor = FRACUNIT * 3 / 2,
  .damage_factor = FRACUNIT / 2,
  .spawn_filter = 1,
  .flags = SI_AUTO_USE_HEALTH
};

static const skill_info_t hexen_sk2_info = {
  .spawn_filter = 2,
  .flags = 0
};

static const skill_info_t hexen_sk3_info = {
  .spawn_filter = 3,
  .flags = SI_DEFAULT_SKILL
};

static const skill_info_t hexen_sk4_info = {
  .spawn_filter = 4,
  .flags = 0
};

static const skill_info_t hexen_sk5_info = {
  .ammo_factor = FRACUNIT * 3 / 2,
  .spawn_filter = 5,
  .flags = SI_FAST_MONSTERS | SI_INSTANT_REACTION
};

const skill_info_t doom_skill_infos[5] = {
  doom_sk1_info,
  doom_sk2_info,
  doom_sk3_info,
  doom_sk4_info,
  doom_sk5_info,
};

const skill_info_t heretic_skill_infos[5] = {
  heretic_sk1_info,
  heretic_sk2_info,
  heretic_sk3_info,
  heretic_sk4_info,
  heretic_sk5_info,
};

const skill_info_t hexen_skill_infos[5] = {
  hexen_sk1_info,
  hexen_sk2_info,
  hexen_sk3_info,
  hexen_sk4_info,
  hexen_sk5_info,
};

const skill_info_t* skill_infos;

int num_skills = 5;

void dsda_UpdateGameSkill(skill_t skill) {
  gameskill = skill;
  skill_info = skill_infos[skill];
}
