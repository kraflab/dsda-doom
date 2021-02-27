//
// Copyright(C) 2020 by Ryan Krafnick
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
//	DSDA Demo
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomtype.h"
#include "m_misc.h"
#include "lprintf.h"

#include "demo.h"

#define INITIAL_DEMO_BUFFER_SIZE 0x20000

static byte* dsda_demo_write_buffer;
static byte* dsda_demo_write_buffer_p;
static int dsda_demo_write_buffer_length;
static char* dsda_demo_name;

static void dsda_EnsureDemoBufferSpace(size_t length) {
  int offset;

  offset = dsda_DemoBufferOffset();

  if (offset + length <= dsda_demo_write_buffer_length) return;

  while (offset + length > dsda_demo_write_buffer_length)
    dsda_demo_write_buffer_length *= 2;

  dsda_demo_write_buffer =
    (byte *)realloc(dsda_demo_write_buffer, dsda_demo_write_buffer_length);

  if (dsda_demo_write_buffer == NULL)
    I_Error("dsda_EnsureDemoBufferSpace: out of memory!");

  dsda_demo_write_buffer_p = dsda_demo_write_buffer + offset;

  lprintf(
    LO_INFO,
    "dsda_EnsureDemoBufferSpace: expanding demo buffer %d\n",
    dsda_demo_write_buffer_length
  );
}

void dsda_InitDemo(char* name) {
  size_t name_size;

  name_size = strlen(name) + 1;
  dsda_demo_name = malloc(name_size);
  memcpy(dsda_demo_name, name, name_size);

  dsda_demo_write_buffer = malloc(INITIAL_DEMO_BUFFER_SIZE);
  if (dsda_demo_write_buffer == NULL)
    I_Error("dsda_InitDemo: unable to initialize demo buffer!");

  dsda_demo_write_buffer_p = dsda_demo_write_buffer;

  dsda_demo_write_buffer_length = INITIAL_DEMO_BUFFER_SIZE;
}

void dsda_WriteToDemo(void* buffer, size_t length) {
  dsda_EnsureDemoBufferSpace(length);

  memcpy(dsda_demo_write_buffer_p, buffer, length);
  dsda_demo_write_buffer_p += length;
}

void dsda_WriteDemoToFile(void) {
  int length;

  length = dsda_DemoBufferOffset();

  if (!M_WriteFile(dsda_demo_name, dsda_demo_write_buffer, length))
    I_Error("dsda_WriteDemoToFile: Failed to write demo file.");

  free(dsda_demo_write_buffer);
  free(dsda_demo_name);
  dsda_demo_write_buffer = NULL;
  dsda_demo_write_buffer_p = NULL;
  dsda_demo_write_buffer_length = 0;
  dsda_demo_name = NULL;
}

int dsda_DemoBufferOffset(void) {
  return dsda_demo_write_buffer_p - dsda_demo_write_buffer;
}

int dsda_CopyDemoBuffer(void* buffer) {
  int offset;

  offset = dsda_DemoBufferOffset();
  memcpy(buffer, dsda_demo_write_buffer, offset);

  return offset;
}

void dsda_SetDemoBufferOffset(int offset) {
  if (dsda_demo_write_buffer == NULL) return;

  // Cannot load forward (demo buffer would desync)
  if (offset > dsda_DemoBufferOffset())
    I_Error("dsda_SetDemoBufferOffset: Impossible time traveling detected.");

  dsda_demo_write_buffer_p = dsda_demo_write_buffer + offset;
}
