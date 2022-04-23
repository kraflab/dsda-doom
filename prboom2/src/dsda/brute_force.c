//
// Copyright(C) 2022 by Ryan Krafnick
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
//	DSDA Brute Force
//

#include <math.h>

#include "d_player.h"
#include "d_ticcmd.h"
#include "doomstat.h"
#include "lprintf.h"
#include "m_random.h"

#include "dsda/key_frame.h"

#include "brute_force.h"

#define MAX_BF_DEPTH 5
#define MAX_BF_CONDITIONS 16

typedef struct {
  int min;
  int max;
  int i;
} bf_range_t;

typedef struct {
  dsda_key_frame_t key_frame;
  bf_range_t forwardmove;
  bf_range_t sidemove;
  bf_range_t angleturn;
} bf_t;

typedef struct {
  dsda_bf_attribute_t attribute;
  dsda_bf_operator_t operator;
  fixed_t value;
} bf_condition_t;

static bf_t brute_force[MAX_BF_DEPTH];
static int bf_depth;
static int bf_logictic;
static int bf_condition_count;
static bf_condition_t bf_condition[MAX_BF_CONDITIONS];
static long long bf_volume;
static long long bf_volume_max;
static dboolean bf_mode;

const char* dsda_bf_attribute_names[dsda_bf_attribute_max] = {
  [dsda_bf_x] = "x",
  [dsda_bf_y] = "y",
  [dsda_bf_z] = "z",
  [dsda_bf_momx] = "vx",
  [dsda_bf_momy] = "vy",
  [dsda_bf_speed] = "spd",
  [dsda_bf_damage] = "dmg",
  [dsda_bf_rng] = "rng",
};

const char* dsda_bf_operator_names[dsda_bf_operator_max] = {
  [dsda_bf_less_than] = "lt",
  [dsda_bf_less_than_or_equal_to] = "lteq",
  [dsda_bf_greater_than] = "gt",
  [dsda_bf_greater_than_or_equal_to] = "gteq",
  [dsda_bf_equal_to] = "eq",
  [dsda_bf_not_equal_to] = "neq"
};

static dboolean dsda_AdvanceBFRange(bf_range_t* range) {
  ++range->i;

  if (range->i > range->max) {
    range->i = range->min;
    return false;
  }

  return true;
}

static dboolean dsda_AdvanceBruteForceFrame(int frame) {
  if (!dsda_AdvanceBFRange(&brute_force[frame].angleturn))
    if (!dsda_AdvanceBFRange(&brute_force[frame].sidemove))
      if (!dsda_AdvanceBFRange(&brute_force[frame].forwardmove))
        return false;

  return true;
}

static int dsda_AdvanceBruteForce(void) {
  int i;

  ++bf_volume;

  for (i = bf_depth - 1; i >= 0; --i)
    if (dsda_AdvanceBruteForceFrame(i))
      break;

  return i;
}

static void dsda_RestoreBFKeyFrame(int frame) {
  dsda_RestoreKeyFrame(&brute_force[frame].key_frame);
}

static void dsda_StoreBFKeyFrame(int frame) {
  dsda_StoreKeyFrame(&brute_force[frame].key_frame, true);
}

static void dsda_PrintBFProgress(void) {
  int percent;

  percent = 100 * bf_volume / bf_volume_max;

  lprintf(LO_INFO, "  %lld / %lld sequences tested (%d%%)!\n", bf_volume, bf_volume_max, percent);
}

#define BF_FAILURE 0
#define BF_SUCCESS 1

static const char* bf_result_text[2] = { "FAILURE", "SUCCESS" };

static void dsda_EndBF(int result) {
  int percent;

  percent = 100 * bf_volume / bf_volume_max;

  lprintf(LO_INFO, "Brute force complete (%s)!\n", bf_result_text[result]);
  dsda_PrintBFProgress();

  if (result == BF_FAILURE)
    dsda_RestoreBFKeyFrame(0);

  bf_mode = false;
}

static dboolean dsda_BFApplyOperator(fixed_t current, int i) {
  switch (bf_condition[i].operator) {
    case dsda_bf_less_than:
      return current < bf_condition[i].value;
    case dsda_bf_less_than_or_equal_to:
      return current <= bf_condition[i].value;
    case dsda_bf_greater_than:
      return current > bf_condition[i].value;
    case dsda_bf_greater_than_or_equal_to:
      return current >= bf_condition[i].value;
    case dsda_bf_equal_to:
      return current == bf_condition[i].value;
    case dsda_bf_not_equal_to:
      return current != bf_condition[i].value;
    default:
      return false;
  }
}

static dboolean dsda_BFConditionReached(int i) {
  player_t* player;

  player = &players[displayplayer];

  switch (bf_condition[i].attribute) {
    case dsda_bf_x:
      return dsda_BFApplyOperator(player->mo->x, i);
    case dsda_bf_y:
      return dsda_BFApplyOperator(player->mo->y, i);
    case dsda_bf_z:
      return dsda_BFApplyOperator(player->mo->z, i);
    case dsda_bf_momx:
      return dsda_BFApplyOperator(player->mo->momx, i);
    case dsda_bf_momy:
      return dsda_BFApplyOperator(player->mo->momy, i);
    case dsda_bf_speed:
      return dsda_BFApplyOperator(P_PlayerSpeed(player), i);
    case dsda_bf_damage:
      {
        extern int player_damage_last_tic;

        return dsda_BFApplyOperator(player_damage_last_tic, i);
      }
    case dsda_bf_rng:
      return dsda_BFApplyOperator(rng.rndindex, i);
    default:
      return false;
  }
}

static dboolean dsda_BFConditionsReached(void) {
  int i, reached;

  reached = 0;
  for (i = 0; i < bf_condition_count; ++i)
    reached += dsda_BFConditionReached(i);

  return reached == bf_condition_count;
}

dboolean dsda_BruteForce(void) {
  return bf_mode;
}

void dsda_ResetBruteForceConditions(void) {
  bf_condition_count = 0;
}

void dsda_AddBruteForceCondition(dsda_bf_attribute_t attribute,
                                 dsda_bf_operator_t operator, fixed_t value) {
  if (bf_condition_count == MAX_BF_CONDITIONS)
    return;

  bf_condition[bf_condition_count].attribute = attribute;
  bf_condition[bf_condition_count].operator = operator;
  bf_condition[bf_condition_count].value = value << FRACBITS;
  ++bf_condition_count;

  lprintf(LO_INFO, "Added brute force condition: %s %s %d\n",
                   dsda_bf_attribute_names[attribute],
                   dsda_bf_operator_names[operator],
                   value);
}

dboolean dsda_StartBruteForce(int depth,
                              int forwardmove_min, int forwardmove_max,
                              int sidemove_min, int sidemove_max,
                              int angleturn_min, int angleturn_max) {
  int i;

  if (bf_depth > MAX_BF_DEPTH)
    return false;

  bf_depth = depth;
  bf_logictic = logictic;
  bf_volume = 0;
  bf_volume_max = pow((forwardmove_max - forwardmove_min + 1) *
                      (sidemove_max - sidemove_min + 1) *
                      (angleturn_max - angleturn_min + 1), depth);

  for (i = 0; i < bf_depth; ++i) {
    brute_force[i].forwardmove.min = forwardmove_min;
    brute_force[i].forwardmove.max = forwardmove_max;
    brute_force[i].forwardmove.i = forwardmove_min;

    brute_force[i].sidemove.min = sidemove_min;
    brute_force[i].sidemove.max = sidemove_max;
    brute_force[i].sidemove.i = sidemove_min;

    brute_force[i].angleturn.min = angleturn_min;
    brute_force[i].angleturn.max = angleturn_max;
    brute_force[i].angleturn.i = angleturn_min;
  }

  lprintf(LO_INFO, "Brute force starting!\n");

  bf_mode = true;

  return true;
}

void dsda_UpdateBruteForce(void) {
  int frame;

  if (dsda_BFConditionsReached()) {
    dsda_EndBF(BF_SUCCESS);

    return;
  }

  frame = logictic - bf_logictic;

  if (frame == bf_depth) {
    if (bf_volume % 1000 == 0)
      dsda_PrintBFProgress();

    frame = dsda_AdvanceBruteForce();

    if (frame >= 0)
      dsda_RestoreBFKeyFrame(frame);
    else
      dsda_EndBF(BF_FAILURE);
  }
  else
    dsda_StoreBFKeyFrame(frame);
}

void dsda_PopBruteForceCommand(ticcmd_t* cmd) {
  bf_t* bf;

  bf = &brute_force[logictic - bf_logictic];

  memset(cmd, 0, sizeof(*cmd));

  cmd->angleturn = bf->angleturn.i << 8;
  cmd->forwardmove = bf->forwardmove.i;
  cmd->sidemove = bf->sidemove.i;
}
