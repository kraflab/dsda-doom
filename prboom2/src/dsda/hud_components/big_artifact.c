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
//	DSDA Big Artifact HUD Component
//

#include "base.h"

#include "big_artifact.h"

static dsda_patch_component_t component;

void dsda_InitBigArtifactHC(int x_offset, int y_offset, int vpt, int* args) {
  dsda_InitPatchHC(&component, x_offset, y_offset, vpt);
}

void dsda_UpdateBigArtifactHC(void) {
  return;
}

void dsda_DrawBigArtifactHC(void) {
  extern void DrawArtifact(int x, int y, int vpt);

  DrawArtifact(component.x, component.y, component.vpt);
}
