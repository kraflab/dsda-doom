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

#include <assert.h>

#include "astar.h"

// Test whether a pointer is tagged
static inline dboolean ph_is_tagged(struct ph_entry* e)
{
  return ((uintptr_t)e & 1) == 1;
}

// Remove tag from pointer so it can be dereferenced
static inline struct ph_entry* ph_untag(struct ph_entry* e)
{
  return (struct ph_entry*)((uintptr_t)e & ~1);
}

// Convert a tagged pointer to NULL to simplify logic
static inline struct ph_entry* ph_tagged_to_null(struct ph_entry* e)
{
  if (ph_is_tagged(e))
    return NULL;
  return e;
}

// Return parent entry by finding tagged pointer at end of sibling list
static struct ph_entry* ph_parent(struct ph_entry* e)
{
  struct ph_entry* cur;

  for (cur = e->next_sibling; !ph_is_tagged(cur); cur = cur->next_sibling)
    ;

  return ph_untag(cur);
}

// Remove an entry from heap given its parent
static void ph_cut(struct ph_entry* p, struct ph_entry* e)
{
  struct ph_entry** cur;

  for (cur = &p->first_child; *cur != e; cur = &(*cur)->next_sibling)
    ;
  *cur = e->next_sibling;
  ph_entry_init(e);
}

// Meld two heaps into one according to priority
static struct ph_entry* ph_meld(struct ph_entry* left, struct ph_entry* right)
{
  // If there is only one heap, return that
  if (left == NULL)
    return right;
  if (right == NULL)
    return left;
  // Make the heap with priority the new root and the other its child
  if (left->priority < right->priority) {
    right->next_sibling = left->first_child;
    left->first_child = right;
    return left;
  } else {
    left->next_sibling = right->first_child;
    right->first_child = left;
    return right;
  }
}

// Insert into heap
static void ph_insert(struct pheap* ph, struct ph_entry* e,
                      unsigned long priority)
{
  e->priority = priority;
  // Just treat new entry as its own heap and meld
  ph->root = ph_meld(ph->root, e);
}

// Decrease key (increase priority) of entry in heap
static void ph_decrease_key(struct pheap* ph, struct ph_entry* e,
                            unsigned long priority)
{
  struct ph_entry* p;
  // Key must decrease
  assert(priority < e->priority);
  e->priority = priority;
  // If entry is already the root, nothing to do
  if (ph->root == e)
    return;
  p = ph_parent(e);
  if (e->priority < p->priority) {
    // Entry now has lower key than its parent, so
    // remove it from heap and reinsert it to restore
    // invariant.
    ph_cut(p, e);
    ph->root = ph_meld(ph->root, e);
  }
}

// Merge list of siblings headed by `e` into new heap
static struct ph_entry* ph_merge(struct ph_entry* e)
{
  struct ph_entry* next;

  // Empty list, empty result
  if (e == NULL)
    return NULL;
  // Singleton list, result is just the single entry
  if ((next = ph_tagged_to_null(e->next_sibling)) == NULL)
    return e;

  // Meld first two entries in list ("pair" them),
  // then meld with recursive merge of remainder of list
  return ph_meld(ph_meld(e, next),
                 ph_merge(ph_tagged_to_null(next->next_sibling)));
}

// Remove entry with lowest key (highest priority) from heap and return it
static struct ph_entry* ph_pop(struct pheap* ph)
{
  struct ph_entry* root;

  root = ph->root;
  if (root) {
    // Merge children of root to get new root of heap
    ph->root = ph_merge(ph_tagged_to_null(root->first_child));
    ph_entry_init(root);
  }
  return root;
}

void as_edge(struct astar* as, struct as_node* node, unsigned long cost,
             unsigned long heuristic)
{
  unsigned long total;
  struct as_node* cur = as->cur;

  if (node->prev != NULL && node->cost <= cost + cur->cost)
    // Equal or more expensive path to existing node, ignore it
    return;

  node->prev = cur;
  node->cost = cost + cur->cost;
  if (node->depth == 0 || node->depth > cur->depth + 1)
    node->depth = cur->depth + 1;
  // Check for overflow
  assert(node->cost >= cost && node->cost >= cur->cost);

  // If this is goal node, just record it and return.
  // All admissible heuristics must be 0 for a goal node.
  if (heuristic == 0) {
    as->goal = node;
    return;
  }

  if (as->max_depth != 0 && node->depth == as->max_depth)
    // Hit depth limit, give up
    return;

  total = node->cost + heuristic;
  // Check for overflow
  assert(total >= node->cost && total >= heuristic);
  if (ph_is_in_heap(&as->queue, &node->ph))
    ph_decrease_key(&as->queue, &node->ph, total);
  else
    ph_insert(&as->queue, &node->ph, total);
}

struct as_node* as_search(struct astar* as, struct as_node* root)
{
  struct as_node* e = NULL;
  uint32_t i = 0;
  uint32_t max = as->max_iterations;

  // Prepare for search
  as->goal = NULL;
  ph_init(&as->queue);
  ph_insert(&as->queue, &root->ph, 0);

  // Search!
  for (i = 0; as->goal == NULL && (max == 0 || i < max); ++i) {
    // Get lowest-key (highest priority) candidate
    as->cur = (struct as_node*)ph_pop(&as->queue);
    if (as->cur == NULL)
      // No more candidates, search failed
      return NULL;
    // Discover edges
    as->visit(as, as->cur, as->user_ctx);
  }

  return as->goal;
}
