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

typedef struct deh_mobj_index_entry_s {
  int index_in;
  int index_out;
  struct deh_mobj_index_entry_s* next;
} deh_mobj_index_entry_t;

#define DEH_MOBJ_INDEX_HASH_SIZE 128

static deh_mobj_index_entry_t deh_mobj_index_hash[DEH_MOBJ_INDEX_HASH_SIZE];

static int deh_mobj_index_start;
static int deh_mobj_index_end;

static int dsda_GetDehMobjIndex(int index) {
  deh_mobj_index_entry_t* entry;

  if (index < deh_mobj_index_start)
    return index;

  entry = &deh_mobj_index_hash[index % DEH_MOBJ_INDEX_HASH_SIZE];

  while (entry->next && entry->index_in != index)
    entry = entry->next;

  if (entry->index_in != index) {
    entry->next = calloc(1, sizeof(*entry));
    entry = entry->next;
    entry->index_in = index;
    entry->index_out = deh_mobj_index_end;
    ++deh_mobj_index_end;
  }

  return entry->index_out;
}

// Dehacked has the index off by 1
int dsda_TranslateDehMobjIndex(int index) {
  return dsda_GetDehMobjIndex(index - 1) + 1;
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

  deh_mobj_index_start = num_mobj_types;
  deh_mobj_index_end = num_mobj_types;
  edited_mobjinfo_bits = calloc(num_mobj_types, sizeof(*edited_mobjinfo_bits));
}

// Changing the renderer causes a reset that accesses this list,
//   so we can't free it.
void dsda_FreeDehMobjInfo(void) {
  // free(edited_mobjinfo_bits);
}
