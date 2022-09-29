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
//	DSDA Features
//

#include <string.h>

#include "z_zone.h"

#include "features.h"

static uint_64_t used_features;

static const char* feature_names[64] = {
  [uf_menu] = "Menu",
  [uf_exhud] = "Extended HUD",
  [uf_advhud] = "Advanced HUD",
  [uf_crosshair] = "Crosshair",
  [uf_quickstartcache] = "Quickstart Cache",
  [uf_100k] = "100K Tracker",

  [uf_iddt] = "IDDT",
  [uf_automap] = "IDBEHOLD Map",
  [uf_liteamp] = "IDBEHOLD Light",
  [uf_build] = "Build Mode",
  [uf_buildzero] = "Build First Frame",
  [uf_bruteforce] = "Brute Force",
  [uf_tracker] = "TAS Tracker",
  [uf_keyframe] = "Key Frame",
  [uf_skip] = "Skip Forward",
  [uf_wipescreen] = "Skip Wipe Screen",
  [uf_speedup] = "Speed Up",
  [uf_slowdown] = "Slow Down",
  [uf_coordinates] = "Show Coordinates",
  [uf_mouselook] = "Mouse Look",
  [uf_weaponalignment] = "Weapon Alignment",
  [uf_commanddisplay] = "Command Display",
  [uf_crosshaircolor] = "Dynamic Crosshair Color",
  [uf_crosshairlock] = "Crosshair Lock",
  [uf_shadows] = "Shadows",
  [uf_painpalette] = "Disable Pain Palette",
  [uf_bonuspalette] = "Disable Bonus Palette",
  [uf_powerpalette] = "Disable Power Palette",
  [uf_healthbar] = "Show Health Bars",
  [uf_alwayssr50] = "Always SR50",
  [uf_maxplayercorpse] = "Edit Corpse Limit",
  [uf_hideweapon] = "Hide Weapon",
};

#define FEATURE_BIT(x) ((uint_64_t) 1 << x)

void dsda_TrackFeature(int feature) {
  used_features |= FEATURE_BIT(feature);
}

void dsda_ResetFeatures(void) {
  used_features = 0;
}

uint_64_t dsda_UsedFeatures(void) {
  return used_features;
}

void dsda_CopyFeatures(byte* result) {
  result[0] = (used_features      ) & 0xff;
  result[1] = (used_features >>  8) & 0xff;
  result[2] = (used_features >> 16) & 0xff;
  result[3] = (used_features >> 24) & 0xff;
  result[4] = (used_features >> 32) & 0xff;
  result[5] = (used_features >> 40) & 0xff;
  result[6] = (used_features >> 48) & 0xff;
  result[7] = (used_features >> 56) & 0xff;
}

char* dsda_DescribeFeatures(void) {
  int i;
  char* description;
  size_t length = 0;
  dboolean first = true;

  for (i = 0; i < 64; ++i)
    if (used_features & FEATURE_BIT(i) && feature_names[i])
      length += strlen(feature_names[i]) + 2; // ", "

  ++length;
  description = Z_Calloc(length, 1);

  for (i = 0; i < 64; ++i)
    if (used_features & FEATURE_BIT(i) && feature_names[i]) {
      if (first)
        first = false;
      else
        strcat(description, ", ");

      strcat(description, feature_names[i]);
    }

  return description;
}
