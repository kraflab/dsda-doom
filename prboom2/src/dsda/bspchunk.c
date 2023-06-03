
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

#include <assert.h>

#include "bspinternal.h"

static const unsigned long long nan_pattern[2] = {0x7ff8000000000000ULL,
                                                  0x7ff8000000000000ULL};

static unsigned int capchunks = 0;
static unsigned int capperims = 0;
static unsigned int cappaths = 0;

static side_t* OppositeSide(line_t* l, side_t* s)
{
  unsigned short num;

  if (l->sidenum[0] == s - sides)
    num = l->sidenum[1];
  else
    num = l->sidenum[0];

  return num == NO_INDEX ? NULL : &sides[num];
}

void IterPerimeterNext(piter_t* iter)
{
  iter->i++;
  if (IterPerimeterValid(iter))
  {
    chunkmeta_t* chunkmeta = iter->chunkmeta;

    iter->frontseg = dsda_gl_rstate.perims[chunkmeta->firstperim + iter->i];
    iter->frontsegmeta = &dsda_gl_rstate.segmeta[iter->frontseg - gl_rstate.segs];
    iter->frontsub = iter->frontsegmeta->subsector;

    assert(iter->frontsub->chunk == chunkmeta - dsda_gl_rstate.chunkmeta);

    if (!iter->frontsegmeta->partner)
    {
      iter->backseg = NULL;
      iter->backsegmeta = NULL;
      iter->backsub = NULL;
      iter->backchunkmeta = NULL;
      iter->backsector = NULL;
    }
    else
    {
      iter->backseg = iter->frontsegmeta->partner;
      iter->backsegmeta =
          &dsda_gl_rstate.segmeta[iter->frontsegmeta->partner - gl_rstate.segs];
      iter->backsub = iter->backsegmeta->subsector;
      iter->backchunkmeta = &dsda_gl_rstate.chunkmeta[iter->backsub->chunk];
      iter->backsector = iter->backsub->sector;
    }

    // Resolve sides
    if (iter->frontseg && iter->frontseg->linedef)
    {
      iter->frontside = iter->frontseg->sidedef;
      iter->backside = OppositeSide(iter->frontseg->linedef, iter->frontside);
    }
    else if (iter->backseg && iter->backseg->linedef)
    {
      // FIXME: can ajbsp generate miniseg/normal seg partners?
      iter->frontside = OppositeSide(iter->frontseg->linedef, iter->frontside);
      iter->backside = iter->frontseg->sidedef;
    }
    else
    {
      iter->frontside = NULL;
      iter->backside = NULL;
    }
  }
}

static void PushChunk(sector_t* sector, gl_chunk_t** chunkout, chunkmeta_t** metaout)
{
  gl_chunk_t* chunk;
  chunkmeta_t* chunkmeta;

  if (gl_rstate.numchunks == capchunks)
  {
    if (capchunks == 0)
      capchunks = 256;
    else
      capchunks *= 2;
    gl_rstate.chunks = Z_Realloc(
        gl_rstate.chunks, sizeof(*gl_rstate.chunks) * capchunks);
    dsda_gl_rstate.chunkmeta = Z_Realloc(
        dsda_gl_rstate.chunkmeta, sizeof(*dsda_gl_rstate.chunkmeta) * capchunks);
  }

  chunk = &gl_rstate.chunks[gl_rstate.numchunks];
  memset(chunk, 0, sizeof(*chunk));
  chunk->sector = sector;
  chunk->firstbleed = -1;
  chunk->floorthrough.depth = UINT_MAX;
  chunk->ceilingthrough.depth = UINT_MAX;
  chunk->floorunder.depth = UINT_MAX;
  chunk->ceilingunder.depth = UINT_MAX;
  chunk->floorover.depth = UINT_MAX;
  chunk->ceilingover.depth = UINT_MAX;

  chunkmeta = &dsda_gl_rstate.chunkmeta[gl_rstate.numchunks++];
  memset(chunkmeta, 0, sizeof(*chunkmeta));
  chunkmeta->firstpoint = -1;
  chunkmeta->subsectors = -1;

  *chunkout = chunk;
  *metaout = chunkmeta;
}

static void DetermineChunks(void)
{
  int i, j;

  for (i = 0; i < gl_rstate.numsubsectors; ++i)
  {
    subsector_t* sub = &gl_rstate.subsectors[i];
    submeta_t* submeta = &dsda_gl_rstate.submeta[i];
    submeta_t* queue = NULL;
    gl_chunk_t* chunk;
    chunkmeta_t* chunkmeta;

    if (sub->chunk != NO_CHUNK)
      continue;

    // Begin a new chunk starting with this subsector
    PushChunk(sub->sector, &chunk, &chunkmeta);
    chunkmeta->subsectors = sub - gl_rstate.subsectors;
    sub->chunk = chunk - gl_rstate.chunks;

    // Hacked subsectors are subject to bleed-through
    if (submeta->flags & SUBF_HACKED)
      chunkmeta->flags |= CHUNKF_BLEED_THROUGH;

    // Degenerate subsectors get their own chunk so they
    // don't muck up perimeters
    // FIXME: is this necessary with ajbsp?
    if (submeta->flags & SUBF_DEGENERATE)
    {
      chunkmeta->flags |= CHUNKF_DEGENERATE;
      continue;
    }

    // Recursively mark reachable subsectors with the chunk
    queue = submeta;

    while (queue)
    {
      submeta = queue;
      sub = &gl_rstate.subsectors[submeta - dsda_gl_rstate.submeta];
      queue = queue->q_next;
      submeta->q_next = NULL;

      for (j = 0; j < sub->numlines; ++j)
      {
        segmeta_t* segmeta = &dsda_gl_rstate.segmeta[sub->firstline + j];
        seg_t* partner = segmeta->partner;
        segmeta_t* psegmeta;
        subsector_t* adjss;
        submeta_t* adjsm;

        if (!partner)
          continue;

        psegmeta = &dsda_gl_rstate.segmeta[partner - gl_rstate.segs];

        adjss = psegmeta->subsector;
        adjsm = &dsda_gl_rstate.submeta[adjss - gl_rstate.subsectors];

        if (adjss->chunk != NO_CHUNK || adjss->sector != chunk->sector ||
            adjsm->flags & SUBF_DEGENERATE)
          continue;

        adjss->chunk = chunk - gl_rstate.chunks;
        adjsm->chunk_next = chunkmeta->subsectors;
        chunkmeta->subsectors = adjsm - dsda_gl_rstate.submeta;

        if (adjsm->flags & SUBF_HACKED)
          chunkmeta->flags |= CHUNKF_BLEED_THROUGH;

        adjsm->q_next = queue;
        queue = adjsm;
      }
    }
  }

  // This doesn't seem to be necessary with ajbsp so far because its
  // assignment of subsectors to sectors (via the first seg) is better
#if 0
  // Break up chunks subject to bleed-through because each subsector
  // might need to render with different flats due to bleed effects
  for (i = 0; i < numchunks; ++i)
  {
    gl_chunk_t* chunk = &chunks[i];
    subsector_t* sub;
    subsector_t* next;

    if (!(chunk->flags & CHUNKF_BLEED_THROUGH) || !chunk->subsectors->chunk_next)
      continue;

    sub = chunk->subsectors->chunk_next;
    chunk->subsectors->chunk_next = NULL;
    for (; sub; sub = next)
    {
      next = sub->chunk_next;
      sub->chunk_next = NULL;
      chunk = PushChunk(sub->sector);
      sub->chunk = chunk - chunks;
      chunk->subsectors = sub;
      chunk->flags |= CHUNKF_BLEED_THROUGH;
    }
  }
#endif

  // We know the number of chunks, these can now be allocated
  gl_rstate.deferred =
      Z_Calloc(sizeof(*gl_rstate.deferred), gl_rstate.numchunks);
  gl_rstate.visible =
      Z_Calloc(sizeof(*gl_rstate.visible), gl_rstate.numchunks);
}

static void PushPerim(seg_t* seg)
{
  if (dsda_gl_rstate.numperims == capperims)
  {
    if (capperims == 0)
      capperims = 256;
    else
      capperims *= 2;
    dsda_gl_rstate.perims = Z_Realloc(
        dsda_gl_rstate.perims, sizeof(*dsda_gl_rstate.perims) * capperims);
  }
  dsda_gl_rstate.perims[dsda_gl_rstate.numperims++] = seg;
}

static void AddChunkPerimeter(chunkmeta_t* chunkmeta, seg_t* seg)
{
  chunkmeta->numperim++;
  PushPerim(seg);
}

static void AnnotateChunkPerimeter(chunkmeta_t* chunkmeta)
{
  subsector_t* sub;
  submeta_t* submeta;
  int subnum;
  int i;
  dboolean interior = true;
  gl_chunk_t* chunk = &gl_rstate.chunks[chunkmeta - dsda_gl_rstate.chunkmeta];

  chunkmeta->firstperim = dsda_gl_rstate.numperims;

  for (subnum = chunkmeta->subsectors; subnum != -1;
       subnum = submeta->chunk_next)
  {
    sub = &gl_rstate.subsectors[subnum];
    submeta = &dsda_gl_rstate.submeta[subnum];

    for (i = 0; i < sub->numlines; ++i)
    {
      seg_t* seg = &gl_rstate.segs[sub->firstline + i];
      segmeta_t* segmeta = &dsda_gl_rstate.segmeta[sub->firstline + i];
      segmeta_t* psegmeta =
          segmeta->partner
              ? &dsda_gl_rstate.segmeta[segmeta->partner - gl_rstate.segs]
              : NULL;

      assert(segmeta->subsector->chunk ==
             chunkmeta - dsda_gl_rstate.chunkmeta);

      if (!psegmeta ||
          psegmeta->subsector->chunk != chunkmeta - dsda_gl_rstate.chunkmeta)
        AddChunkPerimeter(chunkmeta, seg);

      if (!psegmeta)
        interior = false;

      // Render flats if this is not a purely self-referencing chunk
      // (i.e. a fake sector)
      if (seg->linedef && seg->linedef->frontsector != seg->linedef->backsector)
        chunk->flags |= GL_CHUNKF_RENDER_FLATS;
    }
  }

  // Note purely interior chunks, which can speed up some bleed calculations
  if (interior)
    chunkmeta->flags |= CHUNKF_INTERIOR;
}

static void AnnotateChunkPerimeters(void)
{
  int i = 0;

  for (i = 0; i < gl_rstate.numchunks; ++i)
    AnnotateChunkPerimeter(&dsda_gl_rstate.chunkmeta[i]);
}

static void PushPath(chunkmeta_t* chunkmeta, const void* point)
{
  if (dsda_gl_rstate.numpaths == cappaths)
  {
    if (cappaths == 0)
      cappaths = 256;
    else
      cappaths *= 2;
    dsda_gl_rstate.paths = Z_Realloc(dsda_gl_rstate.paths,
                                     sizeof(*dsda_gl_rstate.paths) * cappaths);
  }

  if (chunkmeta->firstpoint == -1)
    chunkmeta->firstpoint = dsda_gl_rstate.numpaths;
  chunkmeta->numpoints++;

  // Avoid putting a NAN in a double local or rvalue at any point, as it's not
  // safe with -ffast-math
  memcpy(&dsda_gl_rstate.paths[dsda_gl_rstate.numpaths++], point,
         sizeof(*dsda_gl_rstate.paths));
}

static dboolean FindChunkPath(chunkmeta_t* chunkmeta)
{
  piter_t iter;
  segflags_t* goalflags;
  int goal = -1;
  dboolean restart = false;
  line_t* lastline = NULL;

  for (;;)
  {
    if (goal != -1 && dsda_gl_rstate.numpaths - 1 != goal &&
        dgeom_PointsEqual(&dsda_gl_rstate.paths[dsda_gl_rstate.numpaths - 1],
                          &dsda_gl_rstate.paths[goal], DGEOM_EPSILON))
    {
      // Mark end-of-contour
      PushPath(chunkmeta, nan_pattern);
      goal = -1;
      lastline = NULL;
    }

    for (IterPerimeter(&iter, chunkmeta);
        IterPerimeterValid(&iter);
        IterPerimeterNext(&iter))
    {
      dline_t l;
      segflags_t* flags;

      if (iter.frontsegmeta->flags & SEGF_MARK)
        continue;

      l = dgeom_DLineFromSeg(iter.frontseg);
      flags = &iter.frontsegmeta->flags;

      if (goal == -1 ||
          dgeom_PointsEqual(&dsda_gl_rstate.paths[dsda_gl_rstate.numpaths - 1],
                            &l.start, DGEOM_EPSILON))
      {
        if (goal == -1)
        {
          if (iter.frontseg->linedef && iter.frontseg->v1 !=
              (iter.frontseg->frontsector == iter.frontseg->linedef->frontsector
                   ? iter.frontseg->linedef->v1
                   : iter.frontseg->linedef->v2))
            // Don't start in the middle of a linedef
            continue;
          // Start a new contour here
          goal = dsda_gl_rstate.numpaths;
          goalflags = flags;
          PushPath(chunkmeta, &l.start);
        }

        if (lastline && iter.frontseg->linedef == lastline)
          // Coalesce segments on same line
          dsda_gl_rstate.paths[dsda_gl_rstate.numpaths - 1] = l.end;
        else
          PushPath(chunkmeta, &l.end);

        lastline = iter.frontseg->linedef;
        *flags |= SEGF_MARK;
        restart = true;
        break;
      }
    }

    if (restart)
    {
      restart = false;
      continue;
    }

    if (goal != -1)
    {
      // Failed to use seg, mark it for debugging purposes
      *goalflags |= SEGF_ORPHAN;
      // Always fail for now
      return false;
#if 0
      // Remove all added points
      chunkmeta->numpoints -= dsda_gl_rstate.numpaths - goal;
      dsda_gl_rstate.numpaths = goal;
      // Clear goal and keep going
      goal = -1;
      lastline = NULL;
      lastlastline = NULL;
      continue;
#endif
    }

    // If we get to this point, we didn't find any more segs
    // to start a contour, so we're done
    break;
  }

  return chunkmeta->numpoints != 0;
}

// Try to find path for chunk
static void AnnotateChunkPath(chunkmeta_t* chunkmeta)
{
  piter_t iter;

  if (!FindChunkPath(chunkmeta))
  {
    // Throw away any contours we found so far on failure
    dsda_gl_rstate.numpaths -= chunkmeta->numpoints;
    chunkmeta->numpoints = 0;
  }

  // Clear marks
  for (IterPerimeter(&iter, chunkmeta);
      IterPerimeterValid(&iter);
      IterPerimeterNext(&iter))
    iter.frontsegmeta->flags &= ~SEGF_MARK;
}

static void AnnotateChunkPaths(void)
{
  int i;

  for (i = 0; i < gl_rstate.numchunks; ++i)
  {
    chunkmeta_t* chunkmeta = &dsda_gl_rstate.chunkmeta[i];

    // Degenerate chunks will obviously not yield a useful path.
    if (chunkmeta->flags & CHUNKF_DEGENERATE)
      continue;

    AnnotateChunkPath(chunkmeta);
  }
}

void AnnotateChunks(void)
{
  DetermineChunks();
  AnnotateChunkPerimeters();
  AnnotateChunkPaths();
}

void ClearChunks(void)
{
  dsda_gl_rstate.numperims = 0;
  capperims = 0;
  if (dsda_gl_rstate.perims)
    Z_Free(dsda_gl_rstate.perims);
  dsda_gl_rstate.perims = NULL;

  gl_rstate.numchunks = 0;
  capchunks = 0;
  if (gl_rstate.chunks)
    Z_Free(gl_rstate.chunks);
  gl_rstate.chunks = NULL;
  if (gl_rstate.deferred)
    Z_Free(gl_rstate.deferred);
  gl_rstate.deferred = NULL;
  if (gl_rstate.visible)
    Z_Free(gl_rstate.visible);
  gl_rstate.visible = NULL;
  if (dsda_gl_rstate.chunkmeta)
    Z_Free(dsda_gl_rstate.chunkmeta);
  dsda_gl_rstate.chunkmeta = NULL;

  dsda_gl_rstate.numpaths = 0;
  cappaths = 0;
  if (dsda_gl_rstate.paths)
    Z_Free(dsda_gl_rstate.paths);
  dsda_gl_rstate.paths = NULL;
}

void ResetChunks(void)
{
  int i;

  for (i = 0; i < gl_rstate.numsubsectors; ++i)
  {
    subsector_t* sub = &gl_rstate.subsectors[i];
    sub->poly = NULL;
  }

  memset(gl_rstate.map_chunks, 0,
         sizeof(*gl_rstate.map_chunks) * gl_rstate.numchunks);
}

dboolean dsda_PointIsEndOfContour(const dpoint_t* p)
{
  // Avoid putting nan into a double local or rvalue at any point, as it's not
  // safe with -ffast-math
  return !memcmp(p, nan_pattern, sizeof(nan_pattern));
}
