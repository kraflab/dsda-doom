//------------------------------------------------------------------------
//
//  AJ-BSP  Copyright (C) 2000-2018  Andrew Apted, et al
//          Copyright (C) 1994-1998  Colin Reed
//          Copyright (C) 1997-1998  Lee Killough
//
//  Originally based on the program 'BSP', version 2.3.
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

#ifndef __AJBSP_BSP_H__
#define __AJBSP_BSP_H__

#define AJBSP_VERSION  "1.07"

//
// Node Build Information Structure
//

#define SPLIT_COST_MIN       1
#define SPLIT_COST_DEFAULT  11
#define SPLIT_COST_MAX      32

class buildinfo_t
{
public:
	// use a faster method to pick nodes
	bool fast;

	// create GL Nodes?
	bool gl_nodes;

	// when these two are false, they create an empty lump
	bool do_blockmap;
	bool do_reject;

	bool force_v5;
	bool force_xnod;
	bool force_compress;	// NOTE: only supported when HAVE_ZLIB is defined

	// the GUI can set this to tell the node builder to stop
	bool cancelled;

	int split_cost;

	// this affects how some messages are shown
	int verbosity;

	// from here on, various bits of internal state
	int total_warnings;
	int total_minor_issues;

public:
	buildinfo_t() :
		fast(false),

		gl_nodes(true),

		do_blockmap(true),
		do_reject  (true),

		force_v5(false),
		force_xnod(false),
		force_compress(false),

		cancelled(false),

		split_cost(SPLIT_COST_DEFAULT),
		verbosity(0),

		total_warnings(0),
		total_minor_issues(0)
	{ }

	~buildinfo_t()
	{ }

public:
	virtual void Print(int level, const char *msg, ...) = 0;
	virtual void Debug(const char *msg, ...) = 0;
	virtual void ShowMap(const char *name) = 0;
	virtual void FatalError(const char *fmt, ...) = 0;
};


typedef enum
{
	// everything went peachy keen
	BUILD_OK = 0,

	// building was cancelled
	BUILD_Cancelled,

	// when saving the map, one or more lumps overflowed
	BUILD_LumpOverflow
}
build_result_e;


namespace ajbsp
{

// set the build information.  must be done before anything else.
void SetInfo(buildinfo_t *info);

// attempt to open a wad.  on failure, the FatalError method in the
// buildinfo_t interface is called.
void OpenInputWad(const char *filename);

// attempt to open a wad.  on failure, the FatalError method in the
// buildinfo_t interface is called.
void OpenOutputWad(const char *filename);

// attempt to open a wad.  on failure, the FatalError method in the
// buildinfo_t interface is called.
void OpenBothWad(const char *filename);

// close a previously opened wad.
void CloseWad();

// create/finish an XWA file
void CreateXWA(const char *filename);
void FinishXWA();

// give the number of levels detected in the wad.
int LevelsInWad();
int LevelsInOutputWad();

// retrieve the name of a particular level.
const char *GetLevelName(int lev_idx);
const char* GetOutputLevelName(int lev_idx);

// build the nodes of a particular level.  if cancelled, returns the
// BUILD_Cancelled result and the wad is unchanged.  otherwise the wad
// is updated to store the new lumps and returns either BUILD_OK or
// BUILD_LumpOverflow if some limits were exceeded.
build_result_e BuildLevel(int lev_idx);


}  // namespace ajbsp


#endif /* __AJBSP_BSP_H__ */

//--- editor settings ---
// vi:ts=4:sw=4:noexpandtab
