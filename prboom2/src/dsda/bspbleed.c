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
//	BSP Analysis
//

#include "dsda/workpool.h"

#include "bspinternal.h"

// Calibrated to cover deep "fake" sectors in maps encountered in practice while
// being as efficient as possible
#define BLEED_THROUGH_DEPTH_LIMIT 3

// Bleed queue
typedef struct
{
  void** chunks;
  unsigned int count;
  unsigned int cap;
} bleedqueue_t;

// Temporary bleed before being added to global array
typedef struct
{
  bleed_t bleed;
  gl_chunk_t* source;
} tempbleed_t;

// Temporary bleed array
typedef struct
{
  tempbleed_t* bleeds;
  unsigned int count;
  unsigned int cap;
  unsigned int start;
} bleeds_t;

// Bleed worker local state
typedef struct
{
  // Queue of chunks to add bleed for
  bleedqueue_t queue;
  // Temporary bleeds
  bleeds_t bleeds;
  // Marked chunks
  byte mark[];
} bleedlocal_t;

static unsigned int capbleeds;

static void PushGlobalBleed(bleed_t* bleed)
{
  if (gl_rstate.numbleeds == capbleeds)
  {
    if (capbleeds == 0)
      capbleeds = 256;
    else
      capbleeds *= 2;
    gl_rstate.bleeds =
        Z_Realloc(gl_rstate.bleeds, sizeof(*gl_rstate.bleeds) * capbleeds);
  }

  gl_rstate.bleeds[gl_rstate.numbleeds++] = *bleed;
}

static void PushBleed(bleeds_t* bleeds, gl_chunk_t* source,
                      gl_chunk_t* target, bleedtype_t type, int depth)
{
  tempbleed_t* bleed;

  if (bleeds->count == bleeds->cap)
  {
    if (bleeds->cap == 0)
      bleeds->cap = 256;
    else
      bleeds->cap *= 2;
    // Zone memory functions are not thread-safe
    bleeds->bleeds = realloc(bleeds->bleeds, sizeof(*bleeds->bleeds) * bleeds->cap);
    if (!bleeds->bleeds)
      abort();
  }

  bleed = &bleeds->bleeds[bleeds->count++];
  bleed->bleed.target = target - gl_rstate.chunks;
  bleed->bleed.type = type;
  bleed->bleed.depth = depth;
  bleed->source = source;
}

static void PushQueue(bleedqueue_t* queue, chunkmeta_t* chunkmeta)
{
  if (queue->count == queue->cap)
  {
    if (queue->cap == 0)
      queue->cap = 256;
    else
      queue->cap *= 2;
    // Zone memory functions are not thread-safe
    queue->chunks = realloc(
        queue->chunks, sizeof(*queue->chunks) * queue->cap);
    if (!queue->chunks)
      abort();
  }
  queue->chunks[queue->count++] = chunkmeta;
}

// Based on logic in gzdoom.
// See https://github.com/ZDoom/gzdoom/blob/6cf91d3/src/rendering/hwrenderer/scene/hw_renderhacks.cpp#L539
static dboolean BleedOverTransitive(chunkmeta_t* chunkmeta, dboolean ceiling,
                                    fixed_t height, bleedlocal_t* bl)
{
  piter_t iter;
  dboolean result = true;

  if (!(chunkmeta->flags & CHUNKF_INTERIOR))
    return false;

  bl->mark[chunkmeta - dsda_gl_rstate.chunkmeta] = true;
  PushQueue(&bl->queue, chunkmeta);

  for (IterPerimeter(&iter, chunkmeta); IterPerimeterValid(&iter);
       IterPerimeterNext(&iter))
  {
    fixed_t backheight;

    // Skip single-sided lines
    if (!iter.backchunkmeta)
      continue;

    // Skip already-visited chunks
    if (bl->mark[iter.backchunkmeta - dsda_gl_rstate.chunkmeta])
      continue;

    // Skip degenerate chunks
    // FIXME: are degenerate chunks a problem with ajbsp?
    if (iter.backchunkmeta->flags & CHUNKF_DEGENERATE)
      continue;

    backheight =
        ceiling ? iter.backsector->ceilingheight : iter.backsector->floorheight;

    // Bleeding stops at textured segs
    if (iter.backside &&
        ((ceiling && iter.backside->toptexture != NO_TEXTURE) ||
         (!ceiling && iter.backside->bottomtexture != NO_TEXTURE)))
      continue;

    // Bleeding stops at another chunk which would produce an
    // equivalent bleed (to prevent rejection by the next test)
    if ((backheight == height &&
         (!iter.frontside ||
          (ceiling && iter.frontside->toptexture == NO_TEXTURE) ||
          (!ceiling && iter.frontside->bottomtexture == NO_TEXTURE))))
      continue;

    // Fail on bleeds that find a chunk with a height not suitable for the
    // bleed type.  This heuristic aborts bleeds that did not appear to be a
    // conscious attempt by the map maker to use flat bleeding effects, to
    // avoid false positives.
    if ((!ceiling && backheight >= height) ||
        (ceiling && backheight <= height))
    {
      result = false;
      break;
    }

    if (!BleedOverTransitive(iter.backchunkmeta, ceiling, height, bl))
    {
      result = false;
      break;
    }
  }

  return result;
}

// FIXME: deduplicate with previous function, although maybe it's already
// confusing enough as-is
static dboolean BleedUnderTransitive(chunkmeta_t* chunkmeta, dboolean ceiling,
                                     fixed_t height, bleedlocal_t* bl,
                                     dboolean fail_on_height_reject)
{
  piter_t iter;
  dboolean result = true;

#if 0
  // Disabled, bleed unders seem to be necessary for non-interior chunks in some cases
  if (fail_on_height_reject && !(chunkmeta->flags & CHUNKF_INTERIOR))
    return false;
#endif

  bl->mark[chunkmeta - dsda_gl_rstate.chunkmeta] = true;
  PushQueue(&bl->queue, chunkmeta);

  for (IterPerimeter(&iter, chunkmeta); IterPerimeterValid(&iter);
       IterPerimeterNext(&iter))
  {
    fixed_t backheight;

    // Skip single-sided lines
    if (!iter.backsector)
      continue;

    // Skip already-visited chunks
    if (bl->mark[iter.backchunkmeta - dsda_gl_rstate.chunkmeta])
      continue;

    // Skip degenerate chunks
    // FIXME: are degenerate chunks a problem with ajbsp?
    if (iter.backchunkmeta->flags & CHUNKF_DEGENERATE)
      continue;

    backheight =
        ceiling ? iter.backsector->ceilingheight : iter.backsector->floorheight;

    // Bleeding stops at textured segs
    if (iter.frontside &&
        ((ceiling && iter.frontside->toptexture != NO_TEXTURE) ||
         (!ceiling && iter.frontside->bottomtexture != NO_TEXTURE)))
      continue;

    // Bleeding stops at another chunk which would produce an
    // equivalent bleed (to prevent rejection by the next test)
    if ((backheight == height &&
         (!iter.backside ||
          (ceiling && iter.backside->toptexture == NO_TEXTURE) ||
          (!ceiling && iter.backside->bottomtexture == NO_TEXTURE))))
      continue;

    // Fail on bleeds that find a chunk with a height not suitable for the
    // bleed type (unless !fail_on_height_reject).  This heuristic aborts bleeds
    // that did not appear to be a conscious attempt by the map maker to use
    // flat bleeding effects, to avoid false positives.
    if ((!ceiling && backheight <= height) ||
        (ceiling && backheight >= height))
    {
      if (!fail_on_height_reject)
        continue;
      result = false;
      break;
    }

    // Follow bleed recursively
    if (!BleedUnderTransitive(iter.backchunkmeta, ceiling, height, bl,
                              fail_on_height_reject))
    {
      result = false;
      break;
    }
  }

  return result;
}

static bleedtype_t BleedType(dboolean ceiling, dboolean over)
{
  if (ceiling && over)
    return BLEED_CEILING_OVER;
  else if (ceiling && !over)
    return BLEED_CEILING_UNDER;
  else if (!ceiling && over)
    return BLEED_FLOOR_OVER;
  else
    return BLEED_FLOOR_UNDER;
}

static void AddBleed(bleeds_t* bleeds, gl_chunk_t* source, gl_chunk_t* target,
                     bleedtype_t type, int depth)
{
  int i;
  unsigned short tchunknum = target - gl_rstate.chunks;

  // Avoid duplicates
  for (i = bleeds->start; i < bleeds->count; ++i)
  {
    tempbleed_t* bleed = &bleeds->bleeds[i];
    if (bleed->source == source && bleed->bleed.target == tchunknum &&
        bleed->bleed.type == type)
    {
      if (depth < bleed->bleed.depth)
        // Register shorter path for same bleed
        bleed->bleed.depth = depth;
      return;
    }
  }

  PushBleed(bleeds, source, target, type, depth);
}

static void BleedProcessQueue(chunkmeta_t* sourcemeta, bleedlocal_t* bl,
                              dboolean ceiling, dboolean over, int depth,
                              dboolean bleed)
{
  unsigned int i;

  for (i = 0; i < bl->queue.count; ++i)
  {
    gl_chunk_t* source;
    gl_chunk_t* target;
    chunkmeta_t* targetmeta = bl->queue.chunks[i];

    bl->mark[targetmeta - dsda_gl_rstate.chunkmeta] = false;

    if (!bleed)
      continue;

    source = GL_Chunk(sourcemeta - dsda_gl_rstate.chunkmeta);
    target = GL_Chunk(targetmeta - dsda_gl_rstate.chunkmeta);
    AddBleed(&bl->bleeds, source, target, BleedType(ceiling, over), depth);
  }

  bl->queue.count = 0;
}

static void BleedChunkPlane(chunkmeta_t* sourcemeta, chunkmeta_t* chunkmeta,
                            dboolean ceiling, int depth, bleedlocal_t* bl)
{
  gl_chunk_t* source = GL_Chunk(sourcemeta - dsda_gl_rstate.chunkmeta);
  int height = ceiling ? source->sector->ceilingheight : source->sector->floorheight;
  piter_t iter;
  const unsigned int dflag = ceiling ? SECF_DYNAMIC_CEILING : SECF_DYNAMIC_FLOOR;

  if (chunkmeta->flags & CHUNKF_DEGENERATE)
    return;

  bl->mark[chunkmeta - dsda_gl_rstate.chunkmeta] = true;

  for (IterPerimeter(&iter, chunkmeta); IterPerimeterValid(&iter);
       IterPerimeterNext(&iter))
  {
    fixed_t backheight;
    dboolean texture_check;
    dboolean height_check;
    dboolean dynamic_check;

    // Skip single-sided lines
    if (!iter.backchunkmeta)
      continue;

    // Skip already-visited chunks
    if (bl->mark[iter.backchunkmeta - dsda_gl_rstate.chunkmeta])
      continue;

    // Skip degenerate chunks
    if (iter.backchunkmeta->flags & CHUNKF_DEGENERATE)
      continue;

    backheight =
        ceiling ? iter.backsector->ceilingheight : iter.backsector->floorheight;

    // Height checks automatically pass if floor/ceiling can move
    dynamic_check = source->sector->flags & dflag || iter.backsector->flags & dflag;

    // Check for bleed over
    //
    //      Upper floor bleeds this way ->
    // -----+~~~~~~~~~~~~
    //      |
    //      |
    //      | <- No texture
    //      |
    //      |
    //      +------------
    //
    // Turn picture upside down for ceiling case

    texture_check =
      // Always go through hacked segs so we include miniseg adjacencies
      // between sectors, which would otherwise not pass this check since
      // they have no sidedefs.  Maps with errant sidedef sectors
      // can produce scenarios that require this.
      iter.frontsegmeta->flags & SEGF_HACKED ||
        (iter.backside &&
         ((ceiling && iter.backside->toptexture == NO_TEXTURE) ||
          (!ceiling && iter.backside->bottomtexture == NO_TEXTURE)));
    height_check = dynamic_check || (!ceiling && backheight < height) ||
                   (ceiling && backheight > height);
    if (texture_check && height_check)
    {
      dboolean bleed =
          BleedOverTransitive(iter.backchunkmeta, ceiling, height, bl);
      BleedProcessQueue(sourcemeta, bl, ceiling, true, depth, bleed);
    }

    // Check for bleed under
    //
    // ------------------+
    //                   |
    //                   |
    //                   | <- No texture
    //                   |
    //                   |
    // ~~~~~~~~~~~~~~~~~~+-----------
    //      <- Lower floor bleeds this way
    //
    // Turn picture upside down for ceiling case
    
    texture_check =
      iter.frontsegmeta->flags & SEGF_HACKED ||
        (iter.frontside &&
         ((ceiling && iter.frontside->toptexture == NO_TEXTURE) ||
          (!ceiling && iter.frontside->bottomtexture == NO_TEXTURE)));
    height_check = dynamic_check || (!ceiling && backheight > height) ||
                   (ceiling && backheight < height);
    if (texture_check && height_check)
    {
      dboolean bleed = BleedUnderTransitive(
          iter.backchunkmeta,
          ceiling,
          height,
          bl,
          // Be permissive crossing minisegs, fail on height mismatches
          // otherwise
          !!iter.frontside);
      BleedProcessQueue(sourcemeta, bl, ceiling, false, depth, bleed);
    }

    // Check for bleed-through to suspected "fake" sectors
    if (depth < BLEED_THROUGH_DEPTH_LIMIT &&
        iter.backchunkmeta->flags & CHUNKF_BLEED_THROUGH &&
        (dynamic_check || height == backheight))
    {
      bleedtype_t type = ceiling ? BLEED_CEILING_THROUGH : BLEED_FLOOR_THROUGH;
      gl_chunk_t* backchunk = GL_Chunk(iter.backchunkmeta - dsda_gl_rstate.chunkmeta);
      AddBleed(&bl->bleeds, source, backchunk, type, depth);
      // Only recurse on non-dynamic bleed-through, as we can get false-positive
      // over/under bleed otherwise
      if (height == backheight)
        BleedChunkPlane(sourcemeta, iter.backchunkmeta, ceiling, depth + 1, bl);
    }
  }
}

static void BleedChunk(chunkmeta_t* chunkmeta, bleedlocal_t* bl)
{
  bl->bleeds.start = bl->bleeds.count;

  BleedChunkPlane(chunkmeta, chunkmeta, false, 0, bl);
  memset(bl->mark, 0, sizeof(*bl->mark) * gl_rstate.numchunks);

  BleedChunkPlane(chunkmeta, chunkmeta, true, 0, bl);
  memset(bl->mark, 0, sizeof(*bl->mark) * gl_rstate.numchunks);
}

// Split chunks into this many per work item to amortize
// synchronization overhead
static const unsigned int work_size = 8;

static void BleedWorkCallback(void* item, void* local, void* context)
{
  chunkmeta_t* chunkmeta = (chunkmeta_t*) item;
  bleedlocal_t* bl = (bleedlocal_t*) local;
  unsigned offset = chunkmeta - dsda_gl_rstate.chunkmeta;
  unsigned int i;

  // Compute bleeds for all chunks in this work item
  for (i = 0; i < work_size && i + offset < gl_rstate.numchunks; ++i)
    BleedChunk(&chunkmeta[i], bl);
}

static void BleedDestroyCallback(void* local, void* context)
{
  bleedlocal_t* bl = (bleedlocal_t*) local;
  unsigned int i;

  // Commit temporary bleeds to global array
  for (i = 0; i < bl->bleeds.count; ++i)
  {
    tempbleed_t* bleed = &bl->bleeds.bleeds[i];

    if (bleed->source->firstbleed == -1)
      bleed->source->firstbleed = gl_rstate.numbleeds;

    PushGlobalBleed(&bleed->bleed);
    bleed->source->numbleeds++;
  }

  // Zone memory functions are not thread-safe
  if (bl->bleeds.bleeds)
    free(bl->bleeds.bleeds);

  if (bl->queue.chunks)
    free(bl->queue.chunks);
}

void AnnotateBleeds(void)
{
  dsda_wp_t wp;
  dsda_wp_params_t wpp;
  int i;

  dsda_WorkPoolInitParams(&wpp);
  wpp.queuesize = gl_rstate.numchunks;
  wpp.work = BleedWorkCallback;
  wpp.destroy = BleedDestroyCallback;
  // Local state includes flexible array of flags for all chunks
  wpp.localsize =
      sizeof(bleedlocal_t) + sizeof(byte) * gl_rstate.numchunks;

  dsda_WorkPoolInit(&wp, &wpp);

  // Enqueue all chunks as work chunks
  for (i = 0; i < gl_rstate.numchunks; i += work_size)
    dsda_WorkPoolEnqueue(&wp, dsda_gl_rstate.chunkmeta + i);

  // Implicitly flushes
  dsda_WorkPoolDestroy(&wp);
}

void ClearBleeds(void)
{
  gl_rstate.numbleeds = 0;
  capbleeds = 0;
  if (gl_rstate.bleeds)
    Z_Free(gl_rstate.bleeds);
  gl_rstate.bleeds = NULL;
}
