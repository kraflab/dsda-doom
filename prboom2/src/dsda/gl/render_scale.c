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

#include "render_scale.h"

#include "gl_opengl.h"

#include "i_video.h"
#include "r_main.h"
#include "st_stuff.h"

int gl_window_width;
int gl_window_height;
int gl_viewport_width;
int gl_viewport_height;
int gl_viewport_x;
int gl_viewport_y;
int gl_statusbar_height;
int gl_scene_width;
int gl_scene_height;
int gl_scene_offset_x;
int gl_scene_offset_y;
float gl_scale_x;
float gl_scale_y;

void dsda_GetSDLWindowSize(SDL_Window* sdl_window) {
  SDL_GetWindowSize(sdl_window, &gl_window_width, &gl_window_height);
}

void dsda_SetRenderViewportParams() {
  float viewport_aspect;

  viewport_aspect = (float)SCREENWIDTH / (float)SCREENHEIGHT;

  // Black bars on left and right of viewport
  if ((int)(gl_window_height * viewport_aspect) < gl_window_width) {
    gl_viewport_width = (int)((float)gl_window_height * viewport_aspect);
    gl_viewport_height = gl_window_height;
    gl_viewport_x = (gl_window_width - gl_viewport_width) >> 1;
    gl_viewport_y = 0;
  }
  // Either matching window's aspect ratio, or black bars on top and bottom (ie 21:9 on a 16:9 display)
  else {
    gl_viewport_width = gl_window_width;
    gl_viewport_height = (int)((float)gl_window_width / viewport_aspect);
    gl_viewport_x = 0;
    gl_viewport_y = (gl_window_height - gl_viewport_height) >> 1;
  }

  gl_scale_x = (float)gl_viewport_width / (float)SCREENWIDTH;
  gl_scale_y = (float)gl_viewport_height / (float)SCREENHEIGHT;

  // elim - This will be zero if no statusbar is being drawn
  gl_statusbar_height = (int)(gl_scale_y * (float)ST_SCALED_HEIGHT) * (viewheight != SCREENHEIGHT);

  gl_scene_offset_x = (int)(viewwindowx * gl_scale_x);
  gl_scene_offset_y = (int)(viewwindowy * gl_scale_y);

  gl_scene_width = gl_viewport_width - (gl_scene_offset_x * 2);
  gl_scene_height = gl_viewport_height - gl_statusbar_height - (gl_scene_offset_y * 2);
}

void dsda_SetRenderViewport() {
  glViewport(gl_viewport_x, gl_viewport_y, gl_viewport_width, gl_viewport_height);
}

void dsda_UpdateStatusBarVisible() {
  int saved_visible;
  int current_visible;

  saved_visible = (gl_statusbar_height > 0);
  current_visible = (viewheight != SCREENHEIGHT);

  if (saved_visible != current_visible) {
    gl_statusbar_height = (int)(gl_scale_y * (float)ST_SCALED_HEIGHT) * (viewheight != SCREENHEIGHT);
    gl_scene_height = gl_viewport_height - gl_statusbar_height - (gl_scene_offset_y * 2);
  }
}
