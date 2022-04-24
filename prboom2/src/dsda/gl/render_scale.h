//
// Copyright(C) 2022 by Ryan Krafnick
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
//	Data for rendering non-exclusive fullscreen in OpenGL
//  Original Author: elim
//

#include "SDL.h"

extern int gl_window_width;
extern int gl_window_height;
extern int gl_viewport_width;
extern int gl_viewport_height;
extern int gl_viewport_x;
extern int gl_viewport_y;
extern int gl_statusbar_height;
extern int gl_scene_width;
extern int gl_scene_height;
extern int gl_scene_offset_x;
extern int gl_scene_offset_y;
extern float gl_scale_x;
extern float gl_scale_y;
extern int gl_letterbox_clear_required;

void dsda_GetSDLWindowSize(SDL_Window* sdl_window);

void dsda_SetRenderViewportParams();

void dsda_SetRenderViewport();

void dsda_SetRenderViewportScissor();

void dsda_SetRenderSceneScissor();

void dsda_UpdateStatusBarVisible();

void dsda_GLLetterboxClear();
