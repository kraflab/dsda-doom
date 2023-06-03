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
//	Work Pool API
//

#include <stddef.h>
#include <stdlib.h>
#include <SDL.h>

// FIXME: is I_Error thread-safe?  We're about to abort anyway...
#include "lprintf.h"

#include "workpool.h"

// Size of cache line, valid for most contemporary architectures
#define WP_CACHELINE 64

// Ersatz maxalign_t from C11
typedef union
{
  intmax_t im;
  long double d;
  void* p;
  void (*f)(void);
} wpmaxalign_t;

// Per-thread state
typedef struct
{
  dsda_wp_t* wp;
  // User state
  wpmaxalign_t user[];
} wplocal_t;

// Pop work item with mutex held
static void* Pop(dsda_wp_t* wp)
{
  void* item;

  while (wp->count == 0)
    SDL_CondWait(wp->consumer_cond, wp->mutex);

  item = *(wp->head++);
  if (item == NULL)
    abort();
  if (wp->head == wp->end)
    wp->head = wp->queue;

  ++wp->pending;

  if (wp->count-- == wp->params.queuesize)
    SDL_CondSignal(wp->producer_cond);

  return item;
}

// Push work item with mutex held
static void Push(dsda_wp_t* wp, void* item)
{
  while (wp->count == wp->params.queuesize)
    SDL_CondWait(wp->producer_cond, wp->mutex);

  *(wp->tail++) = item;
  if (wp->tail == wp->end)
    wp->tail = wp->queue;
 
  wp->count++;
  SDL_CondSignal(wp->consumer_cond);
}

// Signal work item is done with mutex held
static void Done(dsda_wp_t* wp)
{
  if (--wp->pending == 0)
    // FIXME: use a separate cond to avoid spurious wakeups
    SDL_CondBroadcast(wp->producer_cond);
}

static int SDLCALL WorkThread(void* data)
{
  wplocal_t* l = data;
  dsda_wp_t* wp = l->wp;

  SDL_LockMutex(wp->mutex);
  for (;;)
  {
    void* item = Pop(wp);

    if (item == wp)
      // Shutdown signal
      break;

    SDL_UnlockMutex(wp->mutex);
    wp->params.work(item, l->user, wp->params.context);
    SDL_LockMutex(wp->mutex);
    Done(wp);
  }
  // Ack shutdown item
  Done(wp);
  SDL_UnlockMutex(wp->mutex);

  return 0;
}

void dsda_WorkPoolInit(dsda_wp_t* wp, const dsda_wp_params_t* params)
{
  unsigned int i;
  // Stupid C trick to get alignment without C11 alignof
  static const size_t align = offsetof(struct { char a; wplocal_t l; }, l);

  if (!params->work)
    I_Error("dsda_WorkPoolInit called without work callback");

  wp->params = *params;

  if (wp->params.numthreads == 0)
    wp->params.numthreads = SDL_GetCPUCount();

  // Compute total size of thread-local state
  wp->params.localsize += sizeof(wplocal_t);
  // Make sure thread-local states do not share cache lines to prevent cache
  // line bouncing
  if (wp->params.localsize < WP_CACHELINE * 2)
    wp->params.localsize = WP_CACHELINE * 2;
  // Make sure size is suitably aligned for wplocal_t
  if (wp->params.localsize % align)
    wp->params.localsize += wp->params.localsize % align;
  
  // Zone memory functions are not thread-safe
  wp->threads = calloc(wp->params.numthreads, sizeof(*wp->threads));
  if (!wp->threads)
    I_Error("Could not allocate threads array");
  wp->queue = calloc(wp->params.queuesize, sizeof(*wp->queue));
  if (!wp->queue)
    I_Error("Could not allocate work queue");
  wp->locals = calloc(wp->params.numthreads, wp->params.localsize);
  if (!wp->locals)
    I_Error("Could not allocate work thread locals");
  wp->head = wp->queue;
  wp->tail = wp->queue;
  wp->end = wp->queue + wp->params.queuesize;
  wp->count = 0;
  wp->pending = 0;

  wp->mutex = SDL_CreateMutex();
  if (!wp->mutex)
    I_Error("Could not create mutex");
  wp->consumer_cond = SDL_CreateCond();
  if (!wp->consumer_cond)
    I_Error("Could not create condition variable");
  wp->producer_cond = SDL_CreateCond();
  if (!wp->producer_cond)
    I_Error("Could not create condition variable");

  for (i = 0; i < wp->params.numthreads; ++i)
  {
    // Cast through void asserts that result is suitably aligned
    wplocal_t* local =
        (wplocal_t*)(void*)((uint8_t*)wp->locals + wp->params.localsize * i);
    SDL_Thread** thread = &wp->threads[i];

    local->wp = wp;
    if (wp->params.init)
      wp->params.init(local->user, wp->params.context);
    *thread = SDL_CreateThread(WorkThread, "worker", local);
    if (!*thread)
      I_Error("Could not create thread");
  }
}

void dsda_WorkPoolDestroy(dsda_wp_t* wp)
{
  unsigned int i;

  // Enqueue shutdown messages
  for (i = 0; i < wp->params.numthreads; ++i)
    dsda_WorkPoolEnqueue(wp, wp);

  // Wait for outstanding work
  dsda_WorkPoolFlush(wp);

  for (i = 0; i < wp->params.numthreads; ++i)
  {
    wplocal_t* local =
        (wplocal_t*)(void*)((uint8_t*)wp->locals + wp->params.localsize * i);

    SDL_WaitThread(wp->threads[i], NULL);

    if (wp->params.destroy)
      wp->params.destroy(local->user, wp->params.context);
  }

  // Zone memory functions are not thread-safe
  free(wp->threads);
  free(wp->queue);
  free(wp->locals);
  SDL_DestroyCond(wp->consumer_cond);
  SDL_DestroyCond(wp->producer_cond);
  SDL_DestroyMutex(wp->mutex);
}

void dsda_WorkPoolEnqueue(dsda_wp_t* wp, void* item)
{
  SDL_LockMutex(wp->mutex);
  Push(wp, item);
  SDL_UnlockMutex(wp->mutex);
}

void dsda_WorkPoolFlush(dsda_wp_t* wp)
{
  SDL_LockMutex(wp->mutex);
  while (wp->count || wp->pending)
    SDL_CondWait(wp->producer_cond, wp->mutex);
  SDL_UnlockMutex(wp->mutex);
}
