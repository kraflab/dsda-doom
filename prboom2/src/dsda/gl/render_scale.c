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
#include "gl_intern.h"

#include "i_video.h"
#include "r_main.h"
#include "st_stuff.h"

#include <math.h>

int gl_window_width;
int gl_window_height;
int gl_viewport_width;
int gl_viewport_height;
int gl_viewport_x;
int gl_viewport_y;
int gl_statusbar_height;
int gl_scene_width;
int gl_scene_height;
float gl_scale_x;
float gl_scale_y;
int gl_letterbox_clear_required = 0;

static int gl_clear_box_width;
static int gl_clear_box_height;

void dsda_GLGetSDLWindowSize(SDL_Window* sdl_window) {
  SDL_GL_GetDrawableSize(sdl_window, &gl_window_width, &gl_window_height);
}

void dsda_GLSetRenderViewportParams() {
  float viewport_aspect;

  viewport_aspect = (float)SCREENWIDTH / (float)ACTUALHEIGHT;

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
  gl_statusbar_height = (int)ceilf(gl_scale_y * (float)ST_SCALED_HEIGHT) * R_PartialView();

  gl_scene_width = gl_viewport_width;
  gl_scene_height = gl_viewport_height - gl_statusbar_height;

  // elim - If the viewport's x or y coordinate is indented from the window, we need to call glClear
  //        each frame to prevent artifacts smearing on undrawn framebuffer area
  gl_letterbox_clear_required = gl_viewport_x + gl_viewport_y;
  if (gl_letterbox_clear_required) {
    gl_clear_box_width = ((gl_viewport_y != 0) * gl_window_width) + gl_viewport_x;
    gl_clear_box_height = ((gl_viewport_y == 0) * gl_window_height) + gl_viewport_y;
  }
}

void dsda_GLSetRenderViewport() {
  glViewport(gl_viewport_x, gl_viewport_y, gl_viewport_width, gl_viewport_height);
}

void dsda_GLSetRenderViewportScissor() {
  glScissor(gl_viewport_x, gl_viewport_y, gl_viewport_width, gl_viewport_height);
}

void dsda_GLSetRenderSceneScissor() {
  glScissor(gl_viewport_x,
            gl_viewport_y + gl_statusbar_height,
            gl_scene_width, gl_scene_height);
}

void dsda_GLSetScreenSpaceScissor(int x, int y, int w, int h)
{
  glScissor(gl_viewport_x + x * gl_scale_x,
            gl_viewport_y + (SCREENHEIGHT - (y + h)) * gl_scale_y,
            w * gl_scale_x,
            h * gl_scale_y);
}

void dsda_GLUpdateStatusBarVisible() {
  int saved_visible;
  int current_visible;

  saved_visible = (gl_statusbar_height > 0);
  current_visible = R_PartialView();

  if (saved_visible != current_visible) {
    gl_statusbar_height = (int)(gl_scale_y * (float)ST_SCALED_HEIGHT) * R_PartialView();
    gl_scene_height = gl_viewport_height - gl_statusbar_height;
  }
}

void dsda_GLLetterboxClear() {
  if (!gl_letterbox_clear_required)
    return;

  glViewport(0, 0, gl_window_width, gl_window_height);
  glEnable(GL_SCISSOR_TEST);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  // Bottom or left box
  glScissor(0, 0, gl_clear_box_width, gl_clear_box_height);
  glClear(GL_COLOR_BUFFER_BIT);

  // Top or right box
  glScissor(gl_window_width - gl_clear_box_width,
            gl_window_height - gl_clear_box_height,
            gl_clear_box_width, gl_clear_box_height);
  glClear(GL_COLOR_BUFFER_BIT);

  // Reset to expected state before rendering the actual frame starts
  dsda_GLSetRenderViewport();
  dsda_GLSetRenderViewportScissor();
}

void dsda_GLStartMeltRenderTexture() {
  if (!SceneInTexture)
    return;

  gld_InitDrawScene();
  gld_StartDrawScene();
  gld_Set2DMode();
  glViewport(0, 0, SCREENWIDTH, SCREENHEIGHT);
  glScissor(0, 0, SCREENWIDTH, SCREENHEIGHT);
}

void dsda_GLEndMeltRenderTexture() {
  if (!SceneInTexture)
    return;

  GLEXT_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glBindTexture(GL_TEXTURE_2D, glSceneImageTextureFBOTexID);

  dsda_GLFullscreenOrtho2D();
  dsda_GLSetRenderViewport();
  dsda_GLSetRenderViewportScissor();

  glBegin(GL_TRIANGLE_STRIP);
  {
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, gl_window_height);
    glTexCoord2f(1.0f, 1.0f); glVertex2f((float)gl_window_width, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f((float)gl_window_width, (float)gl_window_height);
  }
  glEnd();

  gld_Set2DMode();
  glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
}

void dsda_GLFullscreenOrtho2D() {
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(
    (GLdouble) 0,
    (GLdouble) gl_window_width,
    (GLdouble) gl_window_height,
    (GLdouble) 0,
    (GLdouble) -1.0,
    (GLdouble) 1.0
  );
  glDisable(GL_DEPTH_TEST);
}
