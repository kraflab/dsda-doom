//
// Copyright(C) 2005-2014 Simon Howard
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
//
// Text mode emulation in SDL
//

#ifndef TXT_SDL_H
#define TXT_SDL_H

// The textscreen API itself doesn't need SDL; however, SDL needs its
// headers included where main() is defined.

#include "SDL.h"

// Event callback function type: a function of this type can be used
// to intercept events in the textscreen event processing loop.  
// Returning 1 will cause the event to be eaten; the textscreen code
// will not see it.

typedef int (*TxtSDLEventCallbackFunc)(SDL_Event *event, void *user_data);

void TXT_PreInit(SDL_Window *preset_window, SDL_Renderer *preset_renderer, int opengl);

typedef struct
{
    const char *name;
    const uint8_t *data;
    unsigned int w;
    unsigned int h;
} txt_font_t;

// GL stuff
extern int GL_TXT_Init(void);
extern void GL_TXT_UpdateScreen(void);

#endif /* #ifndef TXT_SDL_H */

