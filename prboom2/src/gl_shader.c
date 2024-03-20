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
 *
 *---------------------------------------------------------------------
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <math.h>
#include <stdarg.h>
#include "doomstat.h"
#include "v_video.h"
#include "gl_opengl.h"
#include "gl_intern.h"
#include "r_main.h"
#include "w_wad.h"
#include "i_system.h"
#include "r_bsp.h"
#include "lprintf.h"
#include "m_file.h"
#include "e6y.h"
#include "r_things.h"
#include "doomdef.h"
#include "dsda/configuration.h"

#include "dsda/utility/string_view.h"

#define MAX_TEXTURES 3
#define MAX_UNIFORMS 10
#define MAX_STACK 10

#define UNIF_VAL_END (-1)

#define UNIF(num, name, type) [num] = {(name), (type)}
#define UNIF_END {NULL, UNIF_COUNT}

typedef enum
{
  UNIF_1F,
  UNIF_2F,
  UNIF_1I,
  UNIF_TEX0,
  UNIF_TEX1,
  UNIF_TEX2,
  UNIF_TEX0D,
  UNIF_TEX1D,
  UNIF_TEX2D,
  UNIF_COUNT
} shader_uniform_type_t;

typedef struct
{
  const char* name;
  shader_uniform_type_t type;
} shader_uniform_t;

typedef struct
{
  const char* name;
  shader_uniform_t unifs[];
} shader_info_t;

typedef struct
{
  const shader_info_t* info;
  GLhandleARB hShader;
  GLhandleARB hVertProg;
  GLhandleARB hFragProg;
  int indices[MAX_UNIFORMS];
  int texds[MAX_TEXTURES];
} shader_t;

typedef struct
{
  const GLchar* name;
  const GLchar* value;
} shader_define_t;

typedef struct
{
  GLchar const** strs;
  GLint* lens;
  unsigned int size;
  unsigned int cap;
} shader_source_t;

typedef union
{
  int i[2];
  float f[2];
} shader_uniform_value_t;

typedef struct
{
  shader_t* shader;
  shader_uniform_value_t unifs[MAX_UNIFORMS];
} shader_frame_t;

static void glsl_ShaderSrcInit(shader_source_t* src)
{
  memset(src, 0, sizeof(*src));
}

static void glsl_ShaderSrcDestroy(shader_source_t* src)
{
  if (src->strs)
    Z_Free(src->strs);
  if (src->lens)
    Z_Free(src->lens);
  glsl_ShaderSrcInit(src);
}

static void glsl_ShaderSrcAppend(shader_source_t* src, const GLchar* str, GLint len)
{
  unsigned int size = src->size;

  if (size && src->lens[size - 1] >= 0 && src->strs[size - 1] + src->lens[size - 1] == str)
  {
    // Fast path -- expand last string to include more of source buffer
    src->lens[src->size - 1] += len;
    return;
  }

  if (src->size == src->cap)
  {
    if (src->cap == 0)
      src->cap = 8;
    else
      src->cap *= 2;

    src->strs = Z_Realloc(src->strs, src->cap * sizeof(*src->strs));
    src->lens = Z_Realloc(src->lens, src->cap * sizeof(*src->lens));
  }

  src->strs[src->size] = str;
  src->lens[src->size] = len;
  ++src->size;
}

static void glsl_ShaderLookup(const char* name, GLchar const** text, GLint* len)
{
  int lump = W_CheckNumForName2(name, ns_prboom);

  if (lump == LUMP_NOT_FOUND)
    I_Error("Could not find shader source: %s\n", name);

  *text = W_LumpByNum(lump);
  *len = W_LumpLength(lump);
}

#define CSLEN(x) (sizeof((x)) - 1)

static void glsl_ShaderSrcAppendDefine(shader_source_t* src,
                                       const shader_define_t* def)
{
  static const char cdefine[] = "#define ";
  static const char cspace[] = " ";
  static const char cnl[] = "\n";

  glsl_ShaderSrcAppend(src, cdefine, CSLEN(cdefine));
  glsl_ShaderSrcAppend(src, def->name, strlen(def->name));
  glsl_ShaderSrcAppend(src, cspace, CSLEN(cspace));
  glsl_ShaderSrcAppend(src, def->value, strlen(def->value));
  glsl_ShaderSrcAppend(src, cnl, CSLEN(cnl));
}

static void glsl_ShaderSrcProcess(shader_source_t* src, const GLchar* text,
                                  GLint len, const shader_define_t* defs,
                                  const shader_define_t* userdefs)
{
  static const char vdir[] = "#version";
  static const char edir[] = "#extension";
  static const char idir[] = "#include";
  static const char iext[] = "GL_GOOGLE_include_directive";
  dsda_string_view_t v;
  dsda_string_view_t line;

  dsda_InitStringView(&v, text, len);

  while (dsda_GetStringViewLine(&v, &line))
  {
    // Output any version and extension directives before defines
    if (dsda_StringViewStartsWith(&line, vdir))
    {
      glsl_ShaderSrcAppend(src, line.string, line.size);
      continue;
    }

    if (dsda_StringViewStartsWith(&line, edir))
    {
      dsda_string_view_t cur;

      dsda_StringViewAtOffset(&line, CSLEN(edir), &cur);
      dsda_StringViewAfterChars(&cur, " \t", &cur);

      if (dsda_StringViewStartsWith(&cur, iext))
        // Omit include extension from output since we're handling it
        continue;
      glsl_ShaderSrcAppend(src, line.string, line.size);
      continue;
    }

    // Output any outstanding defines
    for (; defs && defs->name; ++defs)
      glsl_ShaderSrcAppendDefine(src, defs);

    for (; userdefs && userdefs->name; ++userdefs)
      glsl_ShaderSrcAppendDefine(src, userdefs);

    // Handle include directives
    if (dsda_StringViewStartsWith(&line, idir))
    {
      dsda_string_view_t cur = line;
      char lumpname[9] = {0};
      const GLchar* itext;
      GLint ilen;

      // Parse include name
      if (!dsda_SplitStringViewAfterChar(&line, '"', NULL, &cur) ||
          !dsda_SplitStringViewBeforeChar(&cur, '"', &cur, NULL))
        I_Error("Invalid include syntax: %.*s\n", (int) line.size, line.string);

      // Trim off extension if present
      dsda_SplitStringViewBeforeChar(&cur, '.', &cur, NULL);

      // Truncate and NUL-terminate lump name
      memcpy(lumpname, cur.string, MIN(8, cur.size));

      // Recursively process include source text
      glsl_ShaderLookup(lumpname, &itext, &ilen);
      glsl_ShaderSrcProcess(src, itext, ilen, NULL, NULL);
      continue;
    }

    // Pass line through verbatim
    glsl_ShaderSrcAppend(src, line.string, line.size);
  }
}

static void glsl_ShaderSrcLoad(shader_source_t* src, const char* name,
                               const shader_define_t* defs,
                               const shader_define_t* userdefs)
{
  const GLchar* text;
  int len;

  glsl_ShaderLookup(name, &text, &len);
  glsl_ShaderSrcInit(src);
  glsl_ShaderSrcProcess(src, text, len, defs, userdefs);
}

static shader_t* glsl_ShaderLoad(const shader_info_t* info,
                                 const shader_define_t* userdefs)
{
  shader_source_t src;
  int status;
  char buffer[2048];
  shader_t* shader = NULL;
  const shader_uniform_t* unif;
  unsigned int i;
  int t;

  shader = Z_Malloc(sizeof(*shader));
  shader->info = info;

  for (i = 0; i < MAX_TEXTURES; ++i)
    shader->texds[i] = -1;

  shader->hVertProg = GLEXT_glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
  glsl_ShaderSrcLoad(&src, "gls_v", NULL, userdefs);
  GLEXT_glShaderSourceARB(shader->hVertProg, src.size, src.strs, src.lens);
  glsl_ShaderSrcDestroy(&src);

  GLEXT_glCompileShaderARB(shader->hVertProg);
  GLEXT_glGetInfoLogARB(shader->hVertProg, sizeof(buffer), NULL, buffer);
  GLEXT_glGetObjectParameterivARB(shader->hVertProg,
                                  GL_OBJECT_COMPILE_STATUS_ARB, &status);
  if (status)
    lprintf(LO_DEBUG, "ShaderLoad: Shader \"%s\" (vertex) compiled OK: %s\n",
            info->name, buffer);
  else
    I_Error("ShaderLoad: Error compiling shader \"%s\" (vertex): %s\n",
            info->name, buffer);

  shader->hFragProg = GLEXT_glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
  glsl_ShaderSrcLoad(&src, info->name, NULL, userdefs);
  GLEXT_glShaderSourceARB(shader->hFragProg, src.size, src.strs, src.lens);
  glsl_ShaderSrcDestroy(&src);

  GLEXT_glCompileShaderARB(shader->hFragProg);
  GLEXT_glGetInfoLogARB(shader->hFragProg, sizeof(buffer), NULL, buffer);
  GLEXT_glGetObjectParameterivARB(shader->hFragProg,
                                  GL_OBJECT_COMPILE_STATUS_ARB, &status);
  if (status)
    lprintf(LO_DEBUG, "ShaderLoad: Shader \"%s\" (fragment) compiled OK: %s\n",
            info->name, buffer);
  else
    I_Error("ShaderLoad: Error compiling shader \"%s\" (fragment): %s\n",
            info->name, buffer);

  shader->hShader = GLEXT_glCreateProgramObjectARB();
  GLEXT_glAttachObjectARB(shader->hShader, shader->hVertProg);
  GLEXT_glAttachObjectARB(shader->hShader, shader->hFragProg);
  GLEXT_glLinkProgramARB(shader->hShader);
  GLEXT_glGetInfoLogARB(shader->hShader, sizeof(buffer), NULL, buffer);
  GLEXT_glGetObjectParameterivARB(shader->hShader, GL_OBJECT_LINK_STATUS_ARB,
                                  &status);

  if (status)
    lprintf(LO_DEBUG, "ShaderLoad: Shader \"%s\" linked OK: %s\n", info->name,
            buffer);
  else
    I_Error("ShaderLoad: Error linking shader \"%s\": %s\n", info->name,
            buffer);

  GLEXT_glUseProgramObjectARB(shader->hShader);

  for (unif = info->unifs, i = 0; unif->name; ++unif, ++i)
  {
    int idx;

    if (i >= MAX_UNIFORMS)
      I_Error("ShaderLoad: Too many uniforms in shader \"%s\"\n", info->name);

    idx = GLEXT_glGetUniformLocationARB(shader->hShader, unif->name);
    if (idx == -1)
      I_Error("ShaderLoad: No such uniform \"%s\" in shader \"%s\"\n",
              unif->name, info->name);
    shader->indices[i] = idx;

    switch (unif->type)
    {
    case UNIF_TEX0:
      GLEXT_glUniform1iARB(idx, 0);
      break;
    case UNIF_TEX1:
      GLEXT_glUniform1iARB(idx, 1);
      break;
    case UNIF_TEX2:
      GLEXT_glUniform1iARB(idx, 2);
      break;
    case UNIF_TEX0D:
    case UNIF_TEX1D:
    case UNIF_TEX2D:
      t = unif->type - UNIF_TEX0D;
      if (shader->texds[t] != -1)
        I_Error("ShaderLoad: Duplicate texture dimension uniform: %i\n", i);
      shader->texds[t] = idx;
      break;
    default:
      continue;
    }
  }

  GLEXT_glUseProgramObjectARB(0);

  return shader;
}

static unsigned int texds[MAX_TEXTURES][2];
static shader_frame_t stack[MAX_STACK];
static unsigned int sp = 0;

static shader_frame_t* glsl_ShaderFramePush(void)
{
  if (sp == MAX_STACK - 1)
    I_Error("ShaderFramePush: Max shader stack depth exceeded\n");

  return &stack[sp++];
}

static void glsl_ShaderFrameActivate(const shader_frame_t* frame)
{
  unsigned int i;
  shader_t* shader = frame->shader;
  const shader_uniform_t* unif;

  GLEXT_glUseProgramObjectARB(shader ? shader->hShader : 0);

  if (!frame->shader)
    return;

  for (unif = frame->shader->info->unifs, i = 0; unif->name; ++unif, ++i)
  {
    const shader_uniform_value_t* val = &frame->unifs[i];
    int idx = frame->shader->indices[i];

    switch (unif->type)
    {
    case UNIF_1I:
      GLEXT_glUniform1iARB(idx, val->i[0]);
      break;
    case UNIF_1F:
      GLEXT_glUniform1fARB(idx, val->f[0]);
      break;
    case UNIF_2F:
      GLEXT_glUniform2fARB(idx, val->f[0], val->f[1]);
      break;
    default:
      continue;
    }
  }

  for (i = 0; i < MAX_TEXTURES; ++i)
  {
    int idx = shader->texds[i];
    if (idx != -1)
      GLEXT_glUniform2fARB(idx, (float) texds[i][0], (float) texds[i][1]);
  }
}

static void glsl_ShaderPush(shader_t* shader, ...)
{
  shader_frame_t* frame = glsl_ShaderFramePush();
  va_list ap;
  int num;

  frame->shader = shader;

  if (shader != NULL)
  {
    va_start(ap, shader);

    while ((num = va_arg(ap, int)) >= 0)
    {
      const shader_uniform_t* unif = &frame->shader->info->unifs[num];
      shader_uniform_value_t* val;

      val = &frame->unifs[num];

      switch(unif->type)
      {
      case UNIF_1I:
        val->i[0] = va_arg(ap, GLint);
        break;
      case UNIF_1F:
        val->f[0] = va_arg(ap, double);
        break;
      case UNIF_2F:
        val->f[0] = va_arg(ap, double);
        val->f[1] = va_arg(ap, double);
        break;
      default:
        I_Error("ShaderPush: Can't dynamically set texture uniform type");
      }
    }

    va_end(ap);
  }

  glsl_ShaderFrameActivate(frame);
}

static void glsl_ShaderPop(shader_t* shader)
{
  if (sp == 0)
    I_Error("ShaderPop: Pop of empty shader stack\n");

  if (stack[sp - 1].shader != shader)
    I_Error("ShaderPop: Pop of incorrect shader (\"%s\" != \"%s\"\n",
            shader->info->name, stack[sp - 1].shader->info->name);

  if (--sp != 0)
    glsl_ShaderFrameActivate(&stack[sp - 1]);
  else
    GLEXT_glUseProgramObjectARB(0);
}

static void glsl_ShaderUniform(shader_t* shader, int num, ...)
{
  shader_frame_t* frame;
  va_list ap;
  const shader_uniform_t* unif = &shader->info->unifs[num];
  int idx = shader->indices[num];
  shader_uniform_value_t* val;

  if (sp == 0)
    I_Error("ShaderUniform: Can't modify shader uniform with empty stack\n");

  frame = &stack[sp - 1];
  if (frame->shader != shader)
    I_Error("ShaderUniform: Can't modify shader uniform for inactive shader\n");

  val = &frame->unifs[num];

  va_start(ap, num);

  switch(unif->type)
  {
  case UNIF_1I:
    val->i[0] = va_arg(ap, GLint);
    GLEXT_glUniform1iARB(idx, val->i[0]);
    break;
  case UNIF_1F:
    val->f[0] = va_arg(ap, double);
    GLEXT_glUniform1fARB(idx, val->f[0]);
    break;
  case UNIF_2F:
    val->f[0] = va_arg(ap, double);
    val->f[1] = va_arg(ap, double);
    GLEXT_glUniform2fARB(idx, val->f[0], val->f[1]);
    break;
  default:
    I_Error("ShaderUniform: Can't dynamically set texture uniform type");
  }

  va_end(ap);
}

void glsl_SetTextureDims(int unit, unsigned int width, unsigned int height)
{
  shader_t* shader;
  int idx;

  assert(unit < MAX_TEXTURES);

  texds[unit][0] = width;
  texds[unit][1] = height;

  if (sp > 0 && (shader = stack[sp - 1].shader))
  {
    idx = shader->texds[unit];
    if (idx != -1)
      GLEXT_glUniform2fARB(idx, (float) width, (float) height);
  }
}

enum
{
  MAIN_UNIF_TEX,
  MAIN_UNIF_COLORMAP,
  MAIN_UNIF_LIGHTLEVEL,
  MAIN_UNIF_FADE_MODE
};

enum
{
  FUZZ_UNIF_TEX,
  FUZZ_UNIF_FUZZ,
  FUZZ_UNIF_TEX_D,
  FUZZ_UNIF_FUZZ_D,
  FUZZ_UNIF_RATIO,
  FUZZ_UNIF_SEED
};

static shader_t *sh_main = NULL;
static shader_t *sh_fuzz = NULL;

static const shader_info_t main_info =
{
  .name = "gls_main",
  .unifs =
  {
    UNIF(MAIN_UNIF_TEX, "tex", UNIF_TEX0),
    UNIF(MAIN_UNIF_COLORMAP, "colormap", UNIF_TEX2),
    UNIF(MAIN_UNIF_LIGHTLEVEL, "lightlevel", UNIF_1F),
    UNIF(MAIN_UNIF_FADE_MODE, "fade_mode", UNIF_1I),
    UNIF_END
  }
};

static const shader_info_t fuzz_info =
{
  .name = "gls_fuzz",
  .unifs =
  {
    UNIF(FUZZ_UNIF_TEX, "tex", UNIF_TEX0),
    UNIF(FUZZ_UNIF_TEX_D, "tex_d", UNIF_TEX0D),
    UNIF(FUZZ_UNIF_FUZZ, "fuzz", UNIF_TEX1),
    UNIF(FUZZ_UNIF_FUZZ_D, "fuzz_d", UNIF_TEX1D),
    UNIF(FUZZ_UNIF_RATIO, "ratio", UNIF_1F),
    UNIF(FUZZ_UNIF_SEED, "seed", UNIF_1F),
    UNIF_END
  }
};

void glsl_Init(void)
{
  sh_main = glsl_ShaderLoad(&main_info, NULL);
  sh_fuzz = glsl_ShaderLoad(&fuzz_info, NULL);
}

void glsl_PushNullShader(void)
{
  glsl_ShaderPush(NULL);
}

void glsl_PopNullShader(void)
{
  glsl_ShaderPop(NULL);
}

void glsl_PushMainShader(void)
{
  int mode = dsda_IntConfig(dsda_config_gl_fade_mode);

  glsl_ShaderPush(sh_main,
                  MAIN_UNIF_FADE_MODE, mode,
                  UNIF_VAL_END);
}

void glsl_PopMainShader(void)
{
  glsl_ShaderPop(sh_main);
}

void glsl_SetLightLevel(float lightlevel)
{
  glsl_ShaderUniform(sh_main, MAIN_UNIF_LIGHTLEVEL, lightlevel);
}

void glsl_PushFuzzShader(int tic, int sprite, float ratio)
{
  // Large integers converted to float can lose precision, causing
  // problems in the shader.  Since the tic and sprite count are just
  // used for randomness, munge them down and convert to float with
  // double precision here
  const int factor = 1103515245;
  int seed = 0xD00D;

  seed = seed * factor + tic;
  seed = seed * factor + sprite;
  seed *= factor;

  glsl_ShaderPush(sh_fuzz,
             FUZZ_UNIF_RATIO, ratio,
             FUZZ_UNIF_SEED, (double) seed / INT_MAX,
             UNIF_VAL_END);
}

void glsl_PopFuzzShader(void)
{
  glsl_ShaderPop(sh_fuzz);
}
