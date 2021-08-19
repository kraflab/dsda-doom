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
//	DSDA Mobj Info
//

#include <stdlib.h>
#include <string.h>

#include "p_map.h"

#include "mobjinfo.h"

mobjinfo_t* mobjinfo;
int num_mobj_types;
int mobj_types_zero;
byte* edited_mobjinfo_bits;

static void dsda_ResetMobjInfo(int from, int to) {
  int i;

  for (i = from; i < to; ++i) {
    mobjinfo[i].droppeditem = MT_NULL;
    mobjinfo[i].infighting_group = IG_DEFAULT;
    mobjinfo[i].projectile_group = PG_DEFAULT;
    mobjinfo[i].splash_group = SG_DEFAULT;
    mobjinfo[i].altspeed = NO_ALTSPEED;
    mobjinfo[i].meleerange = MELEERANGE;
  }
}

static void dsda_EnsureCapacity(int limit) {
  while (limit >= num_mobj_types) {
    int old_num_mobj_types = num_mobj_types;

    num_mobj_types *= 2;

    mobjinfo = realloc(mobjinfo, num_mobj_types * sizeof(*mobjinfo));
    memset(mobjinfo + old_num_mobj_types, 0,
      (num_mobj_types - old_num_mobj_types) * sizeof(*mobjinfo));

    edited_mobjinfo_bits =
      realloc(edited_mobjinfo_bits, num_mobj_types * sizeof(*edited_mobjinfo_bits));
    memset(edited_mobjinfo_bits + old_num_mobj_types, 0,
      (num_mobj_types - old_num_mobj_types) * sizeof(*edited_mobjinfo_bits));

    dsda_ResetMobjInfo(old_num_mobj_types, num_mobj_types);
  }
}

dsda_deh_mobjinfo_t dsda_GetDehMobjInfo(int index) {
  dsda_deh_mobjinfo_t deh_mobjinfo;

  dsda_EnsureCapacity(index);

  deh_mobjinfo.info = &mobjinfo[index];
  deh_mobjinfo.edited_bits = &edited_mobjinfo_bits[index];

  return deh_mobjinfo;
}

void dsda_InitializeMobjInfo(int zero, int max, int count) {
  extern dboolean raven;

  num_mobj_types = count;
  mobj_types_zero = zero;

  mobjinfo = calloc(num_mobj_types, sizeof(*mobjinfo));

  if (raven) return;

  edited_mobjinfo_bits = calloc(num_mobj_types, sizeof(*edited_mobjinfo_bits));
}

// Changing the renderer causes a reset that accesses this list,
//   so we can't free it.
void dsda_FreeDehMobjInfo(void) {
  // free(edited_mobjinfo_bits);
}
