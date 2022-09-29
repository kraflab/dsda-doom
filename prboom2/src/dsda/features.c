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

#include "features.h"

static dsda_feature_t used_features;

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

dsda_feature_t dsda_UsedFeatures(void) {
  return used_features;
}
