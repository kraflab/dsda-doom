//
// Copyright(C) 2024 by Ryan Krafnick
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
//	DSDA Aim
//

#include "p_map.h"
#include "tables.h"

#include "dsda/excmd.h"

#include "aim.h"

angle_t dsda_PlayerPitch(player_t* player)
{
  return dsda_FreeAim() ? player->mo->pitch : -(angle_t)(player->lookdir * ANG1 / M_PI);
}

fixed_t dsda_PlayerSlope(player_t* player)
{
  return dsda_FreeAim() ? finetangent[(ANG90 - player->mo->pitch) >> ANGLETOFINESHIFT] :
         raven ? ((player->lookdir) << FRACBITS) / 173 :
         0;
}

int dsda_PitchToLookDir(angle_t pitch)
{
  return -(int) ((((uint64_t) pitch * FIXED_PI) >> FRACBITS) / ANG1);
}

angle_t dsda_LookDirToPitch(int lookdir)
{
  return (angle_t) -FixedDiv(lookdir * ANG1, FIXED_PI);
}

int dsda_PlayerLookDir(player_t* player)
{
  return dsda_FreeAim() ? dsda_PitchToLookDir(player->mo->pitch) : player->lookdir;
}

void dsda_PlayerAim(mobj_t* source, angle_t angle, aim_t* aim, uint64_t target_mask)
{
  aim->angle = angle;

  if (dsda_FreeAim())
  {
    aim->slope = finetangent[(ANG90 - source->pitch) >> ANGLETOFINESHIFT];
    aim->z_offset = 0;
  }
  else
  {
    do
    {
      aim->slope = P_AimLineAttack(source, aim->angle, 16 * 64 * FRACUNIT, target_mask);

      if (!linetarget)
      {
        aim->angle += 1 << 26;
        aim->slope = P_AimLineAttack(source, aim->angle, 16 * 64 * FRACUNIT, target_mask);
      }

      if (!linetarget)
      {
        aim->angle -= 2 << 26;
        aim->slope = P_AimLineAttack(source, aim->angle, 16 * 64 * FRACUNIT, target_mask);
      }

      if (!linetarget) {
        aim->angle = angle;
        aim->slope = raven ? ((source->player->lookdir) << FRACBITS) / 173 : 0;
      }
    }
    while (target_mask && (target_mask = 0, !linetarget));  // killough 8/2/98

    aim->z_offset = raven ? ((source->player->lookdir) << FRACBITS) / 173 : 0;
  }
}

void dsda_PlayerAimBad(mobj_t* source, angle_t angle, aim_t* aim, uint64_t target_mask)
{
  aim->angle = angle;
  aim->z_offset = 0;

  if (dsda_FreeAim())
  {
    aim->slope = finetangent[(ANG90 - source->pitch) >> ANGLETOFINESHIFT];
    aim->z_offset = 0;
  }
  else
  {
    do
    {
      aim->slope = P_AimLineAttack(source, aim->angle, 16 * 64 * FRACUNIT, target_mask);

      if (!linetarget)
      {
        aim->angle += 1 << 26;
        aim->slope = P_AimLineAttack(source, aim->angle, 16 * 64 * FRACUNIT, target_mask);
      }

      if (!linetarget)
      {
        aim->angle -= 2 << 26;
        aim->slope = P_AimLineAttack(source, aim->angle, 16 * 64 * FRACUNIT, target_mask);
      }

      if (heretic && !linetarget)
        aim->slope = ((source->player->lookdir) << FRACBITS) / 173;
    }
    while (target_mask && (target_mask = 0, !linetarget));  // killough 8/2/98
  }
}
