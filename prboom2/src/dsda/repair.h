//
// Copyright(C) 2023 Brian Koropoff
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
//	Unclosed sector repair
//

#include <stdlib.h>

#include "doomtype.h"
#include "z_zone.h"

// Line added by sector repair
#define REPAIR_ADDED 0x1
// Line should be reversed during tessellation
#define REPAIR_BACKWARD 0x2

//
// ID sets
//
// FIXME: move to separate .h/.c?

// Entry in idset
// Uses bitflags for compactness since a lot of these can be allocated
// for vertex->line lookup table
struct identry {
  // Identifier
  uint32_t id : 30;
  // Metadata for any use
  uint32_t meta : 2;
};

// Set of identifiers
struct idset {
  struct identry* entries;
  uint16_t cap;
  uint16_t count;
};

// Initialize set, inline for speed
static inline void ids_init(struct idset* ids)
{
  ids->cap = 0;
  ids->count = 0;
  ids->entries = NULL;
}

// Clear set, inline for speed
static inline void ids_clear(struct idset* ids) { ids->count = 0; }

// Destroy set, inline for speed
static inline void ids_destroy(struct idset* ids)
{
  if (ids->entries)
    Z_Free(ids->entries);
}

// Find entry in set for id, NULL if not found
struct identry* ids_find(struct idset* ls, uint32_t id);
// Add entry to set
void ids_add(struct idset* ls, uint32_t id, uint32_t meta);
// Add all entries in `upd` to `ids`
void ids_update(struct idset* ids, struct idset* upd);

//
// Sector repair
//

// Initialize global data structures for repair
void repair_setup(void);
// Tear down global data structure for repair
void repair_teardown(void);

// Prepare sector for repair and tesselation
//
// - Populates `ls` with lines from sector which are applicable
//   for repair/tesselation
//   * Line is not trivial, isolated, or duplicate
//   * Entry meta indicates whether line is backward with REPAIR_BACKWARD
// - Marks isolated lines
// - Marks the sector as unclosed if it is
void repair_sector_prepare(int num, struct idset* ls);

// Attempt to repair unclosed sector by adding lines
// reachable by common vertices.
//
// lines: initial set of lines (with REPAIR_BACKWARD in meta if applicable).
//        Additional lines are added (with REPAIR_ADDED in meta) on success.
//
// Returns true on success
dboolean repair_sector(struct idset* lines);
