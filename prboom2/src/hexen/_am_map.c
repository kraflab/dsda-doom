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

#define NUMALIAS 3              // Number of antialiased lines.

static byte antialias[NUMALIAS][8] = {
    {83, 84, 85, 86, 87, 88, 89, 90},
    {96, 96, 95, 94, 93, 92, 91, 90},
    {107, 108, 109, 110, 111, 112, 89, 90}
};

void AM_clearFB(int color)
{
    int i, j;
    int dmapx;
    int dmapy;

    if (followplayer)
    {
        dmapx = (MTOF(plr->mo->x) - MTOF(oldplr.x));    //fixed point
        dmapy = (MTOF(oldplr.y) - MTOF(plr->mo->y));

        oldplr.x = plr->mo->x;
        oldplr.y = plr->mo->y;

        mapxstart += dmapx >> 1;
        mapystart += dmapy >> 1;

        while (mapxstart >= (finit_width >> crispy->hires))
            mapxstart -= (finit_width >> crispy->hires);
        while (mapxstart < 0)
            mapxstart += (finit_width >> crispy->hires);
        while (mapystart >= (finit_height >> crispy->hires))
            mapystart -= (finit_height >> crispy->hires);
        while (mapystart < 0)
            mapystart += (finit_height >> crispy->hires);
    }
    else
    {

    }

    //blit the automap background to the screen.
    j = (mapystart & ~crispy->hires) * (finit_width >> crispy->hires);
    for (i = 0; i < SCREENHEIGHT - SBARHEIGHT; i++)
    {
        memcpy(I_VideoBuffer + i * finit_width, maplump + j + mapxstart,
               finit_width - mapxstart);
        memcpy(I_VideoBuffer + i * finit_width + finit_width - mapxstart,
               maplump + j, mapxstart);
        j += finit_width;
        if (j >= (finit_height >> crispy->hires) * (finit_width >> crispy->hires))
            j = 0;
    }
}

// Based on Cohen-Sutherland clipping algorithm but with a slightly
// faster reject and precalculated slopes.  If I need the speed, will
// hash algorithm to the common cases.

dboolean AM_clipMline(mline_t * ml, fline_t * fl)
{
    enum
    { LEFT = 1, RIGHT = 2, BOTTOM = 4, TOP = 8 };
    int outcode1 = 0, outcode2 = 0, outside;
    fpoint_t tmp = { 0, 0 };
    int dx, dy;

#define DOOUTCODE(oc, mx, my) \
  (oc) = 0; \
  if ((my) < 0) (oc) |= TOP; \
  else if ((my) >= f_h) (oc) |= BOTTOM; \
  if ((mx) < 0) (oc) |= LEFT; \
  else if ((mx) >= f_w) (oc) |= RIGHT

    // do trivial rejects and outcodes
    if (ml->a.y > m_y2)
        outcode1 = TOP;
    else if (ml->a.y < m_y)
        outcode1 = BOTTOM;
    if (ml->b.y > m_y2)
        outcode2 = TOP;
    else if (ml->b.y < m_y)
        outcode2 = BOTTOM;
    if (outcode1 & outcode2)
        return false;           // trivially outside

    if (ml->a.x < m_x)
        outcode1 |= LEFT;
    else if (ml->a.x > m_x2)
        outcode1 |= RIGHT;
    if (ml->b.x < m_x)
        outcode2 |= LEFT;
    else if (ml->b.x > m_x2)
        outcode2 |= RIGHT;
    if (outcode1 & outcode2)
        return false;           // trivially outside

    // transform to frame-buffer coordinates.
    fl->a.x = CXMTOF(ml->a.x);
    fl->a.y = CYMTOF(ml->a.y);
    fl->b.x = CXMTOF(ml->b.x);
    fl->b.y = CYMTOF(ml->b.y);
    DOOUTCODE(outcode1, fl->a.x, fl->a.y);
    DOOUTCODE(outcode2, fl->b.x, fl->b.y);
    if (outcode1 & outcode2)
        return false;

    while (outcode1 | outcode2)
    {
        // may be partially inside box
        // find an outside point
        if (outcode1)
            outside = outcode1;
        else
            outside = outcode2;
        // clip to each side
        if (outside & TOP)
        {
            dy = fl->a.y - fl->b.y;
            dx = fl->b.x - fl->a.x;
            tmp.x = fl->a.x + (dx * (fl->a.y)) / dy;
            tmp.y = 0;
        }
        else if (outside & BOTTOM)
        {
            dy = fl->a.y - fl->b.y;
            dx = fl->b.x - fl->a.x;
            tmp.x = fl->a.x + (dx * (fl->a.y - f_h)) / dy;
            tmp.y = f_h - 1;
        }
        else if (outside & RIGHT)
        {
            dy = fl->b.y - fl->a.y;
            dx = fl->b.x - fl->a.x;
            tmp.y = fl->a.y + (dy * (f_w - 1 - fl->a.x)) / dx;
            tmp.x = f_w - 1;
        }
        else if (outside & LEFT)
        {
            dy = fl->b.y - fl->a.y;
            dx = fl->b.x - fl->a.x;
            tmp.y = fl->a.y + (dy * (-fl->a.x)) / dx;
            tmp.x = 0;
        }
        if (outside == outcode1)
        {
            fl->a = tmp;
            DOOUTCODE(outcode1, fl->a.x, fl->a.y);
        }
        else
        {
            fl->b = tmp;
            DOOUTCODE(outcode2, fl->b.x, fl->b.y);
        }
        if (outcode1 & outcode2)
            return false;       // trivially outside
    }

    return true;
}

#undef DOOUTCODE

// Classic Bresenham w/ whatever optimizations I need for speed

void AM_drawFline(fline_t * fl, int color)
{
    register int x, y, dx, dy, sx, sy, ax, ay, d;
    //static fuck = 0;

    switch (color)
    {
        case WALLCOLORS:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y,
                       &antialias[0][0], 8, 3);
            break;
        case FDWALLCOLORS:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y,
                       &antialias[1][0], 8, 3);
            break;
        case CDWALLCOLORS:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y,
                       &antialias[2][0], 8, 3);
            break;
        default:
            {
                // For debugging only
                if (fl->a.x < 0 || fl->a.x >= f_w
                    || fl->a.y < 0 || fl->a.y >= f_h
                    || fl->b.x < 0 || fl->b.x >= f_w
                    || fl->b.y < 0 || fl->b.y >= f_h)
                {
                    //fprintf(stderr, "fuck %d \r", fuck++);
                    return;
                }

#define DOT(xx,yy,cc) fb[(yy)*f_w+(xx)]=(cc)    //the MACRO!

                dx = fl->b.x - fl->a.x;
                ax = 2 * (dx < 0 ? -dx : dx);
                sx = dx < 0 ? -1 : 1;

                dy = fl->b.y - fl->a.y;
                ay = 2 * (dy < 0 ? -dy : dy);
                sy = dy < 0 ? -1 : 1;

                x = fl->a.x;
                y = fl->a.y;

                if (ax > ay)
                {
                    d = ay - ax / 2;
                    while (1)
                    {
                        DOT(x, y, color);
                        if (x == fl->b.x)
                            return;
                        if (d >= 0)
                        {
                            y += sy;
                            d -= ax;
                        }
                        x += sx;
                        d += ay;
                    }
                }
                else
                {
                    d = ax - ay / 2;
                    while (1)
                    {
                        DOT(x, y, color);
                        if (y == fl->b.y)
                            return;
                        if (d >= 0)
                        {
                            x += sx;
                            d -= ay;
                        }
                        y += sy;
                        d += ax;
                    }
                }
            }
    }
}

/* Wu antialiased line drawer.
 * (X0,Y0),(X1,Y1) = line to draw
 * BaseColor = color # of first color in block used for antialiasing, the
 *          100% intensity version of the drawing color
 * NumLevels = size of color block, with BaseColor+NumLevels-1 being the
 *          0% intensity version of the drawing color
 * IntensityBits = log base 2 of NumLevels; the # of bits used to describe
 *          the intensity of the drawing color. 2**IntensityBits==NumLevels
 */
void PUTDOT(short xx, short yy, byte * cc, byte * cm)
{
    static int oldyy;
    static int oldyyshifted;
    byte *oldcc = cc;

    if (xx < 32)
        cc += 7 - (xx >> 2);
    else if (xx > (finit_width - 32))
        cc += 7 - ((finit_width - xx) >> 2);
//      if(cc==oldcc) //make sure that we don't double fade the corners.
//      {
    if (yy < 32)
        cc += 7 - (yy >> 2);
    else if (yy > (finit_height - 32))
        cc += 7 - ((finit_height - yy) >> 2);
//      }
    if (cc > cm && cm != NULL)
    {
        cc = cm;
    }
    else if (cc > oldcc + 6)    // don't let the color escape from the fade table...
    {
        cc = oldcc + 6;
    }
    if (yy == oldyy + 1)
    {
        oldyy++;
        oldyyshifted += (320 << crispy->hires);
    }
    else if (yy == oldyy - 1)
    {
        oldyy--;
        oldyyshifted -= (320 << crispy->hires);
    }
    else if (yy != oldyy)
    {
        oldyy = yy;
        oldyyshifted = yy * (320 << crispy->hires);
    }
    fb[oldyyshifted + xx] = *(cc);
//      fb[(yy)*f_w+(xx)]=*(cc);
}

void DrawWuLine(int X0, int Y0, int X1, int Y1, byte * BaseColor,
                int NumLevels, unsigned short IntensityBits)
{
    unsigned short IntensityShift, ErrorAdj, ErrorAcc;
    unsigned short ErrorAccTemp, Weighting, WeightingComplementMask;
    short DeltaX, DeltaY, Temp, XDir;

    /* Make sure the line runs top to bottom */
    if (Y0 > Y1)
    {
        Temp = Y0;
        Y0 = Y1;
        Y1 = Temp;
        Temp = X0;
        X0 = X1;
        X1 = Temp;
    }
    /* Draw the initial pixel, which is always exactly intersected by
       the line and so needs no weighting */
    PUTDOT(X0, Y0, &BaseColor[0], NULL);

    if ((DeltaX = X1 - X0) >= 0)
    {
        XDir = 1;
    }
    else
    {
        XDir = -1;
        DeltaX = -DeltaX;       /* make DeltaX positive */
    }
    /* Special-case horizontal, vertical, and diagonal lines, which
       require no weighting because they go right through the center of
       every pixel */
    if ((DeltaY = Y1 - Y0) == 0)
    {
        /* Horizontal line */
        while (DeltaX-- != 0)
        {
            X0 += XDir;
            PUTDOT(X0, Y0, &BaseColor[0], NULL);
        }
        return;
    }
    if (DeltaX == 0)
    {
        /* Vertical line */
        do
        {
            Y0++;
            PUTDOT(X0, Y0, &BaseColor[0], NULL);
        }
        while (--DeltaY != 0);
        return;
    }
    //diagonal line.
    if (DeltaX == DeltaY)
    {
        do
        {
            X0 += XDir;
            Y0++;
            PUTDOT(X0, Y0, &BaseColor[0], NULL);
        }
        while (--DeltaY != 0);
        return;
    }
    /* Line is not horizontal, diagonal, or vertical */
    ErrorAcc = 0;               /* initialize the line error accumulator to 0 */
    /* # of bits by which to shift ErrorAcc to get intensity level */
    IntensityShift = 16 - IntensityBits;
    /* Mask used to flip all bits in an intensity weighting, producing the
       result (1 - intensity weighting) */
    WeightingComplementMask = NumLevels - 1;
    /* Is this an X-major or Y-major line? */
    if (DeltaY > DeltaX)
    {
        /* Y-major line; calculate 16-bit fixed-point fractional part of a
           pixel that X advances each time Y advances 1 pixel, truncating the
           result so that we won't overrun the endpoint along the X axis */
        ErrorAdj = ((unsigned long) DeltaX << 16) / (unsigned long) DeltaY;
        /* Draw all pixels other than the first and last */
        while (--DeltaY)
        {
            ErrorAccTemp = ErrorAcc;    /* remember currrent accumulated error */
            ErrorAcc += ErrorAdj;       /* calculate error for next pixel */
            if (ErrorAcc <= ErrorAccTemp)
            {
                /* The error accumulator turned over, so advance the X coord */
                X0 += XDir;
            }
            Y0++;               /* Y-major, so always advance Y */
            /* The IntensityBits most significant bits of ErrorAcc give us the
               intensity weighting for this pixel, and the complement of the
               weighting for the paired pixel */
            Weighting = ErrorAcc >> IntensityShift;
            PUTDOT(X0, Y0, &BaseColor[Weighting], &BaseColor[7]);
            PUTDOT(X0 + XDir, Y0,
                   &BaseColor[(Weighting ^ WeightingComplementMask)],
                   &BaseColor[7]);
        }
        /* Draw the final pixel, which is always exactly intersected by the line
           and so needs no weighting */
        PUTDOT(X1, Y1, &BaseColor[0], NULL);
        return;
    }
    /* It's an X-major line; calculate 16-bit fixed-point fractional part of a
       pixel that Y advances each time X advances 1 pixel, truncating the
       result to avoid overrunning the endpoint along the X axis */
    ErrorAdj = ((unsigned long) DeltaY << 16) / (unsigned long) DeltaX;
    /* Draw all pixels other than the first and last */
    while (--DeltaX)
    {
        ErrorAccTemp = ErrorAcc;        /* remember currrent accumulated error */
        ErrorAcc += ErrorAdj;   /* calculate error for next pixel */
        if (ErrorAcc <= ErrorAccTemp)
        {
            /* The error accumulator turned over, so advance the Y coord */
            Y0++;
        }
        X0 += XDir;             /* X-major, so always advance X */
        /* The IntensityBits most significant bits of ErrorAcc give us the
           intensity weighting for this pixel, and the complement of the
           weighting for the paired pixel */
        Weighting = ErrorAcc >> IntensityShift;
        PUTDOT(X0, Y0, &BaseColor[Weighting], &BaseColor[7]);
        PUTDOT(X0, Y0 + 1,
               &BaseColor[(Weighting ^ WeightingComplementMask)],
               &BaseColor[7]);

    }
    /* Draw the final pixel, which is always exactly intersected by the line
       and so needs no weighting */
    PUTDOT(X1, Y1, &BaseColor[0], NULL);
}

void AM_drawMline(mline_t * ml, int color)
{
    static fline_t fl;

    if (AM_clipMline(ml, &fl))
        AM_drawFline(&fl, color);       // draws it on frame buffer using fb coords

}

void AM_drawGrid(int color)
{
    fixed_t x, y;
    fixed_t start, end;
    mline_t ml;

    // Figure out start of vertical gridlines
    start = m_x;
    if ((start - bmaporgx) % (MAPBLOCKUNITS << FRACBITS))
        start += (MAPBLOCKUNITS << FRACBITS)
            - ((start - bmaporgx) % (MAPBLOCKUNITS << FRACBITS));
    end = m_x + m_w;

    // draw vertical gridlines
    ml.a.y = m_y;
    ml.b.y = m_y + m_h;
    for (x = start; x < end; x += (MAPBLOCKUNITS << FRACBITS))
    {
        ml.a.x = x;
        ml.b.x = x;
        AM_drawMline(&ml, color);
    }

    // Figure out start of horizontal gridlines
    start = m_y;
    if ((start - bmaporgy) % (MAPBLOCKUNITS << FRACBITS))
        start += (MAPBLOCKUNITS << FRACBITS)
            - ((start - bmaporgy) % (MAPBLOCKUNITS << FRACBITS));
    end = m_y + m_h;

    // draw horizontal gridlines
    ml.a.x = m_x;
    ml.b.x = m_x + m_w;
    for (y = start; y < end; y += (MAPBLOCKUNITS << FRACBITS))
    {
        ml.a.y = y;
        ml.b.y = y;
        AM_drawMline(&ml, color);
    }
}

void AM_drawWalls(void)
{
    int i;
    static mline_t l;

    for (i = 0; i < numlines; i++)
    {
        l.a.x = lines[i].v1->x;
        l.a.y = lines[i].v1->y;
        l.b.x = lines[i].v2->x;
        l.b.y = lines[i].v2->y;
        if (cheating || (lines[i].flags & ML_MAPPED))
        {
            if ((lines[i].flags & ML_DONTDRAW) && !cheating)
                continue;
            if (!lines[i].backsector)
            {
                AM_drawMline(&l, WALLCOLORS + lightlev);
            }
            else
            {
                if (lines[i].flags & ML_SECRET) // secret door
                {
                    if (cheating)
                        AM_drawMline(&l, 0);
                    else
                        AM_drawMline(&l, WALLCOLORS + lightlev);
                }
                else if (lines[i].special == 13 || lines[i].special == 83)
                {               // Locked door line -- all locked doors are greed
                    AM_drawMline(&l, GREENKEY);
                }
                else if (lines[i].special == 70 || lines[i].special == 71)
                {               // intra-level teleports are blue
                    AM_drawMline(&l, BLUEKEY);
                }
                else if (lines[i].special == 74 || lines[i].special == 75)
                {               // inter-level teleport/game-winning exit -- both are red
                    AM_drawMline(&l, BLOODRED);
                }
                else if (lines[i].backsector->floorheight
                         != lines[i].frontsector->floorheight)
                {
                    AM_drawMline(&l, FDWALLCOLORS + lightlev);  // floor level change
                }
                else if (lines[i].backsector->ceilingheight
                         != lines[i].frontsector->ceilingheight)
                {
                    AM_drawMline(&l, CDWALLCOLORS + lightlev);  // ceiling level change
                }
                else if (cheating)
                {
                    AM_drawMline(&l, TSWALLCOLORS + lightlev);
                }
            }
        }
        else if (plr->powers[pw_allmap])
        {
            if (!(lines[i].flags & ML_DONTDRAW))
                AM_drawMline(&l, GRAYS + 3);
        }
    }

}

void AM_rotate(fixed_t * x, fixed_t * y, angle_t a)
{
    fixed_t tmpx;

    tmpx = FixedMul(*x, finecosine[a >> ANGLETOFINESHIFT])
        - FixedMul(*y, finesine[a >> ANGLETOFINESHIFT]);
    *y = FixedMul(*x, finesine[a >> ANGLETOFINESHIFT])
        + FixedMul(*y, finecosine[a >> ANGLETOFINESHIFT]);
    *x = tmpx;
}

void AM_drawLineCharacter(mline_t * lineguy, int lineguylines, fixed_t scale,
                          angle_t angle, int color, fixed_t x, fixed_t y)
{
    int i;
    mline_t l;

    for (i = 0; i < lineguylines; i++)
    {
        l.a.x = lineguy[i].a.x;
        l.a.y = lineguy[i].a.y;
        if (scale)
        {
            l.a.x = FixedMul(scale, l.a.x);
            l.a.y = FixedMul(scale, l.a.y);
        }
        if (angle)
            AM_rotate(&l.a.x, &l.a.y, angle);
        l.a.x += x;
        l.a.y += y;

        l.b.x = lineguy[i].b.x;
        l.b.y = lineguy[i].b.y;
        if (scale)
        {
            l.b.x = FixedMul(scale, l.b.x);
            l.b.y = FixedMul(scale, l.b.y);
        }
        if (angle)
            AM_rotate(&l.b.x, &l.b.y, angle);
        l.b.x += x;
        l.b.y += y;

        AM_drawMline(&l, color);
    }
}

void AM_drawPlayers(void)
{
    int i;
    player_t *p;
    static int their_colors[] = {
        AM_PLR1_COLOR,
        AM_PLR2_COLOR,
        AM_PLR3_COLOR,
        AM_PLR4_COLOR,
        AM_PLR5_COLOR,
        AM_PLR6_COLOR,
        AM_PLR7_COLOR,
        AM_PLR8_COLOR
    };
    int their_color = -1;
    int color;

    if (!netgame)
    {
        AM_drawLineCharacter(hexen_player_arrow, HEXEN_NUMPLYRLINES, 0, plr->mo->angle,
                             WHITE, plr->mo->x, plr->mo->y);
        return;
    }

    for (i = 0; i < MAXPLAYERS; i++)
    {
        their_color++;
        p = &players[i];
        if (deathmatch && !singledemo && p != plr)
        {
            continue;
        }
        if (!playeringame[i])
            continue;
        color = their_colors[their_color];
        AM_drawLineCharacter(hexen_player_arrow, HEXEN_NUMPLYRLINES, 0, p->mo->angle,
                             color, p->mo->x, p->mo->y);
    }
}

void AM_drawThings(int colors, int colorrange)
{
    int i;
    mobj_t *t;

    for (i = 0; i < numsectors; i++)
    {
        t = sectors[i].thinglist;
        while (t)
        {
            AM_drawLineCharacter(thintriangle_guy, NUMTHINTRIANGLEGUYLINES,
                                 16 << FRACBITS, t->angle, colors + lightlev,
                                 t->x, t->y);
            t = t->snext;
        }
    }
}

void AM_Drawer(void)
{
    if (!automapactive)
        return;

    UpdateState |= I_FULLSCRN;
    AM_clearFB(BACKGROUND);
    if (grid)
        AM_drawGrid(GRIDCOLORS);
    AM_drawWalls();
    AM_drawPlayers();
    DrawWorldTimer();

    if (cheating == 2)
        AM_drawThings(THINGCOLORS, THINGRANGE);

    MN_DrTextA(P_GetMapName(gamemap), 38, 144);
    if (ShowKills && netgame && deathmatch)
    {
        AM_DrawDeathmatchStats();
    }
}

//===========================================================================
//
// AM_DrawDeathmatchStats
//
//===========================================================================

// 8-player note:  Proper player color names here, too

const char *PlayerColorText[MAXPLAYERS] = {
    "BLUE:",
    "RED:",
    "YELLOW:",
    "GREEN:",
    "JADE:",
    "WHITE:",
    "HAZEL:",
    "PURPLE:"
};

void AM_DrawDeathmatchStats(void)
{
    int i, j, k, m;
    int fragCount[MAXPLAYERS];
    int order[MAXPLAYERS];
    char textBuffer[80];
    int yPosition;

    for (i = 0; i < MAXPLAYERS; i++)
    {
        fragCount[i] = 0;
        order[i] = -1;
    }
    for (i = 0; i < MAXPLAYERS; i++)
    {
        if (!playeringame[i])
        {
            continue;
        }
        else
        {
            for (j = 0; j < MAXPLAYERS; j++)
            {
                if (playeringame[j])
                {
                    fragCount[i] += players[i].frags[j];
                }
            }
            for (k = 0; k < MAXPLAYERS; k++)
            {
                if (order[k] == -1)
                {
                    order[k] = i;
                    break;
                }
                else if (fragCount[i] > fragCount[order[k]])
                {
                    for (m = MAXPLAYERS - 1; m > k; m--)
                    {
                        order[m] = order[m - 1];
                    }
                    order[k] = i;
                    break;
                }
            }
        }
    }
    yPosition = 15;
    for (i = 0; i < MAXPLAYERS; i++)
    {
        if (!playeringame[order[i]])
        {
            continue;
        }
        else
        {
            MN_DrTextA(PlayerColorText[order[i]], 8, yPosition);
            doom_snprintf(textBuffer, sizeof(textBuffer),
                       "%d", fragCount[order[i]]);
            MN_DrTextA(textBuffer, 80, yPosition);
            yPosition += 10;
        }
    }
}

//===========================================================================
//
// DrawWorldTimer
//
//===========================================================================

static void DrawWorldTimer(void)
{
    int days;
    int hours;
    int minutes;
    int seconds;
    int worldTimer;
    char timeBuffer[15];
    char dayBuffer[20];

    worldTimer = players[consoleplayer].worldTimer;

    worldTimer /= 35;
    days = worldTimer / 86400;
    worldTimer -= days * 86400;
    hours = worldTimer / 3600;
    worldTimer -= hours * 3600;
    minutes = worldTimer / 60;
    worldTimer -= minutes * 60;
    seconds = worldTimer;

    doom_snprintf(timeBuffer, sizeof(timeBuffer),
               "%.2d : %.2d : %.2d", hours, minutes, seconds);
    MN_DrTextA(timeBuffer, 240, 8);

    if (days)
    {
        if (days == 1)
        {
            doom_snprintf(dayBuffer, sizeof(dayBuffer), "%.2d DAY", days);
        }
        else
        {
            doom_snprintf(dayBuffer, sizeof(dayBuffer), "%.2d DAYS", days);
        }
        MN_DrTextA(dayBuffer, 240, 20);
        if (days >= 5)
        {
            MN_DrTextA("YOU FREAK!!!", 230, 35);
        }
    }
}
