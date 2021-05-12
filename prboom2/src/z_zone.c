/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      Zone Memory Allocation. Neat.
 *
 * Neat enough to be rewritten by Lee Killough...
 *
 * Must not have been real neat :)
 *
 * Made faster and more general, and added wrappers for all of Doom's
 * memory allocation functions, including malloc() and similar functions.
 * Added line and file numbers, in case of error. Added performance
 * statistics and tunables.
 *-----------------------------------------------------------------------------
 */


// use config.h if autoconf made one -- josh
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#include "z_zone.h"
#include "doomstat.h"
#include "m_argv.h"
#include "v_video.h"
#include "g_game.h"
#include "lprintf.h"

#ifdef DJGPP
#include <dpmi.h>
#endif

// Tunables

// signature for block header
#define ZONEID  0x931d4a11

// Number of mallocs & frees kept in history buffer (must be a power of 2)
#define ZONE_HISTORY 4

// End Tunables

typedef struct memblock {
  unsigned id;
  struct memblock *next,*prev;
  size_t size;
  void **user;
  unsigned char tag;
} memblock_t;

static const size_t HEADER_SIZE = sizeof(memblock_t);

static memblock_t *blockbytag[PU_MAX];

/* Z_Malloc
 * You can pass a NULL user if the tag is not PU_CACHE.
 *
 * cph - the algorithm here was a very simple first-fit round-robin
 *  one - just keep looping around, freeing everything we can until
 *  we get a large enough space
 *
 * This has been changed now; we still do the round-robin first-fit,
 * but we only free the blocks we actually end up using; we don't
 * free all the stuff we just pass on the way.
 */

void *Z_Malloc(size_t size, int tag, void **user)
{
  memblock_t *block = NULL;

  if (tag == PU_CACHE && !user)
    I_Error ("Z_Malloc: An owner is required for purgable blocks");

  if (!size)
    return user ? *user = NULL : NULL;           // malloc(0) returns NULL

  while (!(block = (malloc)(size + HEADER_SIZE))) {
    if (!blockbytag[PU_CACHE])
      I_Error ("Z_Malloc: Failure trying to allocate %lu bytes", (unsigned long) size);
    Z_FreeTag(PU_CACHE);
  }

  if (!blockbytag[tag])
  {
    blockbytag[tag] = block;
    block->next = block->prev = block;
  }
  else
  {
    blockbytag[tag]->prev->next = block;
    block->prev = blockbytag[tag]->prev;
    block->next = blockbytag[tag];
    blockbytag[tag]->prev = block;
  }

  block->size = size;
  block->id = ZONEID;         // signature required in block header
  block->tag = tag;           // tag
  block->user = user;         // user
  block = (memblock_t *)((char *) block + HEADER_SIZE);
  if (user)                   // if there is a user
    *user = block;            // set user to point to new block

  return block;
}

void Z_Free(void *p)
{
  memblock_t *block = (memblock_t *)((char *) p - HEADER_SIZE);

  if (!p)
    return;

  if (block->id != ZONEID)
    I_Error("Z_Free: freed a pointer without ZONEID");
  block->id = 0;              // Nullify id so another free fails

  if (block->user)            // Nullify user if one exists
    *block->user = NULL;

  if (block == block->next)
    blockbytag[block->tag] = NULL;
  else
    if (blockbytag[block->tag] == block)
      blockbytag[block->tag] = block->next;
  block->prev->next = block->next;
  block->next->prev = block->prev;

  (free)(block);
}

void Z_FreeTag(int tag)
{
  memblock_t *block, *end_block;

#ifdef HEAPDUMP
  Z_DumpMemory();
#endif

  if (tag < 0 || tag >= PU_MAX)
    I_Error("Z_FreeTag: Tag %i does not exist", tag);

  block = blockbytag[tag];
  if (!block)
    return;
  end_block = block->prev;
  while (1)
  {
    memblock_t *next = block->next;
    Z_Free((char *) block + HEADER_SIZE);
    if (block == end_block)
      break;
    block = next;               // Advance to next block
  }
}

void Z_ChangeTag(void *ptr, int tag)
{
  memblock_t *block = (memblock_t *)((char *) ptr - HEADER_SIZE);

  // proff - added sanity check, this can happen when an empty lump is locked
  if (!ptr)
    return;

  // proff - do nothing if tag doesn't differ
  if (tag == block->tag)
    return;

  if (block->id != ZONEID)
    I_Error ("Z_ChangeTag: freed a pointer without ZONEID");

  if (tag == PU_CACHE && !block->user)
    I_Error ("Z_ChangeTag: an owner is required for purgable blocks\n");

  if (block == block->next)
    blockbytag[block->tag] = NULL;
  else
    if (blockbytag[block->tag] == block)
      blockbytag[block->tag] = block->next;
  block->prev->next = block->next;
  block->next->prev = block->prev;

  if (!blockbytag[tag])
  {
    blockbytag[tag] = block;
    block->next = block->prev = block;
  }
  else
  {
    blockbytag[tag]->prev->next = block;
    block->prev = blockbytag[tag]->prev;
    block->next = blockbytag[tag];
    blockbytag[tag]->prev = block;
  }

  block->tag = tag;
}

void *Z_Realloc(void *ptr, size_t n, int tag, void **user)
{
  void *p = Z_Malloc(n, tag, user);
  if (ptr)
    {
      memblock_t *block = (memblock_t *)((char *) ptr - HEADER_SIZE);
      memcpy(p, ptr, n <= block->size ? n : block->size);
      Z_Free(ptr);
      if (user) // in case Z_Free nullified same user
        *user=p;
    }
  return p;
}

void *Z_Calloc(size_t n1, size_t n2, int tag, void **user)
{
  return
    (n1*=n2) ? memset(Z_Malloc(n1, tag, user), 0, n1) : NULL;
}

char *Z_Strdup(const char *s, int tag, void **user)
{
  return strcpy(Z_Malloc(strlen(s)+1, tag, user), s);
}
