/*
** gl_sky.cpp
**
** Draws the sky.  Loosely based on the JDoom sky and the ZDoomGL 0.66.2 sky.
**
**---------------------------------------------------------------------------
** Copyright 2003 Tim Stump
** Copyright 2005 Christoph Oelckers
** Copyright 2009 Andrey Budko
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
** 4. Full disclosure of the entire project's source code, except for third
**    party libraries is mandatory. (NOTE: This clause is non-negotiable!)
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gl_opengl.h"

#include <SDL.h>
#include <math.h>

#include "doomstat.h"
#include "v_video.h"
#include "gl_intern.h"
#include "r_plane.h"
#include "r_sky.h"
#include "r_main.h"
#include "sc_man.h"
#include "m_misc.h"
#include "lprintf.h"

#include "e6y.h"

#include "dsda/settings.h"

typedef struct
{
  int mode;
  int vertexcount;
  int vertexindex;
  int use_texture;
} GLSkyLoopDef;

typedef struct
{
  int id;
  int rows, columns;
  int loopcount;
  GLSkyLoopDef *loops;
  vbo_vertex_t *data;
} GLSkyVBO;

int gl_drawskys;
// Sky stretching is rather pointless with the GL renderer
// now that it can handle all sky heights.
int gl_stretchsky = false;

static PalEntry_t *SkyColor;

SkyBoxParams_t SkyBox;
float y_offset_saved;

void gld_InitSky(void)
{
  memset(&SkyBox, 0, sizeof(SkyBox));
  SkyBox.index = -1;
  y_offset_saved = 0;
}

void gld_InitFrameSky(void)
{
  SkyBox.type = SKY_NONE;
  SkyBox.wall.gltexture = NULL;
  SkyBox.x_offset = 0;
  SkyBox.y_offset = 0;
}

void gld_DrawFakeSkyStrips(void)
{
  int i;

  // This draws a valid z-buffer into the stencil's contents to ensure it
  // doesn't get overwritten by the level's geometry.

  // Because some of outdated hardware has no support for
  // glColorMask(0, 0, 0, 0) or something,
  // I need to render fake strips of sky before dome with using
  // full clearing of color buffer (only in compatibility mode)

  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // no graphics
  gld_EnableTexture2D(GL_TEXTURE0_ARB, false);

  for (i = gld_drawinfo.num_items[GLDIT_SWALL] - 1; i >= 0; i--)
  {
    GLWall* wall = gld_drawinfo.items[GLDIT_SWALL][i].item.wall;

    glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(wall->glseg->x1,wall->ytop,wall->glseg->z1);
    glVertex3f(wall->glseg->x1,wall->ybottom,wall->glseg->z1);
    glVertex3f(wall->glseg->x2,wall->ytop,wall->glseg->z2);
    glVertex3f(wall->glseg->x2,wall->ybottom,wall->glseg->z2);
    glEnd();
  }

  gld_EnableTexture2D(GL_TEXTURE0_ARB, true);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

// Sky textures with a zero index should be forced
// See third episode of requiem.wad
void gld_AddSkyTexture(GLWall *wall, int sky1, int sky2, int skytype)
{
  side_t *s = NULL;
  line_t *l = NULL;
  int sky = skytexture;

  wall->gltexture = NULL;

  if (sky1 & PL_SKYFLAT_LINE)
  {
    l = &lines[sky1 & ~PL_SKYFLAT_LINE];
  }
  else if (sky2 & PL_SKYFLAT_LINE)
  {
    l = &lines[sky2 & ~PL_SKYFLAT_LINE];
  }

  if (sky1 & PL_SKYFLAT_SECTOR)
  {
    sky = sky1 & ~PL_SKYFLAT_SECTOR;
  }
  else if (sky2 & PL_SKYFLAT_SECTOR)
  {
    sky = sky2 & ~PL_SKYFLAT_SECTOR;
  }

  if (l)
  {
    s = *l->sidenum + sides;
    wall->gltexture = gld_RegisterSkyTexture(texturetranslation[s->toptexture],
      texturetranslation[s->toptexture] == skytexture || l->special == 271 || l->special == 272);
    if (wall->gltexture)
    {
      // As far as I can tell, textureoffset just adds directly to viewangle and is
      // therefore affected by tiling, but rowoffset is not.
      wall->skyyaw = (double) (viewangle + s->textureoffset) / (double) ANGLE_MAX;
      wall->skypitch = skyYShift;
      wall->skyoffset = (((float)s->rowoffset/(float)FRACUNIT - 28.0f)/wall->gltexture->buffer_height);
      wall->flag = l->special == 282 ? GLDWF_SKY : GLDWF_SKYFLIP;
    }
  }
  else
  {
    wall->gltexture = gld_RegisterSkyTexture(sky, true);
    if (wall->gltexture)
    {
      int h = wall->gltexture->buffer_height;

      wall->skyyaw = skyXShift;
      wall->skypitch = skyYShift;
      // Choose offset based on logic from r_sky.c
      if (h >= 128 && h < 200)
        wall->skyoffset = -28.0f / 128.0f;
      else if (h > 200)
        wall->skyoffset = (200 - h) / 128.0f;
      else
        wall->skyoffset = 0.0f;
      wall->flag = GLDWF_SKY;
    }
  }

  if (wall->gltexture)
  {
    SkyBox.type |= skytype;

    wall->gltexture->flags |= GLTEXTURE_SKY;

    gld_AddDrawItem(GLDIT_SWALL, wall);

    if (!SkyBox.wall.gltexture)
    {
      SkyBox.wall = *wall;

      switch (gl_drawskys)
      {
      case skytype_skydome:
        if (s)
        {
          SkyBox.x_offset = (float)s->textureoffset * 180.0f / (float)ANG180;
          SkyBox.y_offset = (float)s->rowoffset / (float)FRACUNIT;
        }
        else if (raven)
        {
          SkyBox.y_offset = 39.0f;
        }
        break;
      }
    }
  }
}

// The fussy arithmetic to correctly scale and translate the sky texture lives here.
void gld_SkyTransform(GLWall* wall)
{
  // Make apparent scale of sky closer to software
  const float scale_correction = 0.80f;
  // 360 horizontal degrees of sky in texels (4 tilings of default sky)
  float sky_width = 256 * 4;
  // 360 vertical degrees of sky in texels (determined emperically with Heretic
  // software mode)
  float sky_height = 880;
  // Texture dimensions
  float w = wall->gltexture->buffer_width;
  float h = wall->gltexture->buffer_height;
  // Tile factors
  float tilex = sky_width / w;
  float tiley = sky_height / h;
  // Adjustment for tall screens
  float ratio = tallscreen ? (float)ratio_multiplier / ratio_scale : 1.0f;
  // X flip coefficient
  float flipx = wall->flag == GLDWF_SKYFLIP ? -1.0 : 1.0;
  // Scale factors
  float scalex = scale_correction / skyscale * flipx;
  float scaley = scale_correction * ratio * (skystretch ? 2.0 : 1.0f) / skyscale;
  // Translations
  float transx = wall->skyyaw * tilex * flipx;
  float transy = wall->skypitch * tiley;

  // Apply scaling centered at horizon and view yaw
  glTranslatef(transx, wall->skyoffset, 0.0f);
  glScalef(1.0f / scalex, 1.0f / scaley, 1.0);
  //glTranslatef(-transx, -wall->skyoffset, 0.0f);

  // Apply offset
  //glTranslatef(0, wall->skyoffset, 0.0f);

  // Apply view yaw and pitch
  //glTranslatef(transx, transy, 0.0f);

  // The above translations (which are part of logically distinct operations)
  // are commented out and combined here for efficiency
  glTranslatef(0.0, transy, 0.0f);

  // Scale texture coordinates generated by glTexGen so the range [0, 1]
  // corresponds to the full range of the texture as usual
  glScalef(MAP_COEFF / w, MAP_COEFF / h, 1.0f);
}

void gld_DrawStripsSky(void)
{
  int i;
  GLTexture *gltexture = NULL;

  if (gl_drawskys == skytype_standard)
  {
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_Q);

    glColor4fv(gl_whitecolor);

    SetTextureMode(TM_OPAQUE);
  }

  glMatrixMode(GL_TEXTURE);

  for (i = gld_drawinfo.num_items[GLDIT_SWALL] - 1; i >= 0; i--)
  {
    GLWall *wall = gld_drawinfo.items[GLDIT_SWALL][i].item.wall;

    gltexture = (gl_drawskys == skytype_none ? NULL : wall->gltexture);
    gld_BindSkyTexture(gltexture);

    if (!gltexture)
    {
      glColor4f(1.0f,0.0f,0.0f,1.0f);
    }

    if (gltexture)
    {
      glPushMatrix();

      gld_SkyTransform(wall);
    }

    glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(wall->glseg->x1,wall->ytop,wall->glseg->z1);
    glVertex3f(wall->glseg->x1,wall->ybottom,wall->glseg->z1);
    glVertex3f(wall->glseg->x2,wall->ytop,wall->glseg->z2);
    glVertex3f(wall->glseg->x2,wall->ybottom,wall->glseg->z2);
    glEnd();

    if (gltexture)
    {
      glPopMatrix();
    }
  }

  glMatrixMode(GL_MODELVIEW);

  gld_DrawSkyCaps();

  if (gl_drawskys == skytype_standard)
  {
    glDisable(GL_TEXTURE_GEN_Q);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_S);

    SetFrameTextureMode();
  }
}

void gld_DrawSkyCaps(void)
{
  if (SkyBox.type && SkyBox.wall.gltexture)
  {
    if (dsda_MouseLook())
    {
      gld_BindSkyTexture(SkyBox.wall.gltexture);

      glMatrixMode(GL_TEXTURE);
      glPushMatrix();

      gld_SkyTransform(&SkyBox.wall);

      if (SkyBox.type & SKY_CEILING)
      {
        glBegin(GL_TRIANGLE_STRIP);
        glVertex3f(-MAXCOORD,+MAXCOORD,+MAXCOORD);
        glVertex3f(+MAXCOORD,+MAXCOORD,+MAXCOORD);
        glVertex3f(-MAXCOORD,+MAXCOORD,-MAXCOORD);
        glVertex3f(+MAXCOORD,+MAXCOORD,-MAXCOORD);
        glEnd();
      }

      if (SkyBox.type & SKY_FLOOR)
      {
        glBegin(GL_TRIANGLE_STRIP);
        glVertex3f(-MAXCOORD,-MAXCOORD,+MAXCOORD);
        glVertex3f(+MAXCOORD,-MAXCOORD,+MAXCOORD);
        glVertex3f(-MAXCOORD,-MAXCOORD,-MAXCOORD);
        glVertex3f(+MAXCOORD,-MAXCOORD,-MAXCOORD);
        glEnd();
      }

      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
    }
  }
}

//===========================================================================
//
// averageColor
//  input is RGBA8 pixel format.
//	The resulting RGB color can be scaled uniformly so that the highest
//	component becomes one.
//
//===========================================================================
#define APART(c)			(((c)>>24)&0xff)
#define RPART(c)			(((c)>>16)&0xff)
#define GPART(c)			(((c)>>8)&0xff)
#define BPART(c)			((c)&0xff)

void averageColor(PalEntry_t * PalEntry, const unsigned int *data, int size, fixed_t maxout_factor)
{
  int i;
  int maxv;
  unsigned int r, g, b;

  // First clear them.
  r = g = b = 0;
  if (size == 0)
  {
    PalEntry->r = 255;
    PalEntry->g = 255;
    PalEntry->b = 255;
    return;
  }
  for(i = 0; i < size; i++)
  {
    r += BPART(data[i]);
    g += GPART(data[i]);
    b += RPART(data[i]);
  }

  r = r / size;
  g = g / size;
  b = b / size;

  maxv=MAX(MAX(r,g),b);

  if(maxv && maxout_factor)
  {
    maxout_factor = FixedMul(maxout_factor, 255);
    r = r * maxout_factor / maxv;
    g = g * maxout_factor / maxv;
    b = b * maxout_factor / maxv;
  }

  PalEntry->r = r;
  PalEntry->g = g;
  PalEntry->b = b;
  return;
}

// The texture offset to be applied to the texture coordinates in SkyVertex().
static int rows, columns;
static dboolean yflip;
static int texw;
static float yMult, yAdd;
static dboolean foglayer;
static float delta = 0.0f;

//-----------------------------------------------------------------------------
//
//
//
//-----------------------------------------------------------------------------

void gld_GetSkyCapColors(void)
{
  int color, width, height;
  int frame_fixedcolormap_saved;
  unsigned char *buffer = NULL;
  const unsigned char *playpal = V_GetPlaypal();
  const lighttable_t *colormap;
  const lighttable_t *fixedcolormap_saved;
  PalEntry_t *ceiling_rgb = &SkyBox.CeilingSkyColor[0];
  PalEntry_t *floor_rgb = &SkyBox.FloorSkyColor[0];

  // saving current colormap
  fixedcolormap_saved = fixedcolormap;
  frame_fixedcolormap_saved = frame_fixedcolormap;

  fixedcolormap = fullcolormap;
  frame_fixedcolormap = 0;

  gld_BindSkyTexture(SkyBox.wall.gltexture);

  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

  buffer = Z_Malloc(width * height * 4);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

  averageColor(ceiling_rgb, (unsigned int*)buffer, width * MIN(30, height), 0);

  if (height > 30)
  {
    averageColor(floor_rgb,
      ((unsigned int*)buffer) + (height - 30) * width, width * 30, 0);
  }
  else
  {
    *floor_rgb = *ceiling_rgb;
  }

  colormap = fullcolormap + INVERSECOLORMAP * 256 * sizeof(lighttable_t);

  color = V_BestColor(playpal, ceiling_rgb->r, ceiling_rgb->g, ceiling_rgb->b);
  SkyBox.CeilingSkyColor[1].r = playpal[colormap[color] * 3 + 0];
  SkyBox.CeilingSkyColor[1].g = playpal[colormap[color] * 3 + 1];
  SkyBox.CeilingSkyColor[1].b = playpal[colormap[color] * 3 + 2];

  color = V_BestColor(playpal, floor_rgb->r, floor_rgb->g, floor_rgb->b);
  SkyBox.FloorSkyColor[1].r = playpal[colormap[color] * 3 + 0];
  SkyBox.FloorSkyColor[1].g = playpal[colormap[color] * 3 + 1];
  SkyBox.FloorSkyColor[1].b = playpal[colormap[color] * 3 + 2];

  // restorin current colormap
  fixedcolormap = fixedcolormap_saved;
  frame_fixedcolormap = frame_fixedcolormap_saved;

  Z_Free(buffer);
}

//-----------------------------------------------------------------------------
//
//
//
//-----------------------------------------------------------------------------

static void SkyVertex(vbo_vertex_t *vbo, int r, int c)
{
  static const float scale = 10000.0f;
  static const float maxSideAngle = DEG2RAD(60.0f);

  float topAngle = DEG2RAD(c / (float)columns * 360.0f);
  float sideAngle = maxSideAngle * (float)(rows - r) / (float)rows;
  float height = (float)sin(sideAngle);
  float realRadius = scale * (float)cos(sideAngle);
  float x = realRadius * (float)cos(topAngle);
  float y = (!yflip) ? scale * height : -scale * height;
  float z = realRadius * (float)sin(topAngle);
  float timesRepeat;

  timesRepeat = (short)(4 * (256.0f / texw));
  if (timesRepeat == 0.0f)
    timesRepeat = 1.0f;

  if (!foglayer)
  {
    vbo->r = 255;
    vbo->g = 255;
    vbo->b = 255;
    vbo->a = (r == 0 ? 0 : 255);

    // And the texture coordinates.
    if(!yflip)	// Flipped Y is for the lower hemisphere.
    {
      vbo->u = (-timesRepeat * c / (float)columns) ;
      vbo->v = (r / (float)rows) * 1.f * yMult + yAdd;
    }
    else
    {
      vbo->u = (-timesRepeat * c / (float)columns) ;
      vbo->v = ((rows-r)/(float)rows) * 1.f * yMult + yAdd;
    }

    if (SkyBox.wall.flag == GLDWF_SKYFLIP)
      vbo->u = -vbo->u;
  }

  if (r != 4)
  {
    y += 300;
  }

  // And finally the vertex.
  vbo->x =-x / MAP_COEFF;	// Doom mirrors the sky vertically!
  vbo->y = y / MAP_COEFF + delta;
  vbo->z = z / MAP_COEFF;
}

GLSkyVBO sky_vbo[2];

static void gld_BuildSky(int row_count, int col_count, SkyBoxParams_t *sky, int cm)
{
  int texh, c, r;
  vbo_vertex_t *vertex_p;
  int vertex_count = 2 * row_count * (col_count * 2 + 2) + col_count * 2;
  int vbo_idx = (cm == INVERSECOLORMAP ? 1 : 0);
  GLSkyVBO *vbo = &sky_vbo[vbo_idx];

  if ((vbo->columns != col_count) || (vbo->rows != row_count))
  {
    Z_Free(vbo->loops);
    Z_Free(vbo->data);
    vbo->loops = NULL;
    vbo->data = NULL;
  }

  if (!vbo->data)
  {
    vbo->loops = Z_Malloc((row_count * 2 + 2) * sizeof(vbo->loops[0]));
    // create vertex array
    vbo->data = Z_Malloc(vertex_count * sizeof(vbo->data[0]));
  }

  vbo->columns = col_count;
  vbo->rows = row_count;

  texh = sky->wall.gltexture->buffer_height;
  if (texh > 190 && gl_stretchsky)
    texh = 190;
  texw = sky->wall.gltexture->buffer_width;

  vertex_p = &vbo->data[0];
  vbo->loopcount = 0;
  for (yflip = 0; yflip < 2; yflip++)
  {
    vbo->loops[vbo->loopcount].mode = GL_TRIANGLE_FAN;
    vbo->loops[vbo->loopcount].vertexindex = vertex_p - &vbo->data[0];
    vbo->loops[vbo->loopcount].vertexcount = col_count;
    vbo->loops[vbo->loopcount].use_texture = false;
    vbo->loopcount++;

    yAdd = sky->y_offset / texh;
    yMult = (texh <= 180 ? 1.0f : 180.0f / texh);
    if (yflip == 0)
    {
      SkyColor = &sky->CeilingSkyColor[vbo_idx];
    }
    else
    {
      SkyColor = &sky->FloorSkyColor[vbo_idx];
      if (texh <= 180) yMult = 1.0f; else yAdd += 180.0f / texh;
    }

    delta = 0.0f;
    foglayer = true;
    for(c = 0; c < col_count; c++)
    {
      SkyVertex(vertex_p, 1, c);
      vertex_p->r = SkyColor->r;
      vertex_p->g = SkyColor->g;
      vertex_p->b = SkyColor->b;
      vertex_p->a = 255;
      vertex_p++;
    }
    foglayer = false;

    delta = (yflip ? 5.0f : -5.0f) / MAP_COEFF;

    for(r = 0; r < row_count; r++)
    {
      vbo->loops[vbo->loopcount].mode = GL_TRIANGLE_STRIP;
      vbo->loops[vbo->loopcount].vertexindex = vertex_p - &vbo->data[0];
      vbo->loops[vbo->loopcount].vertexcount = 2 * col_count + 2;
      vbo->loops[vbo->loopcount].use_texture = true;
      vbo->loopcount++;

      for(c = 0; c <= col_count; c++)
      {
        SkyVertex(vertex_p++, r + (yflip ? 1 : 0), (c ? c : 0));
        SkyVertex(vertex_p++, r + (yflip ? 0 : 1), (c ? c : 0));
      }
    }
  }
}

//-----------------------------------------------------------------------------
//
//
//
//-----------------------------------------------------------------------------

static void RenderDome(SkyBoxParams_t *sky)
{
  int i, j;
  GLSkyVBO *vbo;
  int vbosize;
  dboolean rebuild_sky;

  if (!sky || !sky->wall.gltexture)
    return;

  if (invul_cm && frame_fixedcolormap == INVERSECOLORMAP)
    vbo = &sky_vbo[1];
  else
    vbo = &sky_vbo[0];

  glRotatef(-180.0f + sky->x_offset, 0.f, 1.f, 0.f);

  rows = 4;
  columns = 64;
  vbosize = 2 * rows * (columns * 2 + 2) + columns * 2;
  rebuild_sky = sky->y_offset != y_offset_saved || sky->wall.gltexture->index != sky->index;

  if (rebuild_sky)
  {
    y_offset_saved = sky->y_offset;

    if (sky->wall.gltexture->index != sky->index)
    {
      sky->index = sky->wall.gltexture->index;
      gld_GetSkyCapColors();
    }

    gld_BuildSky(rows, columns, sky, 0);
    gld_BuildSky(rows, columns, sky, INVERSECOLORMAP);
  }

  if (gl_ext_arb_vertex_buffer_object)
  {
    if (rebuild_sky || !vbo->id)
    {
      if (vbo->id)
      {
        // delete VBO when already exists
        GLEXT_glDeleteBuffersARB(1, &vbo->id);
      }
      // generate a new VBO and get the associated ID
      GLEXT_glGenBuffersARB(1, &vbo->id);
      // bind VBO in order to use
      GLEXT_glBindBufferARB(GL_ARRAY_BUFFER, vbo->id);
      // upload data to VBO
      GLEXT_glBufferDataARB(GL_ARRAY_BUFFER,
        vbosize * sizeof(vbo->data[0]),
        vbo->data, GL_STATIC_DRAW_ARB);
    }
  }

  gld_BindSkyTexture(SkyBox.wall.gltexture);

  if (gl_ext_arb_vertex_buffer_object)
  {
    // bind VBO in order to use
    GLEXT_glBindBufferARB(GL_ARRAY_BUFFER, vbo->id);
  }

  // activate and specify pointers to arrays
  glVertexPointer(3, GL_FLOAT, sizeof(vbo->data[0]), sky_vbo_x);
  glTexCoordPointer(2, GL_FLOAT, sizeof(vbo->data[0]), sky_vbo_u);
  glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vbo->data[0]), sky_vbo_r);

  // activate vertex array, texture coord array and color arrays
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  if (!gl_stretchsky)
  {
    int texh = sky->wall.gltexture->buffer_height;

    if (texh <= 180)
    {
      glScalef(1.0f, (float)texh / 230.0f, 1.0f);
    }
    else
    {
      if (texh > 190)
        glScalef(1.0f, 230.0f / 240.0f, 1.0f);
    }
  }

  for(j = (HaveMouseLook() || !gl_stretchsky ? 0 : 1); j < 2; j++)
  {
    gld_EnableTexture2D(GL_TEXTURE0_ARB, j != 0);

    for(i = 0; i < vbo->loopcount; i++)
    {
      GLSkyLoopDef *loop = &vbo->loops[i];

      if (j == 0 ? loop->use_texture : !loop->use_texture)
        continue;

      glDrawArrays(loop->mode, loop->vertexindex, loop->vertexcount);
    }
  }

  glScalef(1.0f, 1.0f, 1.0f);

  // current color is undefined after glDrawArrays
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

  if (gl_ext_arb_vertex_buffer_object)
  {
    // bind with 0, so, switch back to normal pointer operation
    GLEXT_glBindBufferARB(GL_ARRAY_BUFFER, 0);
  }
  // deactivate color array
  glDisableClientState(GL_COLOR_ARRAY);
}

void gld_DrawDomeSkyBox(void)
{
  if (SkyBox.wall.gltexture)
  {
    GLint shading_mode = GL_FLAT;

    gld_DrawFakeSkyStrips();

    glGetIntegerv(GL_SHADE_MODEL, &shading_mode);
    glShadeModel(GL_SMOOTH);

    glDepthMask(false);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_ALPHA_TEST);
    SetTextureMode(TM_OPAQUE);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glRotatef(roll,  0.0f, 0.0f, 1.0f);
    glRotatef(pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(yaw,   0.0f, 1.0f, 0.0f);
    glScalef(-2.0f, 2.0f, 2.0f);
    glTranslatef(0.f, -1250.0f / MAP_COEFF, 0.f);

    RenderDome(&SkyBox);

    glPopMatrix();

    glEnable(GL_ALPHA_TEST);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(true);

    SetFrameTextureMode();

    glShadeModel(shading_mode);
  }
}
