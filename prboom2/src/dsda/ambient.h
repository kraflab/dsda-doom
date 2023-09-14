//
// Copyright(C) 2023 by Ryan Krafnick
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
//	DSDA Ambient
//

#ifndef __DSDA_AMBIENT__
#define __DSDA_AMBIENT__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  char* sound_name;
  int sfx_id;
  float attenuation;
  float volume;
  int min_tics;
  int max_tics;
} ambient_sfx_t;

ambient_sfx_t* dsda_AmbientSFX(int id);
void dsda_LoadAmbientSndInfo(void);

#ifdef __cplusplus
}
#endif

#endif
