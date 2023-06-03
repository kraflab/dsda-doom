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

#ifndef __DSDA_WORKPOOL__
#define __DSDA_WORKPOOL__

#include <SDL_thread.h>
#include <SDL_mutex.h>

typedef struct
{
  // Number of threads, 0 = number of logical cores
  unsigned int numthreads;
  // Maximum queue size
  unsigned int queuesize;
  // Size of local state
  size_t localsize;
  // User context pointer
  void* context;
  // Work thread-local state initializer callback.
  // Always called sequentially from thread running dsda_WorkPoolInit
  void (*init)(void* local, void* context);
  // Work thread-local state destructor callback.
  // Always called sequentially from thread running dsda_WorkPoolDestroy
  void (*destroy)(void* local, void* context);
  // Work item callback.  Called from work thread.
  void (*work)(void* item, void* local, void* context);
} dsda_wp_params_t;

// Work pool
// Contents are an implementation detail
typedef struct
{
  dsda_wp_params_t params;
  SDL_Thread** threads;
  // Thread-local state
  void* locals;
  // Work queue ring and end
  void** queue;
  void** end;
  // Synchronization
  SDL_mutex* mutex;
  SDL_cond* consumer_cond;
  SDL_cond* producer_cond;
  // Head (pop point) and tail (push point) of queue
  void** head;
  void** tail;
  // Number of popped but pending work items
  size_t pending;
  // Number of items in queue
  unsigned int count;
} dsda_wp_t;

// Initialize work pool parameters to defaults
static inline void dsda_WorkPoolInitParams(dsda_wp_params_t* params)
{
  params->numthreads = 0;
  params->queuesize = 128;
  params->localsize = 0;
  params->init = NULL;
  params->destroy = NULL;
  params->work = NULL;
  params->context = NULL;
}

// Initialize work pool
void dsda_WorkPoolInit(dsda_wp_t* wp, const dsda_wp_params_t* params);
// Destroy work pool.  Implicitly flushes it.
void dsda_WorkPoolDestroy(dsda_wp_t* wp);
// Enqueue work item.  Blocks if queue is full.
void dsda_WorkPoolEnqueue(dsda_wp_t* wp, void* item);
// Wait for all outstanding work items to complete
void dsda_WorkPoolFlush(dsda_wp_t* wp);

#endif
