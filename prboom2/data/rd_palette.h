// Copyright (c) 1993-2011 PrBoom developers (see AUTHORS)
// Licence: GPLv2 or later (see COPYING)

#ifndef RD_PALETTE_H
#define RD_PALETTE_H

// Chained hash lookup to convert rgb triples to palette indices

// load raw palette, create hash table etc.
void palette_init(const char *filename);

// lookup colour index of rgb triple
int palette_getindex(const unsigned char *rgb);

#endif /* RD_PALETTE_H */
