#include "traverse.h"
#include "z_zone.h"

struct edge
{
  line_t* line;
  vertex_t* v1;
  vertex_t* v2;
};

struct state
{
  struct traverse_ctx* ctx;
  struct tpath head;
  unsigned char* blockmap;
  unsigned int vertcount;
  size_t bmapsize;
};

static void NumberVerts(struct state* state)
{
  int i;
  sector_t* sector = state->ctx->sector;

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

static void SetupState(struct state* state, struct traverse_ctx* ctx)
{
  state->ctx = ctx;

  NumberVerts(state);
  state->bmapsize = (state->vertcount * state->vertcount + 7) / 8;
  state->blockmap = Z_Malloc(state->bmapsize);
}

static void DestroyState(struct state* state)
{
  sector_t* sector = state->ctx->sector;
  int i;

  for (i = 0; i < sector->linecount; ++i)
  {
    line_t* l = sector->lines[i];

    l->v1->id = 0;
    l->v1->visited = false;
    l->v2->id = 0;
    l->v2->visited = false;
  }

  Z_Free(state->blockmap);
}

static void BlockMapIndex(struct state* state, uint16_t id1, uint16_t id2,
                          unsigned int* idx, unsigned int* shift)
{
  unsigned int slot = (id1 - 1) * state->vertcount + (id2 - 1);
  *idx = slot / 8;
  *shift = slot % 8;
}

static dboolean BlockMapTest(struct state* state, uint16_t id1, uint16_t id2)
{
  unsigned int idx, shift;

  BlockMapIndex(state, id1, id2, &idx, &shift);

  return (state->blockmap[idx] >> shift) & 1;
}

static void BlockMapSet(struct state* state, uint16_t id1, uint16_t id2)
{
  unsigned int idx, shift;

  BlockMapIndex(state, id1, id2, &idx, &shift);

  state->blockmap[idx] |= (1 << shift);
}

static void BlockMapClear(struct state* state, uint16_t id1, uint16_t id2)
{
  unsigned int idx, shift;

  BlockMapIndex(state, id1, id2, &idx, &shift);

  state->blockmap[idx] &= ~(1 << shift);
}

static vertex_t* FollowEdge(sector_t* sector, vertex_t* v, line_t* line)
{
  if (line->v1 == v && line->frontsector == sector)
    return line->v2;
  if (line->v2 == v && line->backsector == sector)
    return line->v1;
  return NULL;
}

static void Unblock(struct state* state, uint16_t id)
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

static dboolean Traverse(struct state* state, struct tpath* path)
{
  int i = 0;
  struct tpath next;
  struct tpath* cur;
  struct tpath* last;
  struct tpath* cycle;
  struct tpath* saved;
  struct traverse_ctx* ctx = state->ctx;
  sector_t* sector = ctx->sector;
  dboolean found = false;

  next.next = NULL;
  next.prev = path;
  path->next = &next;

  BlockMapSet(state, path->v->id, path->v->id);

  for (i = 0; i < sector->linecount; ++i)
  {
    line_t* l = sector->lines[i];
    next.v = FollowEdge(sector, path->v, l);
    if (next.v == NULL || next.v < state->head.v)
      continue;

    next.line = l;

    if (ctx->pcb)
      ctx->pcb(ctx->sector, &next, ctx->data);

    // Look for a cycle
    if (next.v == state->head.v)
    {
      // Don't consider cycles consisting of a bi-directional line
      if (ctx->ccb && next.line != path->line)
        ctx->ccb(ctx->sector, state->head.next, ctx->data);
      found = true;
      // Terminate this search branch
      continue;
    }

    if (BlockMapTest(state, next.v->id, next.v->id))
      continue;

    // Recursively deepen search
    if (Traverse(state, &next))
      found = true;
  }

  path->next = NULL;

  if (found)
    BlockMapClear(state, path->v->id, path->v->id);
  else
  {
    for (i = 0; i < sector->linecount; ++i)
    {
      line_t* l = sector->lines[i];
      next.v = FollowEdge(sector, path->v, l);
      if (next.v == NULL || next.v == path->v)
        continue;
      BlockMapSet(state, next.v->id, path->v->id);
    }
  }

  return found;
}

void TraverseFrom(struct state* state, vertex_t* v)
{
  state->head.v = v;
  state->head.line = NULL;
  state->head.prev = NULL;
  state->head.next = NULL;

  memset(state->blockmap, 0, state->bmapsize);

  Traverse(state, &state->head);
}

void dsda_TraverseSectorGraph(struct traverse_ctx* ctx)
{
  struct state state;
  sector_t* sector = ctx->sector;
  int i;

  SetupState(&state, ctx);

  // FIXME: this is inefficient compared to the full
  // algorithm implementation that computes strongly-connected
  // components of subgraphs to only start traversals from vertexes
  // that can yield new cycles and avoid visiting cycles twice,
  // but that requires a lot more bookkeeping and this seems to
  // work well enough.
  for (i = 0; i < sector->linecount; ++i)
  {
    line_t* l = sector->lines[i];

    if (!l->v1->visited)
    {
      l->v1->visited = true;
      TraverseFrom(&state, l->v1);
    }

    if (!l->v2->visited)
    {
      l->v2->visited = true;
      TraverseFrom(&state, l->v2);
    }
  }

  DestroyState(&state);
}
