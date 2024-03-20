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
//	DSDA Scroll
//

#include "p_tick.h"
#include "r_state.h"

#include "scroll.h"

static void dsda_UpdateSideScrollerPosition(scroll_t* s, fixed_t dx, fixed_t dy) {
  side_t* side;

  if (!dx && !dy)
    return;

  side = sides + s->affectee;
  if (!s->flags)
  {
    side->textureoffset += dx;
    side->rowoffset += dy;
  }
  else
  {
    if (s->flags & SCROLL_TOP)
    {
      side->textureoffset_top += dx;
      side->rowoffset_top += dy;
    }

    if (s->flags & SCROLL_MID)
    {
      side->textureoffset_mid += dx;
      side->rowoffset_mid += dy;
    }

    if (s->flags & SCROLL_BOTTOM)
    {
      side->textureoffset_bottom += dx;
      side->rowoffset_bottom += dy;
    }
  }
}

static void dsda_UpdateFloorScrollerPosition(scroll_t* s, fixed_t dx, fixed_t dy) {
  sector_t* sec;

  if (!dx && !dy)
    return;

  sec = sectors + s->affectee;
  sec->floor_xoffs += dx;
  sec->floor_yoffs += dy;
}

static void dsda_UpdateCeilingScrollerPosition(scroll_t* s, fixed_t dx, fixed_t dy) {
  sector_t* sec;

  if (!dx && !dy)
    return;

  sec = sectors + s->affectee;
  sec->ceiling_xoffs += dx;
  sec->ceiling_yoffs += dy;
}

static void dsda_UpdateFloorCarryScrollerPosition(scroll_t* s, fixed_t dx, fixed_t dy) {
  sector_t* sec;
  fixed_t height;
  fixed_t waterheight;
  msecnode_t* node;
  mobj_t* thing;

  if (!dx && !dy)
    return;

  // killough 3/7/98: Carry things on floor
  // killough 3/20/98: use new sector list which reflects true members
  // killough 3/27/98: fix carrier bug
  // killough 4/4/98: Underwater, carry things even w/o gravity

  sec = sectors + s->affectee;
  height = sec->floorheight;
  waterheight = sec->heightsec != -1 &&
    sectors[sec->heightsec].floorheight > height ?
    sectors[sec->heightsec].floorheight : INT_MIN;

  for (node = sec->touching_thinglist; node; node = node->m_snext) {
    thing = node->m_thing;

    // Move objects only if on floor or underwater, non-floating, and clipped
    if (
      !(thing->flags & MF_NOCLIP) && (
        !(thing->flags & MF_NOGRAVITY || thing->z > height) ||
        thing->z < waterheight
      )
    ) {
      thing->momx += dx;
      thing->momy += dy;
      thing->intflags |= MIF_SCROLLING;
    }
  }
}

static void dsda_UpdateControlScroller(control_scroll_t* s) {
  fixed_t dx;
  fixed_t dy;
  fixed_t height;
  fixed_t delta;

  dx = s->scroll.dx;
  dy = s->scroll.dy;

  if (s->control != -1) {
    height = sectors[s->control].floorheight + sectors[s->control].ceilingheight;
    delta = height - s->last_height;

    s->last_height = height;

    dx = FixedMul(dx, delta);
    dy = FixedMul(dy, delta);
  }

  if (s->accel)
  {
    s->vdx += dx;
    s->vdy += dy;
  }
  else {
    s->vdx = dx;
    s->vdy = dy;
  }
}

void dsda_UpdateControlSideScroller(control_scroll_t* s) {
  dsda_UpdateControlScroller(s);
  dsda_UpdateSideScrollerPosition(&s->scroll, s->vdx, s->vdy);
}

void dsda_UpdateSideScroller(scroll_t* s) {
  dsda_UpdateSideScrollerPosition(s, s->dx, s->dy);
}

void dsda_UpdateControlFloorScroller(control_scroll_t* s) {
  dsda_UpdateControlScroller(s);
  dsda_UpdateFloorScrollerPosition(&s->scroll, s->vdx, s->vdy);
}

void dsda_UpdateFloorScroller(scroll_t* s) {
  dsda_UpdateFloorScrollerPosition(s, s->dx, s->dy);
}

void dsda_UpdateControlCeilingScroller(control_scroll_t* s) {
  dsda_UpdateControlScroller(s);
  dsda_UpdateCeilingScrollerPosition(&s->scroll, s->vdx, s->vdy);
}

void dsda_UpdateCeilingScroller(scroll_t* s) {
  dsda_UpdateCeilingScrollerPosition(s, s->dx, s->dy);
}

void dsda_UpdateControlFloorCarryScroller(control_scroll_t* s) {
  dsda_UpdateControlScroller(s);
  dsda_UpdateFloorCarryScrollerPosition(&s->scroll, s->vdx, s->vdy);
}

void dsda_UpdateFloorCarryScroller(scroll_t* s) {
  dsda_UpdateFloorScrollerPosition(s, s->dx, s->dy);
}

void dsda_UpdateZDoomFloorScroller(scroll_t* s) {
  sector_t* sec;

  if (!s->dx && !s->dy)
    return;

  sec = sectors + s->affectee;

  if (s->flags & SCROLL_TEXTURE) {
    sec->floor_xoffs -= s->dx;
    sec->floor_yoffs += s->dy;
  }

  if (s->flags & (SCROLL_STATIC | SCROLL_PLAYER | SCROLL_MONSTER)) {
    fixed_t height;
    fixed_t waterheight;
    msecnode_t* node;
    mobj_t* thing;

    height = sec->floorheight;
    waterheight = sec->heightsec != -1 &&
      sectors[sec->heightsec].floorheight > height ?
      sectors[sec->heightsec].floorheight : INT_MIN;

    for (node = sec->touching_thinglist; node; node = node->m_snext) {
      thing = node->m_thing;

      // Move objects only if on floor or underwater, non-floating, and clipped
      if (
        !(thing->flags & MF_NOCLIP) && (
          !(thing->flags & MF_NOGRAVITY || thing->z > height) ||
          thing->z < waterheight
        )
      ) {
        dboolean scroll_it;

        scroll_it = false;
        if (thing->type == MT_SKULL || thing->flags & MF_COUNTKILL) {
          if (s->flags & SCROLL_MONSTER)
            scroll_it = true;
        }
        else if (thing->player) {
          if (s->flags & SCROLL_PLAYER)
            scroll_it = true;
        }
        else {
          if (s->flags & SCROLL_STATIC)
            scroll_it = true;
        }

        if (scroll_it) {
          thing->momx += s->dx * 3 / 32;
          thing->momy += s->dy * 3 / 32;
          thing->intflags |= MIF_SCROLLING;
        }
      }
    }
  }
}

void dsda_UpdateZDoomCeilingScroller(scroll_t* s) {
  sector_t* sec;

  if (!s->dx && !s->dy)
    return;

  sec = sectors + s->affectee;

  if (s->flags & SCROLL_TEXTURE) {
    sec->ceiling_xoffs -= s->dx;
    sec->ceiling_yoffs += s->dy;
  }

  if (s->flags & (SCROLL_STATIC | SCROLL_PLAYER | SCROLL_MONSTER)) {
    msecnode_t* node;
    mobj_t* thing;

    for (node = sec->touching_thinglist; node; node = node->m_snext) {
      thing = node->m_thing;

      if (
        !(thing->flags & MF_NOCLIP) &&
        thing->flags & MF_SPAWNCEILING &&
        thing->flags & MF_NOGRAVITY &&
        thing->z + thing->height == sec->ceilingheight
      ) {
        dboolean scroll_it;

        scroll_it = false;
        if (thing->type == MT_SKULL || thing->flags & MF_COUNTKILL) {
          if (s->flags & SCROLL_MONSTER)
            scroll_it = true;
        }
        else if (thing->player) {
          if (s->flags & SCROLL_PLAYER)
            scroll_it = true;
        }
        else {
          if (s->flags & SCROLL_STATIC)
            scroll_it = true;
        }

        if (scroll_it) {
          thing->momx = s->dx;
          thing->momy = s->dy;
          thing->intflags |= MIF_SCROLLING;
        }
      }
    }
  }
}

void dsda_UpdateThruster(scroll_t* s) {
  sector_t* sec;
  msecnode_t* node;
  mobj_t* thing;

  if (!s->dx && !s->dy)
    return;

  sec = sectors + s->affectee;

  for (node = sec->touching_thinglist; node; node = node->m_snext) {
    dboolean thrust_it;

    thrust_it = false;
    thing = node->m_thing;

    if (thing->flags & MF_NOCLIP)
      continue;

    if (!(thing->flags & MF_NOGRAVITY) && thing->z <= thing->floorz) {
      if (s->flags & THRUST_GROUNDED)
        thrust_it = true;
    }
    else if (
      thing->flags & MF_SPAWNCEILING &&
      thing->flags & MF_NOGRAVITY &&
      thing->z + thing->height == sec->ceilingheight
    ) {
      if (s->flags & THRUST_CEILING)
        thrust_it = true;
    }
    else if (thing->flags & MF_NOGRAVITY || thing->z > thing->floorz) {
      if (s->flags & THRUST_AIRBORNE)
        thrust_it = true;
    }

    if (thrust_it) {
      thrust_it = false;

      if (thing->flags2 & MF2_WINDTHRUST && s->flags & THRUST_WINDTHRUST)
        thrust_it = true;
      else if (thing->type == MT_SKULL || thing->flags & MF_COUNTKILL) {
        if (s->flags & THRUST_MONSTER)
          thrust_it = true;
      }
      else if (thing->player) {
        if (s->flags & THRUST_PLAYER)
          thrust_it = true;
      }
      else if (thing->flags & MF_MISSILE) {
        if (s->flags & THRUST_PROJECTILE)
          thrust_it = true;
      }
      else {
        if (s->flags & THRUST_STATIC)
          thrust_it = true;
      }

      if (thrust_it) {
        thing->momx += s->dx;
        thing->momy += s->dy;
        thing->intflags |= MIF_SCROLLING;
      }
    }
  }
}

static void dsda_InitScroller(scroll_t* scroll, fixed_t dx, fixed_t dy, int affectee, int flags) {
  scroll->dx = dx;
  scroll->dy = dy;
  scroll->flags = flags;
  scroll->affectee = affectee;
}

static void dsda_InitControlScroller(control_scroll_t* scroll, int control, int accel) {
  scroll->accel = accel;
  scroll->vdx = 0;
  scroll->vdy = 0;
  scroll->control = control;
  scroll->last_height = 0;

  if (scroll->control != -1)
    scroll->last_height = sectors[control].floorheight + sectors[control].ceilingheight;
}

void dsda_AddSideScroller(fixed_t dx, fixed_t dy, int affectee, int flags) {
  scroll_t* scroll;

  scroll = Z_MallocLevel(sizeof(*scroll));
  scroll->thinker.function = dsda_UpdateSideScroller;
  dsda_InitScroller(scroll, dx, dy, affectee, flags);
  P_AddThinker(&scroll->thinker);
}

void dsda_AddControlSideScroller(fixed_t dx, fixed_t dy,
                                 int control, int affectee, int accel, int flags) {
  control_scroll_t* scroll;

  scroll = Z_MallocLevel(sizeof(*scroll));
  scroll->scroll.thinker.function = dsda_UpdateControlSideScroller;
  dsda_InitScroller(&scroll->scroll, dx, dy, affectee, flags);
  dsda_InitControlScroller(scroll, control, accel);
  P_AddThinker(&scroll->scroll.thinker);
}

void dsda_AddFloorScroller(fixed_t dx, fixed_t dy, int affectee, int flags) {
  scroll_t* scroll;

  scroll = Z_MallocLevel(sizeof(*scroll));
  scroll->thinker.function = dsda_UpdateFloorScroller;
  dsda_InitScroller(scroll, dx, dy, affectee, flags);
  P_AddThinker(&scroll->thinker);
}

void dsda_AddControlFloorScroller(fixed_t dx, fixed_t dy,
                                  int control, int affectee, int accel, int flags) {
  control_scroll_t* scroll;

  scroll = Z_MallocLevel(sizeof(*scroll));
  scroll->scroll.thinker.function = dsda_UpdateControlFloorScroller;
  dsda_InitScroller(&scroll->scroll, dx, dy, affectee, flags);
  dsda_InitControlScroller(scroll, control, accel);
  P_AddThinker(&scroll->scroll.thinker);
}

void dsda_AddCeilingScroller(fixed_t dx, fixed_t dy, int affectee, int flags) {
  scroll_t* scroll;

  scroll = Z_MallocLevel(sizeof(*scroll));
  scroll->thinker.function = dsda_UpdateCeilingScroller;
  dsda_InitScroller(scroll, dx, dy, affectee, flags);
  P_AddThinker(&scroll->thinker);
}

void dsda_AddControlCeilingScroller(fixed_t dx, fixed_t dy,
                                    int control, int affectee, int accel, int flags) {
  control_scroll_t* scroll;

  scroll = Z_MallocLevel(sizeof(*scroll));
  scroll->scroll.thinker.function = dsda_UpdateControlCeilingScroller;
  dsda_InitScroller(&scroll->scroll, dx, dy, affectee, flags);
  dsda_InitControlScroller(scroll, control, accel);
  P_AddThinker(&scroll->scroll.thinker);
}

void dsda_AddFloorCarryScroller(fixed_t dx, fixed_t dy, int affectee, int flags) {
  scroll_t* scroll;

  scroll = Z_MallocLevel(sizeof(*scroll));
  scroll->thinker.function = dsda_UpdateFloorCarryScroller;
  dsda_InitScroller(scroll, dx, dy, affectee, flags);
  P_AddThinker(&scroll->thinker);
}

void dsda_AddControlFloorCarryScroller(fixed_t dx, fixed_t dy,
                                       int control, int affectee, int accel, int flags) {
  control_scroll_t* scroll;

  scroll = Z_MallocLevel(sizeof(*scroll));
  scroll->scroll.thinker.function = dsda_UpdateControlFloorCarryScroller;
  dsda_InitScroller(&scroll->scroll, dx, dy, affectee, flags);
  dsda_InitControlScroller(scroll, control, accel);
  P_AddThinker(&scroll->scroll.thinker);
}

void dsda_AddZDoomFloorScroller(fixed_t dx, fixed_t dy, int affectee, int flags) {
  scroll_t* scroll;

  scroll = Z_MallocLevel(sizeof(*scroll));
  scroll->thinker.function = dsda_UpdateZDoomFloorScroller;
  dsda_InitScroller(scroll, dx, dy, affectee, flags);
  P_AddThinker(&scroll->thinker);
}

void dsda_AddZDoomCeilingScroller(fixed_t dx, fixed_t dy, int affectee, int flags) {
  scroll_t* scroll;

  scroll = Z_MallocLevel(sizeof(*scroll));
  scroll->thinker.function = dsda_UpdateZDoomCeilingScroller;
  dsda_InitScroller(scroll, dx, dy, affectee, flags);
  P_AddThinker(&scroll->thinker);
}

void dsda_AddThruster(fixed_t dx, fixed_t dy, int affectee, int flags) {
  scroll_t* scroll;

  scroll = Z_MallocLevel(sizeof(*scroll));
  scroll->thinker.function = dsda_UpdateThruster;
  dsda_InitScroller(scroll, dx, dy, affectee, flags);
  P_AddThinker(&scroll->thinker);
}
