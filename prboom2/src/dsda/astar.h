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
//	A* search
//

#include "doomtype.h"
#include <stdlib.h>

//
// Pairing heap
//
// This is a simple priority queue.  See Wikipedia for details:
//
// https://en.wikipedia.org/wiki/Pairing_heap
//
// This implementation uses a tag squeezed into the LSB of the
// child/sibling pointers to indicate that it is actually a pointer
// to the parent.  This trades memory compactness (and cache efficiency)
// for extra instruction overhead.

// Heap entry
// Should be embedded in user's structure
struct ph_entry {
  // If LSB is 1, points to parent instead
  struct ph_entry* next_sibling;
  // If LSB is 1, points to self instead
  struct ph_entry* first_child;
  unsigned long priority;
};

// Heap
struct pheap {
  struct ph_entry* root;
};

// Sets the tag bit on a pointer
static inline struct ph_entry* ph_tag(struct ph_entry* e)
{
  return (struct ph_entry*)((uintptr_t)e | 1);
}

// Initializes an entry, inline for speed
static inline void ph_entry_init(struct ph_entry* e)
{
  e->first_child = ph_tag(e);
  e->next_sibling = NULL;
}

// Initializes a heap, inline for speed
static inline void ph_init(struct pheap* ph) { ph->root = NULL; }

static inline dboolean ph_is_in_heap(struct pheap* ph, struct ph_entry* e)
{
  return e == ph->root || e->next_sibling != NULL;
}

//
// A* search
//
// See Wikipedia for details:
//
// https://en.wikipedia.org/wiki/A*_search_algorithm
//

// Search graph node
// Should be embedded in user structure
struct as_node {
  // Entry in pairing heap
  struct ph_entry ph;
  // Previous node on search path
  struct as_node* prev;
  // Cumulative cost of this path
  unsigned long cost;
  // Depth of this path
  uint32_t depth;
};

// Search context
struct astar {
  // Pairing heap priority queue
  struct pheap queue;
  // Set when goal is reached
  struct as_node* goal;
  // Current node being visited
  struct as_node* cur;
  // User context pointer for visit callback
  void* user_ctx;
  // Callback to visit a node and register edges to reachable nodes
  void (*visit)(struct astar*, struct as_node*, void*);
  // Maximum search depth
  uint32_t max_depth;
  // Maximum iterations
  uint32_t max_iterations;
};

// Initialize search node, inline for speed
static inline void as_node_init(struct as_node* e)
{
  ph_entry_init(&e->ph);
  e->prev = NULL;
  e->cost = 0;
  e->depth = 0;
}

// Initialize search context, inline for speed
static inline void as_init(struct astar* as,
                           void (*visit)(struct astar*, struct as_node*, void*),
                           void* user_ctx)
{
  as->visit = visit;
  as->user_ctx = user_ctx;
  as->max_iterations = 0;
  as->max_depth = 0;
}

static inline void as_set_max_iterations(struct astar* as, uint32_t max_iterations)
{
  as->max_iterations = max_iterations;
}

static inline void as_set_max_depth(struct astar* as, uint32_t max_depth)
{
  as->max_iterations = max_depth;
}

// Register edge in search graph
// This should only be called from the visit callback
void as_edge(struct astar* as, struct as_node* node, unsigned long cost,
             unsigned long heuristic);

// Perform search from root node, returning goal if reached
struct as_node* as_search(struct astar* as, struct as_node* root);
