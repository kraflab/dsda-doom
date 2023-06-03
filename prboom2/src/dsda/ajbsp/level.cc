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

#include "system.h"
#include "local.h"
#include "parse.h"
#include "raw_def.h"
#include "utility.h"
#include "wad.h"

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

#define DEBUG_BLOCKMAP  0
#define DEBUG_REJECT    0

#define DEBUG_LOAD      0
#define DEBUG_BSP       0


namespace ajbsp
{

Wad_file * cur_wad;
Wad_file * out_wad;
Wad_file * xwa_wad;


static int block_x, block_y;
static int block_w, block_h;
static int block_count;

static int block_mid_x = 0;
static int block_mid_y = 0;

static u16_t ** block_lines;

static u16_t *block_ptrs;
static u16_t *block_dups;

static int block_compression;
static int block_overflowed;

#define BLOCK_LIMIT  16000

#define DUMMY_DUP  0xFFFF


void GetBlockmapBounds(int *x, int *y, int *w, int *h)
{
	*x = block_x; *y = block_y;
	*w = block_w; *h = block_h;
}


int CheckLinedefInsideBox(int xmin, int ymin, int xmax, int ymax,
		int x1, int y1, int x2, int y2)
{
	int count = 2;
	int tmp;

	for (;;)
	{
		if (y1 > ymax)
		{
			if (y2 > ymax)
				return false;

			x1 = x1 + (int) ((x2-x1) * (double)(ymax-y1) / (double)(y2-y1));
			y1 = ymax;

			count = 2;
			continue;
		}

		if (y1 < ymin)
		{
			if (y2 < ymin)
				return false;

			x1 = x1 + (int) ((x2-x1) * (double)(ymin-y1) / (double)(y2-y1));
			y1 = ymin;

			count = 2;
			continue;
		}

		if (x1 > xmax)
		{
			if (x2 > xmax)
				return false;

			y1 = y1 + (int) ((y2-y1) * (double)(xmax-x1) / (double)(x2-x1));
			x1 = xmax;

			count = 2;
			continue;
		}

		if (x1 < xmin)
		{
			if (x2 < xmin)
				return false;

			y1 = y1 + (int) ((y2-y1) * (double)(xmin-x1) / (double)(x2-x1));
			x1 = xmin;

			count = 2;
			continue;
		}

		count--;

		if (count == 0)
			break;

		// swap end points
		tmp=x1;  x1=x2;  x2=tmp;
		tmp=y1;  y1=y2;  y2=tmp;
	}

	// linedef touches block
	return true;
}


/* ----- create blockmap ------------------------------------ */

#define BK_NUM    0
#define BK_MAX    1
#define BK_XOR    2
#define BK_FIRST  3

#define BK_QUANTUM  32

static void BlockAdd(int blk_num, int line_index)
{
	u16_t *cur = block_lines[blk_num];

#if DEBUG_BLOCKMAP
	cur_info->Debug("Block %d has line %d\n", blk_num, line_index);
#endif

	if (blk_num < 0 || blk_num >= block_count)
		BugError("BlockAdd: bad block number %d\n", blk_num);

	if (! cur)
	{
		// create empty block
		block_lines[blk_num] = cur = (u16_t *)UtilCalloc(BK_QUANTUM * sizeof(u16_t));
		cur[BK_NUM] = 0;
		cur[BK_MAX] = BK_QUANTUM;
		cur[BK_XOR] = 0x1234;
	}

	if (BK_FIRST + cur[BK_NUM] == cur[BK_MAX])
	{
		// no more room, so allocate some more...
		cur[BK_MAX] += BK_QUANTUM;

		block_lines[blk_num] = cur = (u16_t *)UtilRealloc(cur, cur[BK_MAX] * sizeof(u16_t));
	}

	// compute new checksum
	cur[BK_XOR] = (u16_t) (((cur[BK_XOR] << 4) | (cur[BK_XOR] >> 12)) ^ line_index);

	cur[BK_FIRST + cur[BK_NUM]] = LE_U16(line_index);
	cur[BK_NUM]++;
}


static void BlockAddLine(const linedef_t *L)
{
	int x1 = (int) L->start->x;
	int y1 = (int) L->start->y;
	int x2 = (int) L->end->x;
	int y2 = (int) L->end->y;

	int bx1 = (std::min(x1,x2) - block_x) / 128;
	int by1 = (std::min(y1,y2) - block_y) / 128;
	int bx2 = (std::max(x1,x2) - block_x) / 128;
	int by2 = (std::max(y1,y2) - block_y) / 128;

	int bx, by;
	int line_index = L->index;

#if DEBUG_BLOCKMAP
	cur_info->Debug("BlockAddLine: %d (%d,%d) -> (%d,%d)\n", line_index,
			x1, y1, x2, y2);
#endif

	// handle truncated blockmaps
	if (bx1 < 0) bx1 = 0;
	if (by1 < 0) by1 = 0;
	if (bx2 >= block_w) bx2 = block_w - 1;
	if (by2 >= block_h) by2 = block_h - 1;

	if (bx2 < bx1 || by2 < by1)
		return;

	// handle simple case #1: completely horizontal
	if (by1 == by2)
	{
		for (bx=bx1 ; bx <= bx2 ; bx++)
		{
			int blk_num = by1 * block_w + bx;
			BlockAdd(blk_num, line_index);
		}
		return;
	}

	// handle simple case #2: completely vertical
	if (bx1 == bx2)
	{
		for (by=by1 ; by <= by2 ; by++)
		{
			int blk_num = by * block_w + bx1;
			BlockAdd(blk_num, line_index);
		}
		return;
	}

	// handle the rest (diagonals)

	for (by=by1 ; by <= by2 ; by++)
	for (bx=bx1 ; bx <= bx2 ; bx++)
	{
		int blk_num = by * block_w + bx;

		int minx = block_x + bx * 128;
		int miny = block_y + by * 128;
		int maxx = minx + 127;
		int maxy = miny + 127;

		if (CheckLinedefInsideBox(minx, miny, maxx, maxy, x1, y1, x2, y2))
		{
			BlockAdd(blk_num, line_index);
		}
	}
}


static void CreateBlockmap(void)
{
	block_lines = (u16_t **) UtilCalloc(block_count * sizeof(u16_t *));

	for (int i=0 ; i < num_linedefs ; i++)
	{
		const linedef_t *L = lev_linedefs[i];

		// ignore zero-length lines
		if (L->zero_len)
			continue;

		BlockAddLine(L);
	}
}


static int BlockCompare(const void *p1, const void *p2)
{
	int blk_num1 = ((const u16_t *) p1)[0];
	int blk_num2 = ((const u16_t *) p2)[0];

	const u16_t *A = block_lines[blk_num1];
	const u16_t *B = block_lines[blk_num2];

	if (A == B)
		return 0;

	if (A == NULL) return -1;
	if (B == NULL) return +1;

	if (A[BK_NUM] != B[BK_NUM])
	{
		return A[BK_NUM] - B[BK_NUM];
	}

	if (A[BK_XOR] != B[BK_XOR])
	{
		return A[BK_XOR] - B[BK_XOR];
	}

	return memcmp(A+BK_FIRST, B+BK_FIRST, A[BK_NUM] * sizeof(u16_t));
}


static void CompressBlockmap(void)
{
	int i;
	int cur_offset;
#if DEBUG_BLOCKMAP
	int dup_count=0;
#endif

	int orig_size, new_size;

	block_ptrs = (u16_t *)UtilCalloc(block_count * sizeof(u16_t));
	block_dups = (u16_t *)UtilCalloc(block_count * sizeof(u16_t));

	// sort duplicate-detecting array.  After the sort, all duplicates
	// will be next to each other.  The duplicate array gives the order
	// of the blocklists in the BLOCKMAP lump.

	for (i=0 ; i < block_count ; i++)
		block_dups[i] = (u16_t) i;

	qsort(block_dups, block_count, sizeof(u16_t), BlockCompare);

	// scan duplicate array and build up offset array

	cur_offset = 4 + block_count + 2;

	orig_size = 4 + block_count;
	new_size  = cur_offset;

	for (i=0 ; i < block_count ; i++)
	{
		int blk_num = block_dups[i];
		int count;

		// empty block ?
		if (block_lines[blk_num] == NULL)
		{
			block_ptrs[blk_num] = (u16_t) (4 + block_count);
			block_dups[i] = DUMMY_DUP;

			orig_size += 2;
			continue;
		}

		count = 2 + block_lines[blk_num][BK_NUM];

		// duplicate ?  Only the very last one of a sequence of duplicates
		// will update the current offset value.

		if (i+1 < block_count && BlockCompare(block_dups + i, block_dups + i+1) == 0)
		{
			block_ptrs[blk_num] = (u16_t) cur_offset;
			block_dups[i] = DUMMY_DUP;

			// free the memory of the duplicated block
			UtilFree(block_lines[blk_num]);
			block_lines[blk_num] = NULL;

#if DEBUG_BLOCKMAP
			dup_count++;
#endif

			orig_size += count;
			continue;
		}

		// OK, this block is either the last of a series of duplicates, or
		// just a singleton.

		block_ptrs[blk_num] = (u16_t) cur_offset;

		cur_offset += count;

		orig_size += count;
		new_size  += count;
	}

	if (cur_offset > 65535)
	{
		block_overflowed = true;
		return;
	}

#if DEBUG_BLOCKMAP
	cur_info->Debug("Blockmap: Last ptr = %d  duplicates = %d\n",
			cur_offset, dup_count);
#endif

	block_compression = (orig_size - new_size) * 100 / orig_size;

	// there's a tiny chance of new_size > orig_size
	if (block_compression < 0)
		block_compression = 0;
}


static int CalcBlockmapSize()
{
	// compute size of final BLOCKMAP lump.
	// it does not need to be exact, but it *does* need to be bigger
	// (or equal) to the actual size of the lump.

	// header + null_block + a bit extra
	int size = 20;

	// the pointers (offsets to the line lists)
	size = size + block_count * 2;

	// add size of each block
	for (int i=0 ; i < block_count ; i++)
	{
		int blk_num = block_dups[i];

		// ignore duplicate or empty blocks
		if (blk_num == DUMMY_DUP)
			continue;

		u16_t *blk = block_lines[blk_num];
		SYS_ASSERT(blk);

		size += (1 + (int)(blk[BK_NUM]) + 1) * 2;
	}

	return size;
}


static void WriteBlockmap(void)
{
	int i;

	int max_size = CalcBlockmapSize();

	Lump_c *lump = CreateLevelLump("BLOCKMAP", max_size);

	u16_t null_block[2] = { 0x0000, 0xFFFF };
	u16_t m_zero = 0x0000;
	u16_t m_neg1 = 0xFFFF;

	// fill in header
	raw_blockmap_header_t header;

	header.x_origin = LE_U16(block_x);
	header.y_origin = LE_U16(block_y);
	header.x_blocks = LE_U16(block_w);
	header.y_blocks = LE_U16(block_h);

	lump->Write(&header, sizeof(header));

	// handle pointers
	for (i=0 ; i < block_count ; i++)
	{
		u16_t ptr = LE_U16(block_ptrs[i]);

		if (ptr == 0)
			BugError("WriteBlockmap: offset %d not set.\n", i);

		lump->Write(&ptr, sizeof(u16_t));
	}

	// add the null block which *all* empty blocks will use
	lump->Write(null_block, sizeof(null_block));

	// handle each block list
	for (i=0 ; i < block_count ; i++)
	{
		int blk_num = block_dups[i];

		// ignore duplicate or empty blocks
		if (blk_num == DUMMY_DUP)
			continue;

		u16_t *blk = block_lines[blk_num];
		SYS_ASSERT(blk);

		lump->Write(&m_zero, sizeof(u16_t));
		lump->Write(blk + BK_FIRST, blk[BK_NUM] * sizeof(u16_t));
		lump->Write(&m_neg1, sizeof(u16_t));
	}

	lump->Finish();
}


static void FreeBlockmap(void)
{
	for (int i=0 ; i < block_count ; i++)
	{
		if (block_lines[i])
			UtilFree(block_lines[i]);
	}

	UtilFree(block_lines);
	UtilFree(block_ptrs);
	UtilFree(block_dups);
}


static void FindBlockmapLimits(bbox_t *bbox)
{
	double mid_x = 0;
	double mid_y = 0;

	bbox->minx = bbox->miny = SHRT_MAX;
	bbox->maxx = bbox->maxy = SHRT_MIN;

	for (int i=0 ; i < num_linedefs ; i++)
	{
		const linedef_t *L = lev_linedefs[i];

		if (! L->zero_len)
		{
			double x1 = L->start->x;
			double y1 = L->start->y;
			double x2 = L->end->x;
			double y2 = L->end->y;

			int lx = (int)floor(std::min(x1, x2));
			int ly = (int)floor(std::min(y1, y2));
			int hx = (int)ceil (std::max(x1, x2));
			int hy = (int)ceil (std::max(y1, y2));

			if (lx < bbox->minx) bbox->minx = lx;
			if (ly < bbox->miny) bbox->miny = ly;
			if (hx > bbox->maxx) bbox->maxx = hx;
			if (hy > bbox->maxy) bbox->maxy = hy;

			// compute middle of cluster
			mid_x += (lx + hx) / 2;
			mid_y += (ly + hy) / 2;
		}
	}

	if (num_linedefs > 0)
	{
		block_mid_x = I_ROUND(mid_x / (double)num_linedefs);
		block_mid_y = I_ROUND(mid_y / (double)num_linedefs);
	}

#if DEBUG_BLOCKMAP
	cur_info->Debug("Blockmap lines centered at (%d,%d)\n", block_mid_x, block_mid_y);
#endif
}


void InitBlockmap()
{
	bbox_t map_bbox;

	// find limits of linedefs, and store as map limits
	FindBlockmapLimits(&map_bbox);

	cur_info->Print(2, "    Map limits: (%d,%d) to (%d,%d)\n",
			map_bbox.minx, map_bbox.miny,
			map_bbox.maxx, map_bbox.maxy);

	block_x = map_bbox.minx - (map_bbox.minx & 0x7);
	block_y = map_bbox.miny - (map_bbox.miny & 0x7);

	block_w = ((map_bbox.maxx - block_x) / 128) + 1;
	block_h = ((map_bbox.maxy - block_y) / 128) + 1;

	block_count = block_w * block_h;
}


void PutBlockmap()
{
	if (! cur_info->do_blockmap || num_linedefs == 0)
	{
		// just create an empty blockmap lump
		CreateLevelLump("BLOCKMAP")->Finish();
		return;
	}

	block_overflowed = false;

	// initial phase: create internal blockmap containing the index of
	// all lines in each block.

	CreateBlockmap();

	// -AJA- second phase: compress the blockmap.  We do this by sorting
	//       the blocks, which is a typical way to detect duplicates in
	//       a large list.  This also detects BLOCKMAP overflow.

	CompressBlockmap();

	// final phase: write it out in the correct format

	if (block_overflowed)
	{
		// leave an empty blockmap lump
		CreateLevelLump("BLOCKMAP")->Finish();

		Warning("Blockmap overflowed (lump will be empty)\n");
	}
	else
	{
		WriteBlockmap();

		cur_info->Print(2, "    Blockmap size: %dx%d (compression: %d%%)\n",
				block_w, block_h, block_compression);
	}

	FreeBlockmap();
}


//------------------------------------------------------------------------
// REJECT : Generate the reject table
//------------------------------------------------------------------------


static u8_t *rej_matrix;
static int   rej_total_size;	// in bytes


//
// Allocate the matrix, init sectors into individual groups.
//
static void Reject_Init()
{
	rej_total_size = (num_sectors * num_sectors + 7) / 8;

	rej_matrix = new u8_t[rej_total_size];
	memset(rej_matrix, 0, rej_total_size);

	for (int i=0 ; i < num_sectors ; i++)
	{
		sector_t *sec = lev_sectors[i];

		sec->rej_group = i;
		sec->rej_next = sec->rej_prev = sec;
	}
}


static void Reject_Free()
{
	delete[] rej_matrix;
	rej_matrix = NULL;
}


//
// Algorithm: Initially all sectors are in individual groups.
// Now we scan the linedef list.  For each two-sectored line,
// merge the two sector groups into one.  That's it !
//
static void Reject_GroupSectors()
{
	for (int i=0 ; i < num_linedefs ; i++)
	{
		const linedef_t *line = lev_linedefs[i];

		if (! line->right || ! line->left)
			continue;

		sector_t *sec1 = line->right->sector;
		sector_t *sec2 = line->left->sector;
		sector_t *tmp;

		if (! sec1 || ! sec2 || sec1 == sec2)
			continue;

		// already in the same group ?
		if (sec1->rej_group == sec2->rej_group)
			continue;

		// swap sectors so that the smallest group is added to the biggest
		// group.  This is based on the assumption that sector numbers in
		// wads will generally increase over the set of linedefs, and so
		// (by swapping) we'll tend to add small groups into larger groups,
		// thereby minimising the updates to 'rej_group' fields when merging.

		if (sec1->rej_group > sec2->rej_group)
		{
			tmp = sec1; sec1 = sec2; sec2 = tmp;
		}

		// update the group numbers in the second group

		sec2->rej_group = sec1->rej_group;

		for (tmp=sec2->rej_next ; tmp != sec2 ; tmp=tmp->rej_next)
			tmp->rej_group = sec1->rej_group;

		// merge 'em baby...

		sec1->rej_next->rej_prev = sec2;
		sec2->rej_next->rej_prev = sec1;

		tmp = sec1->rej_next;
		sec1->rej_next = sec2->rej_next;
		sec2->rej_next = tmp;
	}
}


#if DEBUG_REJECT
static void Reject_DebugGroups()
{
	// Note: this routine is destructive to the group numbers

	for (int i=0 ; i < num_sectors ; i++)
	{
		sector_t *sec = lev_sectors[i];
		sector_t *tmp;

		int group = sec->rej_group;
		int num = 0;

		if (group < 0)
			continue;

		sec->rej_group = -1;
		num++;

		for (tmp=sec->rej_next ; tmp != sec ; tmp=tmp->rej_next)
		{
			tmp->rej_group = -1;
			num++;
		}

		cur_info->Debug("Group %d  Sectors %d\n", group, num);
	}
}
#endif


static void Reject_ProcessSectors()
{
	for (int view=0 ; view < num_sectors ; view++)
	{
		for (int target=0 ; target < view ; target++)
		{
			sector_t *view_sec = lev_sectors[view];
			sector_t *targ_sec = lev_sectors[target];

			if (view_sec->rej_group == targ_sec->rej_group)
				continue;

			// for symmetry, do both sides at same time

			int p1 = view * num_sectors + target;
			int p2 = target * num_sectors + view;

			rej_matrix[p1 >> 3] |= (1 << (p1 & 7));
			rej_matrix[p2 >> 3] |= (1 << (p2 & 7));
		}
	}
}


static void Reject_WriteLump()
{
	Lump_c *lump = CreateLevelLump("REJECT", rej_total_size);

	lump->Write(rej_matrix, rej_total_size);
	lump->Finish();
}


//
// For now we only do very basic reject processing, limited to
// determining all isolated groups of sectors (islands that are
// surrounded by void space).
//
void PutReject()
{
	if (! cur_info->do_reject || num_sectors == 0)
	{
		// just create an empty reject lump
		CreateLevelLump("REJECT")->Finish();
		return;
	}

	Reject_Init();
	Reject_GroupSectors();
	Reject_ProcessSectors();

#if DEBUG_REJECT
	Reject_DebugGroups();
#endif

	Reject_WriteLump();
	Reject_Free();

	cur_info->Print(2, "    Reject size: %d\n", rej_total_size);
}


//------------------------------------------------------------------------
// LEVEL : Level structure read/write functions.
//------------------------------------------------------------------------


// Note: ZDoom format support based on code (C) 2002,2003 Randy Heit


// per-level variables

const char *lev_current_name;

int lev_current_idx;
int lev_current_start;

int lev_current_output_idx;

map_format_e lev_format;

bool lev_force_v5;
bool lev_force_xnod;

bool lev_long_name;
bool lev_overflows;


// objects of loaded level, and stuff we've built
std::vector<vertex_t *>  lev_vertices;
std::vector<linedef_t *> lev_linedefs;
std::vector<sidedef_t *> lev_sidedefs;
std::vector<sector_t *>  lev_sectors;
std::vector<thing_t *>   lev_things;

std::vector<seg_t *>     lev_segs;
std::vector<subsec_t *>  lev_subsecs;
std::vector<node_t *>    lev_nodes;
std::vector<walltip_t *> lev_walltips;

int num_old_vert = 0;
int num_new_vert = 0;
int num_real_lines = 0;


/* ----- allocation routines ---------------------------- */

vertex_t *NewVertex()
{
	vertex_t *V = (vertex_t *) UtilCalloc(sizeof(vertex_t));
	V->index = (int)lev_vertices.size();
	lev_vertices.push_back(V);
	return V;
}

linedef_t *NewLinedef()
{
	linedef_t *L = (linedef_t *) UtilCalloc(sizeof(linedef_t));
	L->index = (int)lev_linedefs.size();
	lev_linedefs.push_back(L);
	return L;
}

sidedef_t *NewSidedef()
{
	sidedef_t *S = (sidedef_t *) UtilCalloc(sizeof(sidedef_t));
	S->index = (int)lev_sidedefs.size();
	lev_sidedefs.push_back(S);
	return S;
}

sector_t *NewSector()
{
	sector_t *S = (sector_t *) UtilCalloc(sizeof(sector_t));
	S->index = (int)lev_sectors.size();
	lev_sectors.push_back(S);
	return S;
}

thing_t *NewThing()
{
	thing_t *T = (thing_t *) UtilCalloc(sizeof(thing_t));
	T->index = (int)lev_things.size();
	lev_things.push_back(T);
	return T;
}

seg_t *NewSeg()
{
	seg_t *S = (seg_t *) UtilCalloc(sizeof(seg_t));
	lev_segs.push_back(S);
	return S;
}

subsec_t *NewSubsec()
{
	subsec_t *S = (subsec_t *) UtilCalloc(sizeof(subsec_t));
	lev_subsecs.push_back(S);
	return S;
}

node_t *NewNode()
{
	node_t *N = (node_t *) UtilCalloc(sizeof(node_t));
	lev_nodes.push_back(N);
	return N;
}

walltip_t *NewWallTip()
{
	walltip_t *WT = (walltip_t *) UtilCalloc(sizeof(walltip_t));
	lev_walltips.push_back(WT);
	return WT;
}


/* ----- free routines ---------------------------- */

void FreeVertices()
{
	for (unsigned int i = 0 ; i < lev_vertices.size() ; i++)
		UtilFree((void *) lev_vertices[i]);

	lev_vertices.clear();
}

void FreeLinedefs()
{
	for (unsigned int i = 0 ; i < lev_linedefs.size() ; i++)
		UtilFree((void *) lev_linedefs[i]);

	lev_linedefs.clear();
}

void FreeSidedefs()
{
	for (unsigned int i = 0 ; i < lev_sidedefs.size() ; i++)
		UtilFree((void *) lev_sidedefs[i]);

	lev_sidedefs.clear();
}

void FreeSectors()
{
	for (unsigned int i = 0 ; i < lev_sectors.size() ; i++)
		UtilFree((void *) lev_sectors[i]);

	lev_sectors.clear();
}

void FreeThings()
{
	for (unsigned int i = 0 ; i < lev_things.size() ; i++)
		UtilFree((void *) lev_things[i]);

	lev_things.clear();
}

void FreeSegs()
{
	for (unsigned int i = 0 ; i < lev_segs.size() ; i++)
		UtilFree((void *) lev_segs[i]);

	lev_segs.clear();
}

void FreeSubsecs()
{
	for (unsigned int i = 0 ; i < lev_subsecs.size() ; i++)
		UtilFree((void *) lev_subsecs[i]);

	lev_subsecs.clear();
}

void FreeNodes()
{
	for (unsigned int i = 0 ; i < lev_nodes.size() ; i++)
		UtilFree((void *) lev_nodes[i]);

	lev_nodes.clear();
}

void FreeWallTips()
{
	for (unsigned int i = 0 ; i < lev_walltips.size() ; i++)
		UtilFree((void *) lev_walltips[i]);

	lev_walltips.clear();
}


/* ----- reading routines ------------------------------ */

static vertex_t *SafeLookupVertex(int num)
{
	if (num >= num_vertices)
		cur_info->FatalError("illegal vertex number #%d\n", num);

	return lev_vertices[num];
}

static sector_t *SafeLookupSector(u16_t num)
{
	if (num == 0xFFFF)
		return NULL;

	if (num >= num_sectors)
		cur_info->FatalError("illegal sector number #%d\n", (int)num);

	return lev_sectors[num];
}

static inline sidedef_t *SafeLookupSidedef(u16_t num)
{
	if (num == 0xFFFF)
		return NULL;

	// silently ignore illegal sidedef numbers
	if (num >= (unsigned int)num_sidedefs)
		return NULL;

	return lev_sidedefs[num];
}


void GetVertices()
{
	int count = 0;

	Lump_c *lump = FindLevelLump("VERTEXES");

	if (lump)
		count = lump->Length() / (int)sizeof(raw_vertex_t);

#if DEBUG_LOAD
	cur_info->Debug("GetVertices: num = %d\n", count);
#endif

	if (lump == NULL || count == 0)
		return;

	if (! lump->Seek(0))
		cur_info->FatalError("Error seeking to vertices.\n");

	for (int i = 0 ; i < count ; i++)
	{
		raw_vertex_t raw;

		if (! lump->Read(&raw, sizeof(raw)))
			cur_info->FatalError("Error reading vertices.\n");

		vertex_t *vert = NewVertex();

		vert->x = (double) LE_S16(raw.x);
		vert->y = (double) LE_S16(raw.y);
	}

	num_old_vert = num_vertices;
}


void GetSectors()
{
	int count = 0;

	Lump_c *lump = FindLevelLump("SECTORS");

	if (lump)
		count = lump->Length() / (int)sizeof(raw_sector_t);

	if (lump == NULL || count == 0)
		return;

	if (! lump->Seek(0))
		cur_info->FatalError("Error seeking to sectors.\n");

#if DEBUG_LOAD
	cur_info->Debug("GetSectors: num = %d\n", count);
#endif

	for (int i = 0 ; i < count ; i++)
	{
		raw_sector_t raw;

		if (! lump->Read(&raw, sizeof(raw)))
			cur_info->FatalError("Error reading sectors.\n");

		sector_t *sector = NewSector();

		(void) sector;
	}
}


void GetThings()
{
	int count = 0;

	Lump_c *lump = FindLevelLump("THINGS");

	if (lump)
		count = lump->Length() / (int)sizeof(raw_thing_t);

	if (lump == NULL || count == 0)
		return;

	if (! lump->Seek(0))
		cur_info->FatalError("Error seeking to things.\n");

#if DEBUG_LOAD
	cur_info->Debug("GetThings: num = %d\n", count);
#endif

	for (int i = 0 ; i < count ; i++)
	{
		raw_thing_t raw;

		if (! lump->Read(&raw, sizeof(raw)))
			cur_info->FatalError("Error reading things.\n");

		thing_t *thing = NewThing();

		thing->x    = LE_S16(raw.x);
		thing->y    = LE_S16(raw.y);
		thing->type = LE_U16(raw.type);
	}
}


void GetThingsHexen()
{
	int count = 0;

	Lump_c *lump = FindLevelLump("THINGS");

	if (lump)
		count = lump->Length() / (int)sizeof(raw_hexen_thing_t);

	if (lump == NULL || count == 0)
		return;

	if (! lump->Seek(0))
		cur_info->FatalError("Error seeking to things.\n");

#if DEBUG_LOAD
	cur_info->Debug("GetThingsHexen: num = %d\n", count);
#endif

	for (int i = 0 ; i < count ; i++)
	{
		raw_hexen_thing_t raw;

		if (! lump->Read(&raw, sizeof(raw)))
			cur_info->FatalError("Error reading things.\n");

		thing_t *thing = NewThing();

		thing->x    = LE_S16(raw.x);
		thing->y    = LE_S16(raw.y);
		thing->type = LE_U16(raw.type);
	}
}


void GetSidedefs()
{
	int count = 0;

	Lump_c *lump = FindLevelLump("SIDEDEFS");

	if (lump)
		count = lump->Length() / (int)sizeof(raw_sidedef_t);

	if (lump == NULL || count == 0)
		return;

	if (! lump->Seek(0))
		cur_info->FatalError("Error seeking to sidedefs.\n");

#if DEBUG_LOAD
	cur_info->Debug("GetSidedefs: num = %d\n", count);
#endif

	for (int i = 0 ; i < count ; i++)
	{
		raw_sidedef_t raw;

		if (! lump->Read(&raw, sizeof(raw)))
			cur_info->FatalError("Error reading sidedefs.\n");

		sidedef_t *side = NewSidedef();

		side->sector = SafeLookupSector(LE_S16(raw.sector));
	}
}


void GetLinedefs()
{
	int count = 0;

	Lump_c *lump = FindLevelLump("LINEDEFS");

	if (lump)
		count = lump->Length() / (int)sizeof(raw_linedef_t);

	if (lump == NULL || count == 0)
		return;

	if (! lump->Seek(0))
		cur_info->FatalError("Error seeking to linedefs.\n");

#if DEBUG_LOAD
	cur_info->Debug("GetLinedefs: num = %d\n", count);
#endif

	for (int i = 0 ; i < count ; i++)
	{
		raw_linedef_t raw;

		if (! lump->Read(&raw, sizeof(raw)))
			cur_info->FatalError("Error reading linedefs.\n");

		linedef_t *line;

		vertex_t *start = SafeLookupVertex(LE_U16(raw.start));
		vertex_t *end   = SafeLookupVertex(LE_U16(raw.end));

		start->is_used = true;
		  end->is_used = true;

		line = NewLinedef();

		line->start = start;
		line->end   = end;

		// check for zero-length line
		line->zero_len =
			(fabs(start->x - end->x) < DIST_EPSILON) &&
			(fabs(start->y - end->y) < DIST_EPSILON);

		line->type  = LE_U16(raw.type);
		u16_t flags = LE_U16(raw.flags);
		s16_t tag   = LE_S16(raw.tag);

		line->two_sided   = (flags & MLF_TwoSided) != 0;
		line->is_precious = (tag >= 900 && tag < 1000);

		line->right = SafeLookupSidedef(LE_U16(raw.right));
		line->left  = SafeLookupSidedef(LE_U16(raw.left));

		if (line->right || line->left)
			num_real_lines++;

		line->self_ref = (line->left && line->right &&
				(line->left->sector == line->right->sector));
	}
}


void GetLinedefsHexen()
{
	int count = 0;

	Lump_c *lump = FindLevelLump("LINEDEFS");

	if (lump)
		count = lump->Length() / (int)sizeof(raw_hexen_linedef_t);

	if (lump == NULL || count == 0)
		return;

	if (! lump->Seek(0))
		cur_info->FatalError("Error seeking to linedefs.\n");

#if DEBUG_LOAD
	cur_info->Debug("GetLinedefsHexen: num = %d\n", count);
#endif

	for (int i = 0 ; i < count ; i++)
	{
		raw_hexen_linedef_t raw;

		if (! lump->Read(&raw, sizeof(raw)))
			cur_info->FatalError("Error reading linedefs.\n");

		linedef_t *line;

		vertex_t *start = SafeLookupVertex(LE_U16(raw.start));
		vertex_t *end   = SafeLookupVertex(LE_U16(raw.end));

		start->is_used = true;
		  end->is_used = true;

		line = NewLinedef();

		line->start = start;
		line->end   = end;

		// check for zero-length line
		line->zero_len =
			(fabs(start->x - end->x) < DIST_EPSILON) &&
			(fabs(start->y - end->y) < DIST_EPSILON);

		line->type  = (u8_t) raw.type;
		u16_t flags = LE_U16(raw.flags);

		// -JL- Added missing twosided flag handling that caused a broken reject
		line->two_sided = (flags & MLF_TwoSided) != 0;

		line->right = SafeLookupSidedef(LE_U16(raw.right));
		line->left  = SafeLookupSidedef(LE_U16(raw.left));

		if (line->right || line->left)
			num_real_lines++;

		line->self_ref = (line->left && line->right &&
				(line->left->sector == line->right->sector));
	}
}


static inline int VanillaSegDist(const seg_t *seg)
{
	double lx = seg->side ? seg->linedef->end->x : seg->linedef->start->x;
	double ly = seg->side ? seg->linedef->end->y : seg->linedef->start->y;

	// use the "true" starting coord (as stored in the wad)
	double sx = round(seg->start->x);
	double sy = round(seg->start->y);

	return (int) floor(hypot(sx - lx, sy - ly) + 0.5);
}

static inline int VanillaSegAngle(const seg_t *seg)
{
	// compute the "true" delta
	double dx = round(seg->end->x) - round(seg->start->x);
	double dy = round(seg->end->y) - round(seg->start->y);

	double angle = ComputeAngle(dx, dy);

	if (angle < 0)
		angle += 360.0;

	int result = (int) floor(angle * 65536.0 / 360.0 + 0.5);

	return (result & 0xFFFF);
}


/* ----- UDMF reading routines ------------------------- */

#define UDMF_THING    1
#define UDMF_VERTEX   2
#define UDMF_SECTOR   3
#define UDMF_SIDEDEF  4
#define UDMF_LINEDEF  5

void ParseThingField(thing_t *thing, const std::string& key, token_kind_e kind, const std::string& value)
{
	if (key == "x")
		thing->x = LEX_Double(value);

	if (key == "y")
		thing->y = LEX_Double(value);

	if (key == "type")
		thing->type = LEX_Double(value);
}


void ParseVertexField(vertex_t *vertex, const std::string& key, token_kind_e kind, const std::string& value)
{
	if (key == "x")
		vertex->x = LEX_Double(value);

	if (key == "y")
		vertex->y = LEX_Double(value);
}


void ParseSectorField(sector_t *sector, const std::string& key, token_kind_e kind, const std::string& value)
{
	// nothing actually needed
}


void ParseSidedefField(sidedef_t *side, const std::string& key, token_kind_e kind, const std::string& value)
{
	if (key == "sector")
	{
		int num = LEX_Int(value);

		if (num < 0 || num >= num_sectors)
			cur_info->FatalError("illegal sector number #%d\n", (int)num);

		side->sector = lev_sectors[num];
	}
}


void ParseLinedefField(linedef_t *line, const std::string& key, token_kind_e kind, const std::string& value)
{
	if (key == "v1")
		line->start = SafeLookupVertex(LEX_Int(value));

	if (key == "v2")
		line->end = SafeLookupVertex(LEX_Int(value));

	if (key == "special")
		line->type = LEX_Int(value);

	if (key == "twosided")
		line->two_sided = LEX_Boolean(value);

	if (key == "sidefront")
	{
		int num = LEX_Int(value);

		if (num < 0 || num >= (int)num_sidedefs)
			line->right = NULL;
		else
			line->right = lev_sidedefs[num];
	}

	if (key == "sideback")
	{
		int num = LEX_Int(value);

		if (num < 0 || num >= (int)num_sidedefs)
			line->left = NULL;
		else
			line->left = lev_sidedefs[num];
	}
}


void ParseUDMF_Block(lexer_c& lex, int cur_type)
{
	vertex_t  * vertex = NULL;
	thing_t   * thing  = NULL;
	sector_t  * sector = NULL;
	sidedef_t * side   = NULL;
	linedef_t * line   = NULL;

	switch (cur_type)
	{
		case UDMF_VERTEX:  vertex = NewVertex();  break;
		case UDMF_THING:   thing  = NewThing();   break;
		case UDMF_SECTOR:  sector = NewSector();  break;
		case UDMF_SIDEDEF: side   = NewSidedef(); break;
		case UDMF_LINEDEF: line   = NewLinedef(); break;
		default: break;
	}

	for (;;)
	{
		if (lex.Match("}"))
			break;

		std::string key;
		std::string value;

		token_kind_e tok = lex.Next(key);

		if (tok == TOK_EOF)
			cur_info->FatalError("Malformed TEXTMAP lump: unclosed block\n");

		if (tok != TOK_Ident)
			cur_info->FatalError("Malformed TEXTMAP lump: missing key\n");

		if (! lex.Match("="))
			cur_info->FatalError("Malformed TEXTMAP lump: missing '='\n");

		tok = lex.Next(value);

		if (tok == TOK_EOF || tok == TOK_ERROR || value == "}")
			cur_info->FatalError("Malformed TEXTMAP lump: missing value\n");

		if (! lex.Match(";"))
			cur_info->FatalError("Malformed TEXTMAP lump: missing ';'\n");

		switch (cur_type)
		{
			case UDMF_VERTEX:  ParseVertexField (vertex, key, tok, value); break;
			case UDMF_THING:   ParseThingField  (thing,  key, tok, value); break;
			case UDMF_SECTOR:  ParseSectorField (sector, key, tok, value); break;
			case UDMF_SIDEDEF: ParseSidedefField(side,   key, tok, value); break;
			case UDMF_LINEDEF: ParseLinedefField(line,   key, tok, value); break;

			default: /* just skip it */ break;
		}
	}

	// validate stuff

	if (line != NULL)
	{
		if (line->start == NULL || line->end == NULL)
			cur_info->FatalError("Linedef #%d is missing a vertex!\n", line->index);

		if (line->right || line->left)
			num_real_lines++;

		line->self_ref = (line->left && line->right &&
				(line->left->sector == line->right->sector));
	}
}


void ParseUDMF_Pass(const std::string& data, int pass)
{
	// pass = 1 : vertices, sectors, things
	// pass = 2 : sidedefs
	// pass = 3 : linedefs

	lexer_c lex(data);

	for (;;)
	{
		std::string section;
		token_kind_e tok = lex.Next(section);

		if (tok == TOK_EOF)
			return;

		if (tok != TOK_Ident)
		{
			cur_info->FatalError("Malformed TEXTMAP lump.\n");
			return;
		}

		// ignore top-level assignments
		if (lex.Match("="))
		{
			lex.Next(section);
			if (! lex.Match(";"))
				cur_info->FatalError("Malformed TEXTMAP lump: missing ';'\n");
			continue;
		}

		if (! lex.Match("{"))
			cur_info->FatalError("Malformed TEXTMAP lump: missing '{'\n");

		int cur_type = 0;

		if (section == "thing")
		{
			if (pass == 1)
				cur_type = UDMF_THING;
		}
		else if (section == "vertex")
		{
			if (pass == 1)
				cur_type = UDMF_VERTEX;
		}
		else if (section == "sector")
		{
			if (pass == 1)
				cur_type = UDMF_SECTOR;
		}
		else if (section == "sidedef")
		{
			if (pass == 2)
				cur_type = UDMF_SIDEDEF;
		}
		else if (section == "linedef")
		{
			if (pass == 3)
				cur_type = UDMF_LINEDEF;
		}

		// process the block
		ParseUDMF_Block(lex, cur_type);
	}
}


void ParseUDMF()
{
	Lump_c *lump = FindLevelLump("TEXTMAP");

	if (lump == NULL || ! lump->Seek(0))
		cur_info->FatalError("Error finding TEXTMAP lump.\n");

	int remain = lump->Length();

	// load the lump into this string
	std::string data;

	while (remain > 0)
	{
		char buffer[4096];

		int want = std::min(remain, (int)sizeof(buffer));

		if (! lump->Read(buffer, want))
			cur_info->FatalError("Error reading TEXTMAP lump.\n");

		data.append(buffer, want);

		remain -= want;
	}

	// now parse it...

	// the UDMF spec does not require objects to be in a dependency order.
	// for example: sidedefs may occur *after* the linedefs which refer to
	// them.  hence we perform multiple passes over the TEXTMAP data.

	ParseUDMF_Pass(data, 1);
	ParseUDMF_Pass(data, 2);
	ParseUDMF_Pass(data, 3);

	num_old_vert = num_vertices;
}


/* ----- writing routines ------------------------------ */

static const u8_t *lev_v2_magic = (const u8_t *) "gNd2";
static const u8_t *lev_v5_magic = (const u8_t *) "gNd5";


void MarkOverflow(int flags)
{
	// flags are ignored

	lev_overflows = true;
}


void PutVertices(const char *name, int do_gl)
{
	int count, i;

	// this size is worst-case scenario
	int size = num_vertices * (int)sizeof(raw_vertex_t);

	Lump_c *lump = CreateLevelLump(name, size);

	for (i=0, count=0 ; i < num_vertices ; i++)
	{
		raw_vertex_t raw;

		const vertex_t *vert = lev_vertices[i];

		if ((do_gl ? 1 : 0) != (vert->is_new ? 1 : 0))
		{
			continue;
		}

		raw.x = LE_S16(I_ROUND(vert->x));
		raw.y = LE_S16(I_ROUND(vert->y));

		lump->Write(&raw, sizeof(raw));

		count++;
	}

	lump->Finish();

	if (count != (do_gl ? num_new_vert : num_old_vert))
		BugError("PutVertices miscounted (%d != %d)\n", count,
				do_gl ? num_new_vert : num_old_vert);

	if (! do_gl && count > 65534)
	{
		Failure("Number of vertices has overflowed.\n");
		MarkOverflow(LIMIT_VERTEXES);
	}
}


void PutGLVertices(int do_v5)
{
	int count, i;

	// this size is worst-case scenario
	int size = 4 + num_vertices * (int)sizeof(raw_v2_vertex_t);

	Lump_c *lump = CreateLevelLump("GL_VERT", size);

	if (do_v5)
		lump->Write(lev_v5_magic, 4);
	else
		lump->Write(lev_v2_magic, 4);

	for (i=0, count=0 ; i < num_vertices ; i++)
	{
		raw_v2_vertex_t raw;

		const vertex_t *vert = lev_vertices[i];

		if (! vert->is_new)
			continue;

		raw.x = LE_S32(I_ROUND(vert->x * 65536.0));
		raw.y = LE_S32(I_ROUND(vert->y * 65536.0));

		lump->Write(&raw, sizeof(raw));

		count++;
	}

	lump->Finish();

	if (count != num_new_vert)
		BugError("PutGLVertices miscounted (%d != %d)\n", count, num_new_vert);
}


static inline u16_t VertexIndex16Bit(const vertex_t *v)
{
	if (v->is_new)
		return (u16_t) (v->index | 0x8000U);

	return (u16_t) v->index;
}


static inline u32_t VertexIndex_V5(const vertex_t *v)
{
	if (v->is_new)
		return (u32_t) (v->index | 0x80000000U);

	return (u32_t) v->index;
}


static inline u32_t VertexIndex_XNOD(const vertex_t *v)
{
	if (v->is_new)
		return (u32_t) (num_old_vert + v->index);

	return (u32_t) v->index;
}


void PutSegs()
{
	// this size is worst-case scenario
	int size = num_segs * (int)sizeof(raw_seg_t);

	Lump_c *lump = CreateLevelLump("SEGS", size);

	for (int i=0 ; i < num_segs ; i++)
	{
		raw_seg_t raw;

		const seg_t *seg = lev_segs[i];

		raw.start   = LE_U16(VertexIndex16Bit(seg->start));
		raw.end     = LE_U16(VertexIndex16Bit(seg->end));
		raw.angle   = LE_U16(VanillaSegAngle(seg));
		raw.linedef = LE_U16(seg->linedef->index);
		raw.flip    = LE_U16(seg->side);
		raw.dist    = LE_U16(VanillaSegDist(seg));

		lump->Write(&raw, sizeof(raw));

#if DEBUG_BSP
		cur_info->Debug("PUT SEG: %04X  Vert %04X->%04X  Line %04X %s  "
				"Angle %04X  (%1.1f,%1.1f) -> (%1.1f,%1.1f)\n", seg->index,
				LE_U16(raw.start), LE_U16(raw.end), LE_U16(raw.linedef),
				seg->side ? "L" : "R", LE_U16(raw.angle),
				seg->start->x, seg->start->y, seg->end->x, seg->end->y);
#endif
	}

	lump->Finish();

	if (num_segs > 65534)
	{
		Failure("Number of segs has overflowed.\n");
		MarkOverflow(LIMIT_SEGS);
	}
}


void PutGLSegs_V2()
{
	// should not happen (we should have upgraded to V5)
	SYS_ASSERT(num_segs <= 65534);

	// this size is worst-case scenario
	int size = num_segs * (int)sizeof(raw_gl_seg_t);

	Lump_c *lump = CreateLevelLump("GL_SEGS", size);

	for (int i=0 ; i < num_segs ; i++)
	{
		raw_gl_seg_t raw;

		const seg_t *seg = lev_segs[i];

		raw.start = LE_U16(VertexIndex16Bit(seg->start));
		raw.end   = LE_U16(VertexIndex16Bit(seg->end));
		raw.side  = LE_U16(seg->side);

		if (seg->linedef != NULL)
			raw.linedef = LE_U16(seg->linedef->index);
		else
			raw.linedef = LE_U16(0xFFFF);

		if (seg->partner != NULL)
			raw.partner = LE_U16(seg->partner->index);
		else
			raw.partner = LE_U16(0xFFFF);

		lump->Write(&raw, sizeof(raw));

#if DEBUG_BSP
		cur_info->Debug("PUT GL SEG: %04X  Line %04X %s  Partner %04X  "
				"(%1.1f,%1.1f) -> (%1.1f,%1.1f)\n", seg->index, LE_U16(raw.linedef),
				seg->side ? "L" : "R", LE_U16(raw.partner),
				seg->start->x, seg->start->y, seg->end->x, seg->end->y);
#endif
	}

	lump->Finish();
}


void PutGLSegs_V5()
{
	// this size is worst-case scenario
	int size = num_segs * (int)sizeof(raw_v5_seg_t);

	Lump_c *lump = CreateLevelLump("GL_SEGS", size);

	for (int i=0 ; i < num_segs ; i++)
	{
		raw_v5_seg_t raw;

		const seg_t *seg = lev_segs[i];

		raw.start = LE_U32(VertexIndex_V5(seg->start));
		raw.end   = LE_U32(VertexIndex_V5(seg->end));
		raw.side  = LE_U16(seg->side);

		if (seg->linedef != NULL)
			raw.linedef = LE_U16(seg->linedef->index);
		else
			raw.linedef = LE_U16(0xFFFF);

		if (seg->partner != NULL)
			raw.partner = LE_U32(seg->partner->index);
		else
			raw.partner = LE_U32(0xFFFFFFFF);

		lump->Write(&raw, sizeof(raw));

#if DEBUG_BSP
		cur_info->Debug("PUT V3 SEG: %06X  Line %04X %s  Partner %06X  "
				"(%1.1f,%1.1f) -> (%1.1f,%1.1f)\n", seg->index, LE_U16(raw.linedef),
				seg->side ? "L" : "R", LE_U32(raw.partner),
				seg->start->x, seg->start->y, seg->end->x, seg->end->y);
#endif
	}

	lump->Finish();
}


void PutSubsecs(const char *name, int do_gl)
{
	int size = num_subsecs * (int)sizeof(raw_subsec_t);

	Lump_c * lump = CreateLevelLump(name, size);

	for (int i=0 ; i < num_subsecs ; i++)
	{
		raw_subsec_t raw;

		const subsec_t *sub = lev_subsecs[i];

		raw.first = LE_U16(sub->seg_list->index);
		raw.num   = LE_U16(sub->seg_count);

		lump->Write(&raw, sizeof(raw));

#if DEBUG_BSP
		cur_info->Debug("PUT SUBSEC %04X  First %04X  Num %04X\n",
				sub->index, LE_U16(raw.first), LE_U16(raw.num));
#endif
	}

	if (num_subsecs > 32767)
	{
		Failure("Number of %s has overflowed.\n", do_gl ? "GL subsectors" : "subsectors");
		MarkOverflow(do_gl ? LIMIT_GL_SSECT : LIMIT_SSECTORS);
	}

	lump->Finish();
}


void PutGLSubsecs_V5()
{
	int size = num_subsecs * (int)sizeof(raw_v5_subsec_t);

	Lump_c *lump = CreateLevelLump("GL_SSECT", size);

	for (int i=0 ; i < num_subsecs ; i++)
	{
		raw_v5_subsec_t raw;

		const subsec_t *sub = lev_subsecs[i];

		raw.first = LE_U32(sub->seg_list->index);
		raw.num   = LE_U32(sub->seg_count);

		lump->Write(&raw, sizeof(raw));

#if DEBUG_BSP
		cur_info->Debug("PUT V3 SUBSEC %06X  First %06X  Num %06X\n",
					sub->index, LE_U32(raw.first), LE_U32(raw.num));
#endif
	}

	lump->Finish();
}


static int node_cur_index;

static void PutOneNode(node_t *node, Lump_c *lump)
{
	if (node->r.node)
		PutOneNode(node->r.node, lump);

	if (node->l.node)
		PutOneNode(node->l.node, lump);

	node->index = node_cur_index++;

	raw_node_t raw;

	// note that x/y/dx/dy are always integral in non-UDMF maps
	raw.x  = LE_S16(I_ROUND(node->x));
	raw.y  = LE_S16(I_ROUND(node->y));
	raw.dx = LE_S16(I_ROUND(node->dx));
	raw.dy = LE_S16(I_ROUND(node->dy));

	raw.b1.minx = LE_S16(node->r.bounds.minx);
	raw.b1.miny = LE_S16(node->r.bounds.miny);
	raw.b1.maxx = LE_S16(node->r.bounds.maxx);
	raw.b1.maxy = LE_S16(node->r.bounds.maxy);

	raw.b2.minx = LE_S16(node->l.bounds.minx);
	raw.b2.miny = LE_S16(node->l.bounds.miny);
	raw.b2.maxx = LE_S16(node->l.bounds.maxx);
	raw.b2.maxy = LE_S16(node->l.bounds.maxy);

	if (node->r.node)
		raw.right = LE_U16(node->r.node->index);
	else if (node->r.subsec)
		raw.right = LE_U16(node->r.subsec->index | 0x8000);
	else
		BugError("Bad right child in node %d\n", node->index);

	if (node->l.node)
		raw.left = LE_U16(node->l.node->index);
	else if (node->l.subsec)
		raw.left = LE_U16(node->l.subsec->index | 0x8000);
	else
		BugError("Bad left child in node %d\n", node->index);

	lump->Write(&raw, sizeof(raw));

#if DEBUG_BSP
	cur_info->Debug("PUT NODE %04X  Left %04X  Right %04X  "
			"(%d,%d) -> (%d,%d)\n", node->index, LE_U16(raw.left),
			LE_U16(raw.right), node->x, node->y,
			node->x + node->dx, node->y + node->dy);
#endif
}


static void PutOneNode_V5(node_t *node, Lump_c *lump)
{
	if (node->r.node)
		PutOneNode_V5(node->r.node, lump);

	if (node->l.node)
		PutOneNode_V5(node->l.node, lump);

	node->index = node_cur_index++;

	raw_v5_node_t raw;

	raw.x  = LE_S16(I_ROUND(node->x));
	raw.y  = LE_S16(I_ROUND(node->y));
	raw.dx = LE_S16(I_ROUND(node->dx));
	raw.dy = LE_S16(I_ROUND(node->dy));

	raw.b1.minx = LE_S16(node->r.bounds.minx);
	raw.b1.miny = LE_S16(node->r.bounds.miny);
	raw.b1.maxx = LE_S16(node->r.bounds.maxx);
	raw.b1.maxy = LE_S16(node->r.bounds.maxy);

	raw.b2.minx = LE_S16(node->l.bounds.minx);
	raw.b2.miny = LE_S16(node->l.bounds.miny);
	raw.b2.maxx = LE_S16(node->l.bounds.maxx);
	raw.b2.maxy = LE_S16(node->l.bounds.maxy);

	if (node->r.node)
		raw.right = LE_U32(node->r.node->index);
	else if (node->r.subsec)
		raw.right = LE_U32(node->r.subsec->index | 0x80000000U);
	else
		BugError("Bad right child in V5 node %d\n", node->index);

	if (node->l.node)
		raw.left = LE_U32(node->l.node->index);
	else if (node->l.subsec)
		raw.left = LE_U32(node->l.subsec->index | 0x80000000U);
	else
		BugError("Bad left child in V5 node %d\n", node->index);

	lump->Write(&raw, sizeof(raw));

#if DEBUG_BSP
	cur_info->Debug("PUT V5 NODE %08X  Left %08X  Right %08X  "
			"(%d,%d) -> (%d,%d)\n", node->index, LE_U32(raw.left),
			LE_U32(raw.right), node->x, node->y,
			node->x + node->dx, node->y + node->dy);
#endif
}


void PutNodes(const char *name, int do_v5, node_t *root)
{
	int struct_size = do_v5 ? (int)sizeof(raw_v5_node_t) : (int)sizeof(raw_node_t);

	// this can be bigger than the actual size, but never smaller
	int max_size = (num_nodes + 1) * struct_size;

	Lump_c *lump = CreateLevelLump(name, max_size);

	node_cur_index = 0;

	if (root != NULL)
	{
		if (do_v5)
			PutOneNode_V5(root, lump);
		else
			PutOneNode(root, lump);
	}

	lump->Finish();

	if (node_cur_index != num_nodes)
		BugError("PutNodes miscounted (%d != %d)\n", node_cur_index, num_nodes);

	if (!do_v5 && node_cur_index > 32767)
	{
		Failure("Number of nodes has overflowed.\n");
		MarkOverflow(LIMIT_NODES);
	}
}


void CheckLimits()
{
	if (num_sectors > 65534)
	{
		Failure("Map has too many sectors.\n");
		MarkOverflow(LIMIT_SECTORS);
	}

	if (num_sidedefs > 65534)
	{
		Failure("Map has too many sidedefs.\n");
		MarkOverflow(LIMIT_SIDEDEFS);
	}

	if (num_linedefs > 65534)
	{
		Failure("Map has too many linedefs.\n");
		MarkOverflow(LIMIT_LINEDEFS);
	}

	if (cur_info->gl_nodes && !cur_info->force_v5)
	{
		if (num_old_vert > 32767 ||
			num_new_vert > 32767 ||
			num_segs     > 65534 ||
			num_nodes    > 32767)
		{
			Warning("Forcing V5 of GL-Nodes due to overflows.\n");
			lev_force_v5 = true;
		}
	}

	if (! cur_info->force_xnod)
	{
		if (num_old_vert > 32767 ||
			num_new_vert > 32767 ||
			num_segs     > 32767 ||
			num_nodes    > 32767)
		{
			Warning("Forcing XNOD format nodes due to overflows.\n");
			lev_force_xnod = true;
		}
	}
}


struct Compare_seg_pred
{
	inline bool operator() (const seg_t *A, const seg_t *B) const
	{
		return A->index < B->index;
	}
};

void SortSegs()
{
	// do a sanity check
	for (int i = 0 ; i < num_segs ; i++)
		if (lev_segs[i]->index < 0)
			BugError("Seg %p never reached a subsector!\n", i);

	// sort segs into ascending index
	std::sort(lev_segs.begin(), lev_segs.end(), Compare_seg_pred());

	// remove unwanted segs
	while (lev_segs.size() > 0 && lev_segs.back()->index == SEG_IS_GARBAGE)
	{
		UtilFree((void *) lev_segs.back());
		lev_segs.pop_back();
	}
}


/* ----- ZDoom format writing --------------------------- */

static const u8_t *lev_XNOD_magic = (const u8_t *) "XNOD";
static const u8_t *lev_XGL3_magic = (const u8_t *) "XGL3";
static const u8_t *lev_ZNOD_magic = (const u8_t *) "ZNOD";

void PutZVertices()
{
	int count, i;

	u32_t orgverts = LE_U32(num_old_vert);
	u32_t newverts = LE_U32(num_new_vert);

	ZLibAppendLump(&orgverts, 4);
	ZLibAppendLump(&newverts, 4);

	for (i=0, count=0 ; i < num_vertices ; i++)
	{
		raw_v2_vertex_t raw;

		const vertex_t *vert = lev_vertices[i];

		if (! vert->is_new)
			continue;

		raw.x = LE_S32(I_ROUND(vert->x * 65536.0));
		raw.y = LE_S32(I_ROUND(vert->y * 65536.0));

		ZLibAppendLump(&raw, sizeof(raw));

		count++;
	}

	if (count != num_new_vert)
		BugError("PutZVertices miscounted (%d != %d)\n", count, num_new_vert);
}


void PutZSubsecs()
{
	u32_t raw_num = LE_U32(num_subsecs);
	ZLibAppendLump(&raw_num, 4);

	int cur_seg_index = 0;

	for (int i=0 ; i < num_subsecs ; i++)
	{
		const subsec_t *sub = lev_subsecs[i];

		raw_num = LE_U32(sub->seg_count);
		ZLibAppendLump(&raw_num, 4);

		// sanity check the seg index values
		int count = 0;
		for (const seg_t *seg = sub->seg_list ; seg ; seg = seg->next, cur_seg_index++)
		{
			if (cur_seg_index != seg->index)
				BugError("PutZSubsecs: seg index mismatch in sub %d (%d != %d)\n",
						i, cur_seg_index, seg->index);

			count++;
		}

		if (count != sub->seg_count)
			BugError("PutZSubsecs: miscounted segs in sub %d (%d != %d)\n",
					i, count, sub->seg_count);
	}

	if (cur_seg_index != num_segs)
		BugError("PutZSubsecs miscounted segs (%d != %d)\n", cur_seg_index, num_segs);
}


void PutZSegs()
{
	u32_t raw_num = LE_U32(num_segs);
	ZLibAppendLump(&raw_num, 4);

	for (int i=0 ; i < num_segs ; i++)
	{
		const seg_t *seg = lev_segs[i];

		if (seg->index != i)
			BugError("PutZSegs: seg index mismatch (%d != %d)\n", seg->index, i);

		u32_t v1 = LE_U32(VertexIndex_XNOD(seg->start));
		u32_t v2 = LE_U32(VertexIndex_XNOD(seg->end));

		u16_t line = LE_U16(seg->linedef->index);
		u8_t  side = (u8_t) seg->side;

		ZLibAppendLump(&v1,   4);
		ZLibAppendLump(&v2,   4);
		ZLibAppendLump(&line, 2);
		ZLibAppendLump(&side, 1);
	}
}


void PutXGL3Segs()
{
	u32_t raw_num = LE_U32(num_segs);
	ZLibAppendLump(&raw_num, 4);

	for (int i=0 ; i < num_segs ; i++)
	{
		const seg_t *seg = lev_segs[i];

		if (seg->index != i)
			BugError("PutXGL3Segs: seg index mismatch (%d != %d)\n", seg->index, i);

		u32_t v1      = LE_U32(VertexIndex_XNOD(seg->start));
		u32_t partner = LE_U32(seg->partner ? seg->partner->index : -1);
		u32_t line    = LE_U32(seg->linedef ? seg->linedef->index : -1);
		u8_t  side    = (u8_t) seg->side;

		ZLibAppendLump(&v1,      4);
		ZLibAppendLump(&partner, 4);
		ZLibAppendLump(&line,    4);
		ZLibAppendLump(&side,    1);

#if DEBUG_BSP
		fprintf(stderr, "SEG[%d] v1=%d partner=%d line=%d side=%d\n", i, v1, partner, line, side);
#endif
	}
}


static void PutOneZNode(node_t *node, bool do_xgl3)
{
	raw_v5_node_t raw;

	if (node->r.node)
		PutOneZNode(node->r.node, do_xgl3);

	if (node->l.node)
		PutOneZNode(node->l.node, do_xgl3);

	node->index = node_cur_index++;

	if (do_xgl3)
	{
		u32_t x  = LE_S32(I_ROUND(node->x  * 65536.0));
		u32_t y  = LE_S32(I_ROUND(node->y  * 65536.0));
		u32_t dx = LE_S32(I_ROUND(node->dx * 65536.0));
		u32_t dy = LE_S32(I_ROUND(node->dy * 65536.0));

		ZLibAppendLump(&x,  4);
		ZLibAppendLump(&y,  4);
		ZLibAppendLump(&dx, 4);
		ZLibAppendLump(&dy, 4);
	}
	else
	{
		raw.x  = LE_S16(I_ROUND(node->x));
		raw.y  = LE_S16(I_ROUND(node->y));
		raw.dx = LE_S16(I_ROUND(node->dx));
		raw.dy = LE_S16(I_ROUND(node->dy));

		ZLibAppendLump(&raw.x,  2);
		ZLibAppendLump(&raw.y,  2);
		ZLibAppendLump(&raw.dx, 2);
		ZLibAppendLump(&raw.dy, 2);
	}

	raw.b1.minx = LE_S16(node->r.bounds.minx);
	raw.b1.miny = LE_S16(node->r.bounds.miny);
	raw.b1.maxx = LE_S16(node->r.bounds.maxx);
	raw.b1.maxy = LE_S16(node->r.bounds.maxy);

	raw.b2.minx = LE_S16(node->l.bounds.minx);
	raw.b2.miny = LE_S16(node->l.bounds.miny);
	raw.b2.maxx = LE_S16(node->l.bounds.maxx);
	raw.b2.maxy = LE_S16(node->l.bounds.maxy);

	ZLibAppendLump(&raw.b1, sizeof(raw.b1));
	ZLibAppendLump(&raw.b2, sizeof(raw.b2));

	if (node->r.node)
		raw.right = LE_U32(node->r.node->index);
	else if (node->r.subsec)
		raw.right = LE_U32(node->r.subsec->index | 0x80000000U);
	else
		BugError("Bad right child in V5 node %d\n", node->index);

	if (node->l.node)
		raw.left = LE_U32(node->l.node->index);
	else if (node->l.subsec)
		raw.left = LE_U32(node->l.subsec->index | 0x80000000U);
	else
		BugError("Bad left child in V5 node %d\n", node->index);

	ZLibAppendLump(&raw.right, 4);
	ZLibAppendLump(&raw.left,  4);

#if DEBUG_BSP
	cur_info->Debug("PUT Z NODE %08X  Left %08X  Right %08X  "
			"(%d,%d) -> (%d,%d)\n", node->index, LE_U32(raw.left),
			LE_U32(raw.right), node->x, node->y,
			node->x + node->dx, node->y + node->dy);
#endif
}


void PutZNodes(node_t *root, bool do_xgl3)
{
	u32_t raw_num = LE_U32(num_nodes);
	ZLibAppendLump(&raw_num, 4);

	node_cur_index = 0;

	if (root)
		PutOneZNode(root, do_xgl3);

	if (node_cur_index != num_nodes)
		BugError("PutZNodes miscounted (%d != %d)\n", node_cur_index, num_nodes);
}


static int CalcZDoomNodesSize()
{
	// compute size of the ZDoom format nodes.
	// it does not need to be exact, but it *does* need to be bigger
	// (or equal) to the actual size of the lump.

	int size = 32;  // header + a bit extra

	size += 8 + num_vertices * 8;
	size += 4 + num_subsecs  * 4;
	size += 4 + num_segs     * 11;
	size += 4 + num_nodes    * sizeof(raw_v5_node_t);

	if (cur_info->force_compress)
	{
		// according to RFC1951, the zlib compression worst-case
		// scenario is 5 extra bytes per 32KB (0.015% increase).
		// we are significantly more conservative!

		size += ((size + 255) >> 5);
	}

	return size;
}


void SaveZDFormat(node_t *root_node)
{
	// leave SEGS and SSECTORS empty
	CreateLevelLump("SEGS")->Finish();
	CreateLevelLump("SSECTORS")->Finish();

	int max_size = CalcZDoomNodesSize();

	Lump_c *lump = CreateLevelLump("NODES", max_size);

	if (cur_info->force_compress)
		lump->Write(lev_ZNOD_magic, 4);
	else
		lump->Write(lev_XNOD_magic, 4);

	// the ZLibXXX functions do no compression for XNOD format
	ZLibBeginLump(lump);

	PutZVertices();
	PutZSubsecs();
	PutZSegs();
	PutZNodes(root_node, false);

	ZLibFinishLump();
}


void SaveXGL3Format(Lump_c *lump, node_t *root_node)
{
	// WISH : compute a max_size

	lump->Write(lev_XGL3_magic, 4);

	// disable compression
	bool force_compress = cur_info->force_compress;
	cur_info->force_compress = false;

	ZLibBeginLump(lump);

	PutZVertices();
	PutZSubsecs();
	PutXGL3Segs();
	PutZNodes(root_node, true /* do_xgl3 */);

	ZLibFinishLump();

	cur_info->force_compress = force_compress;
}


/* ----- whole-level routines --------------------------- */

void LoadLevel()
{
	Lump_c *LEV = cur_wad->GetLump(lev_current_start);

	lev_current_name = LEV->Name();
	lev_long_name = false;
	lev_overflows = false;

	cur_info->ShowMap(lev_current_name);

	num_new_vert   = 0;
	num_real_lines = 0;

	if (lev_format == MAPF_UDMF)
	{
		ParseUDMF();
	}
	else
	{
		GetVertices();
		GetSectors();
		GetSidedefs();

		if (lev_format == MAPF_Hexen)
		{
			GetLinedefsHexen();
			GetThingsHexen();
		}
		else
		{
			GetLinedefs();
			GetThings();
		}

		// always prune vertices at end of lump, otherwise all the
		// unused vertices from seg splits would keep accumulating.
		PruneVerticesAtEnd();
	}

	cur_info->Print(2, "    Loaded %d vertices, %d sectors, %d sides, %d lines, %d things\n",
				num_vertices, num_sectors, num_sidedefs, num_linedefs, num_things);

	DetectOverlappingVertices();
	DetectOverlappingLines();

	CalculateWallTips();

	// -JL- Find sectors containing polyobjs
	switch (lev_format)
	{
		case MAPF_Hexen: DetectPolyobjSectors(false); break;
		case MAPF_UDMF:  DetectPolyobjSectors(true);  break;
		default:         break;
	}
}


void FreeLevel()
{
	FreeVertices();
	FreeSidedefs();
	FreeLinedefs();
	FreeSectors();
	FreeThings();
	FreeSegs();
	FreeSubsecs();
	FreeNodes();
	FreeWallTips();
	FreeIntersections();
}


static u32_t CalcGLChecksum(void)
{
	u32_t crc;

	Adler32_Begin(&crc);

	Lump_c *lump = FindLevelLump("VERTEXES");

	if (lump && lump->Length() > 0)
	{
		u8_t *data = new u8_t[lump->Length()];

		if (! lump->Seek(0) ||
		    ! lump->Read(data, lump->Length()))
			cur_info->FatalError("Error reading vertices (for checksum).\n");

		Adler32_AddBlock(&crc, data, lump->Length());
		delete[] data;
	}

	lump = FindLevelLump("LINEDEFS");

	if (lump && lump->Length() > 0)
	{
		u8_t *data = new u8_t[lump->Length()];

		if (! lump->Seek(0) ||
		    ! lump->Read(data, lump->Length()))
			cur_info->FatalError("Error reading linedefs (for checksum).\n");

		Adler32_AddBlock(&crc, data, lump->Length());
		delete[] data;
	}

	Adler32_Finish(&crc);

	return crc;
}


void UpdateGLMarker(Lump_c *marker)
{
	// this is very conservative, around 4 times the actual size
	const int max_size = 512;

	// we *must* compute the checksum BEFORE (re)creating the lump
	// [ otherwise we write data into the wrong part of the file ]
	u32_t crc = CalcGLChecksum();

	out_wad->RecreateLump(marker, max_size);

	if (lev_long_name)
	{
		marker->Printf("LEVEL=%s\n", lev_current_name);
	}

	marker->Printf("BUILDER=%s\n", "AJBSP " AJBSP_VERSION);
	marker->Printf("CHECKSUM=0x%08x\n", crc);

	marker->Finish();
}


static void AddMissingLump(const char *name, const char *after)
{
	if (out_wad->LevelLookupLump(lev_current_output_idx, name) >= 0)
		return;

	int exist = out_wad->LevelLookupLump(lev_current_output_idx, after);

	// if this happens, the level structure is very broken
	if (exist < 0)
	{
		Warning("Missing %s lump -- level structure is broken\n", after);

		exist = out_wad->LevelLastLump(lev_current_output_idx);
	}

	out_wad->InsertPoint(exist + 1);

	out_wad->AddLump(name)->Finish();
}


build_result_e SaveLevel(node_t *root_node)
{
	int out_idx;

	// Note: root_node may be NULL
	for (out_idx = 0; out_idx < LevelsInOutputWad(); ++out_idx)
	{
		const char* name = GetOutputLevelName(out_idx);

		if (!StringCaseCmp(name, GetLevelName(lev_current_idx)))
			break;
	}

	lev_current_output_idx = out_idx;

	out_wad->BeginWrite();

	// remove any existing GL-Nodes
	out_wad->RemoveGLNodes(lev_current_output_idx);

	// ensure all necessary level lumps are present
	AddMissingLump("SEGS",     "VERTEXES");
	AddMissingLump("SSECTORS", "SEGS");
	AddMissingLump("NODES",    "SSECTORS");
	AddMissingLump("REJECT",   "SECTORS");
	AddMissingLump("BLOCKMAP", "REJECT");

	// user preferences
	lev_force_v5   = cur_info->force_v5;
	lev_force_xnod = cur_info->force_xnod;

	// check for overflows...
	// this sets the force_xxx vars if certain limits are breached
	CheckLimits();


	/* --- GL Nodes --- */

	Lump_c * gl_marker = NULL;

	if (cur_info->gl_nodes && num_real_lines > 0)
	{
		// this also removes minisegs and degenerate segs
		SortSegs();

		// create empty marker now, flesh it out later
		gl_marker = CreateGLMarker();

		PutGLVertices(lev_force_v5);

		if (lev_force_v5)
			PutGLSegs_V5();
		else
			PutGLSegs_V2();

		if (lev_force_v5)
			PutGLSubsecs_V5();
		else
			PutSubsecs("GL_SSECT", true);

		PutNodes("GL_NODES", lev_force_v5, root_node);

		// -JL- Add empty PVS lump
		CreateLevelLump("GL_PVS")->Finish();
	}


	/* --- Normal nodes --- */

	// remove all the mini-segs from subsectors
	NormaliseBspTree();

	if (lev_force_xnod && num_real_lines > 0)
	{
		SortSegs();
		SaveZDFormat(root_node);
	}
	else
	{
		// reduce vertex precision for classic DOOM nodes.
		// some segs can become "degenerate" after this, and these
		// are removed from subsectors.
		RoundOffBspTree();

		SortSegs();

		PutVertices("VERTEXES", false);

		PutSegs();
		PutSubsecs("SSECTORS", false);
		PutNodes("NODES", false, root_node);
	}

	PutBlockmap();
	PutReject();

	// keyword support (v5.0 of the specs).
	// must be done *after* doing normal nodes, for proper checksum.
	if (gl_marker)
	{
		UpdateGLMarker(gl_marker);
	}

	out_wad->EndWrite();

	if (lev_overflows)
	{
		// no message here
		// [ in verbose mode, each overflow already printed a message ]
		// [ in normal mode, we don't want any messages at all ]

		return BUILD_LumpOverflow;
	}

	return BUILD_OK;
}

build_result_e SaveGLNodes(node_t *root_node)
{
	int out_idx;
	// Note: root_node may be NULL

	for (out_idx = 0; out_idx < LevelsInOutputWad(); ++out_idx)
	{
		const char* name = GetOutputLevelName(out_idx);

		if (!StringCaseCmp(name, GetLevelName(lev_current_idx)))
			break;
	}

	lev_current_output_idx = out_idx;

	out_wad->BeginWrite();

	// remove any existing GL-Nodes
	out_wad->RemoveGLNodes(lev_current_output_idx);

	// user preferences
	lev_force_v5   = cur_info->force_v5;
	lev_force_xnod = cur_info->force_xnod;

	// check for overflows...
	// this sets the force_xxx vars if certain limits are breached
	CheckLimits();

	/* --- GL Nodes --- */

	Lump_c * gl_marker = NULL;

	if (num_real_lines > 0)
	{
		// this also removes minisegs and degenerate segs
		SortSegs();

		// create empty marker now, flesh it out later
		gl_marker = CreateGLMarker();

		PutGLVertices(lev_force_v5);

		if (lev_force_v5)
			PutGLSegs_V5();
		else
			PutGLSegs_V2();

		if (lev_force_v5)
			PutGLSubsecs_V5();
		else
			PutSubsecs("GL_SSECT", true);

		PutNodes("GL_NODES", lev_force_v5, root_node);

		// -JL- Add empty PVS lump
		// This does us no good
		//CreateLevelLump("GL_PVS")->Finish();
	}

	// keyword support (v5.0 of the specs).
	// must be done *after* doing normal nodes, for proper checksum.
	if (gl_marker)
	{
		UpdateGLMarker(gl_marker);
	}

	out_wad->EndWrite();

	if (lev_overflows)
	{
		// no message here
		// [ in verbose mode, each overflow already printed a message ]
		// [ in normal mode, we don't want any messages at all ]

		return BUILD_LumpOverflow;
	}

	return BUILD_OK;
}

build_result_e SaveUDMF(node_t *root_node)
{
	cur_wad->BeginWrite();

	// remove any existing ZNODES lump
	cur_wad->RemoveZNodes(lev_current_idx);

	Lump_c *lump = CreateLevelLump("ZNODES", -1);

	if (num_real_lines == 0)
	{
		lump->Finish();
	}
	else
	{
		SortSegs();
		SaveXGL3Format(lump, root_node);
	}

	cur_wad->EndWrite();

	return BUILD_OK;
}


build_result_e SaveXWA(node_t *root_node)
{
	xwa_wad->BeginWrite();

	const char *lev_name = GetLevelName(lev_current_idx);
	Lump_c *lump = xwa_wad->AddLump(lev_name);

	if (num_real_lines == 0)
	{
		lump->Finish();
	}
	else
	{
		SortSegs();
		SaveXGL3Format(lump, root_node);
	}

	xwa_wad->EndWrite();

	return BUILD_OK;
}


//----------------------------------------------------------------------

static Lump_c  *zout_lump;

#ifdef HAVE_ZLIB
static z_stream zout_stream;
static Bytef    zout_buffer[1024];
#endif


void ZLibBeginLump(Lump_c *lump)
{
	zout_lump = lump;

	if (! cur_info->force_compress)
		return;

#ifndef HAVE_ZLIB
	cur_info->FatalError("No zlib!\n");
#else
	zout_stream.zalloc = (alloc_func)0;
	zout_stream.zfree  = (free_func)0;
	zout_stream.opaque = (voidpf)0;

	if (Z_OK != deflateInit(&zout_stream, Z_DEFAULT_COMPRESSION))
		cur_info->FatalError("Trouble setting up zlib compression\n");

	zout_stream.next_out  = zout_buffer;
	zout_stream.avail_out = sizeof(zout_buffer);
#endif
}


void ZLibAppendLump(const void *data, int length)
{
	// ASSERT(zout_lump)
	// ASSERT(length > 0)

	if (! cur_info->force_compress)
	{
		zout_lump->Write(data, length);
		return;
	}

#ifndef HAVE_ZLIB
	cur_info->FatalError("No zlib!\n");
#else
	zout_stream.next_in  = (Bytef*)data;   // const override
	zout_stream.avail_in = length;

	while (zout_stream.avail_in > 0)
	{
		int err = deflate(&zout_stream, Z_NO_FLUSH);

		if (err != Z_OK)
			cur_info->FatalError("Trouble compressing %d bytes (zlib)\n", length);

		if (zout_stream.avail_out == 0)
		{
			zout_lump->Write(zout_buffer, sizeof(zout_buffer));

			zout_stream.next_out  = zout_buffer;
			zout_stream.avail_out = sizeof(zout_buffer);
		}
	}
#endif
}


void ZLibFinishLump(void)
{
	if (! cur_info->force_compress)
	{
		zout_lump->Finish();
		zout_lump = NULL;
		return;
	}

#ifndef HAVE_ZLIB
	cur_info->FatalError("No zlib!\n");
#else
	int left_over;

	// ASSERT(zout_stream.avail_out > 0)

	zout_stream.next_in  = Z_NULL;
	zout_stream.avail_in = 0;

	for (;;)
	{
		int err = deflate(&zout_stream, Z_FINISH);

		if (err == Z_STREAM_END)
			break;

		if (err != Z_OK)
			cur_info->FatalError("Trouble finishing compression (zlib)\n");

		if (zout_stream.avail_out == 0)
		{
			zout_lump->Write(zout_buffer, sizeof(zout_buffer));

			zout_stream.next_out  = zout_buffer;
			zout_stream.avail_out = sizeof(zout_buffer);
		}
	}

	left_over = sizeof(zout_buffer) - zout_stream.avail_out;

	if (left_over > 0)
		zout_lump->Write(zout_buffer, left_over);

	deflateEnd(&zout_stream);

	zout_lump->Finish();
	zout_lump = NULL;
#endif
}


/* ---------------------------------------------------------------- */

Lump_c * FindLevelLump(const char *name)
{
	int idx = cur_wad->LevelLookupLump(lev_current_idx, name);

	if (idx < 0)
		return NULL;

	return cur_wad->GetLump(idx);
}

Lump_c * FindOutputLevelLump(const char *name)
{
	if (lev_current_output_idx >= out_wad->LevelCount())
		return NULL;

	int idx = out_wad->LevelLookupLump(lev_current_output_idx, name);

	if (idx < 0)
		return NULL;

	return out_wad->GetLump(idx);
}

Lump_c * CreateLevelLump(const char *name, int max_size)
{
	// look for existing one
	Lump_c *lump = FindOutputLevelLump(name);

	if (lump)
	{
		out_wad->RecreateLump(lump, max_size);
	}
	else
	{
		if (lev_current_output_idx < out_wad->LevelCount())
		{
			int last_idx = out_wad->LevelLastLump(lev_current_output_idx);

			// in UDMF maps, insert before the ENDMAP lump, otherwise insert
			// after the last known lump of the level.
			if (lev_format != MAPF_UDMF)
				last_idx += 1;

			out_wad->InsertPoint(last_idx);
		}

		lump = out_wad->AddLump(name, max_size);
	}

	return lump;
}


Lump_c * CreateGLMarker()
{
	char name_buf[64];

	if (strlen(lev_current_name) <= 5)
	{
		sprintf(name_buf, "GL_%s", lev_current_name);

		lev_long_name = false;
	}
	else
	{
		// support for level names longer than 5 letters
		strcpy(name_buf, "GL_LEVEL");

		lev_long_name = true;
	}

	if (lev_current_output_idx < out_wad->LevelCount())
	{
		int last_idx = out_wad->LevelLastLump(lev_current_idx);
		out_wad->InsertPoint(last_idx + 1);
	}

	Lump_c *marker = out_wad->AddLump(name_buf);

	marker->Finish();

	return marker;
}

Lump_c * FindOutputGLMarker()
{
	char name_buf[64];

	SYS_ASSERT(strlen(lev_current_name) <= 5);
	sprintf(name_buf, "GL_%s", lev_current_name);

	return out_wad->FindLump(name_buf);
}


//------------------------------------------------------------------------
// MAIN STUFF
//------------------------------------------------------------------------

buildinfo_t * cur_info = NULL;

void SetInfo(buildinfo_t *info)
{
	cur_info = info;
}


void OpenInputWad(const char *filename)
{
	cur_wad = Wad_file::Open(filename, 'r');
	if (cur_wad == NULL)
		cur_info->FatalError("Cannot open file: %s\n", filename);
}

void OpenOutputWad(const char* filename)
{
	out_wad = Wad_file::Open(filename, 'a');
	if (out_wad->IsReadOnly())
	{
		delete out_wad;
		out_wad = NULL;

		cur_info->FatalError("file is read only: %s\n", filename);
	}
}


void CreateXWA(const char *filename)
{
	xwa_wad = Wad_file::Open(filename, 'w');
	if (xwa_wad == NULL)
		cur_info->FatalError("Cannot create file: %s\n", filename);

	xwa_wad->BeginWrite();
	xwa_wad->AddLump("XG_START")->Finish();
	xwa_wad->EndWrite();
}


void FinishXWA()
{
	xwa_wad->BeginWrite();
	xwa_wad->AddLump("XG_END")->Finish();
	xwa_wad->EndWrite();
}


void CloseWad()
{
	if (cur_wad != NULL)
	{
		// this closes the file
		delete cur_wad;
		cur_wad = NULL;
	}

	if (xwa_wad != NULL)
	{
		delete xwa_wad;
		xwa_wad = NULL;
	}
}


int LevelsInWad()
{
	if (cur_wad == NULL)
		return 0;

	return cur_wad->LevelCount();
}

int LevelsInOutputWad()
{
	if (out_wad == NULL)
		return 0;

	return out_wad->LevelCount();
}

const char * GetLevelName(int lev_idx)
{
	SYS_ASSERT(cur_wad != NULL);

	int lump_idx = cur_wad->LevelHeader(lev_idx);

	return cur_wad->GetLump(lump_idx)->Name();
}

const char * GetOutputLevelName(int lev_idx)
{
	SYS_ASSERT(out_wad != NULL);

	int lump_idx = out_wad->LevelHeader(lev_idx);

	const char* name = out_wad->GetLump(lump_idx)->Name();

	if (!StringCaseCmpMax(name, "GL_", 3))
		return name + 3;
	return name;
}

/* ----- build nodes for a single level ----- */

build_result_e BuildLevel(int lev_idx)
{
	if (cur_info->cancelled)
		return BUILD_Cancelled;

	node_t   *root_node = NULL;
	subsec_t *root_sub  = NULL;

	lev_current_idx   = lev_idx;
	lev_current_start = cur_wad->LevelHeader(lev_idx);
	lev_format        = cur_wad->LevelFormat(lev_idx);

	LoadLevel();

	InitBlockmap();

	build_result_e ret = BUILD_OK;

	if (num_real_lines > 0)
	{
		bbox_t dummy;

		// create initial segs
		seg_t *list = CreateSegs();

		// recursively create nodes
		ret = BuildNodes(list, 0, &dummy, &root_node, &root_sub);
	}

	if (ret == BUILD_OK)
	{
		cur_info->Print(2, "    Built %d NODES, %d SSECTORS, %d SEGS, %d VERTEXES\n",
				num_nodes, num_subsecs, num_segs, num_old_vert + num_new_vert);

		if (root_node != NULL)
		{
			cur_info->Print(2, "    Heights of subtrees: %d / %d\n",
					ComputeBspHeight(root_node->r.node),
					ComputeBspHeight(root_node->l.node));
		}

		ClockwiseBspTree();

		if (xwa_wad != NULL)
			ret = SaveXWA(root_node);
		else if (lev_format == MAPF_UDMF)
			ret = SaveUDMF(root_node);
		else
			ret = SaveLevel(root_node);
	}
	else
	{
		/* build was Cancelled by the user */
	}

	FreeLevel();

	return ret;
}

build_result_e BuildGLNodes(int lev_idx)
{
	if (cur_info->cancelled)
		return BUILD_Cancelled;

	node_t   *root_node = NULL;
	subsec_t *root_sub  = NULL;
	build_result_e ret = BUILD_OK;

	lev_current_idx   = lev_idx;
	lev_current_start = cur_wad->LevelHeader(lev_idx);
	lev_format        = cur_wad->LevelFormat(lev_idx);

	LoadLevel();

	InitBlockmap();

	if (num_real_lines > 0)
	{
		bbox_t dummy;

		// create initial segs
		seg_t *list = CreateSegs();

		// recursively create nodes
		ret = BuildNodes(list, 0, &dummy, &root_node, &root_sub);
	}

	if (ret == BUILD_OK)
	{
		cur_info->Print(2, "    Built %d NODES, %d SSECTORS, %d SEGS, %d VERTEXES\n",
				num_nodes, num_subsecs, num_segs, num_old_vert + num_new_vert);

		if (root_node != NULL)
		{
			cur_info->Print(2, "    Heights of subtrees: %d / %d\n",
					ComputeBspHeight(root_node->r.node),
					ComputeBspHeight(root_node->l.node));
		}

		ClockwiseBspTree();

		ret = SaveGLNodes(root_node);
	}

	FreeLevel();

	return ret;
}

}  // namespace ajbsp


//--- editor settings ---
// vi:ts=4:sw=4:noexpandtab
