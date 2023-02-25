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

#include <assert.h>
#include <math.h>

#include "astar.h"
#include "r_state.h"
#include "repair.h"
#include "z_zone.h"

#define REPAIR_MAX_DEPTH 4
#define REPAIR_MAX_ITERATIONS 100

struct identry* ids_find(struct idset* ls, uint32_t id)
{
  uint16_t i;

  for (i = 0; i < ls->count; ++i)
    if (ls->entries[i].id == id)
      return &ls->entries[i];

  return NULL;
}

void ids_add(struct idset* ls, uint32_t id, uint32_t meta)
{
  // Maximum possible size of set given 16-bit count/capacity
  static const unsigned int maxcap = (uint16_t)-1;
  unsigned int newcap;
  uint16_t i;
  struct identry ins;

  // This is a set, so don't add duplicate entries
  if (ids_find(ls, id))
    return;

  if (ls->cap == ls->count) {
    // Need to reallocate entries
    newcap = ls->cap ? ls->cap * 2 : 8;
    if (newcap > maxcap)
      // A set should never get this large
      abort();
    ls->entries = Z_Realloc(ls->entries, sizeof(*ls->entries) * newcap);
    if (ls->entries == NULL)
      abort();
    ls->cap = newcap;
  }

  ins.id = id;
  ins.meta = meta;
  ls->entries[ls->count++] = ins;
};

void ids_update(struct idset* ids, struct idset* upd)
{
  uint16_t i;

  for (i = 0; i < upd->count; ++i)
    ids_add(ids, upd->entries[i].id, upd->entries[i].meta);
}

// Set of lines associated with each vertex in map
static struct idset* vertlines = NULL;

// Get set of lines for vertex
static inline struct idset* vl_get(int vid)
{
  assert(vid >= 0 && vid < numvertexes);
  return &vertlines[vid];
}

// Allocate and compute vertlines
void repair_setup(void)
{
  unsigned int i;

  assert(vertlines == NULL);
  vertlines = Z_Calloc(sizeof(*vertlines), numvertexes);
  if (vertlines == NULL)
    abort();

  for (i = 0; i < numvertexes; ++i)
    ids_init(&vertlines[i]);

  for (i = 0; i < numlines; ++i) {
    struct line_s* line = &lines[i];
    struct idset* ls;

    ls = vl_get(line->v1 - vertexes);
    ids_add(ls, i, 0);
    ls = vl_get(line->v2 - vertexes);
    ids_add(ls, i, 0);
  }
}

// Free memory for vertlines
void repair_teardown(void)
{
  unsigned int i;
  if (vertlines == NULL)
    return;

  for (i = 0; i < numvertexes; ++i)
    ids_destroy(&vertlines[i]);

  Z_Free(vertlines);
  vertlines = NULL;
}

// Node in repair search graph
struct rnode {
  // A* node struct
  struct as_node as;
  // Set of lines at this node
  struct idset lines;
  // Next allocated node in list (headed by rctx.nodes)
  struct rnode* next;
  // Search depth of this node
  unsigned int depth;
};

// Repair context
struct rctx {
  // Root node
  struct rnode root;
  // Linked list of allocated nodes
  struct rnode* nodes;
  // Scratch set used to determine closedness of lines
  struct idset verts;
  // Lines already used to generate graph edges in current search iteration
  struct idset lines;
  // Maximum search depth allowed
  unsigned int max_depth;
};

// Heuristic value indicating node is a dead end
static const unsigned long impossible = (unsigned long)-1;

// Compute heuristic value of node in search graph
static unsigned long rnode_heuristic(struct idset* ls, struct idset* scratch)
{
  uint16_t i;
  unsigned int unmatched;

  // Generate set of all vertices for node, with meta indicating
  // how many lines share it
  ids_clear(scratch);
  for (i = 0; i < ls->count; ++i) {
    struct line_s* line = &lines[ls->entries[i].id];
    int v1 = line->v1 - vertexes;
    int v2 = line->v2 - vertexes;
    struct identry* e;

    e = ids_find(scratch, v1);
    if (e == NULL)
      ids_add(scratch, v1, 1);
    else if (e->meta == 2)
      return impossible;
    else
      e->meta++;
    e = ids_find(scratch, v2);
    if (e == NULL)
      ids_add(scratch, v2, 1);
    else if (e->meta == 2)
      return impossible;
    else
      e->meta++;
  }

  // Count vertices missing a second line.
  // This will be 0 if the set of lines is closed.
  unmatched = 0;
  for (i = 0; i < scratch->count; ++i)
    unmatched += 2 - scratch->entries[i].meta;

  // FIXME: this is an admissible heuristic, but not a very good one
  // because it does not represent the expected cost (sum of line lengths).
  // Calculating this while remaining admissible is tricky, and this seems
  // to work well enough in practice.
  return unmatched;
}

// Derives a new node by adding a line to an existing one
static struct rnode* rnode_derive(struct rctx* ctx, struct rnode* rn,
                                  uint32_t vid, uint32_t lid, dboolean first)
{
  struct rnode* pn;
  uint16_t i;
  uint32_t dir;
  dboolean match = false;

  // Temporarily add line for comparison purposes below
  ids_add(&rn->lines, lid, 0);
 
  // FIXME: use a hash table
  for (pn = ctx->nodes; pn && !match; pn = pn->next) {
    if (pn->lines.count != rn->lines.count) {
      match = true;
      // FIXME: use a sorted list or something to avoid N^2 behavior
      for (i = 0; i < rn->lines.count; ++i) {
        if (!ids_find(&pn->lines, rn->lines.entries[i].id)) {
          match = false;
          break;
        }
      }
    }
  }

  // Drop temporary addition
  rn->lines.count--;

  if (match) {
    // If this is a shorter path, mark it
    if (pn->depth > rn->depth + 1)
      pn->depth = rn->depth + 1;
    return pn;
  }

  // Create new node
  pn = Z_Malloc(sizeof(*pn));
  if (pn == NULL)
    abort();
  pn->depth = rn->depth + 1;
  as_node_init(&pn->as);
  ids_init(&pn->lines);
  pn->next = ctx->nodes;
  ctx->nodes = pn;
  ids_update(&pn->lines, &rn->lines);

  // Determine direction of line, which is forward (v1 -> v2) if we
  // reached its v1 from v2 of the previous line, backward otherwise
  dir = (lines[lid].v1 == &vertexes[vid]) ^ first ? 0 : REPAIR_BACKWARD;
  ids_add(&pn->lines, lid, REPAIR_ADDED | dir);

  return pn;
}

static dboolean rnode_visit_vertex(struct astar* as, struct rctx* ctx,
                                   struct rnode* rn, uint32_t vid,
                                   dboolean first)
{
  struct idset* vl = vl_get(vid);
  struct line_s* line;
  struct rnode* pn = NULL;
  uint16_t i, j;
  float dx, dy;
  unsigned long cost, heuristic;
  uint32_t lid;
  uint32_t dir;

  for (i = 0; i < vl->count; ++i) {
    lid = vl->entries[i].id;
    line = &lines[lid];

    // Ignore lines already in set
    if (ids_find(&rn->lines, lid))
      continue;

    // Ignore lines already proposed to A* this iteration
    if (ids_find(&ctx->lines, lid))
      continue;

    ids_add(&ctx->lines, lid, 0);

    // Create new node for search
    pn = rnode_derive(ctx, rn, vid, lid, first);

    dir = (line->v1 == &vertexes[vid]) ^ first ? 0 : REPAIR_BACKWARD;
    ids_add(&pn->lines, lid, REPAIR_ADDED | dir);

    // Compute cost (length) of line, rounded back to fixed point
    dx = (float)line->dx / FRACUNIT;
    dy = (float)line->dy / FRACUNIT;
    cost = (unsigned int)(sqrt(dx * dx + dy * dy) * FRACUNIT + (FRACUNIT >> 1));
    // Ensure all lines have a non-zero cost, or our heuristic could become
    // inadmissible
    if (cost == 0)
      cost = 1;

    // Compute heuristic (expected future cost) of path
    // 0 means we reached the goal, impossible means we can never
    // reach the goal (can't form valid loops from this position in the search
    // graph because more than 2 lines in the set share a vertex).
    heuristic = rnode_heuristic(&pn->lines, &ctx->verts);
    if (heuristic == impossible)
      // No point even putting this in the A* queue
      continue;
    if (pn->depth == ctx->max_depth && heuristic != 0)
      // Hit depth limit and this is not a goal, give up
      continue;

    // Register edge for A* search
    as_edge(as, &pn->as, cost, heuristic);

    // If this was the goal, stop
    if (heuristic == 0)
      return true;
  }

  return false;
}

// Visit a node in the search graph and generate edges
static void rnode_visit(struct astar* as, struct as_node* e, void* data)
{
  struct rctx* ctx = (struct rctx*)data;
  struct rnode* rn = (struct rnode*)e;
  struct idset* verts = &ctx->verts;
  uint16_t i;

  ids_clear(verts);
  ids_clear(&ctx->lines);

  for (i = 0; i < rn->lines.count; ++i) {
    struct line_s* line = &lines[rn->lines.entries[i].id];
    int v1;
    int v2;

    // Order vertices appropriately
    if (rn->lines.entries[i].meta & REPAIR_BACKWARD) {
      v1 = line->v2 - vertexes;
      v2 = line->v1 - vertexes;
    } else {
      v1 = line->v1 - vertexes;
      v2 = line->v2 - vertexes;
    }

    // Look for candidate lines connected to vertices
    if (rnode_visit_vertex(as, ctx, rn, v1, true))
      break;
    if (rnode_visit_vertex(as, ctx, rn, v2, false))
      break;
  }
}

static dboolean is_trivial(line_t* l)
{
  return l->sidenum[0] != NO_INDEX && l->sidenum[1] != NO_INDEX &&
         sides[l->sidenum[0]].sector == sides[l->sidenum[1]].sector;
}

static dboolean is_isolated(line_t* l)
{
  struct idset* vl1 = vl_get(l->v1 - vertexes);
  struct idset* vl2 = vl_get(l->v2 - vertexes);

  return vl1->count < 2 && vl2->count < 2;
}

static dboolean is_duplicate(struct idset* ls, line_t* l)
{
  int i;
  for (i = 0; i < ls->count; ++i) {
    line_t* l2 = &lines[ls->entries[i].id];
    if (l->v1 == l2->v1 && l->v2 == l2->v2 &&
        l->frontsector == l2->frontsector && l->backsector == l2->backsector)
      return true;
  }
  return false;
}

void repair_sector_prepare(int num, struct idset* ls)
{
  int i;
  sector_t* sector = &sectors[num];
  struct idset scratch;

  ids_clear(ls);
  for (i = 0; i < sector->linecount; ++i) {
    uint32_t meta;
    line_t* l = sector->lines[i];

    // Ignore lines which won't help build a closed polygon
    if (is_trivial(l) || is_duplicate(ls, l))
      continue;
    // Also mark isolated lines
    if (is_isolated(l)) {
      l->flags |= RF_ISOLATED;
      continue;
    }

    if (l->sidenum[0] != NO_INDEX && sides[l->sidenum[0]].sector == sector)
      meta = 0;
    else
      meta = REPAIR_BACKWARD;

    ids_add(ls, l - lines, meta);
  }

  // Check for and mark sector closedness
  ids_init(&scratch);
  if (ls->count < 3 || rnode_heuristic(ls, &scratch) != 0)
    sectors[num].flags &= SECTOR_IS_CLOSED;
  else
    sectors[num].flags |= SECTOR_IS_CLOSED;
  ids_destroy(&scratch);
}

dboolean repair_sector(struct idset* lines)
{
  struct rctx ctx;
  struct astar as;
  struct rnode* goal;
  struct rnode* cur;
  struct rnode* next;
  int i;

  // Set up context and root search node
  ctx.nodes = NULL;
  ctx.root.depth = 0;
  as_node_init(&ctx.root.as);
  ids_init(&ctx.root.lines);
  ids_init(&ctx.lines);
  ids_init(&ctx.verts);
  ids_update(&ctx.root.lines, lines);

  // Perform A* search
  as_init(&as, rnode_visit, &ctx);
  as_set_max_depth(&as, REPAIR_MAX_DEPTH);
  as_set_max_iterations(&as, REPAIR_MAX_ITERATIONS);

  goal = (struct rnode*)as_search(&as, &ctx.root.as);
  if (goal != NULL) {
    // Steal lines from goal
    ids_destroy(lines);
    *lines = goal->lines;
    ids_init(&goal->lines);
  }

  // Cleanup
  for (cur = ctx.nodes; cur; cur = next) {
    next = cur->next;
    ids_destroy(&cur->lines);
    Z_Free(cur);
  }

  ids_destroy(&ctx.lines);
  ids_destroy(&ctx.verts);
  ids_destroy(&ctx.root.lines);

  return goal != NULL;
}
