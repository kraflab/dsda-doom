/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 *-----------------------------------------------------------------------------*/

#if (R_DRAWCOLUMN_PIPELINE & RDC_TRANSLUCENT)
#define GETDESTCOLOR(col1, col2) (temptranmap[((col1)<<8)+(col2)])
#else
#define GETDESTCOLOR(col) (col)
#endif

//
// R_FlushWholeOpaque
//
// Flushes the entire columns in the buffer, one at a time.
// This is used when a quad flush isn't possible.
// Opaque version -- no remapping whatsoever.
//
static void R_FLUSHWHOLE_FUNCNAME(void)
{
   // Lovey01 04/29/2023: Scaled software fuzz algorithm
#if (R_DRAWCOLUMN_PIPELINE & RDC_FUZZ)
   int x;
   byte *dest, *topleft;
   int intensitycmap;
   int yl;
   int count, count2, cmask;
   int fp, cs;

   x = temp_x;
   topleft = drawvars.topleft + startx;
   cs = fuzzcellsize;

   while (--x >= 0)
   {
      yl = tempyl[x];
      dest = topleft + yl * drawvars.pitch + x;
      count = tempyh[x] - yl + 1;

      // Draw fuzz stripe independently from vertical screen position
      fp = fuzzpos + (yl / cs);

      // Draw column, cell stripe by cell stripe
      count2 = cs - (yl % cs);
      do
      {
         intensitycmap = fuzzcmaps[fp % FUZZTABLE] << 8;

         count -= count2;

         // if (count < 0) {
         //    count2 += count;
         //    count = 0;
         // }
         cmask = count >> 31;
         count2 += count & cmask;
         count &= ~cmask;

         // Draw cell stripe, pixel by pixel
         do
         {
            *dest = tempfuzzmap[intensitycmap + *dest];
            dest += drawvars.pitch;
         } while (--count2);

         ++fp;
         count2 = cs;
      } while (count);
   }
#else  /* if (R_DRAWCOLUMN_PIPELINE & RDC_FUZZ) */
   byte *source;
   byte *dest;
   int  count, yl;

   while(--temp_x >= 0)
   {
      yl     = tempyl[temp_x];
      source = &tempbuf[temp_x + (yl << 2)];
      dest   = drawvars.topleft + yl*drawvars.pitch + startx + temp_x;
      count  = tempyh[temp_x] - yl + 1;

      while(--count >= 0)
      {
#if (R_DRAWCOLUMN_PIPELINE & RDC_TRANSLUCENT)
         *dest = GETDESTCOLOR(*dest, *source);
#else
         *dest = *source;
#endif

         source += 4;
         dest += drawvars.pitch;
      }
   }
#endif  /* if (R_DRAWCOLUMN_PIPELINE & RDC_FUZZ) */
}

//
// R_FlushHTOpaque
//
// Flushes the head and tail of columns in the buffer in
// preparation for a quad flush.
// Opaque version -- no remapping whatsoever.
//
static void R_FLUSHHEADTAIL_FUNCNAME(void)
{
   byte *source;
   byte *dest;
   int count, colnum = 0;
   int yl, yh;

#if (R_DRAWCOLUMN_PIPELINE & RDC_FUZZ)
   // Lovey01 04/29/2023: Only whole flushes are supported for fuzz
   R_FLUSHWHOLE_FUNCNAME();
   return;
#endif

   while(colnum < 4)
   {
      yl = tempyl[colnum];
      yh = tempyh[colnum];

      // flush column head
      if(yl < commontop)
      {
         source = &tempbuf[colnum + (yl << 2)];
         dest   = drawvars.topleft + yl*drawvars.pitch + startx + colnum;
         count  = commontop - yl;

         while(--count >= 0)
         {
#if (R_DRAWCOLUMN_PIPELINE & RDC_TRANSLUCENT)
            // haleyjd 09/11/04: use temptranmap here
            *dest = GETDESTCOLOR(*dest, *source);
#else
            *dest = *source;
#endif

            source += 4;
            dest += drawvars.pitch;
         }
      }

      // flush column tail
      if(yh > commonbot)
      {
         source = &tempbuf[colnum + ((commonbot + 1) << 2)];
         dest   = drawvars.topleft + (commonbot + 1)*drawvars.pitch + startx + colnum;
         count  = yh - commonbot;

         while(--count >= 0)
         {
#if (R_DRAWCOLUMN_PIPELINE & RDC_TRANSLUCENT)
            // haleyjd 09/11/04: use temptranmap here
            *dest = GETDESTCOLOR(*dest, *source);
#else
            *dest = *source;
#endif

            source += 4;
            dest += drawvars.pitch;
         }
      }
      ++colnum;
   }
}

static void R_FLUSHQUAD_FUNCNAME(void)
{
   byte *source = &tempbuf[commontop << 2];
   byte *dest = drawvars.topleft + commontop*drawvars.pitch + startx;
   int count;
#if (R_DRAWCOLUMN_PIPELINE & RDC_FUZZ)
   // Lovey01 04/29/2023: Only whole flushes are supported for fuzz
   return;
#endif

   count = commonbot - commontop + 1;

#if (R_DRAWCOLUMN_PIPELINE & RDC_TRANSLUCENT)
   while(--count >= 0)
   {
      dest[0] = GETDESTCOLOR(dest[0], source[0]);
      dest[1] = GETDESTCOLOR(dest[1], source[1]);
      dest[2] = GETDESTCOLOR(dest[2], source[2]);
      dest[3] = GETDESTCOLOR(dest[3], source[3]);
      source += 4 * sizeof(byte);
      dest += drawvars.pitch * sizeof(byte);
   }
#else
   if ((sizeof(int) == 4) && (((intptr_t)source % 4) == 0) && (((intptr_t)dest % 4) == 0)) {
      while(--count >= 0)
      {
         *(int *)dest = *(int *)source;
         source += 4 * sizeof(byte);
         dest += drawvars.pitch * sizeof(byte);
      }
   } else {
      while(--count >= 0)
      {
         dest[0] = source[0];
         dest[1] = source[1];
         dest[2] = source[2];
         dest[3] = source[3];
         source += 4 * sizeof(byte);
         dest += drawvars.pitch * sizeof(byte);
      }
   }
#endif
}

#undef GETDESTCOLOR
#undef R_DRAWCOLUMN_PIPELINE
#undef R_FLUSHWHOLE_FUNCNAME
#undef R_FLUSHHEADTAIL_FUNCNAME
#undef R_FLUSHQUAD_FUNCNAME
