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
#include "raw_def.h"
#include "utility.h"
#include "wad.h"


#define DEBUG_WALLTIPS   0
#define DEBUG_POLYOBJ    0
#define DEBUG_WINDOW_FX  0
#define DEBUG_OVERLAPS   0


namespace ajbsp
{

#define SYS_MSG_BUFLEN  4000

static char message_buf[SYS_MSG_BUFLEN];


void Failure(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vsnprintf(message_buf, sizeof(message_buf), fmt, args);
	va_end(args);

	cur_info->Print(1, "    FAILURE: %s", message_buf);
}


void Warning(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vsnprintf(message_buf, sizeof(message_buf), fmt, args);
	va_end(args);

	cur_info->Print(1, "    WARNING: %s", message_buf);

	cur_info->total_warnings++;
}


void MinorIssue(const char *fmt, ...)
{
	if (cur_info->verbosity >= 3)
	{
		va_list args;

		va_start(args, fmt);
		vsnprintf(message_buf, sizeof(message_buf), fmt, args);
		va_end(args);

		cur_info->Print(1, "    ISSUE: %s", message_buf);
	}

	cur_info->total_minor_issues++;
}


//------------------------------------------------------------------------
// ANALYZE : Analyzing level structures
//------------------------------------------------------------------------


#define POLY_BOX_SZ  10


/* ----- polyobj handling ----------------------------- */

void MarkPolyobjSector(sector_t *sector)
{
	if (sector == NULL)
		return;

#if DEBUG_POLYOBJ
	cur_info->Debug("  Marking SECTOR %d\n", sector->index);
#endif

	/* already marked ? */
	if (sector->has_polyobj)
		return;

	// mark all lines of this sector as precious, to prevent (ideally)
	// the sector from being split.
	sector->has_polyobj = true;

	for (int i = 0 ; i < num_linedefs ; i++)
	{
		linedef_t *L = lev_linedefs[i];

		if ((L->right != NULL && L->right->sector == sector) ||
			(L->left  != NULL && L->left ->sector == sector))
		{
			L->is_precious = true;
		}
	}
}


void MarkPolyobjPoint(double x, double y)
{
	int i;
	int inside_count = 0;

	double best_dist = 999999;
	const linedef_t *best_match = NULL;
	sector_t *sector = NULL;

	// -AJA- First we handle the "awkward" cases where the polyobj sits
	//       directly on a linedef or even a vertex.  We check all lines
	//       that intersect a small box around the spawn point.

	int bminx = (int) (x - POLY_BOX_SZ);
	int bminy = (int) (y - POLY_BOX_SZ);
	int bmaxx = (int) (x + POLY_BOX_SZ);
	int bmaxy = (int) (y + POLY_BOX_SZ);

	for (i = 0 ; i < num_linedefs ; i++)
	{
		const linedef_t *L = lev_linedefs[i];

		if (CheckLinedefInsideBox(bminx, bminy, bmaxx, bmaxy,
					(int) L->start->x, (int) L->start->y,
					(int) L->end->x,   (int) L->end->y))
		{
#if DEBUG_POLYOBJ
			cur_info->Debug("  Touching line was %d\n", L->index);
#endif

			if (L->left != NULL)
				MarkPolyobjSector(L->left->sector);

			if (L->right != NULL)
				MarkPolyobjSector(L->right->sector);

			inside_count++;
		}
	}

	if (inside_count > 0)
		return;

	// -AJA- Algorithm is just like in DEU: we cast a line horizontally
	//       from the given (x,y) position and find all linedefs that
	//       intersect it, choosing the one with the closest distance.
	//       If the point is sitting directly on a (two-sided) line,
	//       then we mark the sectors on both sides.

	for (i = 0 ; i < num_linedefs ; i++)
	{
		const linedef_t *L = lev_linedefs[i];

		double x1 = L->start->x;
		double y1 = L->start->y;
		double x2 = L->end->x;
		double y2 = L->end->y;

		/* check vertical range */
		if (fabs(y2 - y1) < DIST_EPSILON)
			continue;

		if ((y > (y1 + DIST_EPSILON) && y > (y2 + DIST_EPSILON)) ||
			(y < (y1 - DIST_EPSILON) && y < (y2 - DIST_EPSILON)))
			continue;

		double x_cut = x1 + (x2 - x1) * (y - y1) / (y2 - y1) - x;

		if (fabs(x_cut) < fabs(best_dist))
		{
			/* found a closer linedef */

			best_match = L;
			best_dist = x_cut;
		}
	}

	if (best_match == NULL)
	{
		Warning("Bad polyobj thing at (%1.0f,%1.0f).\n", x, y);
		return;
	}

	double y1 = best_match->start->y;
	double y2 = best_match->end->y;

#if DEBUG_POLYOBJ
	cur_info->Debug("  Closest line was %d Y=%1.0f..%1.0f (dist=%1.1f)\n",
			best_match->index, y1, y2, best_dist);
#endif

	/* sanity check: shouldn't be directly on the line */
#if DEBUG_POLYOBJ
	if (fabs(best_dist) < DIST_EPSILON)
	{
		cur_info->Debug("  Polyobj FAILURE: directly on the line (%d)\n",
				best_match->index);
	}
#endif

	/* check orientation of line, to determine which side the polyobj is
	 * actually on.
	 */
	if ((y1 > y2) == (best_dist > 0))
		sector = best_match->right ? best_match->right->sector : NULL;
	else
		sector = best_match->left ? best_match->left->sector : NULL;

#if DEBUG_POLYOBJ
	cur_info->Debug("  Sector %d contains the polyobj.\n", sector ? sector->index : -1);
#endif

	if (sector == NULL)
	{
		Warning("Invalid Polyobj thing at (%1.0f,%1.0f).\n", x, y);
		return;
	}

	MarkPolyobjSector(sector);
}


//
// Based on code courtesy of Janis Legzdinsh.
//
void DetectPolyobjSectors(bool is_udmf)
{
	int i;

	// -JL- There's a conflict between Hexen polyobj thing types and Doom thing
	//      types. In Doom type 3001 is for Imp and 3002 for Demon. To solve
	//      this problem, first we are going through all lines to see if the
	//      level has any polyobjs. If found, we also must detect what polyobj
	//      thing types are used - Hexen ones or ZDoom ones. That's why we
	//      are going through all things searching for ZDoom polyobj thing
	//      types. If any found, we assume that ZDoom polyobj thing types are
	//      used, otherwise Hexen polyobj thing types are used.

	// -AJA- With UDMF there is an additional ambiguity, as line type 1 is a
	//       very common door in Doom and Heretic namespaces, but it is also
	//       the HEXTYPE_POLY_EXPLICIT special in Hexen and ZDoom namespaces.
	//
	//       Since the plain "Hexen" namespace is rare for UDMF maps, and ZDoom
	//       ports prefer their own polyobj things, we disable the Hexen polyobj
	//       things in UDMF maps.

	// -JL- First go through all lines to see if level contains any polyobjs
	for (i = 0 ; i < num_linedefs ; i++)
	{
		linedef_t *L = lev_linedefs[i];

		if (L->type == HEXTYPE_POLY_START || L->type == HEXTYPE_POLY_EXPLICIT)
			break;
	}

	if (i == num_linedefs)
	{
		// -JL- No polyobjs in this level
		return;
	}

	// -JL- Detect what polyobj thing types are used - Hexen ones or ZDoom ones
	bool hexen_style = true;

	if (is_udmf)
		hexen_style = false;

	for (i = 0 ; i < num_things ; i++)
	{
		thing_t *T = lev_things[i];

		if (T->type == ZDOOM_PO_SPAWN_TYPE || T->type == ZDOOM_PO_SPAWNCRUSH_TYPE)
		{
			// -JL- A ZDoom style polyobj thing found
			hexen_style = false;
			break;
		}
	}

#if DEBUG_POLYOBJ
	cur_info->Debug("Using %s style polyobj things\n", hexen_style ? "HEXEN" : "ZDOOM");
#endif

	for (i = 0 ; i < num_things ; i++)
	{
		thing_t *T = lev_things[i];

		double x = (double) T->x;
		double y = (double) T->y;

		// ignore everything except polyobj start spots
		if (hexen_style)
		{
			// -JL- Hexen style polyobj things
			if (T->type != PO_SPAWN_TYPE && T->type != PO_SPAWNCRUSH_TYPE)
				continue;
		}
		else
		{
			// -JL- ZDoom style polyobj things
			if (T->type != ZDOOM_PO_SPAWN_TYPE && T->type != ZDOOM_PO_SPAWNCRUSH_TYPE)
				continue;
		}

#if DEBUG_POLYOBJ
		cur_info->Debug("Thing %d at (%1.0f,%1.0f) is a polyobj spawner.\n", i, x, y);
#endif

		MarkPolyobjPoint(x, y);
	}
}


/* ----- analysis routines ----------------------------- */

bool vertex_t::Overlaps(const vertex_t *other) const
{
	double dx = fabs(other->x - x);
	double dy = fabs(other->y - y);

	return (dx < DIST_EPSILON) && (dy < DIST_EPSILON);
}

struct Compare_vertex_X_pred
{
	inline bool operator() (const vertex_t *A, const vertex_t *B) const
	{
		return A->x < B->x;
	}
};

void DetectOverlappingVertices(void)
{
	if (num_vertices < 2)
		return;

	// copy the vertex pointers
	std::vector<vertex_t *> array(lev_vertices);

	// sort the vertices by increasing X coordinate.
	// hence any overlapping vertices will be near each other.
	std::sort(array.begin(), array.end(), Compare_vertex_X_pred());

	// now mark them off
	for (int i=0 ; i < num_vertices - 1 ; i++)
	{
		vertex_t *A = array[i];

		for (int k = i+1 ; k < num_vertices ; k++)
		{
			vertex_t *B = array[k];

			if (B->x > A->x + DIST_EPSILON)
				break;

			if (A->Overlaps(B))
			{
				// found an overlap !
				B->overlap = A->overlap ? A->overlap : A;

#if DEBUG_OVERLAPS
				cur_info->Print(0, "Overlap: #%d + #%d\n", array[i]->index, array[i+1]->index);
#endif
			}
		}
	}

	// update the in-memory linedefs.
	// DOES NOT affect the on-disk linedefs.
	// this is mainly to help the miniseg creation code.

	for (int i=0 ; i < num_linedefs ; i++)
	{
		linedef_t *L = lev_linedefs[i];

		while (L->start->overlap)
		{
			L->start = L->start->overlap;
		}

		while (L->end->overlap)
		{
			L->end = L->end->overlap;
		}
	}
}


void PruneVerticesAtEnd(void)
{
	int old_num = num_vertices;

	// scan all vertices.
	// only remove from the end, so stop when hit a used one.

	for (int i = num_vertices - 1 ; i >= 0 ; i--)
	{
		vertex_t *V = lev_vertices[i];

		if (V->is_used)
			break;

		UtilFree(V);

		lev_vertices.pop_back();
	}

	int unused = old_num - num_vertices;

	if (unused > 0)
	{
		cur_info->Print(2, "    Pruned %d unused vertices at end\n", unused);
	}

	num_old_vert = num_vertices;
}


struct Compare_line_MinX_pred
{
	inline bool operator() (const linedef_t *A, const linedef_t *B) const
	{
		return A->MinX() < B->MinX();
	}
};


void DetectOverlappingLines(void)
{
	// Algorithm:
	//   Sort all lines by minimum X coordinate.
	//   Overlapping lines will then be near each other in this set.
	//   NOTE: does not detect partially overlapping lines.

	std::vector<linedef_t *> array(lev_linedefs);

	std::sort(array.begin(), array.end(), Compare_line_MinX_pred());

	int count = 0;

	for (int i=0 ; i < num_linedefs - 1 ; i++)
	{
		linedef_t *A = array[i];

		for (int k = i+1 ; k < num_linedefs ; k++)
		{
			linedef_t *B = array[k];

			if (B->MinX() > A->MinX() + DIST_EPSILON)
				break;

			// due to DetectOverlappingVertices(), we can compare the vertex pointers
			bool over1 = (A->start == B->start) && (A->end == B->end);
			bool over2 = (A->start == B->end)   && (A->end == B->start);

			if (over1 || over2)
			{
				// found an overlap !

				// keep the lowest numbered one
				if (A->index < B->index)
					A->overlap = B->overlap ? B->overlap : B;
				else
					B->overlap = A->overlap ? A->overlap : A;

				count++;
			}
		}
	}

	if (count > 0)
	{
		cur_info->Print(2, "    Detected %d overlapped linedefs\n", count);
	}
}


/* ----- vertex routines ------------------------------- */

void vertex_t::AddWallTip(double dx, double dy, bool open_left, bool open_right)
{
	SYS_ASSERT(overlap == NULL);

	walltip_t *tip = NewWallTip();
	walltip_t *after;

	tip->angle = ComputeAngle(dx, dy);
	tip->open_left  = open_left;
	tip->open_right = open_right;

	// find the correct place (order is increasing angle)
	for (after=tip_set ; after && after->next ; after=after->next)
	{ }

	while (after && tip->angle + ANG_EPSILON < after->angle)
		after = after->prev;

	// link it in
	tip->next = after ? after->next : tip_set;
	tip->prev = after;

	if (after)
	{
		if (after->next)
			after->next->prev = tip;

		after->next = tip;
	}
	else
	{
		if (tip_set != NULL)
			tip_set->prev = tip;

		tip_set = tip;
	}
}


void CalculateWallTips()
{
	for (int i=0 ; i < num_linedefs ; i++)
	{
		const linedef_t *L = lev_linedefs[i];

		if (L->overlap || L->zero_len)
			continue;

		double x1 = L->start->x;
		double y1 = L->start->y;
		double x2 = L->end->x;
		double y2 = L->end->y;

		bool left  = (L->left  != NULL) && (L->left ->sector != NULL);
		bool right = (L->right != NULL) && (L->right->sector != NULL);

		// note that start->overlap and end->overlap should be NULL
		// due to logic in DetectOverlappingVertices.

		L->start->AddWallTip(x2-x1, y2-y1, left, right);
		L->end  ->AddWallTip(x1-x2, y1-y2, right, left);
	}

#if DEBUG_WALLTIPS
	for (int k=0 ; k < num_vertices ; k++)
	{
		vertex_t *V = lev_vertices[k];

		cur_info->Debug("WallTips for vertex %d:\n", k);

		for (walltip_t *tip = V->tip_set ; tip ; tip = tip->next)
		{
			cur_info->Debug("  Angle=%1.1f left=%d right=%d\n", tip->angle,
					tip->open_left  ? 1 : 0,
					tip->open_right ? 1 : 0);
		}
	}
#endif
}


vertex_t *NewVertexFromSplitSeg(seg_t *seg, double x, double y)
{
	vertex_t *vert = NewVertex();

	vert->x = x;
	vert->y = y;

	vert->is_new  = true;
	vert->is_used = true;

	vert->index = num_new_vert;
	num_new_vert++;

	// compute wall-tip info
	if (seg->linedef == NULL)
	{
		vert->AddWallTip( seg->pdx,  seg->pdy, true, true);
		vert->AddWallTip(-seg->pdx, -seg->pdy, true, true);
	}
	else
	{
		const sidedef_t *front = seg->side ? seg->linedef->left  : seg->linedef->right;
		const sidedef_t *back  = seg->side ? seg->linedef->right : seg->linedef->left;

		bool left  = (back  != NULL) && (back ->sector != NULL);
		bool right = (front != NULL) && (front->sector != NULL);

		vert->AddWallTip( seg->pdx,  seg->pdy, left, right);
		vert->AddWallTip(-seg->pdx, -seg->pdy, right, left);
	}

	return vert;
}


vertex_t *NewVertexDegenerate(vertex_t *start, vertex_t *end)
{
	// this is only called when rounding off the BSP tree and
	// all the segs are degenerate (zero length), hence we need
	// to create at least one seg which won't be zero length.

	double dx = end->x - start->x;
	double dy = end->y - start->y;

	double dlen = hypot(dx, dy);

	vertex_t *vert = NewVertex();

	vert->is_new  = false;
	vert->is_used = true;

	vert->index = num_old_vert;
	num_old_vert++;

	// compute new coordinates

	vert->x = start->x;
	vert->y = start->x;

	if (dlen == 0)
		BugError("NewVertexDegenerate: bad delta!\n");

	dx /= dlen;
	dy /= dlen;

	while (I_ROUND(vert->x) == I_ROUND(start->x) &&
		   I_ROUND(vert->y) == I_ROUND(start->y))
	{
		vert->x += dx;
		vert->y += dy;
	}

	return vert;
}


bool vertex_t::CheckOpen(double dx, double dy) const
{
	const walltip_t *tip;

	double angle = ComputeAngle(dx, dy);

	// first check whether there's a wall-tip that lies in the exact
	// direction of the given direction (which is relative to the
	// vertex).

	for (tip=tip_set ; tip ; tip=tip->next)
	{
		if (fabs(tip->angle - angle) < ANG_EPSILON ||
			fabs(tip->angle - angle) > (360.0 - ANG_EPSILON))
		{
			// found one, hence closed
			return false;
		}
	}

	// OK, now just find the first wall-tip whose angle is greater than
	// the angle we're interested in.  Therefore we'll be on the RIGHT
	// side of that wall-tip.

	for (tip=tip_set ; tip ; tip=tip->next)
	{
		if (angle + ANG_EPSILON < tip->angle)
		{
			// found it
			return tip->open_right;
		}

		if (! tip->next)
		{
			// no more tips, thus we must be on the LEFT side of the tip
			// with the largest angle.

			return tip->open_left;
		}
	}

	// usually won't get here
	return true;
}


}  // namespace ajbsp

//--- editor settings ---
// vi:ts=4:sw=4:noexpandtab
