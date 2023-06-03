//------------------------------------------------------------------------
//  WAD Reading / Writing
//------------------------------------------------------------------------
//
//  AJ-BSP  Copyright (C) 2001-2018  Andrew Apted
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

#ifndef __AJBSP_WAD_H__
#define __AJBSP_WAD_H__

namespace ajbsp
{

class Wad_file;


typedef enum
{
	MAPF_INVALID = 0,

	MAPF_Doom,
	MAPF_Hexen,
	MAPF_UDMF

} map_format_e;


class Lump_c
{
friend class Wad_file;

private:
	Wad_file *parent;

	char *name;

	int l_start;
	int l_length;

	// constructor is private
	Lump_c(Wad_file *_par, const char *_name, int _start, int _len);
	Lump_c(Wad_file *_par, const struct raw_wad_entry_s *entry);

	void MakeEntry(struct raw_wad_entry_s *entry);

public:
	~Lump_c();

	const char *Name() const { return name; }
	int Length() const { return l_length; }

	// do not call this directly, use Wad_file::RenameLump()
	void Rename(const char *new_name);

	// attempt to seek to a position within the lump (default is
	// the beginning).  Returns true if OK, false on error.
	bool Seek(int offset);

	// read some data from the lump, returning true if OK.
	bool Read(void *data, int len);

	// read a line of text, returns true if OK, false on EOF
	bool GetLine(char *buffer, size_t buf_size);

	// write some data to the lump.  Only the lump which had just
	// been created with Wad_file::AddLump() or RecreateLump() can be
	// written to.
	bool Write(const void *data, int len);

	// write some text to the lump
	void Printf(const char *msg, ...);

	// mark the lump as finished (after writing data to it).
	bool Finish();

	// predicate for std::sort()
	struct offset_CMP_pred
	{
		inline bool operator() (const Lump_c * A, const Lump_c * B) const
		{
			return A->l_start < B->l_start;
		}
	};

private:
	// deliberately don't implement these
	Lump_c(const Lump_c& other);
	Lump_c& operator= (const Lump_c& other);
};


//------------------------------------------------------------------------


class Wad_file
{
friend class Lump_c;
friend void W_LoadFlats();
friend void W_LoadTextures_TX_START(Wad_file *wf);

private:
	char *filename;

	char mode;  // mode value passed to ::Open()

	FILE * fp;

	char kind;  // 'P' for PWAD, 'I' for IWAD

	// zero means "currently unknown", which only occurs after a
	// call to BeginWrite() and before any call to AddLump() or
	// the finalizing EndWrite().
	int total_size;

	std::vector<Lump_c *> directory;

	int dir_start;
	int dir_count;

	// these are lump indices (into 'directory' vector)
	std::vector<int> levels;
	std::vector<int> patches;
	std::vector<int> sprites;
	std::vector<int> flats;
	std::vector<int> tx_tex;

	bool begun_write;
	int  begun_max_size;

	// when >= 0, the next added lump is placed _before_ this
	int insert_point;

	// constructor is private
	Wad_file(const char *_name, char _mode, FILE * _fp);

public:
	~Wad_file();

	// open a wad file.
	//
	// mode is similar to the fopen() function:
	//   'r' opens the wad for reading ONLY
	//   'a' opens the wad for appending (read and write)
	//   'w' opens the wad for writing (i.e. create it)
	//
	// Note: if 'a' is used and the file is read-only, it will be
	//       silently opened in 'r' mode instead.
	//
	static Wad_file * Open(const char *filename, char mode = 'a');

	// check the given wad file exists and is a WAD file
	static bool Validate(const char *filename);

	const char *PathName() const { return filename; }
	bool IsReadOnly() const { return mode == 'r'; }

	int TotalSize() const { return total_size; }

	int NumLumps() const { return (int)directory.size(); }
	Lump_c * GetLump(int index);
	Lump_c * FindLump(const char *name);
	int FindLumpNum(const char *name);

	Lump_c * FindLumpInNamespace(const char *name, char group);

	int LevelCount() const { return (int)levels.size(); }
	int LevelHeader(int lev_num);
	int LevelLastLump(int lev_num);

	// these return a level number (0 .. count-1)
	int LevelFind(const char *name);
	int LevelFindByNumber(int number);
	int LevelFindFirst();

	// returns a lump index, -1 if not found
	int LevelLookupLump(int lev_num, const char *name);

	map_format_e LevelFormat(int lev_num);

	void  SortLevels();

	// check whether another program has modified this WAD, and return
	// either true or false.  We test for change in file size, change
	// in directory size or location, and directory contents (CRC).
	// [ NOT USED YET.... ]
	bool WasExternallyModified();

	// backup the current wad into the given filename.
	// returns true if successful, false on error.
	bool Backup(const char *new_filename);

	// all changes to the wad must occur between calls to BeginWrite()
	// and EndWrite() methods.  the on-disk wad directory may be trashed
	// during this period, it will be re-written by EndWrite().
	void BeginWrite();
	void EndWrite();

	// change name of a lump (can be a level marker too)
	void RenameLump(int index, const char *new_name);

	// remove the given lump(s)
	// this will change index numbers on existing lumps
	// (previous results of FindLumpNum or LevelHeader are invalidated).
	void RemoveLumps(int index, int count = 1);

	// this removes the level marker PLUS all associated level lumps
	// which follow it.
	void RemoveLevel(int lev_num);

	// removes any GL-Nodes lumps that are associated with the given level.
	void RemoveGLNodes(int lev_num);

	// removes any ZNODES lump from a UDMF level.
	void RemoveZNodes(int lev_num);

	// insert a new lump.
	// The second form is for a level marker.
	// The 'max_size' parameter (if >= 0) specifies the most data
	// you will write into the lump -- writing more will corrupt
	// something else in the WAD.
	Lump_c * AddLump (const char *name, int max_size = -1);
	Lump_c * AddLevel(const char *name, int max_size = -1, int *lev_num = NULL);

	// setup lump to write new data to it.
	// the old contents are lost.
	void RecreateLump(Lump_c *lump, int max_size = -1);

	// set the insertion point -- the next lump will be added *before*
	// this index, and it will be incremented so that a sequence of
	// AddLump() calls produces lumps in the same order.
	//
	// passing a negative value or invalid index will reset the
	// insertion point -- future lumps get added at the END.
	// RemoveLumps(), RemoveLevel() and EndWrite() also reset it.
	void InsertPoint(int index = -1);

private:
	static Wad_file * Create(const char *filename, char mode);

	// read the existing directory.
	void ReadDirectory();

	void DetectLevels();
	void ProcessNamespaces();

	// look at all the lumps and determine the lowest offset from
	// start of file where we can write new data.  The directory itself
	// is ignored for this.
	int HighWaterMark();

	// look at all lumps in directory and determine the lowest offset
	// where a lump of the given length will fit.  Returns same as
	// HighWaterMark() when no largest gaps exist.  The directory itself
	// is ignored since it will be re-written at EndWrite().
	int FindFreeSpace(int length);

	// find a place (possibly at end of WAD) where we can write some
	// data of max_size (-1 means unlimited), and seek to that spot
	// (possibly writing some padding zeros -- the difference should
	// be no more than a few bytes).  Returns new position.
	int PositionForWrite(int max_size = -1);

	bool FinishLump(int final_size);
	int  WritePadding(int count);

	// write the new directory, updating the dir_xxx variables
	// (including the CRC).
	void WriteDirectory();

	void FixGroup(std::vector<int>& group, int index, int num_added, int num_removed);

private:
	// deliberately don't implement these
	Wad_file(const Wad_file& other);
	Wad_file& operator= (const Wad_file& other);

private:
	// predicate for sorting the levels[] vector
	struct level_name_CMP_pred
	{
	private:
		Wad_file *wad;

	public:
		level_name_CMP_pred(Wad_file * _w) : wad(_w)
		{ }

		inline bool operator() (const int A, const int B) const
		{
			const Lump_c *L1 = wad->directory[A];
			const Lump_c *L2 = wad->directory[B];

			return (strcmp(L1->Name(), L2->Name()) < 0);
		}
	};
};


} // namespace ajbsp

#endif  /* __AJBSP_WAD_H__ */

//--- editor settings ---
// vi:ts=4:sw=4:noexpandtab
