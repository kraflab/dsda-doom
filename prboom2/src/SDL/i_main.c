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
 *      Startup and quit functions. Handles signals, inits the
 *      memory management, then calls D_DoomMain. Also contains
 *      I_Init which does other system-related startup stuff.
 *
 *-----------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <errno.h>

#include "doomdef.h"
#include "m_argv.h"
#include "d_main.h"
#include "m_fixed.h"
#include "i_system.h"
#include "i_video.h"
#include "z_zone.h"
#include "lprintf.h"
#include "m_random.h"
#include "doomstat.h"
#include "g_game.h"
#include "m_misc.h"
#include "i_sound.h"
#include "i_main.h"
#include "r_fps.h"
#include "lprintf.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "e6y.h"
#include "dsda.h"
#include "dsda/analysis.h"
#include "dsda/settings.h"
#include "dsda/split_tracker.h"
#include "dsda/text_file.h"
#include "dsda/time.h"

/* Most of the following has been rewritten by Lee Killough
 *
 * killough 4/13/98: Make clock rate adjustable by scale factor
 * cphipps - much made static
 */

int realtic_clock_rate = 100;

void I_Init(void)
{
  dsda_ResetTimeFunctions(fastdemo);

  /* killough 2/21/98: avoid sound initialization if no sound & no music */
  if (!(nomusicparm && nosfxparm))
    I_InitSound();
}

//e6y
void I_Init2(void)
{
  dsda_ResetTimeFunctions(fastdemo);

  force_singletics_to = gametic + BACKUPTICS;
}

/* cleanup handling -- killough:
 */
static void I_SignalHandler(int s)
{
  char buf[2048];

  signal(s,SIG_IGN);  /* Ignore future instances of this signal.*/

  I_ExeptionProcess(); //e6y

  strcpy(buf,"Exiting on signal: ");
  I_SigString(buf+strlen(buf),2000-strlen(buf),s);

  I_Error("I_SignalHandler: %s", buf);
}

//
// e6y: exeptions handling
//

static ExeptionsList_t current_exception_index;

ExeptionParam_t ExeptionsParams[EXEPTION_MAX + 1] =
{
  {NULL},
  {"gld_CreateScreenSizeFBO: Access violation in glFramebufferTexture2DEXT.\n\n"
    "Are you using ATI graphics? Try to update your drivers\n"},
  {NULL}
};

void I_ExeptionBegin(ExeptionsList_t exception_index)
{
  if (current_exception_index == EXEPTION_NONE)
  {
    current_exception_index = exception_index;
  }
  else
  {
    I_Error("I_SignalStateSet: signal_state set!");
  }
}

void I_ExeptionEnd(void)
{
  current_exception_index = EXEPTION_NONE;
}

void I_ExeptionProcess(void)
{
  if (current_exception_index > EXEPTION_NONE && current_exception_index < EXEPTION_MAX)
  {
    I_Error("%s", ExeptionsParams[current_exception_index].error_message);
  }
}

static void PrintVer(void)
{
  char vbuf[200];
  lprintf(LO_INFO,"%s\n",I_GetVersionString(vbuf,200));
}

// Schedule a function to be called when the program exits.
// If run_if_error is true, the function is called if the exit
// is due to an error (I_Error)
// Copyright(C) 2005-2014 Simon Howard

typedef struct atexit_listentry_s atexit_listentry_t;

struct atexit_listentry_s
{
    atexit_func_t func;
    dboolean run_on_error;
    atexit_listentry_t *next;
    const char* name;
};

static atexit_listentry_t *exit_funcs[exit_priority_max];
static int exit_priority;

void I_AtExit(atexit_func_t func, dboolean run_on_error,
              const char* name, exit_priority_t priority)
{
    atexit_listentry_t *entry;

    entry = malloc(sizeof(*entry));

    entry->func = func;
    entry->run_on_error = run_on_error;
    entry->next = exit_funcs[priority];
    entry->name = name;
    exit_funcs[priority] = entry;
}

void I_SafeExit(int rc)
{
  atexit_listentry_t *entry;

  // Run through all exit functions
  for (; exit_priority < exit_priority_max; ++exit_priority)
  {
    while ((entry = exit_funcs[exit_priority]))
    {
      exit_funcs[exit_priority] = exit_funcs[exit_priority]->next;

      if (rc == 0 || entry->run_on_error)
      {
        lprintf(LO_INFO, "Exit Sequence[%d]: %s (%d)\n", exit_priority, entry->name, rc);
        entry->func();
      }
    }
  }

  exit(rc);
}

static void I_EssentialQuit (void)
{
  if (demorecording)
  {
    G_CheckDemoStatus();
  }
  dsda_ExportTextFile();
  dsda_WriteAnalysis();
  dsda_WriteSplits();
}

static void I_Quit (void)
{
  M_SaveDefaults ();

  // This function frees all WAD data as a side effect (!!!)
  // You MUST NOT call this function before any code that touches lump data (e.g., music shutdown)
  I_DemoExShutdown();
}

#ifdef SECURE_UID
uid_t stored_euid = -1;
#endif

//
// Sets the priority class for the prboom-plus process
//

void I_SetProcessPriority(void)
{
  if (process_priority)
  {
    const char *errbuf = NULL;

#ifdef _WIN32
    {
      DWORD dwPriorityClass = NORMAL_PRIORITY_CLASS;

      if (process_priority == 1)
        dwPriorityClass = HIGH_PRIORITY_CLASS;
      else if (process_priority == 2)
        dwPriorityClass = REALTIME_PRIORITY_CLASS;

      if (SetPriorityClass(GetCurrentProcess(), dwPriorityClass) == 0)
      {
        errbuf = WINError();
      }
    }
#else
    return;
#endif

    if (errbuf == NULL)
    {
      lprintf(LO_INFO, "I_SetProcessPriority: priority for the process is %d\n", process_priority);
    }
    else
    {
      lprintf(LO_ERROR, "I_SetProcessPriority: failed to set priority for the process (%s)\n", errbuf);
    }
  }
}

//int main(int argc, const char * const * argv)
int main(int argc, char **argv)
{
#ifdef SECURE_UID
  /* First thing, revoke setuid status (if any) */
  stored_euid = geteuid();
  if (getuid() != stored_euid)
    if (seteuid(getuid()) < 0)
      fprintf(stderr, "Failed to revoke setuid\n");
    else
      fprintf(stderr, "Revoked uid %d\n",stored_euid);
#endif

  myargc = argc;
  myargv = (char**)malloc(sizeof(myargv[0]) * myargc);
  memcpy(myargv, argv, sizeof(myargv[0]) * myargc);

  // Print the version and exit
  if(M_CheckParm("-v"))
  {
    PrintVer();
    return 0;
  }

  // e6y: Check for conflicts.
  // Conflicting command-line parameters could cause the engine to be confused
  // in some cases. Added checks to prevent this.
  // Example: glboom.exe -record mydemo -playdemo demoname
  ParamsMatchingCheck();

  // e6y: was moved from D_DoomMainSetup
  // init subsystems
  //jff 9/3/98 use logical output routine
  lprintf(LO_INFO,"M_LoadDefaults: Load system defaults.\n");
  M_LoadDefaults();              // load before initing other systems

  /* Version info */
  lprintf(LO_INFO,"\n");
  PrintVer();

  /*
     killough 1/98:

     This fixes some problems with exit handling
     during abnormal situations.

     The old code called I_Quit() to end program,
     while now I_Quit() is installed as an exit
     handler and exit() is called to exit, either
     normally or abnormally. Seg faults are caught
     and the error handler is used, to prevent
     being left in graphics mode or having very
     loud SFX noise because the sound card is
     left in an unstable state.
  */

  I_AtExit(I_EssentialQuit, true, "I_EssentialQuit", exit_priority_first);
  I_AtExit(I_Quit, false, "I_Quit", exit_priority_last);
#ifndef PRBOOM_DEBUG
  if (!M_CheckParm("-devparm"))
  {
    signal(SIGSEGV, I_SignalHandler);
  }
  signal(SIGTERM, I_SignalHandler);
  signal(SIGFPE,  I_SignalHandler);
  signal(SIGILL,  I_SignalHandler);
  signal(SIGINT,  I_SignalHandler);  /* killough 3/6/98: allow CTRL-BRK during init */
  signal(SIGABRT, I_SignalHandler);
#endif

  // Priority class for the prboom-plus process
  I_SetProcessPriority();

  /* cphipps - call to video specific startup code */
  I_PreInitGraphics();

  D_DoomMain ();
  return 0;
}
