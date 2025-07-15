//  Copyright (C) 2023 Fabian Greffrath
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
// DESCRIPTION:
//      Load sound lumps with libsndfile.

#ifndef I_SNDFILE_H
#define I_SNDFILE_H

#include "SDL_audio.h"

void *Load_SNDFile(void *data, SDL_AudioSpec *sample, void **sampledata,
				   Uint32 *samplelen);

#endif
