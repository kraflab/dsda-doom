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

#include "m_fixed.h"

#include "pclass.h"

dsda_pclass_t pclass[NUMCLASSES] = {
  [PCLASS_NULL] = {
    .armor_increment = { 0 },
    .auto_armor_save = 0,
  },

  [PCLASS_FIGHTER] = {
    .armor_increment = { 25 * FRACUNIT, 20 * FRACUNIT, 15 * FRACUNIT, 5 * FRACUNIT },
    .auto_armor_save = 15 * FRACUNIT,
  },

  [PCLASS_CLERIC] = {
    .armor_increment = { 10 * FRACUNIT, 25 * FRACUNIT, 5 * FRACUNIT, 20 * FRACUNIT },
    .auto_armor_save = 10 * FRACUNIT,
  },

  [PCLASS_MAGE] = {
    .armor_increment = { 5 * FRACUNIT, 15 * FRACUNIT, 10 * FRACUNIT, 25 * FRACUNIT },
    .auto_armor_save = 5 * FRACUNIT,
  },

  [PCLASS_PIG] = {
    .armor_increment = { 0 },
    .auto_armor_save = 0,
  },
};
