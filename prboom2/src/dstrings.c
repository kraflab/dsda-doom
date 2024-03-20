/* Emacs style mode select   -*- C -*-
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
 *   Globally defined strings.
 *
 *-----------------------------------------------------------------------------
 */

#include "dstrings.h"
#include "d_deh.h"

static const char* debug_quit_msg = "THIS IS NO MESSAGE!\nPage intentionally left blank.";

// killough 1/18/98: remove hardcoded limit, add const:
const char** endmsg[]=
{
  // DOOM1
  &s_QUITMSG,
  &s_QUITMSG1,  // "please don't leave, there's more\ndemons to toast!",
  &s_QUITMSG2,  // "let's beat it -- this is turning\ninto a bloodbath!",
  &s_QUITMSG3,  // "i wouldn't leave if i were you.\ndos is much worse.",
  &s_QUITMSG4,  // "you're trying to say you like dos\nbetter than me, right?",
  &s_QUITMSG5,  // "don't leave yet -- there's a\ndemon around that corner!",
  &s_QUITMSG6,  // "ya know, next time you come in here\ni'm gonna toast ya.",
  &s_QUITMSG7,  // "go ahead and leave. see if i care.",  // 1/15/98 killough

  // QuitDOOM II messages
  &s_QUITMSG8,  // "you want to quit?\nthen, thou hast lost an eighth!",
  &s_QUITMSG9,  // "don't go now, there's a \ndimensional shambler waiting\nat the dos prompt!",
  &s_QUITMSG10, // "get outta here and go back\nto your boring programs.",
  &s_QUITMSG11, // "if i were your boss, i'd \n deathmatch ya in a minute!",
  &s_QUITMSG12, // "look, bud. you leave now\nand you forfeit your body count!",
  &s_QUITMSG13, // "just leave. when you come\nback, i'll be waiting with a bat.",
  &s_QUITMSG14, // "you're lucky i don't smack\nyou for thinking about leaving.",  // 1/15/98 killough

  // FinalDOOM?

  // Internal debug. Different style, too.
  &debug_quit_msg,  // 1/15/98 killough
};

// killough 1/18/98: remove hardcoded limit and replace with var (silly hack):
const size_t NUM_QUITMESSAGES = sizeof(endmsg)/sizeof(*endmsg) - 1;
