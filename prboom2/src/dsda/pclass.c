//
// Copyright(C) 2021 by Ryan Krafnick
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
//	DSDA Player Class
//

#include "pclass.h"

dsda_pclass_t pclass[NUMCLASSES] = {
  [PCLASS_NULL] = {
    .armor_increment = { 0 },
    .auto_armor_save = 0,
    .forwardmove = { 0x19, 0x32 },
    .sidemove = { 0x18, 0x28 },
    .max_player_move = 0x32,
  },

  [PCLASS_FIGHTER] = {
    .armor_increment = { 25 * FRACUNIT, 20 * FRACUNIT, 15 * FRACUNIT, 5 * FRACUNIT },
    .auto_armor_save = 15 * FRACUNIT,
    .forwardmove = { 0x1D, 0x3C },
    .sidemove = { 0x1B, 0x3B },
    .max_player_move = 0x3C,
  },

  [PCLASS_CLERIC] = {
    .armor_increment = { 10 * FRACUNIT, 25 * FRACUNIT, 5 * FRACUNIT, 20 * FRACUNIT },
    .auto_armor_save = 10 * FRACUNIT,
    .forwardmove = { 0x19, 0x32 },
    .sidemove = { 0x18, 0x28 },
    .max_player_move = 0x32,
  },

  [PCLASS_MAGE] = {
    .armor_increment = { 5 * FRACUNIT, 15 * FRACUNIT, 10 * FRACUNIT, 25 * FRACUNIT },
    .auto_armor_save = 5 * FRACUNIT,
    .forwardmove = { 0x16, 0x2E },
    .sidemove = { 0x15, 0x25 },
    .max_player_move = 0x2D,
  },

  [PCLASS_PIG] = {
    .armor_increment = { 0 },
    .auto_armor_save = 0,
    .forwardmove = { 0x18, 0x31 },
    .sidemove = { 0x17, 0x27 },
    .max_player_move = 0x31,
  },
};
