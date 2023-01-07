//
// Copyright(C) 2023 by Ryan Krafnick
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
// DESCRIPTION:
//	DSDA Destructible
//

#include "p_map.h"
#include "p_maputl.h"
#include "p_spec.h"
#include "r_main.h"

#include "dsda/utility.h"

extern mobj_t* bombsource;
extern mobj_t* bombspot;
extern int bombdamage;
extern int bombdistance;

void dsda_DamageLinedef(line_t *line, mobj_t *source, int damage) {
  if (damage <= 0)
    return;

  line->health -= damage;
  if (line->health < 0)
    line->health = 0;

  P_ActivateLine(line, source, 0, SPAC_DAMAGE | (line->health ? 0 : SPAC_DEATH));
}

static dboolean dsda_RadiusAttackLine(line_t *line) {
  fixed_t dist;
  mobj_t target;
  sector_t* frontsector;
  sector_t* backsector;
  int bombside;
  dboolean sighted;
  const fixed_t fudge = (FRACUNIT >> 4);

  if (!line->health)
    return true;

  bombside = P_PointOnLineSide(bombspot->x, bombspot->y, line);
  if (line->sidenum[bombside] == NO_INDEX)
    return true;

  dist = dsda_FixedDistancePointToLine(line->v1->x, line->v1->y,
                                       line->v2->x, line->v2->y,
                                       bombspot->x, bombspot->y,
                                       &target.x, &target.y);
  dist = (dist >> FRACBITS);
  if (dist >= bombdistance)
    return true;

  // The target is currently "on the line"
  // Move it towards the bomb slightly to make sure it's on the right side
  if (bombspot->x - target.x > fudge)
    target.x += fudge;
  if (target.x - bombspot->x > fudge)
    target.x -= fudge;
  if (bombspot->y - target.y > fudge)
    target.y += fudge;
  if (target.y - bombspot->y > fudge)
    target.y -= fudge;

  // P_CheckSight needs subsector
  target.subsector = R_PointInSubsector(target.x, target.y);

  frontsector = sides[line->sidenum[bombside]].sector;
  if (line->sidenum[!bombside] != NO_INDEX)
    backsector = sides[line->sidenum[!bombside]].sector;
  else
    backsector = NULL;

  sighted = false;

  if (!backsector || line->flags & ML_BLOCKEVERYTHING) {
    if (frontsector->ceilingheight > frontsector->floorheight) {
      target.z = frontsector->floorheight;
      target.height = frontsector->ceilingheight - frontsector->floorheight;

      sighted = P_CheckSight(&target, bombspot);
    }
  }
  else {
    fixed_t front_top, back_top, front_bottom, back_bottom;

    front_top = frontsector->ceilingheight;
    back_top = backsector->ceilingheight;
    front_bottom = frontsector->floorheight;
    back_bottom = backsector->floorheight;

    if (front_top > back_top) {
      target.z = back_top;
      target.height = front_top - back_top;

      sighted = P_CheckSight(&target, bombspot);
    }

    if (!sighted && front_bottom < back_bottom) {
      target.z = front_bottom;
      target.height = back_bottom - front_bottom;

      sighted = P_CheckSight(&target, bombspot);
    }
  }

  if (sighted) {
    int damage;

    damage = P_SplashDamage(dist);

    dsda_DamageLinedef(line, bombsource, damage);
  }

  return true;
}

void dsda_RadiusAttackDestructibles(int xl, int xh, int yl, int yh) {
  int x, y;

  // avoid collision with nested P_BlockLinesIterator
  validcount2++;

  for (y = yl; y <= yh; ++y)
    for (x = xl; x <= xh; ++x)
      P_BlockLinesIterator2(x, y, dsda_RadiusAttackLine);
}
