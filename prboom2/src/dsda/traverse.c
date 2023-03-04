#include <assert.h>

#include "traverse.h"
#include "lprintf.h"
#include "z_zone.h"

// Implements the cycle search algorithm by Donald B. Johnson
// from the paper "Finding All The Elementary Circuits of a Directed Graph",
// published in the SIAM Journal on Computing, March 1975:
//
// https://www.cs.tufts.edu/comp/150GA/homeworks/hw1/Johnson%2075.PDF
//
// This has time complexity O((v + e)(c + 1)), where v and e are the
// vertex and edge counts and c is the cycle count.  DOOM sectors
// have low cyclic complexity unless they have lots of bi-directional lines
// (same sector on both sides), in which case the running time can become
// excessive.  Paths with more than `UNDIR_DEPTH_LIMIT` bi-directional
// lines in a row are abandoned to avoid this.

// There's an exponential "knee" around 10, so this can't go much higher
#define UNDIR_DEPTH_LIMIT 8

// Tracks everything to do with a traversal
struct traverse_state
{
  struct traverse_ctx* ctx;
  sector_t* sector;
  // Start of traversal.  Only cycles for which this vertex is the
  // earliest are allowed.
  struct tpath head;
  // N * N bitmap, where N is the number of nodes (vertcount).
  // This is used to store the B(u) arrays in the algorithm,
  // as well as blocked(u) along the otherwise unused
  // diagonal, i.e. B(u)(u) is blocked(u)
  unsigned char* blockmap;
  // Number of unique vertices
  unsigned int vertcount;
  // Size of blockmap in bytes
  size_t bmapsize;
};

// Count unique vertices and number each with an ID.
// IDs start at 1 so that 0 means "no ID".  These are
// used as indices into the blockmap.  We also use it
// as the vertex ordering key for Johnson's algorithm.
static void NumberVerts(struct traverse_state* state)
{
  sector_t* sector = state->sector;
  int i;

  state->vertcount = 0;

  for (i = 0; i < sector->linecount; ++i)
  {
    line_t* l = sector->lines[i];

    if (!l->v1->id)
    {
      l->v1->id = ++state->vertcount;
    }
    if (!l->v2->id)
    {
      l->v2->id = ++state->vertcount;
    }
  }
}

static void SetupState(struct traverse_state* state, struct traverse_ctx* ctx,
                       sector_t* sector)
{
  state->ctx = ctx;
  state->sector = sector;

  NumberVerts(state);
  // Blockmap size is N * N bits, rounded up to the nearest byte
  state->bmapsize = (state->vertcount * state->vertcount + 7) / 8;
  state->blockmap = Z_Malloc(state->bmapsize);
  if (!state->blockmap)
    I_Error("traverse: Not enough memory for block map");
}

static void DestroyState(struct traverse_state* state)
{
  sector_t* sector = state->sector;
  int i;

  // Clear all IDs
  for (i = 0; i < sector->linecount; ++i)
  {
    line_t* l = sector->lines[i];

    l->v1->id = 0;
    l->v2->id = 0;
  }

  Z_Free(state->blockmap);
}

// Compute index and shift into bitmap
static void BlockMapIndex(struct traverse_state* state, unsigned short id1,
                          unsigned short id2, unsigned int* idx,
                          unsigned int* shift)
{
  unsigned int slot = (id1 - 1) * state->vertcount + (id2 - 1);
  *idx = slot / 8;
  *shift = slot % 8;
}

// Test bit in bitmap
static dboolean BlockMapTest(struct traverse_state* state, unsigned short id1,
                             unsigned short id2)
{
  unsigned int idx, shift;

  BlockMapIndex(state, id1, id2, &idx, &shift);

  return (state->blockmap[idx] >> shift) & 1;
}

// Set bit in bitmap
static void BlockMapSet(struct traverse_state* state, unsigned short id1,
                        unsigned short id2)
{
  unsigned int idx, shift;

  BlockMapIndex(state, id1, id2, &idx, &shift);

  state->blockmap[idx] |= (1 << shift);
}

// Clear bit in bitmap
static void BlockMapClear(struct traverse_state* state, unsigned short id1,
                          unsigned short id2)
{
  unsigned int idx, shift;

  BlockMapIndex(state, id1, id2, &idx, &shift);

  state->blockmap[idx] &= ~(1 << shift);
}

// Returns the vertex from traversing `line` forward from `v`, if applicable
static vertex_t* FollowEdge(sector_t* sector, vertex_t* v, line_t* line)
{
  if (line->v1 == v && line->frontsector == sector)
    return line->v2;
  if (line->v2 == v && line->backsector == sector)
    return line->v1;
  return NULL;
}

// Same as FollowEdge, but for traversing line in reverse
static vertex_t* FollowEdgeR(sector_t* sector, vertex_t* v, line_t* line)
{
  if (line->v1 == v && line->backsector == sector)
    return line->v2;
  if (line->v2 == v && line->frontsector == sector)
    return line->v1;
  return NULL;
}

//
// Kosaraju's algorithm
//
// Finds the strongly-connected components of the (sub-)graph.  In particular, we
// only consider the subgraph with vertices greater than or equal to the vertex
// in the Johnson algorithm outer loop.  We also track the smallest vertex
// that was part of any strongly-connected component.
//
// There are better algorithms for this, but this has very simple bookkeeping
// and the same time complexity compared to the better ones.
//
// https://en.wikipedia.org/wiki/Kosaraju%27s_algorithm

// Pass 1 -- create list of vertices to visit
static void SCCPass1(sector_t* sector, vertex_t** list, unsigned short minid,
                     vertex_t* v)
{
  int i;

  if (v->visited || v->id < minid)
    return;

  v->visited = true;

  for (i = 0; i < sector->linecount; ++i)
  {
    line_t* l = sector->lines[i];
    vertex_t* u = FollowEdge(sector, v, l);
    if (u == NULL)
      continue;
    SCCPass1(sector, list, minid, u);
  }

  v->prev = *list;
  *list = v;
}

// Pass 2 - tag vertices with root of SCC
static void SCCPass2(sector_t* sector, vertex_t** min, unsigned short minid,
                     vertex_t* v, vertex_t* r)
{
  int i;

  if (v->id < minid || v->root)
    return;

  v->root = r;

  // Update minimum vertex in a SCC
  if (!*min || v->id < (*min)->id)
    *min = v;

  for (i = 0; i < sector->linecount; ++i)
  {
    line_t* l = sector->lines[i];
    vertex_t* w = FollowEdgeR(sector, v, l);
    if (w == NULL)
      continue;
    SCCPass2(sector, min, minid, w, r);
  }
}

// Compute SCCs of subgraph of vertices >= minid, return minimum vertex
// in any SCC
static vertex_t* SCCMin(sector_t* sector, unsigned short minid)
{
  int i;
  vertex_t* list = NULL;
  vertex_t* min = NULL;
  vertex_t* cur;

  // Initialize bookkeeping
  for (i = 0; i < sector->linecount; ++i)
  {
    line_t* l = sector->lines[i];
    l->v1->visited = false;
    l->v1->prev = NULL;
    l->v1->root = NULL;
    l->v2->visited = false;
    l->v2->prev = NULL;
    l->v2->root = NULL;
  }

  // Pass 1
  for (i = 0; i < sector->linecount; ++i)
  {
    line_t* l = sector->lines[i];
    SCCPass1(sector, &list, minid, l->v1);
    SCCPass1(sector, &list, minid, l->v2);
  }

  // Pass 2
  for (cur = list; cur; cur = cur->prev)
    SCCPass2(sector, &min, minid, cur, cur);

  return min;
}

// UNBLOCK procedure in paper
static void Unblock(struct traverse_state* state, unsigned short id)
{
  int i;

  BlockMapClear(state, id, id);
  for (i = 1; i <= state->vertcount; ++i)
  {
    if (i == id || !BlockMapTest(state, id, i))
      continue;
    BlockMapClear(state, id, i);
    if (BlockMapTest(state, i, i))
        Unblock(state, i);
  }
}

// Core recursive traversal function, called CIRCUIT in the paper
static dboolean Traverse(struct traverse_state* state, struct tpath* path,
                         unsigned int udepth)
{
  int i = 0;
  struct tpath next;
  struct tpath* cur;
  struct tpath* last;
  struct traverse_ctx* ctx = state->ctx;
  sector_t* sector = state->sector;
  dboolean found = false;
  unsigned int newdepth;

  // Enforce undirected depth limit to prevent excessive runtime on
  // self-referencing sectors with high cyclic complexity
  if (udepth > UNDIR_DEPTH_LIMIT)
  {
    // Let user know paths were abandoned (which is itself a good
    // heuristic for the sector being self-referencing)
    ctx->incomplete = true;
    // Conservatively return true so that unblocking happens,
    // since we don't know if we'd reach a cycle from this point
    // or not
    return true;
  }

  next.next = NULL;
  next.prev = path;
  path->next = &next;

  // Block this vertex from being recursed on
  BlockMapSet(state, path->v->id, path->v->id);

  // Enumerate all outbound edges from this vertex
  for (i = 0; i < sector->linecount; ++i)
  {
    next.line = sector->lines[i];
    next.v = FollowEdge(sector, path->v, next.line);

    // The second check here avoids traversing vertices
    // that aren't in the current strongly-connected
    // component
    if (next.v == NULL || next.v->root != state->head.v->root)
      continue;

    // Tell user about new path
    if (ctx->pcb)
      ctx->pcb(sector, &next, ctx->data);

    // Check for a cycle
    if (next.v == state->head.v)
    {
      // Don't emit cycles consisting of a single bi-directional line.
      // We still need to treat it as a cycle so unblocking
      // occurs.
      if (ctx->ccb && next.line != path->line)
        ctx->ccb(sector, state->head.next, ctx->data);
      found = true;
      // Terminate this search branch
      continue;
    }

    // Don't recurse on this vertex if it's blocked
    if (BlockMapTest(state, next.v->id, next.v->id))
      continue;

    // Compute new undirected line depth
    newdepth = next.line->frontsector == next.line->backsector ? udepth + 1 : udepth;
    // Recursively deepen search, noting if we found a cycle
    if (Traverse(state, &next, newdepth))
      found = true;
  }

  path->next = NULL;

  if (found)
    // Unblock vertices on finding a cycle
    Unblock(state, path->v->id);
  else
  {
    // Register edges in the block map so that
    // we are unblocked when an applicable cycle
    // is found
    for (i = 0; i < sector->linecount; ++i)
    {
      line_t* l = sector->lines[i];
      vertex_t* u = FollowEdge(sector, path->v, l);
      if (u == NULL || u->root != state->head.v->root)
        continue;
      BlockMapSet(state, u->id, path->v->id);
    }
  }

  return found;
}

// Begin traversal from a given vertex, returning the actual
// vertex used from the SCC computation (which may be a later one)
vertex_t* TraverseFrom(struct traverse_state* state, vertex_t* v)
{
  v = SCCMin(state->sector, v->id);
  if (!v)
    return NULL;

  state->head.v = v;
  state->head.line = NULL;
  state->head.prev = NULL;
  state->head.next = NULL;

  memset(state->blockmap, 0, state->bmapsize);

  Traverse(state, &state->head, 0);

  return v;
}

void dsda_TraverseSectorGraph(struct traverse_ctx* ctx, sector_t* sector)
{
  struct traverse_state state;
  int i;
  vertex_t* last = NULL;

  // Don't traverse empty sectors
  if (sector->linecount == 0)
    return;

  SetupState(&state, ctx, sector);

  // Outer loop of Johnson algorithm.
  //
  // `last` is the last vertex processed.
  // If `last` is NULL, we should process the next vertex
  // we come across.  Otherwise, we should clear it once
  // we come across it, so we process the next vertex
  // after it.  This logic is necessary because the Johnson
  // algorithm can jump ahead in the logical list of vertices,
  // but we're traversing the line list and need to keep looping
  // until we catch up.
  //
  // If we had an array of vertices, this loop would be:
  //
  // for i in len(vertex):
  //   next = TraverseFrom(vertex[i])
  //   if next == NULL:
  //     break
  //   else:
  //     i = next->id (note no -1)
  for (i = 0; i < sector->linecount; ++i)
  {
    line_t* l = sector->lines[i];

    if (last == NULL)
    {
      last = TraverseFrom(&state, l->v1);
      if (last == NULL)
        break;
    }
    else if (last == l->v1)
      last = NULL;

    if (last == NULL)
    {
      last = TraverseFrom(&state, l->v2);
      if (last == NULL)
        break;
    }
    else if (last == l->v2)
      last = NULL;
  }

  DestroyState(&state);
}
