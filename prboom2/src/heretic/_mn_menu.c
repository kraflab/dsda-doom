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

// MN_menu.c

#include <stdlib.h>
#include <ctype.h>

#include "deh_str.h"
#include "doomdef.h"
#include "doomkeys.h"
#include "i_input.h"
#include "i_system.h"
#include "i_swap.h"
#include "m_controls.h"
#include "m_misc.h"
#include "p_local.h"
#include "r_local.h"
#include "s_sound.h"
#include "v_video.h"
#include "v_trans.h" // [crispy] dp_translation

// Macros

#define LEFT_DIR 0
#define RIGHT_DIR 1
#define ITEM_HEIGHT 20
#define SELECTOR_XOFFSET (-28)
#define SELECTOR_YOFFSET (-1)
#define SLOTTEXTLEN     16
#define ASCII_CURSOR '['

// Types

typedef enum
{
    ITT_EMPTY,
    ITT_EFUNC,
    ITT_LRFUNC,
    ITT_SETMENU,
    ITT_INERT
} ItemType_t;

typedef enum
{
    MENU_MAIN,
    MENU_EPISODE,
    MENU_SKILL,
    MENU_OPTIONS,
    MENU_OPTIONS2,
    MENU_FILES,
    MENU_LOAD,
    MENU_SAVE,
    MENU_NONE
} MenuType_t;

typedef struct
{
    ItemType_t type;
    const char *text;
    boolean(*func) (int option);
    int option;
    MenuType_t menu;
} MenuItem_t;

typedef struct
{
    int x;
    int y;
    void (*drawFunc) (void);
    int itemCount;
    MenuItem_t *items;
    int oldItPos;
    MenuType_t prevMenu;
} Menu_t;

// Private Functions

static void InitFonts(void);
static void SetMenu(MenuType_t menu);
static boolean SCNetCheck(int option);
static boolean SCQuitGame(int option);
static boolean SCEpisode(int option);
static boolean SCSkill(int option);
static boolean SCMouseSensi(int option);
static boolean SCSfxVolume(int option);
static boolean SCMusicVolume(int option);
static boolean SCScreenSize(int option);
static boolean SCMessages(int option);
static boolean SCEndGame(int option);
static boolean SCInfo(int option);
static void MN_DrawInfo(void);

// External Functions

extern void I_ReInitGraphics(int reinit);
extern void R_ExecuteSetViewSize(void);
extern void AM_LevelInit(void);
extern void AM_initVariables(void);

// External Data

extern int detailLevel;
extern int screenblocks;
extern boolean automapactive;

// Public Data

boolean MenuActive;
int InfoType;
boolean messageson;

// Private Data

static Menu_t *CurrentMenu;
static int CurrentItPos;
static int MenuEpisode;
static int MenuTime;
static boolean soundchanged;

boolean askforquit;
static int typeofask;
static boolean FileMenuKeySteal;
static boolean slottextloaded;
static char SlotText[6][SLOTTEXTLEN + 2];
static char oldSlotText[SLOTTEXTLEN + 2];
static int SlotStatus[6];
static int slotptr;
static int currentSlot;
static int quicksave;
static int quickload;

static Menu_t *Menus[] = {
    &MainMenu,
    &EpisodeMenu,
    &SkillMenu,
    &OptionsMenu,
    &Options2Menu,
    &FilesMenu,
    &LoadMenu,
    &SaveMenu
};

// [crispy] reload current level / go to next level
// adapted from prboom-plus/src/e6y.c:369-449
static int G_ReloadLevel(void)
{
  int result = false;

  if (gamestate == GS_LEVEL)
  {
    // [crispy] restart demos from the map they were started
    if (demorecording)
    {
      gamemap = startmap;
      gameepisode = startepisode;
    }
    G_DeferedInitNew(gameskill, gameepisode, gamemap);
    result = true;
  }

  return result;
}

//---------------------------------------------------------------------------
//
// PROC MN_Drawer
//
//---------------------------------------------------------------------------

const char *QuitEndMsg[] = {
    "ARE YOU SURE YOU WANT TO QUIT?",
    "ARE YOU SURE YOU WANT TO END THE GAME?",
    "DO YOU WANT TO QUICKSAVE THE GAME NAMED",
    "DO YOU WANT TO QUICKLOAD THE GAME NAMED"
};

//---------------------------------------------------------------------------
//
// PROC SCNetCheck
//
//---------------------------------------------------------------------------

static boolean SCNetCheck(int option)
{
    if (!netgame)
    {                           // okay to go into the menu
        return true;
    }
    switch (option)
    {
        case 1:
            P_SetMessage(&players[consoleplayer],
                         "YOU CAN'T START A NEW GAME IN NETPLAY!", true);
            break;
        case 2:
            P_SetMessage(&players[consoleplayer],
                         "YOU CAN'T LOAD A GAME IN NETPLAY!", true);
            break;
        default:
            break;
    }
    MenuActive = false;
    return false;
}

//---------------------------------------------------------------------------
//
// PROC SCQuitGame
//
//---------------------------------------------------------------------------

static boolean SCQuitGame(int option)
{
    MenuActive = false;
    askforquit = true;
    typeofask = 1;              //quit game
    if (!netgame && !demoplayback)
    {
        paused = true;
    }
    return true;
}

//---------------------------------------------------------------------------
//
// PROC SCEndGame
//
//---------------------------------------------------------------------------

static boolean SCEndGame(int option)
{
    if (demoplayback || netgame)
    {
        return false;
    }
    MenuActive = false;
    askforquit = true;
    typeofask = 2;              //endgame
    if (!netgame && !demoplayback)
    {
        paused = true;
    }
    return true;
}

//---------------------------------------------------------------------------
//
// PROC SCMessages
//
//---------------------------------------------------------------------------

static boolean SCMessages(int option)
{
    messageson ^= 1;
    if (messageson)
    {
        P_SetMessage(&players[consoleplayer], DEH_String("MESSAGES ON"), true);
    }
    else
    {
        P_SetMessage(&players[consoleplayer], DEH_String("MESSAGES OFF"), true);
    }
    S_StartSound(NULL, sfx_chat);
    return true;
}

//---------------------------------------------------------------------------
//
// PROC SCEpisode
//
//---------------------------------------------------------------------------

static boolean SCEpisode(int option)
{
    if (gamemode == shareware && option > 1)
    {
        P_SetMessage(&players[consoleplayer],
                     "ONLY AVAILABLE IN THE REGISTERED VERSION", true);
    }
    else
    {
        MenuEpisode = option;
        SetMenu(MENU_SKILL);
    }
    return true;
}

//---------------------------------------------------------------------------
//
// PROC SCSkill
//
//---------------------------------------------------------------------------

static boolean SCSkill(int option)
{
    G_DeferedInitNew(option, MenuEpisode, 1);
    MN_DeactivateMenu();
    return true;
}

//---------------------------------------------------------------------------
//
// PROC SCMouseSensi
//
//---------------------------------------------------------------------------

static boolean SCMouseSensi(int option)
{
    if (option == RIGHT_DIR)
    {
        // [crispy] remove mouse sensitivity limit
        if (mouseSensitivity < 255)
        {
            mouseSensitivity++;
        }
    }
    else if (mouseSensitivity)
    {
        mouseSensitivity--;
    }
    return true;
}

//---------------------------------------------------------------------------
//
// PROC SCSfxVolume
//
//---------------------------------------------------------------------------

static boolean SCSfxVolume(int option)
{
    if (option == RIGHT_DIR)
    {
        if (snd_MaxVolume < 15)
        {
            snd_MaxVolume++;
        }
    }
    else if (snd_MaxVolume)
    {
        snd_MaxVolume--;
    }
    S_SetMaxVolume(false);      // don't recalc the sound curve, yet
    soundchanged = true;        // we'll set it when we leave the menu
    return true;
}

//---------------------------------------------------------------------------
//
// PROC SCMusicVolume
//
//---------------------------------------------------------------------------

static boolean SCMusicVolume(int option)
{
    if (option == RIGHT_DIR)
    {
        if (snd_MusicVolume < 15)
        {
            snd_MusicVolume++;
        }
    }
    else if (snd_MusicVolume)
    {
        snd_MusicVolume--;
    }
    S_SetMusicVolume();
    return true;
}

//---------------------------------------------------------------------------
//
// PROC SCScreenSize
//
//---------------------------------------------------------------------------

static boolean SCScreenSize(int option)
{
    if (option == RIGHT_DIR)
    {
        if (screenblocks < 11)
        {
            screenblocks++;
        }
    }
    else if (screenblocks > 3)
    {
        screenblocks--;
    }
    R_SetViewSize(screenblocks, detailLevel);
    return true;
}

//---------------------------------------------------------------------------
//
// PROC SCInfo
//
//---------------------------------------------------------------------------

static boolean SCInfo(int option)
{
    InfoType = 1;
    S_StartSound(NULL, sfx_dorcls);
    if (!netgame && !demoplayback)
    {
        paused = true;
    }
    return true;
}

//---------------------------------------------------------------------------
//
// FUNC MN_Responder
//
//---------------------------------------------------------------------------

boolean MN_Responder(event_t * event)
{
    int charTyped;
    int key;
    int i;
    MenuItem_t *item;
    extern void D_StartTitle(void);
    extern void G_CheckDemoStatus(void);
    char *textBuffer;

    // In testcontrols mode, none of the function keys should do anything
    // - the only key is escape to quit.

    if (testcontrols)
    {
        if (event->type == ev_quit
         || (event->type == ev_keydown
          && (event->data1 == key_menu_activate
           || event->data1 == key_menu_quit)))
        {
            I_Quit();
            return true;
        }

        return false;
    }

    // "close" button pressed on window?
    if (event->type == ev_quit)
    {
        // First click on close = bring up quit confirm message.
        // Second click = confirm quit.

        if (!MenuActive && askforquit && typeofask == 1)
        {
            G_CheckDemoStatus();
            I_Quit();
        }
        else
        {
            SCQuitGame(0);
            S_StartSound(NULL, sfx_chat);
        }
        return true;
    }

    // Allow the menu to be activated from a joystick button if a button
    // is bound for joybmenu.
    if (event->type == ev_joystick)
    {
        if (joybmenu >= 0 && (event->data1 & (1 << joybmenu)) != 0)
        {
            MN_ActivateMenu();
            return true;
        }
    }

    if (event->type != ev_keydown)
    {
        return false;
    }

    key = event->data1;
    charTyped = event->data2;

    if (InfoType)
    {
        if (gamemode == shareware)
        {
            InfoType = (InfoType + 1) % 5;
        }
        else
        {
            InfoType = (InfoType + 1) % 4;
        }
        if (key == KEY_ESCAPE)
        {
            InfoType = 0;
        }
        if (!InfoType)
        {
            paused = false;
            MN_DeactivateMenu();
            SB_state = -1;      //refresh the statbar
            BorderNeedRefresh = true;
        }
        S_StartSound(NULL, sfx_dorcls);
        return (true);          //make the info screen eat the keypress
    }

    if ((ravpic && key == KEY_F1) ||
        (key != 0 && key == key_menu_screenshot))
    {
        G_ScreenShot();
        return (true);
    }

    if (askforquit)
    {
        if (key == key_menu_confirm)
        {
            switch (typeofask)
            {
                case 1:
                    G_CheckDemoStatus();
                    I_Quit();
                    return false;

                case 2:
                    players[consoleplayer].messageTics = 0;
                    //set the msg to be cleared
                    players[consoleplayer].message = NULL;
                    paused = false;
                    I_SetPalette(W_CacheLumpName
                                 ("PLAYPAL", PU_CACHE));
                    D_StartTitle();     // go to intro/demo mode.
                    break;

                case 3:
                    P_SetMessage(&players[consoleplayer],
                                 "QUICKSAVING....", false);
                    FileMenuKeySteal = true;
                    SCSaveGame(quicksave - 1);
                    BorderNeedRefresh = true;
                    break;

                case 4:
                    P_SetMessage(&players[consoleplayer],
                                 "QUICKLOADING....", false);
                    SCLoadGame(quickload - 1);
                    BorderNeedRefresh = true;
                    break;

                default:
                    break;
            }

            askforquit = false;
            typeofask = 0;

            return true;
        }
        else if (key == key_menu_abort || key == KEY_ESCAPE)
        {
            players[consoleplayer].messageTics = 1;  //set the msg to be cleared
            askforquit = false;
            typeofask = 0;
            paused = false;
            UpdateState |= I_FULLSCRN;
            BorderNeedRefresh = true;
            return true;
        }

        return false;           // don't let the keys filter thru
    }

    if (!MenuActive && !chatmodeon)
    {
        if (key == key_menu_decscreen)
        {
            if (automapactive)
            {               // Don't screen size in automap
                return (false);
            }
            SCScreenSize(LEFT_DIR);
            S_StartSound(NULL, sfx_keyup);
            BorderNeedRefresh = true;
            UpdateState |= I_FULLSCRN;
            return (true);
        }
        else if (key == key_menu_incscreen)
        {
            if (automapactive)
            {               // Don't screen size in automap
                return (false);
            }
            SCScreenSize(RIGHT_DIR);
            S_StartSound(NULL, sfx_keyup);
            BorderNeedRefresh = true;
            UpdateState |= I_FULLSCRN;
            return (true);
        }
        else if (key == key_menu_help)           // F1
        {
            SCInfo(0);      // start up info screens
            MenuActive = true;
            return (true);
        }
        else if (key == key_menu_save)           // F2 (save game)
        {
            if (gamestate == GS_LEVEL && !demoplayback)
            {
                MenuActive = true;
                FileMenuKeySteal = false;
                MenuTime = 0;
                CurrentMenu = &SaveMenu;
                CurrentItPos = CurrentMenu->oldItPos;
                if (!netgame && !demoplayback)
                {
                    paused = true;
                }
                S_StartSound(NULL, sfx_dorcls);
                slottextloaded = false;     //reload the slot text, when needed
            }
            return true;
        }
        else if (key == key_menu_load)           // F3 (load game)
        {
            if (SCNetCheck(2))
            {
                MenuActive = true;
                FileMenuKeySteal = false;
                MenuTime = 0;
                CurrentMenu = &LoadMenu;
                CurrentItPos = CurrentMenu->oldItPos;
                if (!netgame && !demoplayback)
                {
                    paused = true;
                }
                S_StartSound(NULL, sfx_dorcls);
                slottextloaded = false;     //reload the slot text, when needed
            }
            return true;
        }
        else if (key == key_menu_volume)         // F4 (volume)
        {
            MenuActive = true;
            FileMenuKeySteal = false;
            MenuTime = 0;
            CurrentMenu = &Options2Menu;
            CurrentItPos = CurrentMenu->oldItPos;
            if (!netgame && !demoplayback)
            {
                paused = true;
            }
            S_StartSound(NULL, sfx_dorcls);
            slottextloaded = false; //reload the slot text, when needed
            return true;
        }
        else if (key == key_menu_detail)          // F5 (detail)
        {
            // F5 isn't used in Heretic. (detail level)
            return true;
        }
        else if (key == key_menu_qsave)           // F6 (quicksave)
        {
            if (gamestate == GS_LEVEL && !demoplayback)
            {
                if (!quicksave || quicksave == -1)
                {
                    MenuActive = true;
                    FileMenuKeySteal = false;
                    MenuTime = 0;
                    CurrentMenu = &SaveMenu;
                    CurrentItPos = CurrentMenu->oldItPos;
                    if (!netgame && !demoplayback)
                    {
                        paused = true;
                    }
                    S_StartSound(NULL, sfx_dorcls);
                    slottextloaded = false; //reload the slot text, when needed
                    quicksave = -1;
                    P_SetMessage(&players[consoleplayer],
                                 "CHOOSE A QUICKSAVE SLOT", true);
                }
                else
                {
                    askforquit = true;
                    typeofask = 3;
                    if (!netgame && !demoplayback)
                    {
                        paused = true;
                    }
                    S_StartSound(NULL, sfx_chat);
                }
            }
            return true;
        }
        else if (key == key_menu_endgame)         // F7 (end game)
        {
            if (gamestate == GS_LEVEL && !demoplayback)
            {
                S_StartSound(NULL, sfx_chat);
                SCEndGame(0);
            }
            return true;
        }
        else if (key == key_menu_messages)        // F8 (toggle messages)
        {
            SCMessages(0);
            return true;
        }
        else if (key == key_menu_qload)           // F9 (quickload)
        {
            if (!quickload || quickload == -1)
            {
                MenuActive = true;
                FileMenuKeySteal = false;
                MenuTime = 0;
                CurrentMenu = &LoadMenu;
                CurrentItPos = CurrentMenu->oldItPos;
                if (!netgame && !demoplayback)
                {
                    paused = true;
                }
                S_StartSound(NULL, sfx_dorcls);
                slottextloaded = false;     //reload the slot text, when needed
                quickload = -1;
                P_SetMessage(&players[consoleplayer],
                             "CHOOSE A QUICKLOAD SLOT", true);
            }
            else
            {
                askforquit = true;
                if (!netgame && !demoplayback)
                {
                    paused = true;
                }
                typeofask = 4;
                S_StartSound(NULL, sfx_chat);
            }
            return true;
        }
        else if (key == key_menu_quit)            // F10 (quit)
        {
            if (gamestate == GS_LEVEL)
            {
                SCQuitGame(0);
                S_StartSound(NULL, sfx_chat);
            }
            return true;
        }
        else if (key == key_menu_gamma)           // F11 (gamma correction)
        {
            usegamma++;
            if (usegamma > 4+4) // [crispy] intermediate gamma levels
            {
                usegamma = 0;
            }
            I_SetPalette((byte *) W_CacheLumpName("PLAYPAL", PU_CACHE));
            return true;
        }
        // [crispy] those two can be considered as shortcuts for the ENGAGE cheat
        // and should be treated as such, i.e. add "if (!netgame)"
        else if (!netgame && key != 0 && key == key_menu_reloadlevel)
        {
	    if (G_ReloadLevel())
		return true;
        }
        else if (!netgame && key != 0 && key == key_menu_nextlevel)
        {
	    if (G_GotoNextLevel())
		return true;
        }

    }

    if (!MenuActive)
    {
        if (key == key_menu_activate || gamestate == GS_DEMOSCREEN || demoplayback)
        {
            MN_ActivateMenu();
            return (true);
        }
        return (false);
    }
    if (!FileMenuKeySteal)
    {
        item = &CurrentMenu->items[CurrentItPos];

        if (key == key_menu_down)            // Next menu item
        {
            do
            {
                if (CurrentItPos + 1 > CurrentMenu->itemCount - 1)
                {
                    CurrentItPos = 0;
                }
                else
                {
                    CurrentItPos++;
                }
            }
            while (CurrentMenu->items[CurrentItPos].type == ITT_EMPTY);
            S_StartSound(NULL, sfx_switch);
            return (true);
        }
        else if (key == key_menu_up)         // Previous menu item
        {
            do
            {
                if (CurrentItPos == 0)
                {
                    CurrentItPos = CurrentMenu->itemCount - 1;
                }
                else
                {
                    CurrentItPos--;
                }
            }
            while (CurrentMenu->items[CurrentItPos].type == ITT_EMPTY);
            S_StartSound(NULL, sfx_switch);
            return (true);
        }
        else if (key == key_menu_left)       // Slider left
        {
            if (item->type == ITT_LRFUNC && item->func != NULL)
            {
                item->func(LEFT_DIR);
                S_StartSound(NULL, sfx_keyup);
            }
            return (true);
        }
        else if (key == key_menu_right)      // Slider right
        {
            if (item->type == ITT_LRFUNC && item->func != NULL)
            {
                item->func(RIGHT_DIR);
                S_StartSound(NULL, sfx_keyup);
            }
            return (true);
        }
        else if (key == key_menu_forward)    // Activate item (enter)
        {
            if (item->type == ITT_SETMENU)
            {
                SetMenu(item->menu);
            }
            else if (item->func != NULL)
            {
                CurrentMenu->oldItPos = CurrentItPos;
                if (item->type == ITT_LRFUNC)
                {
                    item->func(RIGHT_DIR);
                }
                else if (item->type == ITT_EFUNC)
                {
                    if (item->func(item->option))
                    {
                        if (item->menu != MENU_NONE)
                        {
                            SetMenu(item->menu);
                        }
                    }
                }
            }
            S_StartSound(NULL, sfx_dorcls);
            return (true);
        }
        else if (key == key_menu_activate)     // Toggle menu
        {
            MN_DeactivateMenu();
            return (true);
        }
        else if (key == key_menu_back)         // Go back to previous menu
        {
            S_StartSound(NULL, sfx_switch);
            if (CurrentMenu->prevMenu == MENU_NONE)
            {
                MN_DeactivateMenu();
            }
            else
            {
                SetMenu(CurrentMenu->prevMenu);
            }
            return (true);
        }
        else if (charTyped != 0)
        {
            // Jump to menu item based on first letter:

            for (i = 0; i < CurrentMenu->itemCount; i++)
            {
                if (CurrentMenu->items[i].text)
                {
                    if (toupper(charTyped)
                        == toupper(DEH_String(CurrentMenu->items[i].text)[0]))
                    {
                        CurrentItPos = i;
                        return (true);
                    }
                }
            }
        }

        return (false);
    }
    else
    {
        // Editing file names
        // When typing a savegame name, we use the fully shifted and
        // translated input value from event->data3.
        charTyped = event->data3;

        textBuffer = &SlotText[currentSlot][slotptr];
        if (key == KEY_BACKSPACE)
        {
            if (slotptr)
            {
                *textBuffer = 0;
                slotptr--;
                textBuffer = &SlotText[currentSlot][slotptr];
                *textBuffer = ASCII_CURSOR;
            }
            return (true);
        }
        if (key == KEY_ESCAPE)
        {
            memset(SlotText[currentSlot], 0, SLOTTEXTLEN + 2);
            M_StringCopy(SlotText[currentSlot], oldSlotText,
                         sizeof(SlotText[currentSlot]));
            SlotStatus[currentSlot]--;
            MN_DeactivateMenu();
            return (true);
        }
        if (key == KEY_ENTER)
        {
            SlotText[currentSlot][slotptr] = 0; // clear the cursor
            item = &CurrentMenu->items[CurrentItPos];
            CurrentMenu->oldItPos = CurrentItPos;
            if (item->type == ITT_EFUNC)
            {
                item->func(item->option);
                if (item->menu != MENU_NONE)
                {
                    SetMenu(item->menu);
                }
            }
            return (true);
        }
        if (slotptr < SLOTTEXTLEN && key != KEY_BACKSPACE)
        {
            if (isalpha(charTyped))
            {
                *textBuffer++ = toupper(charTyped);
                *textBuffer = ASCII_CURSOR;
                slotptr++;
                return (true);
            }
            if (isdigit(charTyped) || charTyped == ' '
              || charTyped == ',' || charTyped == '.' || charTyped == '-'
              || charTyped == '!')
            {
                *textBuffer++ = charTyped;
                *textBuffer = ASCII_CURSOR;
                slotptr++;
                return (true);
            }
        }
        return (true);
    }
    return (false);
}

//---------------------------------------------------------------------------
//
// PROC MN_ActivateMenu
//
//---------------------------------------------------------------------------

void MN_ActivateMenu(void)
{
    if (MenuActive)
    {
        return;
    }
    if (paused)
    {
        S_ResumeSound();
    }
    MenuActive = true;
    FileMenuKeySteal = false;
    MenuTime = 0;
    CurrentMenu = &MainMenu;
    CurrentItPos = CurrentMenu->oldItPos;
    if (!netgame && !demoplayback)
    {
        paused = true;
    }
    S_StartSound(NULL, sfx_dorcls);
    slottextloaded = false;     //reload the slot text, when needed
}

//---------------------------------------------------------------------------
//
// PROC MN_DeactivateMenu
//
//---------------------------------------------------------------------------

void MN_DeactivateMenu(void)
{
    if (CurrentMenu != NULL)
    {
        CurrentMenu->oldItPos = CurrentItPos;
    }
    MenuActive = false;
    if (FileMenuKeySteal)
    {
        I_StopTextInput();
    }
    if (!netgame)
    {
        paused = false;
    }
    S_StartSound(NULL, sfx_dorcls);
    if (soundchanged)
    {
        S_SetMaxVolume(true);   //recalc the sound curve
        soundchanged = false;
    }
    players[consoleplayer].message = NULL;
    players[consoleplayer].messageTics = 1;
}
