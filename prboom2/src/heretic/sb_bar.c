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

// SB_bar.c

#include "doomstat.h"
#include "m_cheat.h"
#include "m_random.h"
#include "v_video.h"
#include "r_main.h"
#include "w_wad.h"

#include "dsda/settings.h"

#include "heretic/def.h"
#include "heretic/dstrings.h"

#include "sb_bar.h"

#define STARTREDPALS            1
#define STARTBONUSPALS          9
#define NUMREDPALS              8
#define NUMBONUSPALS            4

// Private Functions

static void ShadeLine(int x, int y, int height, int shade);
static void ShadeChain(void);
static void DrINumber(signed int val, int x, int y);
static void DrBNumber(signed int val, int x, int y);
static void DrawCommonBar(void);
static void DrawMainBar(void);
static void DrawInventoryBar(void);
static void DrawFullScreenStuff(void);

// Public Data

dboolean inventory;
int curpos;
int inv_ptr;
int ArtifactFlash;
int SB_state = -1;
int playerkeys = 0;

// Private Data

static int DisplayTicker = 0;
static int HealthMarker;
static int ChainWiggle;
static player_t *CPlayer;

int LumpLTFACE;
int LumpRTFACE;
int LumpBARBACK;
int LumpCHAIN;
int LumpSTATBAR;
int LumpLIFEGEM;
int LumpLTFCTOP;
int LumpRTFCTOP;
int LumpSELECTBOX;
int LumpINVLFGEM1;
int LumpINVLFGEM2;
int LumpINVRTGEM1;
int LumpINVRTGEM2;
int LumpINumbers[10];
int LumpNEGATIVE;
int LumpSmNumbers[10];
int LumpBLACKSQ;
int LumpINVBAR;
int LumpARMCLEAR;
int LumpCHAINBACK;
int FontBNumBase;
int spinbooklump;
int spinflylump;

char namearti[][10] = {
    {"ARTIBOX"},                // none
    {"ARTIINVU"},               // invulnerability
    {"ARTIINVS"},               // invisibility
    {"ARTIPTN2"},               // health
    {"ARTISPHL"},               // superhealth
    {"ARTIPWBK"},               // tomeofpower
    {"ARTITRCH"},               // torch
    {"ARTIFBMB"},               // firebomb
    {"ARTIEGGC"},               // egg
    {"ARTISOAR"},               // fly
    {"ARTIATLP"}                // teleport
};
int lumparti[11];

//---------------------------------------------------------------------------
//
// PROC SB_Init
//
//---------------------------------------------------------------------------

void SB_Init(void)
{
    int i;
    int startLump;
    extern patchnum_t grnrock;
    extern patchnum_t brdr_t, brdr_b, brdr_l, brdr_r;
    extern patchnum_t brdr_tl, brdr_tr, brdr_bl, brdr_br;

    // magic globals that ends up in the background
    R_SetFloorNum(&grnrock, "FLOOR30");
    R_SetPatchNum(&brdr_t, DEH_String("bordt"));
    R_SetPatchNum(&brdr_b, DEH_String("bordb"));
    R_SetPatchNum(&brdr_l, DEH_String("bordl"));
    R_SetPatchNum(&brdr_r, DEH_String("bordr"));
    R_SetPatchNum(&brdr_tl, DEH_String("bordtl"));
    R_SetPatchNum(&brdr_tr, DEH_String("bordtr"));
    R_SetPatchNum(&brdr_bl, DEH_String("bordbl"));
    R_SetPatchNum(&brdr_br, DEH_String("bordbr"));

    for (i = 0; i < 11; ++i)
    {
      lumparti[i] = (W_CheckNumForName)(DEH_String(namearti[i]), ns_sprites);
    }

    LumpLTFACE = W_GetNumForName(DEH_String("LTFACE"));
    LumpRTFACE = W_GetNumForName(DEH_String("RTFACE"));
    LumpBARBACK = W_GetNumForName(DEH_String("BARBACK"));
    LumpINVBAR = W_GetNumForName(DEH_String("INVBAR"));
    LumpCHAIN = W_GetNumForName(DEH_String("CHAIN"));
    if (deathmatch)
    {
        LumpSTATBAR = W_GetNumForName(DEH_String("STATBAR"));
    }
    else
    {
        LumpSTATBAR = W_GetNumForName(DEH_String("LIFEBAR"));
    }
    if (!netgame)
    {                           // single player game uses red life gem
        LumpLIFEGEM = W_GetNumForName(DEH_String("LIFEGEM2"));
    }
    else
    {
        LumpLIFEGEM = W_GetNumForName(DEH_String("LIFEGEM0") + consoleplayer);
    }
    LumpLTFCTOP = W_GetNumForName(DEH_String("LTFCTOP"));
    LumpRTFCTOP = W_GetNumForName(DEH_String("RTFCTOP"));
    LumpSELECTBOX = W_GetNumForName(DEH_String("SELECTBOX"));
    LumpINVLFGEM1 = W_GetNumForName(DEH_String("INVGEML1"));
    LumpINVLFGEM2 = W_GetNumForName(DEH_String("INVGEML2"));
    LumpINVRTGEM1 = W_GetNumForName(DEH_String("INVGEMR1"));
    LumpINVRTGEM2 = W_GetNumForName(DEH_String("INVGEMR2"));
    LumpBLACKSQ = W_GetNumForName(DEH_String("BLACKSQ"));
    LumpARMCLEAR = W_GetNumForName(DEH_String("ARMCLEAR"));
    LumpCHAINBACK = W_GetNumForName(DEH_String("CHAINBACK"));
    startLump = W_GetNumForName(DEH_String("IN0"));
    for (i = 0; i < 10; i++)
    {
        LumpINumbers[i] = startLump + i;
    }
    LumpNEGATIVE = W_GetNumForName(DEH_String("NEGNUM"));
    FontBNumBase = W_GetNumForName(DEH_String("FONTB16"));
    startLump = W_GetNumForName(DEH_String("SMALLIN0"));
    for (i = 0; i < 10; i++)
    {
        LumpSmNumbers[i] = startLump + i;
    }
    spinbooklump = W_GetNumForName(DEH_String("SPINBK0"));
    spinflylump = W_GetNumForName(DEH_String("SPFLY0"));
}

//---------------------------------------------------------------------------
//
// PROC SB_Ticker
//
//---------------------------------------------------------------------------

void SB_Ticker(void)
{
    int delta;
    int curHealth;

    if (leveltime & 1 && !(paused || (!demoplayback && menuactive && !netgame)))
    {
        ChainWiggle = P_Random(pr_heretic) & 1;
    }
    curHealth = players[consoleplayer].mo->health;
    if (curHealth < 0)
    {
        curHealth = 0;
    }
    if (curHealth < HealthMarker)
    {
        delta = (HealthMarker - curHealth) >> 2;
        if (delta < 1)
        {
            delta = 1;
        }
        else if (delta > 8)
        {
            delta = 8;
        }
        HealthMarker -= delta;
    }
    else if (curHealth > HealthMarker)
    {
        delta = (curHealth - HealthMarker) >> 2;
        if (delta < 1)
        {
            delta = 1;
        }
        else if (delta > 8)
        {
            delta = 8;
        }
        HealthMarker += delta;
    }
}

//---------------------------------------------------------------------------
//
// PROC DrINumber
//
// Draws a three digit number.
//
//---------------------------------------------------------------------------

static void DrINumber(signed int val, int x, int y)
{
    int lump;
    int oldval;

    oldval = val;
    if (val < 0)
    {
        if (val < -9)
        {
            V_DrawNamePatch(x + 1, y + 1, 0, DEH_String("LAME"), CR_DEFAULT, VPT_STRETCH);
        }
        else
        {
            val = -val;
            V_DrawNumPatch(x + 18, y, 0, LumpINumbers[val], CR_DEFAULT, VPT_STRETCH);
            V_DrawNumPatch(x + 9, y, 0, LumpNEGATIVE, CR_DEFAULT, VPT_STRETCH);
        }
        return;
    }
    if (val > 99)
    {
        lump = LumpINumbers[val / 100];
        V_DrawNumPatch(x, y, 0, lump, CR_DEFAULT, VPT_STRETCH);
    }
    val = val % 100;
    if (val > 9 || oldval > 99)
    {
        lump = LumpINumbers[val / 10];
        V_DrawNumPatch(x + 9, y, 0, lump, CR_DEFAULT, VPT_STRETCH);
    }
    val = val % 10;
    lump = LumpINumbers[val];
    V_DrawNumPatch(x + 18, y, 0, lump, CR_DEFAULT, VPT_STRETCH);
}

//---------------------------------------------------------------------------
//
// PROC DrBNumber
//
// Draws a three digit number using FontB
//
//---------------------------------------------------------------------------

static void DrBNumber(signed int val, int x, int y)
{
    int lump;
    int xpos;
    int oldval;

    oldval = val;
    xpos = x;
    if (val < 0)
    {
        val = 0;
    }
    if (val > 99)
    {
        lump = FontBNumBase + val / 100;
        V_DrawShadowedNumPatch(xpos + 6 - R_NumPatchWidth(lump) / 2, y, lump);
    }
    val = val % 100;
    xpos += 12;
    if (val > 9 || oldval > 99)
    {
        lump = FontBNumBase + val / 10;
        V_DrawShadowedNumPatch(xpos + 6 - R_NumPatchWidth(lump) / 2, y, lump);
    }
    val = val % 10;
    xpos += 12;
    lump = FontBNumBase + val;
    V_DrawShadowedNumPatch(xpos + 6 - R_NumPatchWidth(lump) / 2, y, lump);
}

//---------------------------------------------------------------------------
//
// PROC DrSmallNumber
//
// Draws a small two digit number.
//
//---------------------------------------------------------------------------

static void DrSmallNumber(int val, int x, int y)
{
    int lump;

    if (val == 1)
    {
        return;
    }
    if (val > 9)
    {
        lump = LumpSmNumbers[val / 10];
        V_DrawNumPatch(x, y, 0, lump, CR_DEFAULT, VPT_STRETCH);
    }
    val = val % 10;
    lump = LumpSmNumbers[val];
    V_DrawNumPatch(x + 4, y, 0, lump, CR_DEFAULT, VPT_STRETCH);
}

//---------------------------------------------------------------------------
//
// PROC ShadeLine
//
//---------------------------------------------------------------------------

static void ShadeLine(int x, int y, int height, int shade)
{
    // HERETIC_TODO: ShadeLine
    // byte *dest;
    // byte *shades;
    //
    // x <<= crispy->hires;
    // y <<= crispy->hires;
    // height <<= crispy->hires;
    //
    // shades = colormaps + 9 * 256 + shade * 2 * 256;
    // dest = I_VideoBuffer + y * SCREENWIDTH + x;
    // while (height--)
    // {
    //     if (crispy->hires)
    //         *(dest + 1) = *(shades + *dest);
    //     *(dest) = *(shades + *dest);
    //     dest += SCREENWIDTH;
    // }
}

//---------------------------------------------------------------------------
//
// PROC ShadeChain
//
//---------------------------------------------------------------------------

static void ShadeChain(void)
{
    int i;

    for (i = 0; i < 16; i++)
    {
        ShadeLine(277 + i, 190, 10, i / 2);
        ShadeLine(19 + i, 190, 10, 7 - (i / 2));
    }
}

//---------------------------------------------------------------------------
//
// PROC SB_Drawer
//
//---------------------------------------------------------------------------

char ammopic[][10] = {
    {"INAMGLD"},
    {"INAMBOW"},
    {"INAMBST"},
    {"INAMRAM"},
    {"INAMPNX"},
    {"INAMLOB"}
};

static int oldarti = 0;
static int oldartiCount = 0;
static int oldfrags = -9999;
static int oldammo = -1;
static int oldarmor = -1;
static int oldweapon = -1;
static int oldhealth = -1;
static int oldlife = -1;
static int oldkeys = -1;

void SB_Drawer(dboolean statusbaron, dboolean refresh, dboolean fullmenu)
{
    int frame;
    static dboolean hitCenterFrame;

    if (refresh || fullmenu || V_GetMode() == VID_MODEGL) SB_state = -1;

    if (!statusbaron)
    {
        SB_PaletteFlash();
        return;
    }

    CPlayer = &players[consoleplayer];
    if (viewheight == SCREENHEIGHT && !(automapmode & am_active))
    {
        DrawFullScreenStuff();
        SB_state = -1;
    }
    else
    {
        if (SB_state == -1)
        {
            V_DrawNumPatch(0, 158, 0, LumpBARBACK, CR_DEFAULT, VPT_STRETCH);
            if (players[consoleplayer].cheats & CF_GODMODE)
            {
                V_DrawNamePatch(16, 167, 0, DEH_String("GOD1"), CR_DEFAULT, VPT_STRETCH);
                V_DrawNamePatch(287, 167, 0, DEH_String("GOD2"), CR_DEFAULT, VPT_STRETCH);
            }
            oldhealth = -1;
        }
        DrawCommonBar();
        if (!inventory)
        {
            if (SB_state != 0)
            {
                // Main interface
                V_DrawNumPatch(34, 160, 0, LumpSTATBAR, CR_DEFAULT, VPT_STRETCH);
                oldarti = 0;
                oldammo = -1;
                oldarmor = -1;
                oldweapon = -1;
                oldfrags = -9999;       //can't use -1, 'cuz of negative frags
                oldlife = -1;
                oldkeys = -1;
            }
            DrawMainBar();
            SB_state = 0;
        }
        else
        {
            if (SB_state != 1)
            {
                V_DrawNumPatch(34, 160, 0, LumpINVBAR, CR_DEFAULT, VPT_STRETCH);
            }
            DrawInventoryBar();
            SB_state = 1;
        }
    }
    SB_PaletteFlash();

    // Flight icons
    if (CPlayer->powers[pw_flight])
    {
        if (CPlayer->powers[pw_flight] > BLINKTHRESHOLD
            || !(CPlayer->powers[pw_flight] & 16))
        {
            frame = (leveltime / 3) & 15;
            if (CPlayer->mo->flags2 & MF2_FLY)
            {
                if (hitCenterFrame && (frame != 15 && frame != 0))
                {
                    V_DrawNumPatch(20, 17, 0, spinflylump + 15, CR_DEFAULT, VPT_STRETCH);
                }
                else
                {
                    V_DrawNumPatch(20, 17, 0, spinflylump + frame, CR_DEFAULT, VPT_STRETCH);
                    hitCenterFrame = false;
                }
            }
            else
            {
                if (!hitCenterFrame && (frame != 15 && frame != 0))
                {
                    V_DrawNumPatch(20, 17, 0, spinflylump + frame, CR_DEFAULT, VPT_STRETCH);
                    hitCenterFrame = false;
                }
                else
                {
                    V_DrawNumPatch(20, 17, 0, spinflylump + 15, CR_DEFAULT, VPT_STRETCH);
                    hitCenterFrame = true;
                }
            }
            BorderTopRefresh = true;
            // UpdateState |= I_MESSAGES;
        }
        else
        {
            BorderTopRefresh = true;
            // UpdateState |= I_MESSAGES;
        }
    }

    if (CPlayer->powers[pw_weaponlevel2] && !CPlayer->chickenTics)
    {
        if (CPlayer->powers[pw_weaponlevel2] > BLINKTHRESHOLD
            || !(CPlayer->powers[pw_weaponlevel2] & 16))
        {
            frame = (leveltime / 3) & 15;
            V_DrawNumPatch(300, 17, 0, spinbooklump + frame, CR_DEFAULT, VPT_STRETCH);
            BorderTopRefresh = true;
            // UpdateState |= I_MESSAGES;
        }
        else
        {
            BorderTopRefresh = true;
            // UpdateState |= I_MESSAGES;
        }
    }
}

// sets the new palette based upon current values of player->damagecount
// and player->bonuscount
void SB_PaletteFlash(void)
{
    static int sb_palette = 0;
    int palette;

    CPlayer = &players[consoleplayer];

    if (CPlayer->damagecount)
    {
        palette = (CPlayer->damagecount + 7) >> 3;
        if (palette >= NUMREDPALS)
        {
            palette = NUMREDPALS - 1;
        }
        palette += STARTREDPALS;
    }
    else if (CPlayer->bonuscount)
    {
        palette = (CPlayer->bonuscount + 7) >> 3;
        if (palette >= NUMBONUSPALS)
        {
            palette = NUMBONUSPALS - 1;
        }
        palette += STARTBONUSPALS;
    }
    else
    {
        palette = 0;
    }
    if (palette != sb_palette)
    {
        SB_state = -1;
        sb_palette = palette;
        V_SetPalette(sb_palette);
    }
}

//---------------------------------------------------------------------------
//
// PROC DrawCommonBar
//
//---------------------------------------------------------------------------

void DrawCommonBar(void)
{
    int chainY;
    int healthPos;

    if (!dsda_HideHorns())
    {
      V_DrawNumPatch(0,  148, 0, LumpLTFCTOP, CR_DEFAULT, VPT_STRETCH);
      V_DrawNumPatch(290,  148, 0, LumpRTFCTOP, CR_DEFAULT, VPT_STRETCH);
    }

    if (oldhealth != HealthMarker)
    {
        oldhealth = HealthMarker;
        healthPos = HealthMarker;
        if (healthPos < 0)
        {
            healthPos = 0;
        }
        if (healthPos > 100)
        {
            healthPos = 100;
        }
        healthPos = (healthPos * 256) / 100;
        chainY =
            (HealthMarker == CPlayer->mo->health) ? 191 : 191 + ChainWiggle;
        V_DrawNumPatch(0,  190, 0, LumpCHAINBACK, CR_DEFAULT, VPT_STRETCH);
        V_DrawNumPatch(2 + (healthPos % 17),  chainY, 0, LumpCHAIN, CR_DEFAULT, VPT_STRETCH);
        V_DrawNumPatch(17 + healthPos,  chainY, 0, LumpLIFEGEM, CR_DEFAULT, VPT_STRETCH);
        V_DrawNumPatch(0,  190, 0, LumpLTFACE, CR_DEFAULT, VPT_STRETCH);
        V_DrawNumPatch(276,  190, 0, LumpRTFACE, CR_DEFAULT, VPT_STRETCH);
        ShadeChain();
        // UpdateState |= I_STATBAR;
    }
}

//---------------------------------------------------------------------------
//
// PROC DrawMainBar
//
//---------------------------------------------------------------------------

void DrawMainBar(void)
{
    int i;
    int temp;

    // Ready artifact
    if (ArtifactFlash)
    {
        V_DrawNumPatch(180,  161, 0, LumpBLACKSQ, CR_DEFAULT, VPT_STRETCH);

        temp = W_GetNumForName(DEH_String("useartia")) + ArtifactFlash - 1;

        V_DrawNumPatch(182, 161, 0, temp, CR_DEFAULT, VPT_STRETCH);
        ArtifactFlash--;
        oldarti = -1;           // so that the correct artifact fills in after the flash
        // UpdateState |= I_STATBAR;
    }
    else if (oldarti != CPlayer->readyArtifact
             || oldartiCount != CPlayer->inventory[inv_ptr].count)
    {
        V_DrawNumPatch(180,  161, 0, LumpBLACKSQ, CR_DEFAULT, VPT_STRETCH);
        if (CPlayer->readyArtifact > 0)
        {
            V_DrawNumPatch(
              179, 160, 0, lumparti[CPlayer->readyArtifact], CR_DEFAULT, VPT_STRETCH
            );

            DrSmallNumber(CPlayer->inventory[inv_ptr].count, 201, 182);
        }
        oldarti = CPlayer->readyArtifact;
        oldartiCount = CPlayer->inventory[inv_ptr].count;
        // UpdateState |= I_STATBAR;
    }

    // Frags
    if (deathmatch)
    {
        temp = 0;
        for (i = 0; i < MAXPLAYERS; i++)
        {
            temp += CPlayer->frags[i];
        }
        if (temp != oldfrags)
        {
            V_DrawNumPatch(57,  171, 0, LumpARMCLEAR, CR_DEFAULT, VPT_STRETCH);
            DrINumber(temp, 61, 170);
            oldfrags = temp;
            // UpdateState |= I_STATBAR;
        }
    }
    else
    {
        temp = HealthMarker;
        if (temp < 0)
        {
            temp = 0;
        }
        else if (temp > 100)
        {
            temp = 100;
        }
        if (oldlife != temp)
        {
            oldlife = temp;
            V_DrawNumPatch(57,  171, 0, LumpARMCLEAR, CR_DEFAULT, VPT_STRETCH);
            DrINumber(temp, 61, 170);
            // UpdateState |= I_STATBAR;
        }
    }

    // Keys
    if (oldkeys != playerkeys)
    {
        if (CPlayer->cards[key_yellow])
        {
            V_DrawNamePatch(153, 164, 0, DEH_String("ykeyicon"), CR_DEFAULT, VPT_STRETCH);
        }
        if (CPlayer->cards[key_green])
        {
            V_DrawNamePatch(153, 172, 0, DEH_String("gkeyicon"), CR_DEFAULT, VPT_STRETCH);
        }
        if (CPlayer->cards[key_blue])
        {
            V_DrawNamePatch(153, 180, 0, DEH_String("bkeyicon"), CR_DEFAULT, VPT_STRETCH);
        }
        oldkeys = playerkeys;
        // UpdateState |= I_STATBAR;
    }
    // Ammo
    temp = CPlayer->ammo[wpnlev1info[CPlayer->readyweapon].ammo];
    if (oldammo != temp || oldweapon != CPlayer->readyweapon)
    {
        V_DrawNumPatch(108,  161, 0, LumpBLACKSQ, CR_DEFAULT, VPT_STRETCH);
        if (temp && CPlayer->readyweapon > 0 && CPlayer->readyweapon < 7)
        {
            DrINumber(temp, 109, 162);
            V_DrawNamePatch(
              111, 172, 0, DEH_String(ammopic[CPlayer->readyweapon - 1]), CR_DEFAULT, VPT_STRETCH
            );
        }
        oldammo = temp;
        oldweapon = CPlayer->readyweapon;
        // UpdateState |= I_STATBAR;
    }

    // Armor
    if (oldarmor != CPlayer->armorpoints)
    {
        V_DrawNumPatch(224,  171, 0, LumpARMCLEAR, CR_DEFAULT, VPT_STRETCH);
        DrINumber(CPlayer->armorpoints, 228, 170);
        oldarmor = CPlayer->armorpoints;
        // UpdateState |= I_STATBAR;
    }
}

//---------------------------------------------------------------------------
//
// PROC DrawInventoryBar
//
//---------------------------------------------------------------------------

void DrawInventoryBar(void)
{
    int i;
    int x;
    int lump;

    x = inv_ptr - curpos;
    // UpdateState |= I_STATBAR;
    V_DrawNumPatch(34,  160, 0, LumpINVBAR, CR_DEFAULT, VPT_STRETCH);
    for (i = 0; i < 7; i++)
    {
        if (CPlayer->inventorySlotNum > x + i
            && CPlayer->inventory[x + i].type != arti_none)
        {
            V_DrawNumPatch(
              50 + i * 31, 160, 0,
              lumparti[CPlayer->inventory[x + i].type], CR_DEFAULT, VPT_STRETCH
            );
            DrSmallNumber(CPlayer->inventory[x + i].count, 69 + i * 31, 182);
        }
    }
    V_DrawNumPatch(50 + curpos * 31,  189, 0, LumpSELECTBOX, CR_DEFAULT, VPT_STRETCH);
    if (x != 0)
    {
        lump = !(leveltime & 4) ? LumpINVLFGEM1 : LumpINVLFGEM2;
        V_DrawNumPatch(38, 159, 0, lump, CR_DEFAULT, VPT_STRETCH);
    }
    if (CPlayer->inventorySlotNum - x > 7)
    {
        lump = !(leveltime & 4) ? LumpINVRTGEM1 : LumpINVRTGEM2;
        V_DrawNumPatch(269, 159, 0, lump, CR_DEFAULT, VPT_STRETCH);
    }
}

void DrawFullScreenStuff(void)
{
    const char *name;
    int lump;
    int i;
    int x;
    int temp;

    // UpdateState |= I_FULLSCRN;
    if (CPlayer->mo->health > 0)
    {
        DrBNumber(CPlayer->mo->health, 5, 180);
    }
    else
    {
        DrBNumber(0, 5, 180);
    }
    if (deathmatch)
    {
        temp = 0;
        for (i = 0; i < MAXPLAYERS; i++)
        {
            if (playeringame[i])
            {
                temp += CPlayer->frags[i];
            }
        }
        DrINumber(temp, 45, 185);
    }
    if (!inventory)
    {
        if (CPlayer->readyArtifact > 0)
        {
            lump = lumparti[CPlayer->readyArtifact];
            V_DrawTLNamePatch(286, 170, DEH_String("ARTIBOX"));
            V_DrawNumPatch(286, 170, 0, lump, CR_DEFAULT, VPT_STRETCH);
            DrSmallNumber(CPlayer->inventory[inv_ptr].count, 307, 192);
        }
    }
    else
    {
        x = inv_ptr - curpos;
        for (i = 0; i < 7; i++)
        {
            V_DrawTLNamePatch(50 + i * 31, 168, DEH_String("ARTIBOX"));
            if (CPlayer->inventorySlotNum > x + i
                && CPlayer->inventory[x + i].type != arti_none)
            {
                lump = lumparti[CPlayer->inventory[x + i].type];
                V_DrawNumPatch(50 + i * 31, 168, 0, lump, CR_DEFAULT, VPT_STRETCH);
                DrSmallNumber(CPlayer->inventory[x + i].count, 69 + i * 31, 190);
            }
        }
        V_DrawNumPatch(50 + curpos * 31,  197, 0, LumpSELECTBOX, CR_DEFAULT, VPT_STRETCH);
        if (x != 0)
        {
            lump = !(leveltime & 4) ? LumpINVLFGEM1 : LumpINVLFGEM2;
            V_DrawNumPatch(38, 167, 0, lump, CR_DEFAULT, VPT_STRETCH);
        }
        if (CPlayer->inventorySlotNum - x > 7)
        {
            lump = !(leveltime & 4) ? LumpINVRTGEM1 : LumpINVRTGEM2;
            V_DrawNumPatch(269, 167, 0, lump, CR_DEFAULT, VPT_STRETCH);
        }
    }
}

//--------------------------------------------------------------------------
//
// FUNC SB_Responder
//
//--------------------------------------------------------------------------

dboolean SB_Responder(event_t * ev)
{
  // Note to self: doom logic
  // // Filter automap on/off.
  // if (ev->type == ev_keyup && (ev->data1 & 0xffff0000) == AM_MSGHEADER)
  //   {
  //     switch(ev->data1)
  //       {
  //       case AM_MSGENTERED:
  //         st_gamestate = AutomapState;
  //         st_firsttime = true;
  //         break;
  //
  //       case AM_MSGEXITED:
  //         st_gamestate = FirstPersonState;
  //         break;
  //       }
  //   }
  return M_CheatResponder(ev);
}
