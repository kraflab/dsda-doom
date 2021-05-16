# MBF21 Level Editing Spec

This file outlines the changes that MBF21 introduces on top of the Boom spec for level editing. This file does not include the changes to things, weapons, code pointers, etc.

## Generalized Sector Types

| Dec  | Bit | Description            |
|------|-----|------------------------|
| 4096 | 12  | Alternate damage mode  |
| 8192 | 13  | Kill grounded monsters |

#### Alternate damage mode

The meaning of the damage bits changes when the alternate damage mode bit is set.

| Dec | Bit 6-5 | Description                                                   |
|-----|---------|---------------------------------------------------------------|
| 0   | 00      | Kills a player unless they have a rad suit or invulnerability |
| 32  | 01      | Kills a player                                                |
| 64  | 10      | Kills all players and exits the map (normal exit)             |
| 96  | 11      | Kills all players and exits the map (secret exit)             |

## Linedef Flags

| Dec  | Bit | Description          |
|------|-----|----------------------|
| 4096 | 12  | Block land monsters  |
| 8192 | 13  | Block players        |

## Linedef Types

#### Line scroll special variants

From the boom spec, type 255 is defined like so:

> 255    -- Scroll Wall Using Sidedef Offsets                   
>
> For simplicity, a static scroller is provided that scrolls the first sidedef of a linedef, based on its x- and y- offsets. No tag is used. The x offset controls the rate of horizontal scrolling, 1 unit per frame per x offset, and the y offset controls the rate of vertical scrolling, 1 unit per frame per y offset.

MBF21 has 3 types which operate like type 255, but where the special line determines the speed / direction with which all tagged lines scroll.

- 1024 is the standard scroller.
- 1025 is the displacement scroller variant.
- 1026 is the accelerative scroller variant.

The displacement scroller, as defined in the boom spec:

> In the first kind, displacement scrolling, the position of the scrolled objects or walls changes proportionally to the motion of the floor or ceiling in the sector on the first sidedef of the scrolling trigger. The proportionality is set by the length of the linedef trigger. If it is 32 units long, the wall, floor, ceiling, or object moves 1 unit for each unit the floor or ceiling of the controlling sector moves. If it is 64 long, they move 2 units per unit of relative floor/ceiling motion in the controlling sector and so on.

The accelerative scroller, as defined in the boom spec:

> The second kind of dynamic scrollers, accelerative scrollers, also react to height changes in the sector on the first sidedef of the linedef trigger, but the RATE of scrolling changes, not the displacement. That is, changing the controlling sector's height speeds up or slows down the scrolling by the change in height times the trigger's length, divided by 32.

## Generalized Linedef Types
- Generalized crusher walkover lines have been fixed (no change to actual spec).
