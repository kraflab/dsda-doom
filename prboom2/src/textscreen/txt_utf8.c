//
// Copyright(C) 2005-2014 Simon Howard
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "txt_utf8.h"

// Decode UTF-8 character, incrementing *ptr over the decoded bytes.

unsigned int TXT_DecodeUTF8(const char **ptr)
{
    const char *p = *ptr;
    unsigned int c;

    // UTF-8 decode.

    if ((*p & 0x80) == 0)                     // 1 character (ASCII):
    {
        c = *p;
        *ptr += 1;
    }
    else if ((p[0] & 0xe0) == 0xc0            // 2 character:
          && (p[1] & 0xc0) == 0x80)
    {
        c = ((p[0] & 0x1f) << 6)
          |  (p[1] & 0x3f);
        *ptr += 2;
    }
    else if ((p[0] & 0xf0) == 0xe0            // 3 character:
          && (p[1] & 0xc0) == 0x80
          && (p[2] & 0xc0) == 0x80)
    {
        c = ((p[0] & 0x0f) << 12)
          | ((p[1] & 0x3f) << 6)
          |  (p[2] & 0x3f);
        *ptr += 3;
    }
    else if ((p[0] & 0xf8) == 0xf0            // 4 character:
          && (p[1] & 0xc0) == 0x80
          && (p[2] & 0xc0) == 0x80
          && (p[3] & 0xc0) == 0x80)
    {
        c = ((p[0] & 0x07) << 18)
          | ((p[1] & 0x3f) << 12)
          | ((p[2] & 0x3f) << 6)
          |  (p[3] & 0x3f);
        *ptr += 4;
    }
    else
    {
        // Decode failure.
        // Don't bother with 5/6 byte sequences.

        c = 0;
    }

    return c;
}
