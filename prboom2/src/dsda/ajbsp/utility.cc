//------------------------------------------------------------------------
//  UTILITIES
//------------------------------------------------------------------------
//
//  Copyright (C) 2001-2018 Andrew Apted
//  Copyright (C) 1997-2003 André Majorel et al
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
//------------------------------------------------------------------------

#include "system.h"
#include "local.h"
#include "utility.h"

#ifdef WIN32
#include <io.h>
#else  // UNIX or MACOSX
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#ifdef __APPLE__
#include <sys/param.h>
#endif

namespace ajbsp
{

//------------------------------------------------------------------------
//  FILENAMES
//------------------------------------------------------------------------

bool HasExtension(const char *filename)
{
	int A = (int)strlen(filename) - 1;

	if (A > 0 && filename[A] == '.')
		return false;

	for (; A >= 0 ; A--)
	{
		if (filename[A] == '.')
			return true;

		if (filename[A] == '/')
			break;

#ifdef WIN32
		if (filename[A] == '\\' || filename[A] == ':')
			break;
#endif
	}

	return false;
}

//
// MatchExtension
//
// When ext is NULL, checks if the file has no extension.
//
bool MatchExtension(const char *filename, const char *ext)
{
	if (! ext)
		return ! HasExtension(filename);

	int A = (int)strlen(filename) - 1;
	int B = (int)strlen(ext) - 1;

	for (; B >= 0 ; B--, A--)
	{
		if (A < 0)
			return false;

		if (toupper(filename[A]) != toupper(ext[B]))
			return false;
	}

	return (A >= 1) && (filename[A] == '.');
}


//
// ReplaceExtension
//
// When ext is NULL, any existing extension is removed.
//
// Returned string is a COPY.
//
char *ReplaceExtension(const char *filename, const char *ext)
{
	SYS_ASSERT(filename[0] != 0);

	size_t total_len = strlen(filename) + (ext ? strlen(ext) : 0);

	char *buffer = StringNew((int)total_len + 10);

	strcpy(buffer, filename);

	char *dot_pos = buffer + strlen(buffer) - 1;

	for (; dot_pos >= buffer && *dot_pos != '.' ; dot_pos--)
	{
		if (*dot_pos == '/')
			break;

#ifdef WIN32
		if (*dot_pos == '\\' || *dot_pos == ':')
			break;
#endif
	}

	if (dot_pos < buffer || *dot_pos != '.')
		dot_pos = NULL;

	if (! ext)
	{
		if (dot_pos)
			dot_pos[0] = 0;

		return buffer;
	}

	if (dot_pos)
		dot_pos[1] = 0;
	else
		strcat(buffer, ".");

	strcat(buffer, ext);

	return buffer;
}


const char *FindBaseName(const char *filename)
{
	// Find the base name of the file (i.e. without any path).
	// The result always points within the given string.
	//
	// Example:  "C:\Foo\Bar.wad"  ->  "Bar.wad"

	const char *pos = filename + strlen(filename) - 1;

	for (; pos >= filename ; pos--)
	{
		if (*pos == '/')
			return pos + 1;

#ifdef WIN32
		if (*pos == '\\' || *pos == ':')
			return pos + 1;
#endif
	}

	return filename;
}

//------------------------------------------------------------------------
// FILE MANAGEMENT
//------------------------------------------------------------------------

bool FileExists(const char *filename)
{
	FILE *fp = fopen(filename, "rb");

	if (fp)
	{
		fclose(fp);
		return true;
	}

	return false;
}


bool FileCopy(const char *src_name, const char *dest_name)
{
	char buffer[1024];

	FILE *src = fopen(src_name, "rb");
	if (! src)
		return false;

	FILE *dest = fopen(dest_name, "wb");
	if (! dest)
	{
		fclose(src);
		return false;
	}

	while (true)
	{
		size_t rlen = fread(buffer, 1, sizeof(buffer), src);
		if (rlen == 0)
			break;

		size_t wlen = fwrite(buffer, 1, rlen, dest);
		if (wlen != rlen)
			break;
	}

	bool was_OK = !ferror(src) && !ferror(dest);

	fclose(dest);
	fclose(src);

	return was_OK;
}


bool FileRename(const char *old_name, const char *new_name)
{
#ifdef WIN32
	return (::MoveFile(old_name, new_name) != 0);

#else // UNIX or MACOSX
	return (rename(old_name, new_name) == 0);
#endif
}


bool FileDelete(const char *filename)
{
#ifdef WIN32
	return (::DeleteFile(filename) != 0);

#else // UNIX or MACOSX
	return (remove(filename) == 0);
#endif
}

//------------------------------------------------------------------------
// STRINGS
//------------------------------------------------------------------------

//
// a case-insensitive strcmp()
//
int StringCaseCmp(const char *s1, const char *s2)
{
	for (;;)
	{
		int A = tolower(*s1++);
		int B = tolower(*s2++);

		if (A != B)
			return A - B;

		if (A == 0)
			return 0;
	}
}


//
// a case-insensitive strncmp()
//
int StringCaseCmpMax(const char *s1, const char *s2, size_t len)
{
	SYS_ASSERT(len != 0);

	for (;;)
	{
		if (len == 0)
			return 0;

		int A = tolower(*s1++);
		int B = tolower(*s2++);

		if (A != B)
			return A - B;

		if (A == 0)
			return 0;

		len--;
	}
}


char *StringNew(int length)
{
	// length does not include the trailing NUL.

	char *s = (char *) calloc(length + 1, 1);

	if (! s)
		cur_info->FatalError("Out of memory (%d bytes for string)\n", length);

	return s;
}


char *StringDup(const char *orig, int limit)
{
	if (! orig)
		return NULL;

	if (limit < 0)
	{
		char *s = strdup(orig);

		if (! s)
			cur_info->FatalError("Out of memory (copy string)\n");

		return s;
	}

	char * s = StringNew(limit+1);
	strncpy(s, orig, limit);
	s[limit] = 0;

	return s;
}


char *StringUpper(const char *name)
{
	char *copy = StringDup(name);

	for (char *p = copy; *p; p++)
		*p = toupper(*p);

	return copy;
}


char *StringPrintf(const char *str, ...)
{
	// Algorithm: keep doubling the allocated buffer size
	// until the output fits. Based on code by Darren Salt.

	char *buf = NULL;
	int buf_size = 128;

	for (;;)
	{
		va_list args;
		int out_len;

		buf_size *= 2;

		buf = (char*)realloc(buf, buf_size);
		if (!buf)
			cur_info->FatalError("Out of memory (formatting string)\n");

		va_start(args, str);
		out_len = vsnprintf(buf, buf_size, str, args);
		va_end(args);

		// old versions of vsnprintf() simply return -1 when
		// the output doesn't fit.
		if (out_len < 0 || out_len >= buf_size)
			continue;

		return buf;
	}
}


void StringFree(char *str)
{
	if (str)
	{
		free((void*) str);
	}
}


//------------------------------------------------------------------------
// MEMORY ALLOCATION
//------------------------------------------------------------------------

//
// Allocate memory with error checking.  Zeros the memory.
//
void *UtilCalloc(int size)
{
	void *ret = calloc(1, size);

	if (!ret)
		cur_info->FatalError("Out of memory (cannot allocate %d bytes)\n", size);

	return ret;
}


//
// Reallocate memory with error checking.
//
void *UtilRealloc(void *old, int size)
{
	void *ret = realloc(old, size);

	if (!ret)
		cur_info->FatalError("Out of memory (cannot reallocate %d bytes)\n", size);

	return ret;
}


//
// Free the memory with error checking.
//
void UtilFree(void *data)
{
	if (data == NULL)
		BugError("Trying to free a NULL pointer\n");

	free(data);
}


//------------------------------------------------------------------------
// MATH STUFF
//------------------------------------------------------------------------

//
// rounds the value _up_ to the nearest power of two.
//
int RoundPOW2(int x)
{
	if (x <= 2)
		return x;

	x--;

	for (int tmp = x >> 1 ; tmp ; tmp >>= 1)
		x |= tmp;

	return x + 1;
}


//
// Compute angle of line from (0,0) to (dx,dy).
// Result is degrees, where 0 is east and 90 is north.
//
double ComputeAngle(double dx, double dy)
{
	double angle;

	if (dx == 0)
		return (dy > 0) ? 90.0 : 270.0;

	angle = atan2((double) dy, (double) dx) * 180.0 / M_PI;

	if (angle < 0)
		angle += 360.0;

	return angle;
}


//------------------------------------------------------------------------
//  Adler-32 CHECKSUM Code
//------------------------------------------------------------------------

void Adler32_Begin(u32_t *crc)
{
	*crc = 1;
}

void Adler32_AddBlock(u32_t *crc, const u8_t *data, int length)
{
	u32_t s1 = (*crc) & 0xFFFF;
	u32_t s2 = ((*crc) >> 16) & 0xFFFF;

	for ( ; length > 0 ; data++, length--)
	{
		s1 = (s1 + *data) % 65521;
		s2 = (s2 + s1)    % 65521;
	}

	*crc = (s2 << 16) | s1;
}

void Adler32_Finish(u32_t *crc)
{
	/* nothing to do */
}

} // namespace ajbsp

//--- editor settings ---
// vi:ts=4:sw=4:noexpandtab
