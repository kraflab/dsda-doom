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
//	DSDA Input Display
//

#include "st_stuff.h"
#include "hu_lib.h"
#include "hu_stuff.h"
#include "doomstat.h"

#include "dsda.h"
#include "dsda/global.h"
#include "dsda/settings.h"
#include "dsda/hud.h"

#include "command_display.h"

#define INPUT_TEXT_X 198

typedef struct dsda_command_display_s {
  ticcmd_t cmd;
  int repeat;
  char text[200];
  hu_textline_t hu_text;
  struct dsda_command_display_s* next;
  struct dsda_command_display_s* prev;
} dsda_command_display_t;

static dsda_command_display_t* command_history;
static dsda_command_display_t* current_command;
static int dsda_command_history_size = 20;

void dsda_InitCommandDisplay(patchnum_t* font) {
  int i;

  command_history = calloc(dsda_command_history_size, sizeof(*command_history));
  current_command = command_history;

  for (i = 0; i < dsda_command_history_size; ++i) {
    HUlib_initTextLine(
      &command_history[i].hu_text,
      INPUT_TEXT_X,
      200 - g_st_height - 8 * (i + 1),
      font,
      HU_FONTSTART,
      g_cr_gray,
      VPT_ALIGN_RIGHT_BOTTOM
    );

    command_history[i].hu_text.space_width = 5;

    if (i > 0) {
      command_history[i].prev = &command_history[i - 1];
      command_history[i - 1].next = &command_history[i];
    }
  }

  command_history[0].prev = &command_history[dsda_command_history_size - 1];
  command_history[dsda_command_history_size - 1].next = &command_history[0];
}

void dsda_AddCommandToCommandDisplay(ticcmd_t* cmd) {
  char* s;
  int length = 0;
  signed char angleturn;

  angleturn = (cmd->angleturn + 128) >> 8;

  if (
    angleturn == cmd->forwardmove &&
    angleturn == cmd->sidemove &&
    angleturn == 0 &&
    cmd->buttons == 0
  )
    return;

  if (
    current_command->cmd.angleturn == cmd->angleturn &&
    current_command->cmd.forwardmove == cmd->forwardmove &&
    current_command->cmd.sidemove == cmd->sidemove &&
    current_command->cmd.buttons == cmd->buttons
  )
    ++current_command->repeat;
  else {
    current_command->repeat = 0;
    current_command = current_command->next;
    current_command->cmd = *cmd;
  }

  current_command->text[0] = '\0';

  if (current_command->repeat)
    length += sprintf(current_command->text + length, "x%-3d ", current_command->repeat + 1);
  else
    length += sprintf(current_command->text + length, "     ");

  if (cmd->forwardmove > 0)
    length += sprintf(current_command->text + length, "MF%2d ", cmd->forwardmove);
  else if (cmd->forwardmove < 0)
    length += sprintf(current_command->text + length, "MB%2d ", -cmd->forwardmove);
  else
    length += sprintf(current_command->text + length, "     ");

  if (cmd->sidemove > 0)
    length += sprintf(current_command->text + length, "SR%2d ", cmd->sidemove);
  else if (cmd->sidemove < 0)
    length += sprintf(current_command->text + length, "SL%2d ", -cmd->sidemove);
  else
    length += sprintf(current_command->text + length, "     ");

  if (angleturn > 0)
    length += sprintf(current_command->text + length, "TL%2d ", angleturn);
  else if (angleturn < 0)
    length += sprintf(current_command->text + length, "TR%2d ", -angleturn);
  else
    length += sprintf(current_command->text + length, "     ");

  if (cmd->buttons) {
    if (!(cmd->buttons & BT_SPECIAL)) {
      if (cmd->buttons & BT_ATTACK)
        length += sprintf(current_command->text + length, "A");
      else
        length += sprintf(current_command->text + length, " ");

      if (cmd->buttons & BT_USE)
        length += sprintf(current_command->text + length, "U");
      else
        length += sprintf(current_command->text + length, " ");

      if (cmd->buttons & BT_CHANGE) {
        int weapon = (cmd->buttons & BT_WEAPONMASK) >> BT_WEAPONSHIFT;

        length += sprintf(current_command->text + length, "C%d", weapon);
      }
    }
  }

  HUlib_clearTextLine(&current_command->hu_text);
  s = current_command->text;
  while (*s) HUlib_addCharToTextLine(&current_command->hu_text, *(s++));
}

void dsda_DrawCommandDisplay(void) {
  int i;
  dsda_command_display_t* command = current_command;

  for (i = 0; i < dsda_command_history_size; ++i) {
    command->hu_text.y = 200 - g_st_height - 8 * (i + 1);
    HUlib_drawTextLine(&command->hu_text, false);
    command = command->prev;
  }
}

void dsda_EraseCommandDisplay(void) {
  int i;
  dsda_command_display_t* command = current_command;

  for (i = 0; i < dsda_command_history_size; ++i) {
    HUlib_eraseTextLine(&command->hu_text);
    command = command->prev;
  }
}
