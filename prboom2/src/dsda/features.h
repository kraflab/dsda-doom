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

#include "doomtype.h"

typedef uint_64_t dsda_feature_t;

// TODO: commented features require configuration refactor

#define UF_NONE 0

#define UF_MENU            (dsda_feature_t) 0x00000001
// #define UF_EXHUD           (dsda_feature_t) 0x00000000
// #define UF_ADVHUD          (dsda_feature_t) 0x00000000

#define UF_IDDT            (dsda_feature_t) 0x0000000100000000
#define UF_AUTOMAP         (dsda_feature_t) 0x0000000200000000
#define UF_LITEAMP         (dsda_feature_t) 0x0000000400000000
#define UF_BUILD           (dsda_feature_t) 0x0000000800000000
#define UF_BUILDZERO       (dsda_feature_t) 0x0000001000000000
#define UF_BRUTEFORCE      (dsda_feature_t) 0x0000002000000000
#define UF_TRACKER         (dsda_feature_t) 0x0000004000000000
#define UF_KEYFRAME        (dsda_feature_t) 0x0000008000000000
#define UF_SKIP            (dsda_feature_t) 0x0000010000000000
// #define UF_SPEEDUP         (dsda_feature_t) 0x0000000000000000
// #define UF_SLOWDOWN        (dsda_feature_t) 0x0000000000000000
// #define UF_COORDINATES     (dsda_feature_t) 0x0000000000000000
// #define UF_MLOOK           (dsda_feature_t) 0x0000000000000000
// #define UF_WEAPONALIGNMENT (dsda_feature_t) 0x0000000000000000
// #define UF_COMMANDDISPLAY  (dsda_feature_t) 0x0000000000000000
// #define UF_CROSSHAIRCOLOR  (dsda_feature_t) 0x0000000000000000
// #define UF_CROSSHAIRLOCK   (dsda_feature_t) 0x0000000000000000
// #define UF_SHADOWS         (dsda_feature_t) 0x0000000000000000
// #define UF_PAINPALETTE     (dsda_feature_t) 0x0000000000000000
// #define UF_BONUSPALETTE    (dsda_feature_t) 0x0000000000000000
// #define UF_POWERPALETTE    (dsda_feature_t) 0x0000000000000000
// #define UF_HEALTHBAR       (dsda_feature_t) 0x0000000000000000
// #define UF_SHOWALIVE       (dsda_feature_t) 0x0000000000000000
// #define UF_ALWAYSSR50      (dsda_feature_t) 0x0000000000000000

void dsda_TrackFeature(dsda_feature_t feature);
void dsda_ResetFeatures(void);
dsda_feature_t dsda_UsedFeatures(void);
