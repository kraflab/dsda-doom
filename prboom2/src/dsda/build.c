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
//	DSDA Build Mode
//

#include "doomstat.h"

#include "dsda/demo.h"
#include "dsda/input.h"
#include "dsda/settings.h"

#include "build.h"

static dboolean build_mode;
static dboolean advance_frame;
static ticcmd_t build_cmd;

static signed char forward50(void) {
  return pclass[players[consoleplayer].pclass].forwardmove[1];
}

static signed char strafe40(void) {
  return pclass[players[consoleplayer].pclass].sidemove[1];
}

static signed char strafe50(void) {
  return forward50();
}

static signed short shortTic(void) {
  return (1 << 8);
}

static signed short maxAngle(void) {
  return (128 << 8);
}

static void buildForward(void) {
  if (build_cmd.forwardmove == forward50())
    build_cmd.forwardmove = 0;
  else
    build_cmd.forwardmove = forward50();
}

static void buildBackward(void) {
  if (build_cmd.forwardmove == -forward50())
    build_cmd.forwardmove = 0;
  else
    build_cmd.forwardmove = -forward50();
}

static void buildStrafeRight(void) {
  if (build_cmd.sidemove == strafe50())
    build_cmd.sidemove = 0;
  else
    build_cmd.sidemove = strafe50();
}

static void buildStrafeLeft(void) {
  if (build_cmd.sidemove == -strafe50())
    build_cmd.sidemove = 0;
  else
    build_cmd.sidemove = -strafe50();
}

static void buildTurnRight(void) {
  if (build_cmd.angleturn == maxAngle())
    build_cmd.angleturn = 0;
  else
    build_cmd.angleturn += shortTic();
}

static void buildTurnLeft(void) {
  if (build_cmd.angleturn == -maxAngle())
    build_cmd.angleturn = 0;
  else
    build_cmd.angleturn -= shortTic();
}

static void buildUse(void) {
  build_cmd.buttons ^= BT_USE;
}

static void buildFire(void) {
  build_cmd.buttons ^= BT_ATTACK;
}

static void buildWeapon(int weapon) {
  int cmdweapon;

  cmdweapon = weapon << BT_WEAPONSHIFT;

  if (build_cmd.buttons & BT_CHANGE && (build_cmd.buttons & BT_WEAPONMASK) == cmdweapon)
    build_cmd.buttons &= ~BT_CHANGE;
  else
    build_cmd.buttons |= BT_CHANGE;

  build_cmd.buttons &= ~BT_WEAPONMASK;
  if (build_cmd.buttons & BT_CHANGE)
    build_cmd.buttons |= cmdweapon;
}

static void resetCmd(void) {
  memset(&build_cmd, 0, sizeof(build_cmd));
}

angle_t dsda_BuildModeViewAngleOffset(void) {
  if (!build_mode)
    return 0;

  return build_cmd.angleturn << 16;
}

dboolean dsda_AllowBuilding(void) {
  return !dsda_StrictMode() && !demoplayback;
}

dboolean dsda_BuildMode(void) {
  return build_mode;
}

void dsda_CopyBuildCmd(ticcmd_t* cmd) {
  *cmd = build_cmd;
  dsda_JoinDemoCmd(cmd);
}

dboolean dsda_BuildResponder(event_t* ev) {
  if (!dsda_AllowBuilding())
    return false;

  if (dsda_InputActivated(dsda_input_build)) {
    build_mode = !build_mode;
    paused ^= PAUSE_BUILDMODE;

    return true;
  }

  if (!build_mode)
    return false;

  if (dsda_InputActivated(dsda_input_build_advance_frame)) {
    advance_frame = gametic;

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_reset_command)) {
    resetCmd();

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_forward)) {
    buildForward();

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_backward)) {
    buildBackward();

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_strafe_right)) {
    buildStrafeRight();

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_strafe_left)) {
    buildStrafeLeft();

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_turn_right)) {
    buildTurnRight();

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_turn_left)) {
    buildTurnLeft();

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_use)) {
    buildUse();

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_fire)) {
    buildFire();

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_weapon1)) {
    buildWeapon(0);

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_weapon2)) {
    buildWeapon(1);

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_weapon3)) {
    buildWeapon(2);

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_weapon4)) {
    buildWeapon(3);

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_weapon5)) {
    buildWeapon(4);

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_weapon6)) {
    buildWeapon(5);

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_weapon7)) {
    buildWeapon(6);

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_weapon8)) {
    buildWeapon(7);

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_weapon9)) {
    if (!demo_compatibility && gamemode == commercial)
      buildWeapon(8);

    return true;
  }

  return false;
}

dboolean dsda_AdvanceFrame(void) {
  dboolean result;

  result = advance_frame;
  advance_frame = false;

  return result;
}
