// Copyright (c) 1993-2011 PrBoom developers (see AUTHORS)
// Licence: GPLv2 or later (see COPYING)

#ifndef RD_SOUND_H
#define RD_SOUND_H

#include <stddef.h>

// Convert WAVE files to Doom sound format

// convert wav file to doom sound format
size_t wav_to_doom(void **lumpdata, const char *filename);

#endif /* RD_SOUND_H */
