//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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


#ifndef __XDDEFS__
#define __XDDEFS__

typedef PACKED_STRUCT (
{
    char name[8];
    dboolean masked;
    short width;
    short height;
    int obsolete;
    short patchcount;
    mappatch_t patches[1];
}) maptexture_t;

#endif // __XDDEFS__
