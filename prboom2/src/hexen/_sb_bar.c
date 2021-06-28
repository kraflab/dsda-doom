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

//==========================================================================
//
// DrawMainBar
//
//==========================================================================

void DrawMainBar(void)
{
    int i, j, k;
    int temp;
    patch_t *manaPatch1, *manaPatch2;
    patch_t *manaVialPatch1, *manaVialPatch2;

    manaPatch1 = NULL;
    manaPatch2 = NULL;
    manaVialPatch1 = NULL;
    manaVialPatch2 = NULL;

    // Ready artifact
    if (ArtifactFlash)
    {
        V_DrawNumPatch(144, 160, 0, LumpARTICLEAR, CR_DEFAULT, VPT_STRETCH);
        V_DrawNumPatch(148, 164, 0, W_GetNumForName("useartia")
                                             + ArtifactFlash - 1, CR_DEFAULT, VPT_STRETCH);
        ArtifactFlash--;
        oldarti = -1;           // so that the correct artifact fills in after the flash
    }
    else if (oldarti != CPlayer->readyArtifact
             || oldartiCount != CPlayer->inventory[inv_ptr].count)
    {
        V_DrawNumPatch(144, 160, 0, LumpARTICLEAR, CR_DEFAULT, VPT_STRETCH);
        if (CPlayer->readyArtifact > 0)
        {
            V_DrawNumPatch(143, 163, 0,
                           lumparti[CPlayer->readyArtifact], CR_DEFAULT, VPT_STRETCH);
            if (CPlayer->inventory[inv_ptr].count > 1)
            {
                DrSmallNumber(CPlayer->inventory[inv_ptr].count, 162, 184);
            }
        }
        oldarti = CPlayer->readyArtifact;
        oldartiCount = CPlayer->inventory[inv_ptr].count;
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
            V_DrawNumPatch(38, 162, 0, LumpKILLS, CR_DEFAULT, VPT_STRETCH);
            DrINumber(temp, 40, 176);
            oldfrags = temp;
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
            V_DrawNumPatch(41, 178, 0, LumpARMCLEAR, CR_DEFAULT, VPT_STRETCH);
            if (temp >= 25)
            {
                DrINumber(temp, 40, 176);
            }
            else
            {
                DrRedINumber(temp, 40, 176);
            }
        }
    }
    // Mana
    temp = CPlayer->ammo[0];
    if (oldmana1 != temp)
    {
        V_DrawNumPatch(77, 178, 0, LumpMANACLEAR, CR_DEFAULT, VPT_STRETCH);
        DrSmallNumber(temp, 79, 181);
        manaVialPatch1 = (patch_t *) 1; // force a vial update
        if (temp == 0)
        {                       // Draw Dim Mana icon
            manaPatch1 = LumpMANADIM1;
        }
        else if (oldmana1 == 0)
        {
            manaPatch1 = LumpMANABRIGHT1;
        }
        oldmana1 = temp;
    }
    temp = CPlayer->ammo[1];
    if (oldmana2 != temp)
    {
        V_DrawNumPatch(109, 178, 0, LumpMANACLEAR, CR_DEFAULT, VPT_STRETCH);
        DrSmallNumber(temp, 111, 181);
        manaVialPatch1 = (patch_t *) 1; // force a vial update
        if (temp == 0)
        {                       // Draw Dim Mana icon
            manaPatch2 = LumpMANADIM2;
        }
        else if (oldmana2 == 0)
        {
            manaPatch2 = LumpMANABRIGHT2;
        }
        oldmana2 = temp;
    }
    if (oldweapon != CPlayer->readyweapon || manaPatch1 || manaPatch2
        || manaVialPatch1)
    {                           // Update mana graphics based upon mana count/weapon type
        if (CPlayer->readyweapon == wp_first)
        {
            manaPatch1 = LumpMANADIM1;
            manaPatch2 = LumpMANADIM2;
            manaVialPatch1 = LumpMANAVIALDIM1;
            manaVialPatch2 = LumpMANAVIALDIM2;
        }
        else if (CPlayer->readyweapon == wp_second)
        {
            if (!manaPatch1)
            {
                manaPatch1 = LumpMANABRIGHT1;
            }
            manaVialPatch1 = LumpMANAVIAL1;
            manaPatch2 = LumpMANADIM2;
            manaVialPatch2 = LumpMANAVIALDIM2;
        }
        else if (CPlayer->readyweapon == wp_third)
        {
            manaPatch1 = LumpMANADIM1;
            manaVialPatch1 = LumpMANAVIALDIM1;
            if (!manaPatch2)
            {
                manaPatch2 = LumpMANABRIGHT2;
            }
            manaVialPatch2 = LumpMANAVIAL2;
        }
        else
        {
            manaVialPatch1 = LumpMANAVIAL1;
            manaVialPatch2 = LumpMANAVIAL2;
            if (!manaPatch1)
            {
                manaPatch1 = LumpMANABRIGHT1;
            }
            if (!manaPatch2)
            {
                manaPatch2 = LumpMANABRIGHT2;
            }
        }
        V_DrawNumPatch(77, 164, 0, manaPatch1, CR_DEFAULT, VPT_STRETCH);
        V_DrawNumPatch(110, 164, 0, manaPatch2, CR_DEFAULT, VPT_STRETCH);
        V_DrawNumPatch(94, 164, 0, manaVialPatch1, CR_DEFAULT, VPT_STRETCH);
        for (i = 165; i < 187 - (22 * CPlayer->ammo[0]) / MAX_MANA; i++)
        {
         for (j = 0; j <= crispy->hires; j++)
          for (k = 0; k <= crispy->hires; k++)
          {
            I_VideoBuffer[((i << crispy->hires) + j) * SCREENWIDTH + ((95 << crispy->hires) + k)] = 0;
            I_VideoBuffer[((i << crispy->hires) + j) * SCREENWIDTH + ((96 << crispy->hires) + k)] = 0;
            I_VideoBuffer[((i << crispy->hires) + j) * SCREENWIDTH + ((97 << crispy->hires) + k)] = 0;
          }
        }
        V_DrawNumPatch(102, 164, 0, manaVialPatch2, CR_DEFAULT, VPT_STRETCH);
        for (i = 165; i < 187 - (22 * CPlayer->ammo[1]) / MAX_MANA; i++)
        {
         for (j = 0; j <= crispy->hires; j++)
          for (k = 0; k <= crispy->hires; k++)
          {
            I_VideoBuffer[((i << crispy->hires) + j) * SCREENWIDTH + ((103 << crispy->hires) + k)] = 0;
            I_VideoBuffer[((i << crispy->hires) + j) * SCREENWIDTH + ((104 << crispy->hires) + k)] = 0;
            I_VideoBuffer[((i << crispy->hires) + j) * SCREENWIDTH + ((105 << crispy->hires) + k)] = 0;
          }
        }
        oldweapon = CPlayer->readyweapon;
    }
    // Armor
    temp = AutoArmorSave[CPlayer->pclass]
        + CPlayer->armorpoints[ARMOR_ARMOR] +
        CPlayer->armorpoints[ARMOR_SHIELD] +
        CPlayer->armorpoints[ARMOR_HELMET] +
        CPlayer->armorpoints[ARMOR_AMULET];
    if (oldarmor != temp)
    {
        oldarmor = temp;
        V_DrawNumPatch(255, 178, 0, LumpARMCLEAR, CR_DEFAULT, VPT_STRETCH);
        DrINumber(FixedDiv(temp, 5 * FRACUNIT) >> FRACBITS, 250, 176);
    }
    // Weapon Pieces
    if (oldpieces != CPlayer->pieces)
    {
        DrawWeaponPieces();
        oldpieces = CPlayer->pieces;
    }
}
