/* Emacs style mode select   -*- C++ -*-
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
 * DESCRIPTION:
 *  DOOM selection menu, options, episode etc. (aka Big Font menus)
 *  Sliders and icons. Kinda widget stuff.
 *  Setup Menus.
 *  Extended HELP screens.
 *  Dynamic HELP screen.
 *
 *-----------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <fcntl.h>

#include "doomdef.h"
#include "doomstat.h"
#include "dstrings.h"
#include "d_main.h"
#include "v_video.h"
#include "w_wad.h"
#include "r_main.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "g_game.h"
#include "s_sound.h"
#include "sounds.h"
#include "m_menu.h"
#include "d_deh.h"
#include "m_misc.h"
#include "lprintf.h"
#include "am_map.h"
#include "i_main.h"
#include "i_system.h"
#include "i_video.h"
#include "i_sound.h"
#include "r_demo.h"
#include "r_fps.h"
#include "f_finale.h"
#include "e6y.h"//e6y
#include "dsda/global.h"
#include "dsda/settings.h"
#include "dsda/key_frame.h"
#include "dsda/input.h"
#include "dsda/palette.h"
#include "dsda/save.h"
#include "dsda/console.h"
#include "heretic/mn_menu.h"
#ifdef _WIN32
#include "e6y_launcher.h"
#endif

extern dboolean  message_dontfuckwithme;

extern dboolean chat_on;          // in heads-up code

extern const char* g_menu_flat;
extern patchnum_t* g_menu_font;
extern int g_menu_save_page_size;
extern int g_menu_font_spacing;
extern int g_menu_cr_title;
extern int g_menu_cr_set;
extern int g_menu_cr_item;
extern int g_menu_cr_hilite;
extern int g_menu_cr_select;
extern int g_menu_cr_disable;

//
// defaulted values
//

int mouseSensitivity_horiz; // has default   //  killough
int mouseSensitivity_vert;  // has default

int showMessages;    // Show messages has default, 0 = off, 1 = on

int hide_setup=1; // killough 5/15/98

// Blocky mode, has default, 0 = high, 1 = normal
//int     detailLevel;    obsolete -- killough
int screenblocks;    // has default

int screenSize;      // temp for screenblocks (0-9)

int quickSaveSlot;   // -1 = no quicksave slot picked!
int quickSavePage;

int messageToPrint;  // 1 = message to be printed

// CPhipps - static const
static const char* messageString; // ...and here is the message string!

// message x & y
int     messx;
int     messy;
int     messageLastMenuActive;

dboolean messageNeedsInput; // timed message = no input from user

void (*messageRoutine)(int response);

/* killough 8/15/98: when changes are allowed to sync-critical variables */
static int allow_changes(void)
{
 return !(demoplayback || demorecording || netgame);
}

static void M_UpdateCurrent(default_t* def)
{
  /* cph - requires rewrite of m_misc.c */
  if (def->current) {
    if (allow_changes())  /* killough 8/15/98 */
  *def->current = *def->location.pi;
    else if (*def->current != *def->location.pi)
  warn_about_changes(S_LEVWARN); /* killough 8/15/98 */
  }
}

int warning_about_changes, print_warning_about_changes;

/* cphipps - M_DrawBackground renamed and moved to v_video.c */

dboolean menu_background = 1; // do Boom fullscreen menus have backgrounds?

static void M_DrawBackground(const char *flat, int scrn)
{
  if (menu_background)
    V_DrawBackground(flat, scrn);
}

// we are going to be entering a savegame string

int saveStringEnter;
int saveSlot;        // which slot to save in
int saveCharIndex;   // which char we're editing
// old save description before edit
char saveOldString[SAVESTRINGSIZE];

dboolean inhelpscreens; // indicates we are in or just left a help screen

dboolean BorderNeedRefresh;

enum menuactive_e menuactive;    // The menus are up

#define SKULLXOFF  -32
#define LINEHEIGHT  16

char savegamestrings[10][SAVESTRINGSIZE];

short itemOn;           // menu item skull is on (for Big Font menus)
short skullAnimCounter; // skull animation counter
short whichSkull;       // which skull to draw (he blinks)

// graphic name of skulls

const char skullName[2][/*8*/9] = {"M_SKULL1","M_SKULL2"};

menu_t* currentMenu; // current menudef

// phares 3/30/98
// externs added for setup menus

int mapcolor_me;    // cph

extern int map_point_coordinates; // killough 10/98
extern int map_level_stat;

extern char* chat_macros[];  // chat macros
extern const char* shiftxform;
extern default_t defaults[];
extern int numdefaults;

// end of externs added for setup menus

//
// PROTOTYPES
//
void M_NewGame(int choice);
void M_Episode(int choice);
void M_ChooseSkill(int choice);
void M_LoadGame(int choice);
void M_SaveGame(int choice);
void M_Options(int choice);
void M_EndGame(int choice);
void M_ReadThis(int choice);
void M_ReadThis2(int choice);
void M_QuitDOOM(int choice);

void M_ChangeMessages(int choice);
void M_ChangeSensitivity(int choice);
void M_SfxVol(int choice);
void M_MusicVol(int choice);
/* void M_ChangeDetail(int choice);  unused -- killough */
void M_SizeDisplay(int choice);
void M_StartGame(int choice);
void M_Sound(int choice);

void M_Mouse(int choice, int *sens);      /* killough */
void M_MouseVert(int choice);
void M_MouseHoriz(int choice);
void M_DrawMouse(void);

void M_FinishReadThis(int choice);
void M_FinishHelp(int choice);            // killough 10/98
void M_LoadSelect(int choice);
void M_SaveSelect(int choice);
void M_ReadSaveStrings(void);
void M_QuickSave(void);
void M_QuickLoad(void);

void M_DrawMainMenu(void);
void M_DrawReadThis1(void);
void M_DrawReadThis2(void);
void M_DrawNewGame(void);
void M_DrawEpisode(void);
void M_DrawOptions(void);
void M_DrawSound(void);
void M_DrawLoad(void);
void M_DrawSave(void);
void M_DrawSetup(void);                                     // phares 3/21/98
void M_DrawHelp (void);                                     // phares 5/04/98

void M_DrawSaveLoadBorder(int x,int y);
void M_DrawThermo(int x,int y,int thermWidth,int thermDot);
void M_DrawEmptyCell(menu_t *menu,int item);
void M_DrawSelCell(menu_t *menu,int item);
void M_WriteText(int x, int y, const char *string, int cm);
int  M_StringWidth(const char *string);
int  M_StringHeight(const char *string);
void M_DrawTitle(int x, int y, const char *patch, int cm,
                 const char *alttext, int altcm);
void M_StartMessage(const char *string,void *routine,dboolean input);
void M_StopMessage(void);
void M_ClearMenus (void);

// phares 3/30/98
// prototypes added to support Setup Menus and Extended HELP screens

int  M_GetKeyString(int,int);
void M_Setup(int choice);
void M_KeyBindings(int choice);
void M_Weapons(int);
void M_StatusBar(int);
void M_Automap(int);
void M_Messages(int);
void M_ChatStrings(int);
void M_InitExtendedHelp(void);
void M_ExtHelpNextScreen(int);
void M_ExtHelp(int);
static int M_GetPixelWidth(const char*);
void M_DrawKeybnd(void);
void M_DrawWeapons(void);
static void M_DrawString(int cx, int cy, int color, const char* ch);
static void M_DrawMenuString(int,int,int);
static void M_DrawStringCentered(int,int,int,const char*);
void M_DrawStatusHUD(void);
void M_DrawExtHelp(void);
void M_DrawAutoMap(void);
void M_DrawMessages(void);
void M_DrawChatStrings(void);
void M_ChangeDemoSmoothTurns(void);
void M_ChangeTextureParams(void);
void M_General(int);      // killough 10/98
void M_DrawGeneral(void); // killough 10/98
void M_ChangeFullScreen(void);
void M_ChangeVideoMode(void);
void M_ChangeUseGLSurface(void);
void M_ChangeApplyPalette(void);

menu_t NewDef;                                              // phares 5/04/98

// end of prototypes added to support Setup Menus and Extended HELP screens

/////////////////////////////////////////////////////////////////////////////
//
// DOOM MENUS
//

/////////////////////////////
//
// MAIN MENU
//

// main_e provides numerical values for which Big Font screen you're on

enum
{
  newgame = 0,
  loadgame,
  savegame,
  options,
  readthis,
  quitdoom,
  main_end
} main_e;

//
// MainMenu is the definition of what the main menu Screen should look
// like. Each entry shows that the cursor can land on each item (1), the
// built-in graphic lump (i.e. "M_NGAME") that should be displayed,
// the program which takes over when an item is selected, and the hotkey
// associated with the item.
//

menuitem_t MainMenu[]=
{
  {1,"M_NGAME", M_NewGame, 'n', "NEW GAME"},
  {1,"M_OPTION",M_Options, 'o', "OPTIONS"},
  {1,"M_LOADG", M_LoadGame,'l', "LOAD GAME"},
  {1,"M_SAVEG", M_SaveGame,'s', "SAVE GAME"},
  {1,"M_RDTHIS",M_ReadThis,'r', "READ THIS"},
  {1,"M_QUITG", M_QuitDOOM,'q', "QUIT GAME"}
};

menu_t MainDef =
{
  main_end,       // number of menu items
  NULL,           // previous menu screen
  MainMenu,       // table that defines menu items
  M_DrawMainMenu, // drawing routine
  97,64,          // initial cursor position
  0               // last menu item the user was on
};

//
// M_DrawMainMenu
//

void M_DrawMainMenu(void)
{
  if (heretic) return MN_DrawMainMenu();

  // CPhipps - patch drawing updated
  V_DrawNamePatch(94, 2, 0, "M_DOOM", CR_DEFAULT, VPT_STRETCH);
}

/////////////////////////////
//
// Read This! MENU 1 & 2
//

// There are no menu items on the Read This! screens, so read_e just
// provides a placeholder to maintain structure.

enum
{
  rdthsempty1,
  read1_end
} read_e;

enum
{
  rdthsempty2,
  read2_end
} read_e2;

enum               // killough 10/98
{
  helpempty,
  help_end
} help_e;


// The definitions of the Read This! screens

menuitem_t ReadMenu1[] =
{
  {1,"",M_ReadThis2,0}
};

menuitem_t ReadMenu2[]=
{
  {1,"",M_FinishReadThis,0}
};

menuitem_t HelpMenu[]=    // killough 10/98
{
  {1,"",M_FinishHelp,0}
};

menu_t ReadDef1 =
{
  read1_end,
  &MainDef,
  ReadMenu1,
  M_DrawReadThis1,
  330,175,
  //280,185,              // killough 2/21/98: fix help screens
  0
};

menu_t ReadDef2 =
{
  read2_end,
  &ReadDef1,
  ReadMenu2,
  M_DrawReadThis2,
  330,175,
  0
};

menu_t HelpDef =           // killough 10/98
{
  help_end,
  &HelpDef,
  HelpMenu,
  M_DrawHelp,
  330,175,
  0
};

//
// M_ReadThis
//

void M_ReadThis(int choice)
{
  M_SetupNextMenu(&ReadDef1);
}

void M_ReadThis2(int choice)
{
  M_SetupNextMenu(&ReadDef2);
}

void M_FinishReadThis(int choice)
{
  M_SetupNextMenu(&MainDef);
}

void M_FinishHelp(int choice)        // killough 10/98
{
  M_SetupNextMenu(&MainDef);
}

//
// Read This Menus
// Had a "quick hack to fix romero bug"
//
// killough 10/98: updated with new screens

void M_DrawReadThis1(void)
{
  inhelpscreens = true;
  if (gamemode == shareware)
  {
    // e6y: wide-res
    V_FillBorder(-1, 0);
    V_DrawNamePatch(0, 0, 0, "HELP2", CR_DEFAULT, VPT_STRETCH);
  }
  else
    M_DrawCredits();
}

//
// Read This Menus - optional second page.
//
// killough 10/98: updated with new screens

void M_DrawReadThis2(void)
{
  inhelpscreens = true;
  M_DrawCredits();
}

/////////////////////////////
//
// EPISODE SELECT
//

//
// episodes_e provides numbers for the episode menu items. The default is
// 4, to accomodate Ultimate Doom. If the user is running anything else,
// this is accounted for in the code.
//

enum
{
  ep1,
  ep2,
  ep3,
  ep4,
  ep_end
} episodes_e;

// The definitions of the Episodes menu

menuitem_t EpisodeMenu[]=  // added a few free entries for UMAPINFO
{
  {1,"M_EPI1", M_Episode,'k'},
  {1,"M_EPI2", M_Episode,'t'},
  {1,"M_EPI3", M_Episode,'i'},
  {1,"M_EPI4", M_Episode,'t'},
  {1,"", M_Episode,'0'},
  {1,"", M_Episode,'0'},
  {1,"", M_Episode,'0'},
  {1,"", M_Episode,'0'}
};

menu_t EpiDef =
{
  ep_end,        // # of menu items
  &MainDef,      // previous menu
  EpisodeMenu,   // menuitem_t ->
  M_DrawEpisode, // drawing routine ->
  48,63,         // x,y
  ep1            // lastOn
};

// This is for customized episode menus
int EpiCustom;
short EpiMenuMap[8] = { 1, 1, 1, 1, -1, -1, -1, -1 }, EpiMenuEpi[8] = { 1,2,3,4,-1,-1,-1,-1 };

//
//    M_Episode
//
int epiChoice;

void M_ClearEpisodes(void)
{
  EpiDef.numitems = 0;
}

void M_AddEpisode(const char *map, const char *gfx, const char *txt, const char *alpha)
{
  if (!EpiCustom)
  {
    EpiCustom = true;
    NewDef.prevMenu = &EpiDef;

    if (gamemode == commercial || gamemission == chex)
      EpiDef.numitems = 0;
  }

  {
    int epi, mapnum;
    if (EpiDef.numitems >= 8) return;
    G_ValidateMapName(map, &epi, &mapnum);
    EpiMenuEpi[EpiDef.numitems] = epi;
    EpiMenuMap[EpiDef.numitems] = mapnum;
    strncpy(EpisodeMenu[EpiDef.numitems].name, gfx, 8);
    EpisodeMenu[EpiDef.numitems].name[8] = 0;
    EpisodeMenu[EpiDef.numitems].alttext = txt ? strdup(txt) : NULL;
    EpisodeMenu[EpiDef.numitems].alphaKey = alpha ? *alpha : 0;
    EpiDef.numitems++;
  }
  if (EpiDef.numitems <= 4)
  {
    EpiDef.y = 63;
  }
  else
  {
    EpiDef.y = 63 - (EpiDef.numitems - 4) * (LINEHEIGHT / 2);
  }
}

void M_DrawEpisode(void)
{
  if (heretic) return;

  // CPhipps - patch drawing updated
  V_DrawNamePatch(54, EpiDef.y - 25, 0, "M_EPISOD", CR_DEFAULT, VPT_STRETCH);
}

void M_Episode(int choice)
{
  if (!EpiCustom)
  {
    if ((gamemode == shareware) && choice) {
      M_StartMessage(s_SWSTRING, NULL, false); // Ty 03/27/98 - externalized
      M_SetupNextMenu(&ReadDef1);
      return;
    }

    // Yet another hack...
    if ((gamemode == registered) && (choice > 2) && !EpiCustom)
    {
      lprintf(LO_WARN,
        "M_Episode: 4th episode requires UltimateDOOM\n");
      choice = 0;
    }
  }
  epiChoice = choice;
  M_SetupNextMenu(&NewDef);
}

/////////////////////////////
//
// NEW GAME
//

// numerical values for the New Game menu items

enum
{
  killthings,
  toorough,
  hurtme,
  violence,
  nightmare,
  newg_end
} newgame_e;

// The definitions of the New Game menu

menuitem_t NewGameMenu[]=
{
  {1,"M_JKILL", M_ChooseSkill, 'i'},
  {1,"M_ROUGH", M_ChooseSkill, 'h'},
  {1,"M_HURT",  M_ChooseSkill, 'h'},
  {1,"M_ULTRA", M_ChooseSkill, 'u'},
  {1,"M_NMARE", M_ChooseSkill, 'n'}
};

menu_t NewDef =
{
  newg_end,       // # of menu items
  &EpiDef,        // previous menu
  NewGameMenu,    // menuitem_t ->
  M_DrawNewGame,  // drawing routine ->
  48,63,          // x,y
  hurtme          // lastOn
};

//
// M_NewGame
//

void M_DrawNewGame(void)
{
  if (heretic) return;

  // CPhipps - patch drawing updated
  V_DrawNamePatch(96, 14, 0, "M_NEWG", CR_DEFAULT, VPT_STRETCH);
  V_DrawNamePatch(54, 38, 0, "M_SKILL",CR_DEFAULT, VPT_STRETCH);
}

/* cph - make `New Game' restart the level in a netgame */
static void M_RestartLevelResponse(int ch)
{
  if (ch != 'y')
    return;

  if (demorecording)
    I_SafeExit(0);

  currentMenu->lastOn = itemOn;
  M_ClearMenus ();
  G_RestartLevel ();
}

void M_NewGame(int choice)
{
  if (netgame && !demoplayback) {
    if (compatibility_level < lxdoom_1_compatibility)
      M_StartMessage(s_NEWGAME,NULL,false); // Ty 03/27/98 - externalized
    else // CPhipps - query restarting the level
      M_StartMessage(s_RESTARTLEVEL,M_RestartLevelResponse,true);
    return;
  }

  if (demorecording) {  /* killough 5/26/98: exclude during demo recordings */
    M_StartMessage("you can't start a new game\n"
       "while recording a demo!\n\n"PRESSKEY,
       NULL, false); // killough 5/26/98: not externalized
    return;
  }

  // Chex Quest disabled the episode select screen, as did Doom II.
  if (((gamemode == commercial || gamemission == chex) && !EpiCustom) || EpiDef.numitems == 1)
    M_SetupNextMenu(&NewDef);
  else
  {
    epiChoice = 0;
    M_SetupNextMenu(&EpiDef);
  }
}

// CPhipps - static
static void M_VerifyNightmare(int ch)
{
  if (ch != 'y')
    return;

  G_DeferedInitNew(nightmare,EpiMenuEpi[epiChoice], EpiMenuMap[epiChoice]);
  M_ClearMenus ();
}

void M_ChooseSkill(int choice)
{
  if (choice == nightmare)
    {   // Ty 03/27/98 - externalized
      M_StartMessage(s_NIGHTMARE,M_VerifyNightmare,true);
      return;
    }
  if (EpiMenuEpi[epiChoice] == -1 || EpiMenuMap[epiChoice] == -1) return;  // There is no map to start here.

  G_DeferedInitNew(choice, EpiMenuEpi[epiChoice], EpiMenuMap[epiChoice]);
  M_ClearMenus ();
}

/////////////////////////////
//
// LOAD GAME MENU
//

// numerical values for the Load Game slots

enum
{
  load1,
  load2,
  load3,
  load4,
  load5,
  load6,
  load7,
  load_end
} load_e;

static int save_page = 0;
static const int save_page_limit = 16;

#define SAVE_PAGE_STRING_SIZE 16
char save_page_string[SAVE_PAGE_STRING_SIZE];

// The definitions of the Load Game screen

menuitem_t LoadMenue[]=
{
  {1,"", M_LoadSelect,'1'},
  {1,"", M_LoadSelect,'2'},
  {1,"", M_LoadSelect,'3'},
  {1,"", M_LoadSelect,'4'},
  {1,"", M_LoadSelect,'5'},
  {1,"", M_LoadSelect,'6'},
  {1,"", M_LoadSelect,'7'}, //jff 3/15/98 extend number of slots
  {1,"", M_LoadSelect,'8'},
};

menu_t LoadDef =
{
  load_end,
  &MainDef,
  LoadMenue,
  M_DrawLoad,
  80,34, //jff 3/15/98 move menu up
  0
};

#define LOADGRAPHIC_Y 8

//
// M_LoadGame & Cie.
//

void M_DrawLoad(void)
{
  int i;

  if (heretic) return MN_DrawLoad();

  //jff 3/15/98 use symbolic load position
  // CPhipps - patch drawing updated
  V_DrawNamePatch(72 ,LOADGRAPHIC_Y, 0, "M_LOADG", CR_DEFAULT, VPT_STRETCH);
  for (i = 0 ; i < load_end ; i++) {
    M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
    M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i], CR_DEFAULT);
  }

  M_WriteText(LoadDef.x, LoadDef.y + LINEHEIGHT * load_end, save_page_string, CR_DEFAULT);
}

//
// Draw border for the savegame description
//

void M_DrawSaveLoadBorder(int x,int y)
{
  int i;

  V_DrawNamePatch(x-8, y+7, 0, "M_LSLEFT", CR_DEFAULT, VPT_STRETCH);

  for (i = 0 ; i < 24 ; i++)
    {
      V_DrawNamePatch(x, y+7, 0, "M_LSCNTR", CR_DEFAULT, VPT_STRETCH);
      x += 8;
    }

  V_DrawNamePatch(x, y+7, 0, "M_LSRGHT", CR_DEFAULT, VPT_STRETCH);
}

//
// User wants to load this game
//

void M_LoadSelect(int choice)
{
  // CPhipps - Modified so savegame filename is worked out only internal
  //  to g_game.c, this only passes the slot.

  // killough 3/16/98, 5/15/98: add slot, cmd
  G_LoadGame(choice + save_page * g_menu_save_page_size, false);
  M_ClearMenus();
}

//
// killough 5/15/98: add forced loadgames
//

static char *forced_loadgame_message;

static void M_VerifyForcedLoadGame(int ch)
{
  if (ch=='y')
    G_ForcedLoadGame();
  free(forced_loadgame_message);    // free the message strdup()'ed below
  M_ClearMenus();
}

void M_ForcedLoadGame(const char *msg)
{
  forced_loadgame_message = strdup(msg); // free()'d above
  M_StartMessage(forced_loadgame_message, M_VerifyForcedLoadGame, true);
}

//
// Selected from DOOM menu
//

void M_LoadGame (int choice)
{
  // killough 5/26/98: exclude during demo recordings
  if (demorecording)
  {
    M_StartMessage("you can't load a game\n"
       "while recording a demo!\n\n"PRESSKEY,
       NULL, false); // killough 5/26/98: not externalized
    return;
  }

  M_SetupNextMenu(&LoadDef);
  M_ReadSaveStrings();
}

/////////////////////////////
//
// SAVE GAME MENU
//

// The definitions of the Save Game screen

menuitem_t SaveMenu[]=
{
  {1,"", M_SaveSelect,'1'},
  {1,"", M_SaveSelect,'2'},
  {1,"", M_SaveSelect,'3'},
  {1,"", M_SaveSelect,'4'},
  {1,"", M_SaveSelect,'5'},
  {1,"", M_SaveSelect,'6'},
  {1,"", M_SaveSelect,'7'}, //jff 3/15/98 extend number of slots
  {1,"", M_SaveSelect,'8'},
};

menu_t SaveDef =
{
  load_end, // same number of slots as the Load Game screen
  &MainDef,
  SaveMenu,
  M_DrawSave,
  80,34, //jff 3/15/98 move menu up
  0
};

//
// M_ReadSaveStrings
//  read the strings from the savegame files
//
void M_ReadSaveStrings(void)
{
  int i;

  for (i = 0 ; i < load_end ; i++) {
    char *name;               // killough 3/22/98
    FILE *fp;  // killough 11/98: change to use stdio

    /* killough 3/22/98
     * cph - add not-demoplayback parameter */
    name = dsda_SaveGameName(i + save_page * g_menu_save_page_size, false);
    fp = fopen(name,"rb");
    free(name);
    if (!fp) {   // Ty 03/27/98 - externalized:
      strcpy(&savegamestrings[i][0],s_EMPTYSTRING);
      LoadMenue[i].status = 0;
      continue;
    }
    fread(&savegamestrings[i], SAVESTRINGSIZE, 1, fp);
    fclose(fp);
    LoadMenue[i].status = 1;
  }

  snprintf(save_page_string, SAVE_PAGE_STRING_SIZE, "PAGE %d/%d", save_page + 1, save_page_limit);
}

//
//  M_SaveGame & Cie.
//
void M_DrawSave(void)
{
  int i;

  if (heretic) return MN_DrawSave();

  //jff 3/15/98 use symbolic load position
  // CPhipps - patch drawing updated
  V_DrawNamePatch(72, LOADGRAPHIC_Y, 0, "M_SAVEG", CR_DEFAULT, VPT_STRETCH);
  for (i = 0 ; i < load_end ; i++)
    {
    M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
    M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i], CR_DEFAULT);
    }

  M_WriteText(LoadDef.x, LoadDef.y + LINEHEIGHT * load_end, save_page_string, CR_DEFAULT);

  if (saveStringEnter)
    {
    i = M_StringWidth(savegamestrings[saveSlot]);
    M_WriteText(LoadDef.x + i,LoadDef.y+LINEHEIGHT*saveSlot,"_", CR_DEFAULT);
    }
}

//
// M_Responder calls this when user is finished
//
static void M_DoSave(int slot)
{
  G_SaveGame(slot + save_page * g_menu_save_page_size, savegamestrings[slot]);
  M_ClearMenus();

  // PICK QUICKSAVE SLOT YET?
  if (quickSaveSlot == -2)
  {
    quickSaveSlot = slot;
    quickSavePage = save_page;
  }
}

//
// User wants to save. Start string input for M_Responder
//
void M_SaveSelect(int choice)
{
  // we are going to be intercepting all chars
  saveStringEnter = 1;

  saveSlot = choice;
  strcpy(saveOldString,savegamestrings[choice]);
  if (!strcmp(savegamestrings[choice],s_EMPTYSTRING)) // Ty 03/27/98 - externalized
    savegamestrings[choice][0] = 0;
  saveCharIndex = strlen(savegamestrings[choice]);
}

//
// Selected from DOOM menu
//
void M_SaveGame (int choice)
{
  // killough 10/6/98: allow savegames during single-player demo playback
  if (!usergame && (!demoplayback || netgame))
    {
    M_StartMessage(s_SAVEDEAD,NULL,false); // Ty 03/27/98 - externalized
    return;
    }

  if (gamestate != GS_LEVEL)
    return;

  M_SetupNextMenu(&SaveDef);
  M_ReadSaveStrings();
}

/////////////////////////////
//
// OPTIONS MENU
//

// numerical values for the Options menu items

enum
{
  general, // killough 10/98
  // killough 4/6/98: move setup to be a sub-menu of OPTIONs
  setup,                                                    // phares 3/21/98
  endgame,
  messages,
  /*    detail, obsolete -- killough */
  scrnsize,
  option_empty1,
  mousesens,
  /* option_empty2, submenu now -- killough */
  soundvol,
  opt_end
} options_e;

// The definitions of the Options menu

menuitem_t OptionsMenu[]=
{
  // killough 4/6/98: move setup to be a sub-menu of OPTIONs
  {1,"M_GENERL", M_General, 'g', "GENERAL"},      // killough 10/98
  {1,"M_SETUP",  M_Setup,   's', "SETUP"},        // phares 3/21/98
  {1,"M_ENDGAM", M_EndGame,'e',  "END GAME"},
  {1,"M_MESSG",  M_ChangeMessages,'m', "MESSAGES:"},
  /*    {1,"M_DETAIL",  M_ChangeDetail,'g'},  unused -- killough */
  {2,"M_SCRNSZ", M_SizeDisplay,'s', "SCREEN SIZE"},
  {-1,"",0},
  {1,"M_MSENS",  M_ChangeSensitivity,'m', "MOUSE SENSITIVITY"},
  /* {-1,"",0},  replaced with submenu -- killough */
  {1,"M_SVOL",   M_Sound,'s', "SOUND VOLUME"},
};

menu_t OptionsDef =
{
  opt_end,
  &MainDef,
  OptionsMenu,
  M_DrawOptions,
  60,37,
  0
};

//
// M_Options
//
char detailNames[2][9] = {"M_GDHIGH","M_GDLOW"};
char msgNames[2][9]  = {"M_MSGOFF","M_MSGON"};


void M_DrawOptions(void)
{
  if (heretic) return MN_DrawOptions();

  // CPhipps - patch drawing updated
  // proff/nicolas 09/20/98 -- changed for hi-res
  V_DrawNamePatch(108, 15, 0, "M_OPTTTL", CR_DEFAULT, VPT_STRETCH);

  if ((W_CheckNumForName("M_GENERL") < 0) || (W_CheckNumForName("M_SETUP") < 0))
  {
    M_WriteText(OptionsDef.x + M_StringWidth("MESSAGES: "),
      OptionsDef.y+8-(M_StringHeight("ONOFF")/2)+LINEHEIGHT*messages,
      showMessages ? "ON" : "OFF", CR_DEFAULT);
  }
  else
  {
    V_DrawNamePatch(OptionsDef.x + 120, OptionsDef.y+LINEHEIGHT*messages, 0,
      msgNames[showMessages], CR_DEFAULT, VPT_STRETCH);
  }

  M_DrawThermo(OptionsDef.x,OptionsDef.y+LINEHEIGHT*(scrnsize+1),
   9,screenSize);
}

void M_Options(int choice)
{
  M_SetupNextMenu(&OptionsDef);
}

/////////////////////////////
//
// M_QuitDOOM
//
int quitsounds[8] =
{
  sfx_pldeth,
  sfx_dmpain,
  sfx_popain,
  sfx_slop,
  sfx_telept,
  sfx_posit1,
  sfx_posit3,
  sfx_sgtatk
};

int quitsounds2[8] =
{
  sfx_vilact,
  sfx_getpow,
  sfx_boscub,
  sfx_slop,
  sfx_skeswg,
  sfx_kntdth,
  sfx_bspact,
  sfx_sgtatk
};

static void M_QuitResponse(int ch)
{
  if (ch != 'y')
    return;

  //e6y: Optional removal of a quit sound
  if (!heretic && !netgame && showendoom && !nosfxparm && snd_card)
  {
    int i;

    if (gamemode == commercial)
      S_StartSound(NULL,quitsounds2[(gametic>>2)&7]);
    else
      S_StartSound(NULL,quitsounds[(gametic>>2)&7]);

    // wait till all sounds stopped or 3 seconds are over
    i = 30;
    while (i>0) {
      I_uSleep(100000); // CPhipps - don't thrash cpu in this loop
      if (!I_AnySoundStillPlaying())
        break;
      i--;
    }
  }
  //e6y: I_SafeExit instead of exit - prevent recursive exits
  I_SafeExit(0); // killough
}

void M_QuitDOOM(int choice)
{
  static char endstring[160];

  // We pick index 0 which is language sensitive,
  // or one at random, between 1 and maximum number.
  // Ty 03/27/98 - externalized DOSY as a string s_DOSY that's in the sprintf
  if (language != english)
    sprintf(endstring,"%s\n\n%s",s_DOSY, endmsg[0] );
  else         // killough 1/18/98: fix endgame message calculation:
    sprintf(endstring,"%s\n\n%s", endmsg[gametic%(NUM_QUITMESSAGES-1)+1], s_DOSY);

  if (dsda_SkipQuitPrompt())
    M_QuitResponse('y');
  else
    M_StartMessage(endstring,M_QuitResponse,true);
}

/////////////////////////////
//
// SOUND VOLUME MENU
//

// numerical values for the Sound Volume menu items
// The 'empty' slots are where the sliding scales appear.

enum
{
  sfx_vol,
  sfx_empty1,
  music_vol,
  sfx_empty2,
  sound_end
} sound_e;

// The definitions of the Sound Volume menu

menuitem_t SoundMenu[]=
{
  {2,"M_SFXVOL",M_SfxVol,'s'},
  {-1,"",0},
  {2,"M_MUSVOL",M_MusicVol,'m'},
  {-1,"",0}
};

menu_t SoundDef =
{
  sound_end,
  &OptionsDef,
  SoundMenu,
  M_DrawSound,
  80,64,
  0
};

//
// Change Sfx & Music volumes
//

void M_DrawSound(void)
{
  if (heretic) return MN_DrawSound();

  // CPhipps - patch drawing updated
  V_DrawNamePatch(60, 38, 0, "M_SVOL", CR_DEFAULT, VPT_STRETCH);

  M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(sfx_vol+1),16,snd_SfxVolume);

  M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(music_vol+1),16,snd_MusicVolume);
}

void M_Sound(int choice)
{
  M_SetupNextMenu(&SoundDef);
}

void M_SfxVol(int choice)
{
  switch(choice)
    {
    case 0:
      if (snd_SfxVolume)
        snd_SfxVolume--;
      break;
    case 1:
      if (snd_SfxVolume < 15)
        snd_SfxVolume++;
      break;
    }

  S_SetSfxVolume(snd_SfxVolume /* *8 */);
}

void M_MusicVol(int choice)
{
  switch(choice)
    {
    case 0:
      if (snd_MusicVolume)
        snd_MusicVolume--;
      break;
    case 1:
      if (snd_MusicVolume < 15)
        snd_MusicVolume++;
      break;
    }

  S_SetMusicVolume(snd_MusicVolume /* *8 */);
}

/////////////////////////////
//
// MOUSE SENSITIVITY MENU -- killough
//

// numerical values for the Mouse Sensitivity menu items
// The 'empty' slots are where the sliding scales appear.

enum
{
  mouse_horiz,
  mouse_empty1,
  mouse_vert,
  mouse_empty2,

//e6y
  mouse_mlook,
  mouse_empty3,
  mouse_accel,
  mouse_empty4,

  mouse_end
} mouse_e;

// The definitions of the Mouse Sensitivity menu

menuitem_t MouseMenu[]=
{
  {2,"M_HORSEN",M_MouseHoriz,'h', "HORIZONTAL"},
  {-1,"",0},
  {2,"M_VERSEN",M_MouseVert,'v', "VERTICAL"},
  {-1,"",0}

  //e6y
  ,
  {2,"M_LOKSEN",M_MouseMLook,'l', "MOUSE LOOK"},
  {-1,"",0},
  {2,"M_ACCEL",M_MouseAccel,'a', "ACCELERATION"},
  {-1,"",0}
};

menu_t MouseDef =
{
  mouse_end,
  &OptionsDef,
  MouseMenu,
  M_DrawMouse,
  60,37,//e6y
  0
};

//
// Change Mouse Sensitivities -- killough
//

void M_DrawMouse(void)
{
  if (heretic) return MN_DrawMouse();

  // CPhipps - patch drawing updated
  V_DrawNamePatch(60, 15, 0, "M_MSENS", CR_DEFAULT, VPT_STRETCH);//e6y

  M_DrawThermo(MouseDef.x, MouseDef.y + LINEHEIGHT * (mouse_horiz + 1),
               200, mouseSensitivity_horiz);

  M_DrawThermo(MouseDef.x, MouseDef.y + LINEHEIGHT * (mouse_vert + 1),
               200, mouseSensitivity_vert);

  M_DrawThermo(MouseDef.x, MouseDef.y + LINEHEIGHT * (mouse_mlook + 1),
               200, mouseSensitivity_mlook);

  M_DrawThermo(MouseDef.x, MouseDef.y + LINEHEIGHT * (mouse_accel + 1),
               200, mouse_acceleration);
}

void M_ChangeSensitivity(int choice)
{
  M_SetupNextMenu(&MouseDef);      // killough

  //  switch(choice)
  //      {
  //    case 0:
  //      if (mouseSensitivity)
  //        mouseSensitivity--;
  //      break;
  //    case 1:
  //      if (mouseSensitivity < 9)
  //        mouseSensitivity++;
  //      break;
  //      }
}

void M_MouseHoriz(int choice)
{
  M_Mouse(choice, &mouseSensitivity_horiz);
}

void M_MouseVert(int choice)
{
  M_Mouse(choice, &mouseSensitivity_vert);
}

void M_Mouse(int choice, int *sens)
{
  switch(choice)
    {
    case 0:
      if (*sens)
        --*sens;
      break;
    case 1:
      ++*sens;
      break;
    }
}

/////////////////////////////
//
//    M_QuickSave
//

char tempstring[80];

static void M_QuickSaveResponse(int ch)
{
  if (ch == 'y')  {
    M_DoSave(quickSaveSlot);
    S_StartSound(NULL,g_sfx_swtchx);
  }
}

void M_QuickSave(void)
{
  if (!usergame && (!demoplayback || netgame)) { /* killough 10/98 */
    S_StartSound(NULL,g_sfx_oof);
    return;
  }

  if (gamestate != GS_LEVEL)
    return;

  if (quickSaveSlot < 0) {
    M_StartControlPanel();
    M_ReadSaveStrings();
    M_SetupNextMenu(&SaveDef);
    quickSaveSlot = -2; // means to pick a slot now
    return;
  }
  else
  {
    save_page = quickSavePage;
    M_ReadSaveStrings();
  }
  sprintf(tempstring,s_QSPROMPT,savegamestrings[quickSaveSlot]); // Ty 03/27/98 - externalized
  M_StartMessage(tempstring,M_QuickSaveResponse,true);
}

/////////////////////////////
//
// M_QuickLoad
//

static void M_QuickLoadResponse(int ch)
{
  if (ch == 'y') {
    M_LoadSelect(quickSaveSlot);
    S_StartSound(NULL,g_sfx_swtchx);
  }
}

void M_QuickLoad(void)
{
  // cph - removed restriction against quickload in a netgame

  if (demorecording) {  // killough 5/26/98: exclude during demo recordings
    M_StartMessage("you can't quickload\n"
       "while recording a demo!\n\n"PRESSKEY,
       NULL, false); // killough 5/26/98: not externalized
    return;
  }

  if (quickSaveSlot < 0) {
    M_StartMessage(s_QSAVESPOT,NULL,false); // Ty 03/27/98 - externalized
    return;
  }
  else
  {
    save_page = quickSavePage;
    M_ReadSaveStrings();
  }
  sprintf(tempstring,s_QLPROMPT,savegamestrings[quickSaveSlot]); // Ty 03/27/98 - externalized
  M_StartMessage(tempstring,M_QuickLoadResponse,true);
}

/////////////////////////////
//
// M_EndGame
//

static void M_EndGameResponse(int ch)
{
  if (ch != 'y')
    return;

  // killough 5/26/98: make endgame quit if recording or playing back demo
  if (demorecording || singledemo)
    G_CheckDemoStatus();

  currentMenu->lastOn = itemOn;
  M_ClearMenus ();
  D_StartTitle ();
}

void M_EndGame(int choice)
{
  if (netgame)
    {
    M_StartMessage(s_NETEND,NULL,false); // Ty 03/27/98 - externalized
    return;
    }
  M_StartMessage(s_ENDGAME,M_EndGameResponse,true); // Ty 03/27/98 - externalized
}

/////////////////////////////
//
//    Toggle messages on/off
//

void M_ChangeMessages(int choice)
{
  // warning: unused parameter `int choice'
  choice = 0;
  showMessages = 1 - showMessages;

  if (!showMessages)
    players[consoleplayer].message = s_MSGOFF; // Ty 03/27/98 - externalized
  else
    players[consoleplayer].message = s_MSGON ; // Ty 03/27/98 - externalized

  message_dontfuckwithme = true;
}

/////////////////////////////
//
// CHANGE DISPLAY SIZE
//
// jff 2/23/98 restored to pre-HUD state
// hud_active controlled soley by F5=key_detail (key_hud)
// hud_displayed is toggled by + or = in fullscreen
// hud_displayed is cleared by -

void M_SizeDisplay(int choice)
{
  switch(choice) {
  case 0:
    if (screenSize > 0) {
      screenblocks--;
      screenSize--;
      hud_displayed = 0;
    }
    break;
  case 1:
    if (screenSize < 8) {
      screenblocks++;
      screenSize++;
    }
    else
      hud_displayed = !hud_displayed;
    break;
  }
  R_SetViewSize (screenblocks /*, detailLevel obsolete -- killough */);
}

//
// End of Original Menus
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//
// SETUP MENU (phares)
//
// We've added a set of Setup Screens from which you can configure a number
// of variables w/o having to restart the game. There are 7 screens:
//
//    Key Bindings
//    Weapons
//    Status Bar / HUD
//    Automap
//    Messages
//    Chat Strings
//
// killough 10/98: added Compatibility and General menus
//

/////////////////////////////
//
// booleans for setup screens
// these tell you what state the setup screens are in, and whether any of
// the overlay screens (automap colors, reset button message) should be
// displayed

dboolean setup_active      = false; // in one of the setup screens
dboolean set_keybnd_active = false; // in key binding setup screens
dboolean set_weapon_active = false; // in weapons setup screen
dboolean set_status_active = false; // in status bar/hud setup screen
dboolean set_auto_active   = false; // in automap setup screen
dboolean set_mess_active   = false; // in messages setup screen
dboolean set_chat_active   = false; // in chat string setup screen
dboolean setup_select      = false; // changing an item
dboolean setup_gather      = false; // gathering keys for value
dboolean colorbox_active   = false; // color palette being shown
dboolean default_verify    = false; // verify reset defaults decision
dboolean set_general_active = false;

/////////////////////////////
//
// set_menu_itemon is an index that starts at zero, and tells you which
// item on the current screen the cursor is sitting on.
//
// current_setup_menu is a pointer to the current setup menu table.

static int set_menu_itemon; // which setup item is selected?   // phares 3/98
static setup_menu_t* current_setup_menu; // points to current setup menu table

// save the setup menu's itemon value in the S_END element's x coordinate

static int M_GetSetupMenuItemOn (void)
{
  const setup_menu_t* menu = current_setup_menu;

  if (menu)
  {
    while (!(menu->m_flags & S_END))
      menu++;

    return menu->m_x;
  }

  return 0;
}

static void M_SetSetupMenuItemOn (const int x)
{
  setup_menu_t* menu = current_setup_menu;

  if (menu)
  {
    while (!(menu->m_flags & S_END))
      menu++;

    menu->m_x = x;
  }
}

/////////////////////////////
//
// The menu_buffer is used to construct strings for display on the screen.

static char menu_buffer[66];

/////////////////////////////
//
// The setup_e enum is used to provide a unique number for each group of Setup
// Screens.

enum
{
  set_key_bindings,
  set_weapons,
  set_statbar,
  set_automap,
  set_messages,
  set_chatstrings,
  set_setup_end
} setup_e;

int setup_screen; // the current setup screen. takes values from setup_e

/////////////////////////////
//
// SetupMenu is the definition of what the main Setup Screen should look
// like. Each entry shows that the cursor can land on each item (1), the
// built-in graphic lump (i.e. "M_KEYBND") that should be displayed,
// the program which takes over when an item is selected, and the hotkey
// associated with the item.

menuitem_t SetupMenu[]=
{
  {1,"M_KEYBND",M_KeyBindings,'k', "KEY BINDINGS"},
  {1,"M_WEAP"  ,M_Weapons,    'w', "WEAPONS"},
  {1,"M_STAT"  ,M_StatusBar,  's', "STATUS BAR / HUD"},
  {1,"M_AUTO"  ,M_Automap,    'a', "AUTOMAP"},
  {1,"M_MESS"  ,M_Messages,   'm', "MESSAGES"},
  {1,"M_CHAT"  ,M_ChatStrings,'c', "CHAT STRINGS"},
};

/////////////////////////////
//
// M_DoNothing does just that: nothing. Just a placeholder.

static void M_DoNothing(int choice)
{
}

/////////////////////////////
//
// Items needed to satisfy the 'Big Font' menu structures:
//
// the generic_setup_e enum mimics the 'Big Font' menu structures, but
// means nothing to the Setup Menus.

enum
{
  generic_setupempty1,
  generic_setup_end
} generic_setup_e;

// Generic_Setup is a do-nothing definition that the mainstream Menu code
// can understand, while the Setup Menu code is working. Another placeholder.

menuitem_t Generic_Setup[] =
{
  {1,"",M_DoNothing,0}
};

/////////////////////////////
//
// SetupDef is the menu definition that the mainstream Menu code understands.
// This is used by M_Setup (below) to define what is drawn and what is done
// with the main Setup screen.

menu_t  SetupDef =
{
  set_setup_end, // number of Setup Menu items (Key Bindings, etc.)
  &OptionsDef,   // menu to return to when BACKSPACE is hit on this menu
  SetupMenu,     // definition of items to show on the Setup Screen
  M_DrawSetup,   // program that draws the Setup Screen
  59,37,         // x,y position of the skull (modified when the skull is
                 // drawn). The skull is parked on the upper-left corner
                 // of the Setup screens, since it isn't needed as a cursor
  0              // last item the user was on for this menu
};

/////////////////////////////
//
// Here are the definitions of the individual Setup Menu screens. They
// follow the format of the 'Big Font' menu structures. See the comments
// for SetupDef (above) to help understand what each of these says.

menu_t KeybndDef =
{
  generic_setup_end,
  &SetupDef,
  Generic_Setup,
  M_DrawKeybnd,
  34,5,      // skull drawn here
  0
};

menu_t WeaponDef =
{
  generic_setup_end,
  &SetupDef,
  Generic_Setup,
  M_DrawWeapons,
  34,5,      // skull drawn here
  0
};

menu_t StatusHUDDef =
{
  generic_setup_end,
  &SetupDef,
  Generic_Setup,
  M_DrawStatusHUD,
  34,5,      // skull drawn here
  0
};

menu_t AutoMapDef =
{
  generic_setup_end,
  &SetupDef,
  Generic_Setup,
  M_DrawAutoMap,
  34,5,      // skull drawn here
  0
};

menu_t MessageDef =                                         // phares 4/08/98
{
  generic_setup_end,
  &SetupDef,
  Generic_Setup,
  M_DrawMessages,
  34,5,      // skull drawn here
  0
};

menu_t ChatStrDef =                                         // phares 4/10/98
{
  generic_setup_end,
  &SetupDef,
  Generic_Setup,
  M_DrawChatStrings,
  34,5,      // skull drawn here
  0
};

menu_t GeneralDef =                                           // killough 10/98
{
  generic_setup_end,
  &OptionsDef,
  Generic_Setup,
  M_DrawGeneral,
  34,5,      // skull drawn here
  0
};

/////////////////////////////
//
// Draws the Title for the main Setup screen

void M_DrawSetup(void)
{
  if (heretic) return MN_DrawSetup();

  // CPhipps - patch drawing updated
  M_DrawTitle(124, 15, "M_SETUP", CR_DEFAULT, "SETUP", g_cr_gold);
}

/////////////////////////////
//
// Uses the SetupDef structure to draw the menu items for the main
// Setup screen

void M_Setup(int choice)
{
  M_SetupNextMenu(&SetupDef);
}

// Data used by the Automap color selection code

#define CHIP_SIZE 7 // size of color block for colored items

#define COLORPALXORIG ((320 - 16*(CHIP_SIZE+1))/2)
#define COLORPALYORIG ((200 - 16*(CHIP_SIZE+1))/2)

#define PAL_BLACK   0
#define PAL_WHITE   4

// Data used by the Chat String editing code

#define CHAT_STRING_BFR_SIZE 128

// chat strings must fit in this screen space
// killough 10/98: reduced, for more general uses
#define MAXCHATWIDTH         272

int   chat_index;
char* chat_string_buffer; // points to new chat strings while editing

/////////////////////////////
//
// phares 4/17/98:
// Added 'Reset to Defaults' Button to Setup Menu screens
// This is a small button that sits in the upper-right-hand corner of
// the first screen for each group. It blinks when selected, thus the
// two patches, which it toggles back and forth.

char ResetButtonName[2][8] = {"M_BUTT1","M_BUTT2"};

/////////////////////////////
//
// phares 4/18/98:
// Consolidate Item drawing code
//
// M_DrawItem draws the description of the provided item (the left-hand
// part). A different color is used for the text depending on whether the
// item is selected or not, or whether it's about to change.

// CPhipps - static, hanging else removed, const parameter
static void M_DrawItem(const setup_menu_t* s)
{
  int x = s->m_x;
  int y = s->m_y;
  int flags = s->m_flags;
  if (flags & S_RESET)
  {
    // This item is the reset button
    // Draw the 'off' version if this isn't the current menu item
    // Draw the blinking version in tune with the blinking skull otherwise

    // proff/nicolas 09/20/98 -- changed for hi-res
    // CPhipps - Patch drawing updated, reformatted
    if (!heretic)
      V_DrawNamePatch(
        x, y, 0,
        ResetButtonName[(flags & (S_HILITE|S_SELECT)) ? whichSkull : 0],
        CR_DEFAULT, VPT_STRETCH
      );
  }
  else // Draw the item string
  {
    char *p, *t;
    int w = 0;
    int color =
      flags & S_DISABLE ? g_menu_cr_disable : //e6y
      flags & S_SELECT ? g_menu_cr_select :
      flags & S_HILITE ? g_menu_cr_hilite :
      flags & (S_TITLE|S_NEXT|S_PREV) ? g_menu_cr_title : g_menu_cr_item; // killough 10/98

    /* killough 10/98:
     * Enhance to support multiline text separated by newlines.
     * This supports multiline items on horizontally-crowded menus.
     */

    for (p = t = strdup(s->m_text); (p = strtok(p,"\n")); y += 8, p = NULL)
    {      /* killough 10/98: support left-justification: */
      strcpy(menu_buffer,p);
      if (!(flags & S_LEFTJUST))
        w = M_GetPixelWidth(menu_buffer) + 4;
      M_DrawMenuString(x - w, y ,color);
      // print a blinking "arrow" next to the currently highlighted menu item
      if (s == current_setup_menu + set_menu_itemon && whichSkull)
        M_DrawString(x - w - 8, y, color, ">");
    }
    free(t);
  }
}

// If a number item is being changed, allow up to N keystrokes to 'gather'
// the value. Gather_count tells you how many you have so far. The legality
// of what is gathered is determined by the low/high settings for the item.

#define MAXGATHER 5
int  gather_count;
char gather_buffer[MAXGATHER+1];  // killough 10/98: make input character-based

/////////////////////////////
//
// phares 4/18/98:
// Consolidate Item Setting drawing code
//
// M_DrawSetting draws the setting of the provided item (the right-hand
// part. It determines the text color based on whether the item is
// selected or being changed. Then, depending on the type of item, it
// displays the appropriate setting value: yes/no, a key binding, a number,
// a paint chip, etc.

static void M_DrawSetting(const setup_menu_t* s)
{
  int x = s->m_x, y = s->m_y, flags = s->m_flags, color;

  // Determine color of the text. This may or may not be used later,
  // depending on whether the item is a text string or not.

  color =
    flags & S_DISABLE ? g_menu_cr_disable : //e6y
    flags & S_SELECT ? g_menu_cr_select : flags & S_HILITE ? g_menu_cr_hilite : g_menu_cr_set;

  // Is the item a YES/NO item?

  if (flags & S_YESNO) {
    strcpy(menu_buffer,*s->var.def->location.pi ? "YES" : "NO");
    if (s == current_setup_menu + set_menu_itemon && whichSkull && !setup_select)
      strcat(menu_buffer, " <");
    M_DrawMenuString(x,y,color);
    return;
  }

  // Is the item a simple number?

  if (flags & S_NUM) {
    // killough 10/98: We must draw differently for items being gathered.
    if (flags & (S_HILITE|S_SELECT) && setup_gather) {
      gather_buffer[gather_count] = 0;
      strcpy(menu_buffer, gather_buffer);
    }
    else
      sprintf(menu_buffer,"%d",*s->var.def->location.pi);
    if (s == current_setup_menu + set_menu_itemon && whichSkull && !setup_select)
      strcat(menu_buffer, " <");
    M_DrawMenuString(x,y,color);
    return;
  }

  // Is the item a key binding?

  if (flags & S_INPUT) {
    int i;
    int offset = 0;
    const char* format;
    dboolean any_input = false;
    dsda_input_t* input;
    input = dsda_Input(s->input);

    // Draw the input bound to the action
    menu_buffer[0] = '\0';

    for (i = 0; i < input->num_keys; ++i)
    {
      if (any_input)
      {
        menu_buffer[offset++] = '/';
        menu_buffer[offset] = '\0';
      }

      offset = M_GetKeyString(input->key[i], offset);
      any_input = true;
    }

    if (input->mouseb != -1)
    {
      if (any_input)
        format = "/MB%d";
      else
        format = "MB%d";

      sprintf(menu_buffer + strlen(menu_buffer), format, input->mouseb + 1);
      any_input = true;
    }

    if (input->joyb != -1)
    {
      if (any_input)
        format = "/JSB%d";
      else
        format = "JSB%d";

      sprintf(menu_buffer + strlen(menu_buffer), format, input->joyb + 1);
      any_input = true;
    }

    // "NONE"
    if (!any_input)
      M_GetKeyString(0, 0);

    if (s == current_setup_menu + set_menu_itemon && whichSkull && !setup_select)
      strcat(menu_buffer, " <");

    M_DrawMenuString(x, y, color);

    return;
  }

  // Is the item a weapon number?
  // OR, Is the item a colored text string from the Automap?
  //
  // killough 10/98: removed special code, since the rest of the engine
  // already takes care of it, and this code prevented the user from setting
  // their overall weapons preferences while playing Doom 1.
  //
  // killough 11/98: consolidated weapons code with color range code

  if (flags & (S_WEAP|S_CRITEM)) // weapon number or color range
    {
      sprintf(menu_buffer,"%d", *s->var.def->location.pi);
      if (s == current_setup_menu + set_menu_itemon && whichSkull && !setup_select)
        M_DrawString(x + 8, y, color, " <");
      M_DrawMenuString(x,y, flags & S_CRITEM ? *s->var.def->location.pi : color);
      return;
    }

  // Is the item a paint chip?

  if (flags & S_COLOR) // Automap paint chip
    {
      int ch;

      ch = *s->var.def->location.pi;
      // proff 12/6/98: Drawing of colorchips completly changed for hi-res, it now uses a patch
      // draw the paint chip
      // e6y: wide-res
      {
        int xx = x, yy = y - 1, ww = 8, hh = 8;
        V_GetWideRect(&xx, &yy, &ww, &hh, VPT_STRETCH);
        V_FillRect(0, xx, yy, ww, hh, PAL_BLACK);
        xx = x + 1, yy = y, ww = 6, hh = 6;
        V_GetWideRect(&xx, &yy, &ww, &hh, VPT_STRETCH);
        V_FillRect(0, xx, yy, ww, hh, (byte)ch);
      }

      if (!ch) // don't show this item in automap mode
  V_DrawNamePatch(x+1,y,0,"M_PALNO", CR_DEFAULT, VPT_STRETCH);
  if (s == current_setup_menu + set_menu_itemon && whichSkull && !setup_select)
    M_DrawString(x + 8, y, color, " <");
      return;
    }

  // Is the item a chat string?
  // killough 10/98: or a filename?

  if (flags & S_STRING) {
    /* cph - cast to char* as it's really a Z_Strdup'd string (see m_misc.h) */
    union { const char **c; char **s; } u; // type punning via unions
    char *text;

    u.c = s->var.def->location.ppsz;
    text = *(u.s);

    // Are we editing this string? If so, display a cursor under
    // the correct character.

    if (setup_select && (s->m_flags & (S_HILITE|S_SELECT))) {
      int cursor_start, char_width;
      char c[2];

      // If the string is too wide for the screen, trim it back,
      // one char at a time until it fits. This should only occur
      // while you're editing the string.

      while (M_GetPixelWidth(text) >= MAXCHATWIDTH) {
  int len = strlen(text);
  text[--len] = 0;
  if (chat_index > len)
    chat_index--;
      }

      // Find the distance from the beginning of the string to
      // where the cursor should be drawn, plus the width of
      // the char the cursor is under..

      *c = text[chat_index]; // hold temporarily
      c[1] = 0;
      char_width = M_GetPixelWidth(c);
      if (char_width == 1)
  char_width = 7; // default for end of line
      text[chat_index] = 0; // NULL to get cursor position
      cursor_start = M_GetPixelWidth(text);
      text[chat_index] = *c; // replace stored char

      // Now draw the cursor
      // proff 12/6/98: Drawing of cursor changed for hi-res
      // e6y: wide-res
      if (x + cursor_start + char_width < BASE_WIDTH)
      {
        int xx = (x+cursor_start-1), yy = y, ww = char_width, hh = 9;
        V_GetWideRect(&xx, &yy, &ww, &hh, VPT_STRETCH);
        V_FillRect(0, xx, yy, ww, hh, PAL_WHITE);
      }
    }

    // Draw the setting for the item

    strcpy(menu_buffer,text);
    if (s == current_setup_menu + set_menu_itemon && whichSkull && !setup_select)
      strcat(menu_buffer, " <");
    M_DrawMenuString(x,y,color);
    return;
  }

  // Is the item a selection of choices?

  if (flags & S_CHOICE) {
    if (s->var.def->type == def_int) {
      if (s->selectstrings == NULL) {
        sprintf(menu_buffer,"%d",*s->var.def->location.pi);
      } else {
        strcpy(menu_buffer,s->selectstrings[*s->var.def->location.pi]);
      }
    }

    if (s->var.def->type == def_str) {
      sprintf(menu_buffer,"%s", *s->var.def->location.ppsz);
    }

    if (s == current_setup_menu + set_menu_itemon && whichSkull && !setup_select)
      strcat(menu_buffer, " <");
    M_DrawMenuString(x,y,color);
    return;
  }
}

/////////////////////////////
//
// M_DrawScreenItems takes the data for each menu item and gives it to
// the drawing routines above.

// CPhipps - static, const parameter, formatting
static void M_DrawScreenItems(const setup_menu_t* src)
{
  if (print_warning_about_changes > 0) { /* killough 8/15/98: print warning */
  //e6y
    if (warning_about_changes & S_CANT_GL_ARB_MULTITEXTURE) {
  strcpy(menu_buffer, "Extension GL_ARB_multitexture not found");
  M_DrawMenuString(30,176,g_cr_red);
  } else
    if (warning_about_changes & S_CANT_GL_ARB_MULTISAMPLEFACTOR) {
  strcpy(menu_buffer, "Mast be even number like 0-none, 2, 4, 6");
  M_DrawMenuString(30,176,g_cr_red);
  } else

    if (warning_about_changes & S_BADVAL) {
  strcpy(menu_buffer, "Value out of Range");
  M_DrawMenuString(100,176,g_cr_red);
    } else if (warning_about_changes & S_PRGWARN) {
        strcpy(menu_buffer, "Warning: Program must be restarted to see changes");
  M_DrawMenuString(3, 176, g_cr_red);
    } else if (warning_about_changes & S_BADVID) {
        strcpy(menu_buffer, "Video mode not supported");
  M_DrawMenuString(80,176,g_cr_red);
    } else {
  strcpy(menu_buffer, "Warning: Changes are pending until next game");
        M_DrawMenuString(18,184,g_cr_red);
    }
  }

  while (!(src->m_flags & S_END)) {

    // See if we're to draw the item description (left-hand part)

    if (src->m_flags & S_SHOWDESC)
      M_DrawItem(src);

    // See if we're to draw the setting (right-hand part)

    if (src->m_flags & S_SHOWSET)
      M_DrawSetting(src);
    src++;
  }
}

/////////////////////////////
//
// Data used to draw the "are you sure?" dialogue box when resetting
// to defaults.

#define VERIFYBOXXORG 66
#define VERIFYBOXYORG 88

// And the routine to draw it.

static void M_DrawDefVerify(void)
{
  // proff 12/6/98: Drawing of verify box changed for hi-res, it now uses a patch
  V_DrawNamePatch(VERIFYBOXXORG,VERIFYBOXYORG,0,"M_VBOX",CR_DEFAULT,VPT_STRETCH);
  // The blinking messages is keyed off of the blinking of the
  // cursor skull.

  if (whichSkull) { // blink the text
    strcpy(menu_buffer,"Reset to defaults? (Y or N)");
    M_DrawMenuString(VERIFYBOXXORG+8,VERIFYBOXYORG+8,g_cr_red);
  }
}


/////////////////////////////
//
// phares 4/18/98:
// M_DrawInstructions writes the instruction text just below the screen title
//
// cph 2006/08/06 - go back to the Boom version, and then clean up by using
// M_DrawStringCentered (much better than all those magic 'x' valies!)

static void M_DrawInstructions(void)
{
  int flags = current_setup_menu[set_menu_itemon].m_flags;

  // There are different instruction messages depending on whether you
  // are changing an item or just sitting on it.

  if (setup_select) {
    switch (flags & (S_INPUT | S_YESNO | S_WEAP | S_NUM | S_COLOR | S_CRITEM | S_CHAT | S_RESET | S_FILE | S_CHOICE | S_NAME)) {
      case S_INPUT:
        M_DrawStringCentered(160, 20, g_menu_cr_select, "Press key or button for this action");
        break;

    case S_YESNO:
      M_DrawStringCentered(160, 20, g_menu_cr_select, "Press ENTER key to toggle");
      break;
    case S_WEAP:
      M_DrawStringCentered(160, 20, g_menu_cr_select, "Enter weapon number");
      break;
    case S_NUM:
      M_DrawStringCentered(160, 20, g_menu_cr_select, "Enter value. Press ENTER when finished.");
      break;
    case S_COLOR:
      M_DrawStringCentered(160, 20, g_menu_cr_select, "Select color and press enter");
      break;
    case S_CRITEM:
      M_DrawStringCentered(160, 20, g_menu_cr_select, "Enter value");
      break;
    case S_CHAT:
      M_DrawStringCentered(160, 20, g_menu_cr_select, "Type/edit chat string and Press ENTER");
      break;
    case S_FILE:
      M_DrawStringCentered(160, 20, g_menu_cr_select, "Type/edit filename and Press ENTER");
      break;
    case S_CHOICE:
      M_DrawStringCentered(160, 20, g_menu_cr_select, "Press left or right to choose");
      break;
    case S_NAME:
      M_DrawStringCentered(160, 20, g_menu_cr_select, "Type / edit author and Press ENTER");
      break;
    case S_RESET:
      break;
#ifdef SIMPLECHECKS
    default:
      lprintf(LO_WARN,"Unrecognised menu item type %d", flags);
#endif
    }
  } else {
    if (flags & S_RESET)
      M_DrawStringCentered(160, 20, g_menu_cr_hilite, "Press ENTER key to reset to defaults");
    else if (flags & S_INPUT)
      M_DrawStringCentered(160, 20, g_menu_cr_hilite, "Press Enter to Change, Del to Clear");
    else
      M_DrawStringCentered(160, 20, g_menu_cr_hilite, "Press Enter to Change");
  }
}


/////////////////////////////
//
// The Key Binding Screen tables.

#define KB_X  160
#define KB_PREV  57
#define KB_NEXT 310
#define KB_Y   31

// phares 4/16/98:
// X,Y position of reset button. This is the same for every screen, and is
// only defined once here.

#define X_BUTTON 301
#define Y_BUTTON   3

// Definitions of the (in this case) four key binding screens.

setup_menu_t keys_settings1[];
setup_menu_t keys_settings2[];
setup_menu_t keys_settings3[];
setup_menu_t keys_settings4[];
setup_menu_t keys_settings5[];
setup_menu_t keys_settings6[];
setup_menu_t keys_settings7[];
setup_menu_t keys_settings8[];
setup_menu_t heretic_keys_settings1[];
setup_menu_t heretic_keys_settings2[];
setup_menu_t dsda_keys_settings[];

// The table which gets you from one screen table to the next.

setup_menu_t* keys_settings[] =
{
  keys_settings1,
  keys_settings2,
  keys_settings3,
  keys_settings4,
  keys_settings5,
  keys_settings6,
  keys_settings7,
  keys_settings8,
  heretic_keys_settings1,
  heretic_keys_settings2,
  dsda_keys_settings,
  NULL
};

int mult_screens_index; // the index of the current screen in a set

// The first Key Binding screen table.
// Note that the Y values are ascending. If you need to add something to
// this table, (well, this one's not a good example, because it's full)
// you need to make sure the Y values still make sense so everything gets
// displayed.
//
// Note also that the first screen of each set has a line for the reset
// button. If there is more than one screen in a set, the others don't get
// the reset button.
//
// Note also that this screen has a "NEXT ->" line. This acts like an
// item, in that 'activating' it moves you along to the next screen. If
// there's a "<- PREV" item on a screen, it behaves similarly, moving you
// to the previous screen. If you leave these off, you can't move from
// screen to screen.

setup_menu_t keys_settings1[] =  // Key Binding screen strings
{
  {"INPUT PROFILE",S_NUM,m_null,KB_X,KB_Y,{"input_profile"}},
  {"MOVEMENT"    ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y+2*8},
  {"FORWARD"     ,S_INPUT     ,m_scrn,KB_X,KB_Y+3*8,{0},dsda_input_forward},
  {"BACKWARD"    ,S_INPUT     ,m_scrn,KB_X,KB_Y+4*8,{0},dsda_input_backward},
  {"TURN LEFT"   ,S_INPUT     ,m_scrn,KB_X,KB_Y+5*8,{0},dsda_input_turnleft},
  {"TURN RIGHT"  ,S_INPUT     ,m_scrn,KB_X,KB_Y+6*8,{0},dsda_input_turnright},
  {"RUN"         ,S_INPUT     ,m_scrn,KB_X,KB_Y+7*8,{0},dsda_input_speed},
  {"STRAFE LEFT" ,S_INPUT     ,m_scrn,KB_X,KB_Y+8*8,{0},dsda_input_strafeleft},
  {"STRAFE RIGHT",S_INPUT     ,m_scrn,KB_X,KB_Y+9*8,{0},dsda_input_straferight},
  {"STRAFE"      ,S_INPUT     ,m_scrn,KB_X,KB_Y+10*8,{0},dsda_input_strafe},
  {"AUTORUN"     ,S_INPUT     ,m_scrn,KB_X,KB_Y+11*8,{0},dsda_input_autorun},
  {"180 TURN"    ,S_INPUT     ,m_scrn,KB_X,KB_Y+12*8,{0},dsda_input_reverse},
  {"USE"         ,S_INPUT     ,m_scrn,KB_X,KB_Y+13*8,{0},dsda_input_use},
  {"JUMP/FLY UP" ,S_INPUT     ,m_scrn,KB_X,KB_Y+14*8,{0},dsda_input_flyup},
  {"FLY DOWN"    ,S_INPUT     ,m_scrn,KB_X,KB_Y+15*8,{0},dsda_input_flydown},
  {"MOUSE LOOK"  ,S_INPUT     ,m_scrn,KB_X,KB_Y+16*8,{0},dsda_input_mlook},
  {"NO VERTICAL MOUSE",S_INPUT,m_scrn,KB_X,KB_Y+17*8,{0},dsda_input_novert},

  // Button for resetting to defaults
  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},

  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {keys_settings2}},

  // Final entry
  {0,S_SKIP|S_END,m_null}

};

setup_menu_t keys_settings2[] =  // Key Binding screen strings
{
  {"SCREEN"      ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y},

  // phares 4/13/98:
  // key_help and key_escape can no longer be rebound. This keeps the
  // player from getting themselves in a bind where they can't remember how
  // to get to the menus, and can't remember how to get to the help screen
  // to give them a clue as to how to get to the menus. :)

  // Also, the keys assigned to these functions cannot be bound to other
  // functions. Introduce an S_KEEP flag to show that you cannot swap this
  // key with other keys in the same 'group'. (m_scrn, etc.)

  {"HELP"        ,S_SKIP|S_KEEP ,m_scrn,0   ,0    ,{0},dsda_input_help},
  {"MENU"        ,S_SKIP|S_KEEP ,m_scrn,0   ,0    ,{0},dsda_input_escape},
  // killough 10/98: hotkey for entering setup menu:
  {"SETUP"       ,S_INPUT     ,m_scrn,KB_X,KB_Y+ 1*8,{0},dsda_input_setup},
  {"PAUSE"       ,S_INPUT     ,m_scrn,KB_X,KB_Y+ 2*8,{0},dsda_input_pause},
  {"AUTOMAP"     ,S_INPUT     ,m_scrn,KB_X,KB_Y+ 3*8,{0},dsda_input_map},
  {"VOLUME"      ,S_INPUT     ,m_scrn,KB_X,KB_Y+ 4*8,{0},dsda_input_soundvolume},
  {"HUD"         ,S_INPUT     ,m_scrn,KB_X,KB_Y+ 5*8,{0},dsda_input_hud},
  {"MESSAGES"    ,S_INPUT     ,m_scrn,KB_X,KB_Y+ 6*8,{0},dsda_input_messages},
  {"GAMMA FIX"   ,S_INPUT     ,m_scrn,KB_X,KB_Y+ 7*8,{0},dsda_input_gamma},
  {"SPY"         ,S_INPUT     ,m_scrn,KB_X,KB_Y+ 8*8,{0},dsda_input_spy},
  {"LARGER VIEW" ,S_INPUT     ,m_scrn,KB_X,KB_Y+ 9*8,{0},dsda_input_zoomin},
  {"SMALLER VIEW",S_INPUT     ,m_scrn,KB_X,KB_Y+10*8,{0},dsda_input_zoomout},
  {"SCREENSHOT"  ,S_INPUT     ,m_scrn,KB_X,KB_Y+11*8,{0},dsda_input_screenshot},
  {"GAME"        ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y+12*8},
  {"SAVE"        ,S_INPUT     ,m_scrn,KB_X,KB_Y+13*8,{0},dsda_input_savegame},
  {"LOAD"        ,S_INPUT     ,m_scrn,KB_X,KB_Y+14*8,{0},dsda_input_loadgame},
  {"QUICKSAVE"   ,S_INPUT     ,m_scrn,KB_X,KB_Y+15*8,{0},dsda_input_quicksave},
  {"QUICKLOAD"   ,S_INPUT     ,m_scrn,KB_X,KB_Y+16*8,{0},dsda_input_quickload},
  {"END GAME"    ,S_INPUT     ,m_scrn,KB_X,KB_Y+17*8,{0},dsda_input_endgame},
  {"QUIT"        ,S_INPUT     ,m_scrn,KB_X,KB_Y+18*8,{0},dsda_input_quit},
  {"<- PREV", S_SKIP|S_PREV,m_null,KB_PREV,KB_Y+20*8, {keys_settings1}},
  {"NEXT ->", S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {keys_settings3}},

  // Final entry

  {0,S_SKIP|S_END,m_null}
};

setup_menu_t keys_settings3[] =  // Key Binding screen strings
{
  {"WEAPONS" ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y},
  {"FIST"    ,S_INPUT       ,m_scrn,KB_X,KB_Y+ 1*8,{0},dsda_input_weapon1},
  {"PISTOL"  ,S_INPUT       ,m_scrn,KB_X,KB_Y+ 2*8,{0},dsda_input_weapon2},
  {"SHOTGUN" ,S_INPUT       ,m_scrn,KB_X,KB_Y+ 3*8,{0},dsda_input_weapon3},
  {"CHAINGUN",S_INPUT       ,m_scrn,KB_X,KB_Y+ 4*8,{0},dsda_input_weapon4},
  {"ROCKET"  ,S_INPUT       ,m_scrn,KB_X,KB_Y+ 5*8,{0},dsda_input_weapon5},
  {"PLASMA"  ,S_INPUT       ,m_scrn,KB_X,KB_Y+ 6*8,{0},dsda_input_weapon6},
  {"BFG",     S_INPUT       ,m_scrn,KB_X,KB_Y+ 7*8,{0},dsda_input_weapon7},
  {"CHAINSAW",S_INPUT       ,m_scrn,KB_X,KB_Y+ 8*8,{0},dsda_input_weapon8},
  {"SSG"     ,S_INPUT       ,m_scrn,KB_X,KB_Y+ 9*8,{0},dsda_input_weapon9},
  {"NEXT"    ,S_INPUT       ,m_scrn,KB_X,KB_Y+11*8,{0},dsda_input_nextweapon},
  {"PREVIOUS",S_INPUT       ,m_scrn,KB_X,KB_Y+12*8,{0},dsda_input_prevweapon},
  {"BEST"    ,S_INPUT       ,m_scrn,KB_X,KB_Y+13*8,{0},dsda_input_toggleweapon},
  {"FIRE"    ,S_INPUT       ,m_scrn,KB_X,KB_Y+15*8,{0},dsda_input_fire},

  {"<- PREV",S_SKIP|S_PREV,m_null,KB_PREV,KB_Y+20*8, {keys_settings2}},
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {keys_settings4}},

  // Final entry

  {0,S_SKIP|S_END,m_null}

};

setup_menu_t keys_settings4[] =  // Key Binding screen strings
{
  {"AUTOMAP"    ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y},
  {"FOLLOW"     ,S_INPUT     ,m_map ,KB_X,KB_Y+ 1*8,{0},dsda_input_map_follow},
  {"ZOOM IN"    ,S_INPUT     ,m_map ,KB_X,KB_Y+ 2*8,{0},dsda_input_map_zoomin},
  {"ZOOM OUT"   ,S_INPUT     ,m_map ,KB_X,KB_Y+ 3*8,{0},dsda_input_map_zoomout},
  {"SHIFT UP"   ,S_INPUT     ,m_map ,KB_X,KB_Y+ 4*8,{0},dsda_input_map_up},
  {"SHIFT DOWN" ,S_INPUT     ,m_map ,KB_X,KB_Y+ 5*8,{0},dsda_input_map_down},
  {"SHIFT LEFT" ,S_INPUT     ,m_map ,KB_X,KB_Y+ 6*8,{0},dsda_input_map_left},
  {"SHIFT RIGHT",S_INPUT     ,m_map ,KB_X,KB_Y+ 7*8,{0},dsda_input_map_right},
  {"MARK PLACE" ,S_INPUT     ,m_map ,KB_X,KB_Y+ 8*8,{0},dsda_input_map_mark},
  {"CLEAR MARKS",S_INPUT     ,m_map ,KB_X,KB_Y+ 9*8,{0},dsda_input_map_clear},
  {"FULL/ZOOM"  ,S_INPUT     ,m_map ,KB_X,KB_Y+10*8,{0},dsda_input_map_gobig},
  {"GRID"       ,S_INPUT     ,m_map ,KB_X,KB_Y+11*8,{0},dsda_input_map_grid},
  {"ROTATE"     ,S_INPUT     ,m_map ,KB_X,KB_Y+12*8,{0},dsda_input_map_rotate},
  {"OVERLAY"    ,S_INPUT     ,m_map ,KB_X,KB_Y+13*8,{0},dsda_input_map_overlay},
#ifdef GL_DOOM
  {"TEXTURED"   ,S_INPUT     ,m_map ,KB_X,KB_Y+14*8,{0},dsda_input_map_textured},
#endif

  {"<- PREV" ,S_SKIP|S_PREV,m_null,KB_PREV,KB_Y+20*8, {keys_settings3}},
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {keys_settings5}},

  // Final entry

  {0,S_SKIP|S_END,m_null}
};

setup_menu_t keys_settings5[] =  // Key Binding screen strings
{
  {"CHATTING"   ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y+0*8},
  {"BEGIN CHAT" ,S_INPUT     ,m_scrn,KB_X,KB_Y+1*8,{0},dsda_input_chat},
  {"PLAYER 1"   ,S_INPUT     ,m_scrn,KB_X,KB_Y+2*8,{0},dsda_input_chat_dest0},
  {"PLAYER 2"   ,S_INPUT     ,m_scrn,KB_X,KB_Y+3*8,{0},dsda_input_chat_dest1},
  {"PLAYER 3"   ,S_INPUT     ,m_scrn,KB_X,KB_Y+4*8,{0},dsda_input_chat_dest2},
  {"PLAYER 4"   ,S_INPUT     ,m_scrn,KB_X,KB_Y+5*8,{0},dsda_input_chat_dest3},
  {"BACKSPACE"  ,S_INPUT     ,m_scrn,KB_X,KB_Y+6*8,{0},dsda_input_chat_backspace},
  {"ENTER"      ,S_INPUT     ,m_scrn,KB_X,KB_Y+7*8,{0},dsda_input_chat_enter},

  {"<- PREV" ,S_SKIP|S_PREV,m_null,KB_PREV,KB_Y+20*8, {keys_settings4}},
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {keys_settings6}},

  // Final entry

  {0,S_SKIP|S_END,m_null}
};

//e6y
setup_menu_t keys_settings6[] =  // Key Binding screen strings
{
  {"GAME SPEED"           ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y},
  {"SPEED UP"             ,S_INPUT   ,m_scrn,KB_X,KB_Y+ 1*8,{0},dsda_input_speed_up},
  {"SPEED DOWN"           ,S_INPUT   ,m_scrn,KB_X,KB_Y+ 2*8,{0},dsda_input_speed_down},
  {"RESET TO DEFAULT"     ,S_INPUT   ,m_scrn,KB_X,KB_Y+ 3*8,{0},dsda_input_speed_default},
  {"STEP OF CHANGE (0-AUTO)" ,S_NUM  ,m_null,KB_X,KB_Y+ 4*8, {"speed_step"}},
  {"DEMOS"                ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y+5*8},
  {"START/STOP SKIPPING"  ,S_INPUT   ,m_scrn,KB_X,KB_Y+ 6*8,{0},dsda_input_demo_skip},
  {"END LEVEL"            ,S_INPUT   ,m_scrn,KB_X,KB_Y+ 7*8,{0},dsda_input_demo_endlevel},
  {"CAMERA MODE"          ,S_INPUT   ,m_scrn,KB_X,KB_Y+ 8*8,{0},dsda_input_walkcamera},
  {"JOIN"                 ,S_INPUT   ,m_scrn,KB_X,KB_Y+ 9*8,{0},dsda_input_join_demo},
  {"MISC"                 ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y+10*8},
  {"RESTART CURRENT MAP"  ,S_INPUT   ,m_scrn,KB_X,KB_Y+ 11*8,{0},dsda_input_restart},
  {"NEXT LEVEL"           ,S_INPUT   ,m_scrn,KB_X,KB_Y+ 12*8,{0},dsda_input_nextlevel},
#ifdef GL_DOOM
  {"Show Alive Monsters"  ,S_INPUT   ,m_scrn,KB_X,KB_Y+13*8,{0},dsda_input_showalive},
#endif

  {"<- PREV",S_SKIP|S_PREV,m_null,KB_PREV,KB_Y+20*8, {keys_settings5}},
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {keys_settings7}},
  // Final entry
  {0,S_SKIP|S_END,m_null}
};

setup_menu_t keys_settings7[] =
{
  {"MENUS"       ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y+0*8},
  {"NEXT ITEM"   ,S_INPUT     ,m_menu,KB_X,KB_Y+1*8,{0},dsda_input_menu_down},
  {"PREV ITEM"   ,S_INPUT     ,m_menu,KB_X,KB_Y+2*8,{0},dsda_input_menu_up},
  {"LEFT"        ,S_INPUT     ,m_menu,KB_X,KB_Y+3*8,{0},dsda_input_menu_left},
  {"RIGHT"       ,S_INPUT     ,m_menu,KB_X,KB_Y+4*8,{0},dsda_input_menu_right},
  {"BACKSPACE"   ,S_INPUT     ,m_menu,KB_X,KB_Y+5*8,{0},dsda_input_menu_backspace},
  {"SELECT ITEM" ,S_INPUT     ,m_menu,KB_X,KB_Y+6*8,{0},dsda_input_menu_enter},
  {"EXIT"        ,S_INPUT     ,m_menu,KB_X,KB_Y+7*8,{0},dsda_input_menu_escape},
  {"CLEAR"       ,S_INPUT     ,m_menu,KB_X,KB_Y+8*8,{0},dsda_input_menu_clear},

  {"<- PREV",S_SKIP|S_PREV,m_null,KB_PREV,KB_Y+20*8, {keys_settings6}},
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {keys_settings8}},
  // Final entry
  {0,S_SKIP|S_END,m_null}
};

setup_menu_t keys_settings8[] =
{
  { "CHEATS", S_SKIP | S_TITLE, m_null, KB_X, KB_Y + 0 * 8 },
  { "God Mode", S_INPUT, m_scrn, KB_X, KB_Y + 1 * 8, { 0 }, dsda_input_iddqd },
  { "Ammo & Keys", S_INPUT, m_scrn, KB_X, KB_Y + 2 * 8, { 0 }, dsda_input_idkfa },
  { "Ammo", S_INPUT, m_scrn, KB_X, KB_Y + 3 * 8, { 0 }, dsda_input_idfa },
  { "No Clipping", S_INPUT, m_scrn, KB_X, KB_Y + 4 * 8, { 0 }, dsda_input_idclip },
  { "Health", S_INPUT, m_scrn, KB_X, KB_Y + 5 * 8, { 0 }, dsda_input_idbeholdh },
  { "Armor", S_INPUT, m_scrn, KB_X, KB_Y + 6 * 8, { 0 }, dsda_input_idbeholdm },
  { "Invulnerability", S_INPUT, m_scrn, KB_X, KB_Y + 7 * 8, { 0 }, dsda_input_idbeholdv },
  { "Berserk", S_INPUT, m_scrn, KB_X, KB_Y + 8 * 8, { 0 }, dsda_input_idbeholds },
  { "Partial Invisibility", S_INPUT, m_scrn, KB_X, KB_Y + 9 * 8, { 0 }, dsda_input_idbeholdi },
  { "Radiation Suit", S_INPUT, m_scrn, KB_X, KB_Y + 10 * 8, { 0 }, dsda_input_idbeholdr },
  { "Computer Area Map", S_INPUT, m_scrn, KB_X, KB_Y + 11 * 8, { 0 }, dsda_input_idbeholda },
  { "Light Amplification", S_INPUT, m_scrn, KB_X, KB_Y + 12 * 8, { 0 }, dsda_input_idbeholdl },
  { "Show Position", S_INPUT, m_scrn, KB_X, KB_Y + 13 * 8, { 0 }, dsda_input_idmypos },
  { "Show FPS", S_INPUT, m_scrn, KB_X, KB_Y + 14 * 8, { 0 }, dsda_input_idrate },
  { "Reveal Map", S_INPUT, m_scrn, KB_X, KB_Y + 15 * 8, { 0 }, dsda_input_iddt },
  { "Reset Health", S_INPUT, m_scrn, KB_X, KB_Y + 16 * 8, { 0 }, dsda_input_ponce },
  { "Tome of Power", S_INPUT, m_scrn, KB_X, KB_Y + 17 * 8, { 0 }, dsda_input_shazam },
  { "Chicken", S_INPUT, m_scrn, KB_X, KB_Y + 18 * 8, { 0 }, dsda_input_chicken },

  { "<- PREV", S_SKIP | S_PREV, m_null, KB_PREV, KB_Y + 20 * 8, { keys_settings7 } },
  { "NEXT ->", S_SKIP | S_NEXT, m_null, KB_NEXT, KB_Y + 20 * 8, { heretic_keys_settings1 } },

  // Final entry
  { 0, S_SKIP | S_END, m_null }
};

setup_menu_t heretic_keys_settings1[] = {
  { "HERETIC MOVEMENT", S_SKIP | S_TITLE, m_null, KB_X, KB_Y },
  { "LOOK UP", S_INPUT, m_scrn, KB_X, KB_Y + 1 * 8, { 0 }, dsda_input_lookup },
  { "LOOK DOWN", S_INPUT, m_scrn, KB_X, KB_Y + 2 * 8, { 0 }, dsda_input_lookdown },
  { "LOOK CENTER", S_INPUT, m_scrn, KB_X, KB_Y + 3 * 8, { 0 }, dsda_input_lookcenter },
  { "FLY UP", S_INPUT, m_scrn, KB_X, KB_Y + 4 * 8, { 0 }, dsda_input_flyup },
  { "FLY DOWN", S_INPUT, m_scrn, KB_X, KB_Y + 5 * 8, { 0 }, dsda_input_flydown },
  { "FLY CENTER", S_INPUT, m_scrn, KB_X, KB_Y + 6 * 8, { 0 }, dsda_input_flycenter },

  { "<- PREV", S_SKIP | S_PREV, m_null, KB_PREV, KB_Y + 20 * 8, { keys_settings8 } },
  { "NEXT ->", S_SKIP | S_NEXT, m_null, KB_NEXT, KB_Y + 20 * 8, { heretic_keys_settings2 } },

  // Final entry
  { 0, S_SKIP | S_END, m_null }
};

setup_menu_t heretic_keys_settings2[] = {
  { "HERETIC INVENTORY", S_SKIP | S_TITLE, m_null, KB_X, KB_Y },
  { "USE ARTIFACT", S_INPUT, m_scrn, KB_X, KB_Y + 1 * 8, { 0 }, dsda_input_use_artifact },
  { "USE TOME OF POWER", S_INPUT, m_scrn, KB_X, KB_Y + 2 * 8, { 0 }, dsda_input_arti_tome },
  { "USE QUARTZ FLASK", S_INPUT, m_scrn, KB_X, KB_Y + 3 * 8, { 0 }, dsda_input_arti_quartz },
  { "USE MYSTIC URN", S_INPUT, m_scrn, KB_X, KB_Y + 4 * 8, { 0 }, dsda_input_arti_urn },
  { "USE TIMEBOMB", S_INPUT, m_scrn, KB_X, KB_Y + 5 * 8, { 0 }, dsda_input_arti_bomb },
  { "USE RING OF INVINCIBILITY", S_INPUT, m_scrn, KB_X, KB_Y + 6 * 8, { 0 }, dsda_input_arti_ring },
  { "USE CHAOS DEVICE", S_INPUT, m_scrn, KB_X, KB_Y + 7 * 8, { 0 }, dsda_input_arti_chaosdevice },
  { "USE SHADOWSPHERE", S_INPUT, m_scrn, KB_X, KB_Y + 8 * 8, { 0 }, dsda_input_arti_shadowsphere },
  { "USE WINGS OF WRATH", S_INPUT, m_scrn, KB_X, KB_Y + 9 * 8, { 0 }, dsda_input_arti_wings },
  { "USE TORCH", S_INPUT, m_scrn, KB_X, KB_Y + 10 * 8, { 0 }, dsda_input_arti_torch },
  { "USER MORPH OVUM", S_INPUT, m_scrn, KB_X, KB_Y + 11 * 8, { 0 }, dsda_input_arti_morph },
  { "INVENTORY LEFT", S_INPUT, m_scrn, KB_X, KB_Y + 12 * 8, { 0 }, dsda_input_invleft },
  { "INVENTORY RIGHT", S_INPUT, m_scrn, KB_X, KB_Y + 13 * 8, { 0 }, dsda_input_invright },

  { "<- PREV", S_SKIP | S_PREV, m_null, KB_PREV, KB_Y + 20 * 8, { heretic_keys_settings1 } },
  { "NEXT ->", S_SKIP | S_NEXT, m_null, KB_NEXT, KB_Y + 20 * 8, { dsda_keys_settings } },

  // Final entry
  { 0, S_SKIP | S_END, m_null }
};

setup_menu_t dsda_keys_settings[] = {
  { "DSDA-Doom Keys", S_SKIP | S_TITLE, m_null, KB_X, KB_Y + 0 * 8 },
  { "Store Quick Key Frame", S_INPUT, m_scrn, KB_X, KB_Y + 1 * 8, { 0 }, dsda_input_store_quick_key_frame },
  { "Restore Quick Key Frame", S_INPUT, m_scrn, KB_X, KB_Y + 2 * 8, { 0 }, dsda_input_restore_quick_key_frame },
  { "Rewind", S_INPUT, m_scrn, KB_X, KB_Y + 3 * 8, { 0 }, dsda_input_rewind },
  { "Cycle Input Profile", S_INPUT, m_scrn, KB_X, KB_Y + 4 * 8, { 0 }, dsda_input_cycle_profile },
  { "Cycle Palette", S_INPUT, m_scrn, KB_X, KB_Y + 5 * 8, { 0 }, dsda_input_cycle_palette },
  { "Toggle Command Display", S_INPUT, m_scrn, KB_X, KB_Y + 6 * 8, { 0 }, dsda_input_command_display },
  { "Toggle Strict Mode", S_INPUT, m_scrn, KB_X, KB_Y + 7 * 8, { 0 }, dsda_input_strict_mode },
  { "Open Console", S_INPUT, m_scrn, KB_X, KB_Y + 8 * 8, { 0 }, dsda_input_console },
  { "Toggle Coord. Display", S_INPUT, m_scrn, KB_X, KB_Y + 9 * 8, { 0 }, dsda_input_coordinate_display },

  { "<- PREV", S_SKIP | S_PREV, m_null, KB_PREV, KB_Y + 20 * 8, { heretic_keys_settings2 } },
  { 0, S_SKIP | S_END, m_null }
};

// Setting up for the Key Binding screen. Turn on flags, set pointers,
// locate the first item on the screen where the cursor is allowed to
// land.

void M_KeyBindings(int choice)
{
  M_SetupNextMenu(&KeybndDef);

  setup_active = true;
  setup_screen = ss_keys;
  set_keybnd_active = true;
  setup_select = false;
  default_verify = false;
  setup_gather = false;
  mult_screens_index = 0;
  current_setup_menu = keys_settings[0];
  set_menu_itemon = M_GetSetupMenuItemOn();
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}

// The drawing part of the Key Bindings Setup initialization. Draw the
// background, title, instruction line, and items.

void M_DrawKeybnd(void)
{
  menuactive = mnact_full;

  // Set up the Key Binding screen

  M_DrawBackground(g_menu_flat, 0); // Draw background

  // proff/nicolas 09/20/98 -- changed for hi-res
  M_DrawTitle(84, 2, "M_KEYBND", CR_DEFAULT, "KEY BINDINGS", g_cr_gold);
  M_DrawInstructions();
  M_DrawScreenItems(current_setup_menu);

  // If the Reset Button has been selected, an "Are you sure?" message
  // is overlayed across everything else.

  if (default_verify)
    M_DrawDefVerify();
}

/////////////////////////////
//
// The Weapon Screen tables.

#define WP_X 203
#define WP_Y  33

static const char *weapon_attack_alignment_strings[] = {
  "OFF", "HORIZONTAL", "CENTERED", "BOBBING", NULL
};

// There's only one weapon settings screen (for now). But since we're
// trying to fit a common description for screens, it gets a setup_menu_t,
// which only has one screen definition in it.
//
// Note that this screen has no PREV or NEXT items, since there are no
// neighboring screens.

enum {           // killough 10/98: enum for y-offset info
  weap_bobbing,
  weap_attack_alignment,
  weap_stub1,
  weap_pref1,
  weap_pref2,
  weap_pref3,
  weap_pref4,
  weap_pref5,
  weap_pref6,
  weap_pref7,
  weap_pref8,
  weap_pref9,
};

setup_menu_t weap_settings1[];

setup_menu_t* weap_settings[] =
{
  weap_settings1,
  NULL
};

setup_menu_t weap_settings1[] =  // Weapons Settings screen
{
  {"ENABLE BOBBING",S_YESNO,m_null,WP_X, WP_Y+weap_bobbing*8, {"player_bobbing"}},
  {"WEAPON ATTACK ALIGNMENT",S_CHOICE,m_null,WP_X, WP_Y+weap_attack_alignment*8, {"weapon_attack_alignment"}, 0, NULL, weapon_attack_alignment_strings},

  {"1ST CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref1*8, {"weapon_choice_1"}},
  {"2nd CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref2*8, {"weapon_choice_2"}},
  {"3rd CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref3*8, {"weapon_choice_3"}},
  {"4th CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref4*8, {"weapon_choice_4"}},
  {"5th CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref5*8, {"weapon_choice_5"}},
  {"6th CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref6*8, {"weapon_choice_6"}},
  {"7th CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref7*8, {"weapon_choice_7"}},
  {"8th CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref8*8, {"weapon_choice_8"}},
  {"9th CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref9*8, {"weapon_choice_9"}},

  // Button for resetting to defaults
  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},

  // Final entry
  {0,S_SKIP|S_END,m_null}

};

// Setting up for the Weapons screen. Turn on flags, set pointers,
// locate the first item on the screen where the cursor is allowed to
// land.

void M_Weapons(int choice)
{
  M_SetupNextMenu(&WeaponDef);

  setup_active = true;
  setup_screen = ss_weap;
  set_weapon_active = true;
  setup_select = false;
  default_verify = false;
  setup_gather = false;
  mult_screens_index = 0;
  current_setup_menu = weap_settings[0];
  set_menu_itemon = M_GetSetupMenuItemOn();
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}


// The drawing part of the Weapons Setup initialization. Draw the
// background, title, instruction line, and items.

void M_DrawWeapons(void)
{
  menuactive = mnact_full;

  M_DrawBackground(g_menu_flat, 0); // Draw background

  // proff/nicolas 09/20/98 -- changed for hi-res
  M_DrawTitle(109, 2, "M_WEAP", CR_DEFAULT, "WEAPONS", g_cr_gold);
  M_DrawInstructions();
  M_DrawScreenItems(current_setup_menu);

  // If the Reset Button has been selected, an "Are you sure?" message
  // is overlayed across everything else.

  if (default_verify)
    M_DrawDefVerify();
}

/////////////////////////////
//
// The Status Bar / HUD tables.

#define SB_X 203
#define SB_Y  31

// Screen table definitions

setup_menu_t stat_settings1[];
//e6y
setup_menu_t stat_settings2[];

setup_menu_t* stat_settings[] =
{
  stat_settings1,
  //e6y
  stat_settings2,
  NULL
};

setup_menu_t stat_settings1[] =  // Status Bar and HUD Settings screen
{
  {"STATUS BAR"        ,S_SKIP|S_TITLE,m_null,SB_X,SB_Y+ 1*8 },

  {"USE RED NUMBERS"   ,S_YESNO, m_null,SB_X,SB_Y+ 2*8, {"sts_always_red"}},
  {"GRAY %"            ,S_YESNO, m_null,SB_X,SB_Y+ 3*8, {"sts_pct_always_gray"}},
  {"SINGLE KEY DISPLAY",S_YESNO, m_null,SB_X,SB_Y+ 4*8, {"sts_traditional_keys"}},

  {"HEADS-UP DISPLAY"  ,S_SKIP|S_TITLE,m_null,SB_X,SB_Y+ 6*8},

  {"HEALTH LOW/OK"     ,S_NUM       ,m_null,SB_X,SB_Y+ 7*8, {"health_red"}},
  {"HEALTH OK/GOOD"    ,S_NUM       ,m_null,SB_X,SB_Y+ 8*8, {"health_yellow"}},
  {"HEALTH GOOD/EXTRA" ,S_NUM       ,m_null,SB_X,SB_Y+ 9*8, {"health_green"}},
  {"ARMOR COLOR DEPENDS ON TYPE",S_YESNO, m_null,SB_X,SB_Y+ 10*8, {"sts_armorcolor_type"}},
  {"ARMOR LOW/OK"      ,S_NUM       ,m_null,SB_X,SB_Y+11*8, {"armor_red"}},
  {"ARMOR OK/GOOD"     ,S_NUM       ,m_null,SB_X,SB_Y+12*8, {"armor_yellow"}},
  {"ARMOR GOOD/EXTRA"  ,S_NUM       ,m_null,SB_X,SB_Y+13*8, {"armor_green"}},
  {"AMMO LOW/OK"       ,S_NUM       ,m_null,SB_X,SB_Y+14*8, {"ammo_red"}},
  {"AMMO OK/GOOD"      ,S_NUM       ,m_null,SB_X,SB_Y+15*8, {"ammo_yellow"}},
  {"BACKPACK CHANGES THRESHOLDS",S_CHOICE,m_null,SB_X,SB_Y+16*8,
   {"ammo_colour_behaviour"},0,NULL,ammo_colour_behaviour_list},

  // Button for resetting to defaults
  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},

  //e6y
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,SB_Y+20*8, {stat_settings2}},
  // Final entry
  {0,S_SKIP|S_END,m_null}
};

//e6y
#define ADVHUD_X 284
setup_menu_t stat_settings2[] =
{
  {"ADVANCED HUD SETTINGS"       ,S_SKIP|S_TITLE,m_null,ADVHUD_X,SB_Y+1*8},
  {"REPORT REVEALED SECRETS"     ,S_YESNO     ,m_null,ADVHUD_X,SB_Y+ 2*8, {"hudadd_secretarea"}},
  {"MAX TOTALS"                  ,S_YESNO     ,m_null,ADVHUD_X,SB_Y+ 3*8, {"hudadd_maxtotals"}},
  {"SHOW GAMESPEED"              ,S_YESNO     ,m_null,ADVHUD_X,SB_Y+ 4*8, {"hudadd_gamespeed"}},
  {"SHOW LEVELTIME"              ,S_YESNO     ,m_null,ADVHUD_X,SB_Y+ 5*8, {"hudadd_leveltime"}},
  {"SHOW DEMOTIME"               ,S_YESNO     ,m_null,ADVHUD_X,SB_Y+ 6*8, {"hudadd_demotime"}},
  {"SHOW PROGRESS BAR DURING DEMO PLAYBACK" ,S_YESNO     ,m_null,ADVHUD_X,SB_Y+ 7*8, {"hudadd_demoprogressbar"}},

  {"CROSSHAIR SETTINGS"            ,S_SKIP|S_TITLE,m_null,ADVHUD_X,SB_Y+9*8},
  {"ENABLE CROSSHAIR"              ,S_CHOICE   ,m_null,ADVHUD_X,SB_Y+10*8, {"hudadd_crosshair"}, 0, 0, crosshair_str},
  {"SCALE CROSSHAIR"               ,S_YESNO    ,m_null,ADVHUD_X,SB_Y+11*8, {"hudadd_crosshair_scale"}},
  {"CHANGE CROSSHAIR COLOR BY PLAYER HEALTH" ,S_YESNO    ,m_null,ADVHUD_X,SB_Y+12*8, {"hudadd_crosshair_health"}},
  {"CHANGE CROSSHAIR COLOR ON TARGET"        ,S_YESNO    ,m_null,ADVHUD_X,SB_Y+13*8, {"hudadd_crosshair_target"}},
  {"LOCK CROSSHAIR ON TARGET"                ,S_YESNO    ,m_null,ADVHUD_X,SB_Y+14*8, {"hudadd_crosshair_lock_target"}},
  {"DEFAULT CROSSHAIR COLOR"                 ,S_CRITEM   ,m_null,ADVHUD_X,SB_Y+15*8, {"hudadd_crosshair_color"}},
  {"TARGET CROSSHAIR COLOR"                  ,S_CRITEM   ,m_null,ADVHUD_X,SB_Y+16*8, {"hudadd_crosshair_target_color"}},

  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},
  {"<- PREV",S_SKIP|S_PREV,m_null,KB_PREV,SB_Y+20*8, {stat_settings1}},
  {0,S_SKIP|S_END,m_null}
};

// Setting up for the Status Bar / HUD screen. Turn on flags, set pointers,
// locate the first item on the screen where the cursor is allowed to
// land.

void M_StatusBar(int choice)
{
  M_SetupNextMenu(&StatusHUDDef);

  setup_active = true;
  setup_screen = ss_stat;
  set_status_active = true;
  setup_select = false;
  default_verify = false;
  setup_gather = false;
  mult_screens_index = 0;
  current_setup_menu = stat_settings[0];
  set_menu_itemon = M_GetSetupMenuItemOn();
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}


// The drawing part of the Status Bar / HUD Setup initialization. Draw the
// background, title, instruction line, and items.

void M_DrawStatusHUD(void)
{
  menuactive = mnact_full;

  M_DrawBackground(g_menu_flat, 0); // Draw background

  // proff/nicolas 09/20/98 -- changed for hi-res
  M_DrawTitle(59, 2, "M_STAT", CR_DEFAULT, "STATUS BAR / HUD", g_cr_gold);
  M_DrawInstructions();
  M_DrawScreenItems(current_setup_menu);

  // If the Reset Button has been selected, an "Are you sure?" message
  // is overlayed across everything else.

  if (default_verify)
    M_DrawDefVerify();
}


/////////////////////////////
//
// The Automap tables.

#define AU_X    275
#define AU_Y     31
#define AU_PREV KB_PREV
#define AU_NEXT KB_NEXT

setup_menu_t auto_settings1[];
setup_menu_t auto_settings2[];
setup_menu_t auto_settings3[];

setup_menu_t* auto_settings[] =
{
  auto_settings1,
  auto_settings2,
  auto_settings3,
  NULL
};

setup_menu_t auto_settings1[] =  // 1st AutoMap Settings screen
{
  {"Show Kills/Secrts/Items statistics",      S_YESNO,m_null,AU_X,AU_Y+0*8, {"map_level_stat"}},
  {"Show coordinates of automap pointer",     S_YESNO,m_null,AU_X,AU_Y+1*8, {"map_point_coord"}},  // killough 10/98
  {"Show Secrets only after entering",        S_YESNO,m_null,AU_X,AU_Y+2*8, {"map_secret_after"}},
  {"Update unexplored parts in automap mode", S_YESNO,m_null,AU_X,AU_Y+3*8, {"map_always_updates"}},
  {"Grid cell size 8..256, -1 for autosize",  S_NUM,  m_null,AU_X,AU_Y+4*8, {"map_grid_size"}, 0, M_ChangeMapGridSize},
  {"Scroll / Zoom speed  (1..32)",            S_NUM,  m_null,AU_X,AU_Y+5*8, {"map_scroll_speed"}},
  {"Use mouse wheel for zooming",             S_YESNO,m_null,AU_X,AU_Y+6*8, {"map_wheel_zoom"}},
  {"Apply multisampling",                     S_YESNO,m_null,AU_X,AU_Y+7*8, {"map_use_multisamling"}, 0, M_ChangeMapMultisamling},
#ifdef GL_DOOM
  {"Enable textured display",                 S_YESNO,m_null,AU_X,AU_Y+8*8, {"map_textured"}, 0, M_ChangeMapTextured},
  {"Things appearance",                       S_CHOICE,m_null,AU_X,AU_Y+9*8, {"map_things_appearance"}, 0, NULL, map_things_appearance_list},
  {"Translucency percentage",                 S_SKIP|S_TITLE,m_null,AU_X,AU_Y+10*8},
  {"Textured automap",                        S_NUM,  m_null,AU_X,AU_Y+11*8, {"map_textured_trans"}},
  {"Textured automap in overlay mode",        S_NUM,  m_null,AU_X,AU_Y+12*8, {"map_textured_overlay_trans"}},
  {"Lines in overlay mode",                   S_NUM,  m_null,AU_X,AU_Y+13*8, {"map_lines_overlay_trans"}},
#endif

  // Button for resetting to defaults
  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},

  {"NEXT ->",S_SKIP|S_NEXT,m_null,AU_NEXT,AU_Y+20*8, {auto_settings2}},

  // Final entry
  {0,S_SKIP|S_END,m_null}
};

setup_menu_t auto_settings2[] =  // 2st AutoMap Settings screen
{
  {"background", S_COLOR, m_null, AU_X, AU_Y, {"mapcolor_back"}},
  {"grid lines", S_COLOR, m_null, AU_X, AU_Y + 1*8, {"mapcolor_grid"}},
  {"normal 1s wall", S_COLOR, m_null,AU_X,AU_Y+ 2*8, {"mapcolor_wall"}},
  {"line at floor height change", S_COLOR, m_null, AU_X, AU_Y+ 3*8, {"mapcolor_fchg"}},
  {"line at ceiling height change"      ,S_COLOR,m_null,AU_X,AU_Y+ 4*8, {"mapcolor_cchg"}},
  {"line at sector with floor = ceiling",S_COLOR,m_null,AU_X,AU_Y+ 5*8, {"mapcolor_clsd"}},
  {"red key"                            ,S_COLOR,m_null,AU_X,AU_Y+ 6*8, {"mapcolor_rkey"}},
  {"blue key"                           ,S_COLOR,m_null,AU_X,AU_Y+ 7*8, {"mapcolor_bkey"}},
  {"yellow key"                         ,S_COLOR,m_null,AU_X,AU_Y+ 8*8, {"mapcolor_ykey"}},
  {"red door"                           ,S_COLOR,m_null,AU_X,AU_Y+ 9*8, {"mapcolor_rdor"}},
  {"blue door"                          ,S_COLOR,m_null,AU_X,AU_Y+10*8, {"mapcolor_bdor"}},
  {"yellow door"                        ,S_COLOR,m_null,AU_X,AU_Y+11*8, {"mapcolor_ydor"}},

  {"AUTOMAP LEVEL TITLE COLOR"      ,S_CRITEM,m_null,AU_X,AU_Y+12*8, {"hudcolor_titl"}},
  {"AUTOMAP COORDINATES COLOR"      ,S_CRITEM,m_null,AU_X,AU_Y+13*8, {"hudcolor_xyco"}},

  {"Automap Statistics Titles Color",S_CRITEM,m_null,AU_X,AU_Y+14*8, {"hudcolor_mapstat_title"}},
  {"Automap Statistics Values Color",S_CRITEM,m_null,AU_X,AU_Y+15*8, {"hudcolor_mapstat_value"}},
  {"Automap Statistics Time Color"  ,S_CRITEM,m_null,AU_X,AU_Y+16*8, {"hudcolor_mapstat_time"}},

  // Button for resetting to defaults
  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},

  {"<- PREV",S_SKIP|S_PREV,m_null,AU_PREV,AU_Y+20*8, {auto_settings1}},
  {"NEXT ->",S_SKIP|S_NEXT,m_null,AU_NEXT,AU_Y+20*8, {auto_settings3}},

  // Final entry
  {0,S_SKIP|S_END,m_null}

};

setup_menu_t auto_settings3[] =  // 3nd AutoMap Settings screen
{
  {"teleporter line"                ,S_COLOR ,m_null,AU_X,AU_Y, {"mapcolor_tele"}},
  {"secret sector boundary"         ,S_COLOR ,m_null,AU_X,AU_Y+ 1*8, {"mapcolor_secr"}},
  //jff 4/23/98 add exit line to automap
  {"exit line"                      ,S_COLOR ,m_null,AU_X,AU_Y+ 2*8, {"mapcolor_exit"}},
  {"computer map unseen line"       ,S_COLOR ,m_null,AU_X,AU_Y+ 3*8, {"mapcolor_unsn"}},
  {"line w/no floor/ceiling changes",S_COLOR ,m_null,AU_X,AU_Y+ 4*8, {"mapcolor_flat"}},
  {"general sprite"                 ,S_COLOR ,m_null,AU_X,AU_Y+ 5*8, {"mapcolor_sprt"}},
  {"countable enemy sprite"         ,S_COLOR ,m_null,AU_X,AU_Y+ 6*8, {"mapcolor_enemy"}},      // cph 2006/06/30
  {"countable item sprite"          ,S_COLOR ,m_null,AU_X,AU_Y+ 7*8, {"mapcolor_item"}},       // mead 3/4/2003
  {"crosshair"                      ,S_COLOR ,m_null,AU_X,AU_Y+ 8*8, {"mapcolor_hair"}},
  {"single player arrow"            ,S_COLOR ,m_null,AU_X,AU_Y+ 9*8, {"mapcolor_sngl"}},
  {"your colour in multiplayer"     ,S_COLOR ,m_null,AU_X,AU_Y+10*8, {"mapcolor_me"}},

  {"friends"                        ,S_COLOR ,m_null,AU_X,AU_Y+12*8, {"mapcolor_frnd"}},        // killough 8/8/98

  {"<- PREV",S_SKIP|S_PREV,m_null,AU_PREV,AU_Y+20*8, {auto_settings2}},

  // Final entry

  {0,S_SKIP|S_END,m_null}

};


// Setting up for the Automap screen. Turn on flags, set pointers,
// locate the first item on the screen where the cursor is allowed to
// land.

void M_Automap(int choice)
{
  M_SetupNextMenu(&AutoMapDef);

  setup_active = true;
  setup_screen = ss_auto;
  set_auto_active = true;
  setup_select = false;
  colorbox_active = false;
  default_verify = false;
  setup_gather = false;
  mult_screens_index = 0;
  current_setup_menu = auto_settings[0];
  set_menu_itemon = M_GetSetupMenuItemOn();
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}

// Data used by the color palette that is displayed for the player to
// select colors.

int color_palette_x; // X position of the cursor on the color palette
int color_palette_y; // Y position of the cursor on the color palette
byte palette_background[16*(CHIP_SIZE+1)+8];

// M_DrawColPal() draws the color palette when the user needs to select a
// color.

// phares 4/1/98: now uses a single lump for the palette instead of
// building the image out of individual paint chips.

static void M_DrawColPal(void)
{
  int cpx, cpy;

  // Draw a background, border, and paint chips

  // proff/nicolas 09/20/98 -- changed for hi-res
  // CPhipps - patch drawing updated
  V_DrawNamePatch(COLORPALXORIG-5, COLORPALYORIG-5, 0, "M_COLORS", CR_DEFAULT, VPT_STRETCH);

  // Draw the cursor around the paint chip
  // (cpx,cpy) is the upper left-hand corner of the paint chip

  cpx = COLORPALXORIG+color_palette_x*(CHIP_SIZE+1)-1;
  cpy = COLORPALYORIG+color_palette_y*(CHIP_SIZE+1)-1;
  // proff 12/6/98: Drawing of colorchips completly changed for hi-res, it now uses a patch
  V_DrawNamePatch(cpx,cpy,0,"M_PALSEL",CR_DEFAULT,VPT_STRETCH); // PROFF_GL_FIX
}

// The drawing part of the Automap Setup initialization. Draw the
// background, title, instruction line, and items.

void M_DrawAutoMap(void)
{
  menuactive = mnact_full;

  M_DrawBackground(g_menu_flat, 0); // Draw background

  // CPhipps - patch drawing updated
  M_DrawTitle(109, 2, "M_AUTO", CR_DEFAULT, "AUTOMAP", g_cr_gold);
  M_DrawInstructions();
  M_DrawScreenItems(current_setup_menu);

  // If a color is being selected, need to show color paint chips

  if (colorbox_active)
    M_DrawColPal();

  // If the Reset Button has been selected, an "Are you sure?" message
  // is overlayed across everything else.

  else if (default_verify)
    M_DrawDefVerify();
}

/////////////////////////////
//
// The General table.
// killough 10/10/98

extern int usejoystick, usemouse, default_mus_card, default_snd_card;
extern int detect_voices, realtic_clock_rate, tran_filter_pct;

setup_menu_t gen_settings1[], gen_settings2[], gen_settings3[];
setup_menu_t gen_settings4[], gen_settings5[], gen_settings6[];
setup_menu_t gen_settings7[], gen_settings8[];
setup_menu_t dsda_gen_settings[];

setup_menu_t* gen_settings[] =
{
  gen_settings1,
  gen_settings2,
  gen_settings3,
  gen_settings4,
  gen_settings5,
  gen_settings6,
  gen_settings7,
  gen_settings8,
  dsda_gen_settings,
  NULL
};

#define G_X 226
#define GF_X 76
#define G_Y 23
#define G_X2 284

static const char *videomodes[] = {
  "8bit", "32bit",
#ifdef GL_DOOM
  "OpenGL",
#endif
  NULL};

static const char *gltexformats[] = {
  "GL_RGBA","GL_RGB5_A1", "GL_RGBA4", NULL};

setup_menu_t gen_settings1[] = { // General Settings screen1

  {"Video",                          S_SKIP|S_TITLE,     m_null, G_X, G_Y+ 1*8},
  {"Video mode",                     S_CHOICE,           m_null, G_X, G_Y+ 2*8, {"videomode"}, 0, M_ChangeVideoMode, videomodes},
  {"Screen Resolution",              S_CHOICE,           m_null, G_X, G_Y+ 3*8, {"screen_resolution"}, 0, M_ChangeVideoMode, screen_resolutions_list},
  {"Aspect Ratio",                   S_CHOICE,           m_null, G_X, G_Y+ 4*8, {"render_aspect"}, 0, M_ChangeAspectRatio, render_aspects_list},
  {"Fullscreen Video mode",          S_YESNO,            m_null, G_X, G_Y+ 5*8, {"use_fullscreen"}, 0, M_ChangeFullScreen},
  {"Software Exclusive Fullscreen",  S_YESNO,            m_null, G_X, G_Y+ 6*8, {"exclusive_fullscreen"}, 0, M_ChangeVideoMode},
  {"Status Bar and Menu Appearance", S_CHOICE,           m_null, G_X, G_Y+ 7*8, {"render_stretch_hud"}, 0, M_ChangeStretch, render_stretch_list},
  {"Vertical Sync",                  S_YESNO,            m_null, G_X, G_Y+ 8*8, {"render_vsync"}, 0, M_ChangeVideoMode},
  {"Enable Translucency",            S_YESNO,            m_null, G_X, G_Y+ 9*8, {"translucency"}, 0, M_Trans},
  {"Translucency filter percentage", S_NUM,              m_null, G_X, G_Y+10*8, {"tran_filter_pct"}, 0, M_Trans},
  {"Uncapped Framerate",             S_YESNO,            m_null, G_X, G_Y+11*8, {"uncapped_framerate"}, 0, M_ChangeUncappedFrameRate},

  {"Sound & Music",                  S_SKIP|S_TITLE,     m_null, G_X, G_Y+13*8},
  {"Number of Sound Channels",       S_NUM|S_PRGWARN,    m_null, G_X, G_Y+14*8, {"snd_channels"}},
  {"Enable v1.1 Pitch Effects",      S_YESNO,            m_null, G_X, G_Y+15*8, {"pitched_sounds"}},
  {"PC Speaker emulation",           S_YESNO|S_PRGWARN,  m_null, G_X, G_Y+16*8, {"snd_pcspeaker"}},
  {"Preferred MIDI player",          S_CHOICE|S_PRGWARN, m_null, G_X, G_Y+17*8, {"snd_midiplayer"}, 0, M_ChangeMIDIPlayer, midiplayers},
  {"Disable Sound Cutoffs",          S_YESNO,            m_null, G_X, G_Y+18*8, {"full_sounds"}},

  // Button for resetting to defaults
  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},

  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {gen_settings2}},
  {0,S_SKIP|S_END,m_null}
};

static const char *gen_skillstrings[] = {
  // Dummy first option because defaultskill is 1-based
  "", "ITYTD", "HNTR", "HMP", "UV", "NM", NULL
};

static const char *gen_compstrings[] =
{
  "Default",
  "Doom v1.2",
  "Doom v1.666",
  "Doom/2 v1.9",
  "Ultimate Doom",
  "Final Doom",
  "DosDoom",
  "TASDoom",
  "Boom's vanilla",
  "Boom v2.01",
  "Boom",
  "LxDoom",
  "MBF",
  "PrBoom 2.03b",
  "PrBoom 2.1.x",
  "PrBoom 2.2.x",
  "PrBoom 2.3.x",
  "PrBoom 2.4.0",
  "Latest PrBoom+",
  "~",
  "~",
  "~",
  "MBF21",
  NULL
};

setup_menu_t gen_settings2[] = { // General Settings screen2

  {"Input Devices",                    S_SKIP|S_TITLE, m_null, G_X, G_Y+ 1*8},
  {"Enable Mouse",                     S_YESNO, m_null, G_X, G_Y+ 2*8, {"use_mouse"}},
  {"Enable Joystick",                  S_YESNO, m_null, G_X, G_Y+ 3*8, {"use_joystick"}},

  {"Files Preloaded at Game Startup",  S_SKIP|S_TITLE, m_null, G_X, G_Y + 5*8},
  {"WAD # 1",                          S_FILE, m_null, GF_X, G_Y+ 6*8, {"wadfile_1"}},
  {"WAD #2",                           S_FILE, m_null, GF_X, G_Y+ 7*8, {"wadfile_2"}},
  {"DEH/BEX # 1",                      S_FILE, m_null, GF_X, G_Y+ 8*8, {"dehfile_1"}},
  {"DEH/BEX #2",                       S_FILE, m_null, GF_X, G_Y+ 9*8, {"dehfile_2"}},

  {"Miscellaneous",                    S_SKIP|S_TITLE,  m_null, G_X, G_Y+11*8},
  {"Maximum number of player corpses", S_NUM|S_PRGWARN, m_null, G_X, G_Y+12*8, {"max_player_corpse"}},
  {"Game speed, percentage of normal", S_NUM|S_PRGWARN, m_null, G_X, G_Y+13*8, {"realtic_clock_rate"}},
  {"Default skill level",              S_CHOICE,        m_null, G_X, G_Y+14*8, {"default_skill"}, 0, NULL, gen_skillstrings},
  {"Default compatibility level",      S_CHOICE,        m_null, G_X, G_Y+15*8, {"default_compatibility_level"}, 0, NULL, &gen_compstrings[1]},
  {"Show ENDOOM screen",               S_YESNO,         m_null, G_X, G_Y+16*8, {"showendoom"}},
  {"Fullscreen menu background",       S_YESNO, m_null, G_X, G_Y + 17*8, {"menu_background"}},
#ifdef USE_WINDOWS_LAUNCHER
  {"Use In-Game Launcher",             S_CHOICE,        m_null, G_X, G_Y+ 18*8, {"launcher_enable"}, 0, NULL, launcher_enable_states},
#endif


  {"<- PREV",S_SKIP|S_PREV, m_null, KB_PREV, KB_Y+20*8, {gen_settings1}},
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {gen_settings3}},
  {0,S_SKIP|S_END,m_null}
};

setup_menu_t gen_settings3[] = { // General Settings screen2
  {"Demos",                       S_SKIP|S_TITLE, m_null, G_X, G_Y+ 1*8},
  {"Overwrite Existing",          S_YESNO, m_null, G_X, G_Y+ 2*8, {"demo_overwriteexisting"}},
  {"Smooth Demo Playback",        S_YESNO, m_null, G_X, G_Y+ 3*8, {"demo_smoothturns"}, 0, M_ChangeDemoSmoothTurns},
  {"Smooth Demo Playback Factor", S_NUM,   m_null, G_X, G_Y+ 4*8, {"demo_smoothturnsfactor"}, 0, M_ChangeDemoSmoothTurns},
  {"Quickstart Window (ms)",      S_NUM,   m_null, G_X, G_Y+ 5*8, {"quickstart_window_ms"}},

  {"Movements",                   S_SKIP|S_TITLE,m_null,G_X, G_Y+7*8},
  {"Permanent Strafe50 (TAS)",    S_YESNO, m_null, G_X, G_Y+ 8*8, {"movement_strafe50"}, 0, M_ChangeSpeed},
  {"Strafe50 On Turns (TAS)",     S_YESNO, m_null, G_X, G_Y+ 9*8, {"movement_strafe50onturns"}, 0, M_ChangeSpeed},

  {"Mouse",                       S_SKIP|S_TITLE,m_null, G_X, G_Y+11*8},
  {"Dbl-Click As Use",            S_YESNO, m_null, G_X, G_Y+12*8, {"mouse_doubleclick_as_use"}},
  {"Carry Fractional Tics",       S_YESNO, m_null, G_X, G_Y+13*8, {"mouse_carrytics"}},
  {"Enable Mouselook",            S_YESNO, m_null, G_X, G_Y+14*8, {"movement_mouselook"}, 0, M_ChangeMouseLook},
  {"No Vertical Mouse",           S_YESNO, m_null, G_X, G_Y+15*8, {"movement_mousenovert"}},
  {"Invert Mouse",                S_YESNO, m_null, G_X, G_Y+16*8, {"movement_mouseinvert"}, 0, M_ChangeMouseInvert},
  {"Max View Pitch",              S_NUM,   m_null, G_X, G_Y+17*8, {"movement_maxviewpitch"}, 0, M_ChangeMaxViewPitch},
  {"Mouse Strafe Divisor",        S_NUM,   m_null, G_X, G_Y+18*8, {"movement_mousestrafedivisor"}},

  {"<- PREV",S_SKIP|S_PREV, m_null,KB_PREV, KB_Y+20*8, {gen_settings2}},
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {gen_settings4}},
  {0,S_SKIP|S_END,m_null}
};

static const char *renderfilters[] = {"none", "point", "linear", "rounded"};
static const char *edgetypes[] = {"jagged", "sloped"};

setup_menu_t gen_settings4[] = { // General Settings screen3
  {"Display Options",          S_SKIP|S_TITLE, m_null, G_X, G_Y+ 1*8},
  {"Filter for walls",           S_CHOICE, m_null, G_X, G_Y+ 2*8, {"filter_wall"}, 0, NULL, renderfilters},
  {"Filter for floors/ceilings", S_CHOICE, m_null, G_X, G_Y+ 3*8, {"filter_floor"}, 0, NULL, renderfilters},
  {"Filter for sprites",         S_CHOICE, m_null, G_X, G_Y+ 4*8, {"filter_sprite"}, 0, NULL, renderfilters},
  {"Filter for patches",         S_CHOICE, m_null, G_X, G_Y+ 5*8, {"filter_patch"}, 0, NULL, renderfilters},
  {"Filter for lighting",        S_CHOICE, m_null, G_X, G_Y+ 6*8, {"filter_z"}, 0, NULL, renderfilters},

  {"Drawing of sprite edges",    S_CHOICE, m_null, G_X, G_Y+ 8*8, {"sprite_edges"}, 0, NULL, edgetypes},
  {"Drawing of patch edges",     S_CHOICE, m_null, G_X, G_Y+ 9*8, {"patch_edges"}, 0, NULL, edgetypes},
  {"Flashing HOM indicator",     S_YESNO,  m_null, G_X, G_Y+10*8, {"flashing_hom"}},

  // prboom-plus
  {"Wipe Screen Effect",         S_YESNO,  m_null, G_X, G_Y+12*8, {"render_wipescreen"}},
  {"Change Palette On Pain",     S_YESNO,  m_null, G_X, G_Y+14*8, {"palette_ondamage"}, 0, M_ChangeApplyPalette},
  {"Change Palette On Bonus",    S_YESNO,  m_null, G_X, G_Y+15*8, {"palette_onbonus"}, 0, M_ChangeApplyPalette},
  {"Change Palette On Powers",   S_YESNO,  m_null, G_X, G_Y+16*8, {"palette_onpowers"}, 0, M_ChangeApplyPalette},

  {"<- PREV",S_SKIP|S_PREV, m_null, KB_PREV, KB_Y+20*8, {gen_settings3}},
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {gen_settings5}},
  // Final entry

  {0,S_SKIP|S_END,m_null}
};

setup_menu_t gen_settings5[] = { // General Settings screen3
  {"Software Options",               S_SKIP|S_TITLE, m_null, G_X, G_Y+1*8},
  {"Screen Multiple Factor (1-None)", S_NUM,m_null,G_X,G_Y+2*8, {"render_screen_multiply"}, 0, M_ChangeScreenMultipleFactor},
  {"Integer Screen Scaling",    S_YESNO,  m_null, G_X, G_Y+3*8, {"integer_scaling"}, 0, M_ChangeScreenMultipleFactor},
#ifdef GL_DOOM
  {"OpenGL Options",             S_SKIP|S_TITLE,m_null,G_X,G_Y+5*8},
  {"Multisampling (0-None)",    S_NUM|S_PRGWARN|S_CANT_GL_ARB_MULTISAMPLEFACTOR,m_null,G_X,G_Y+6*8, {"render_multisampling"}, 0, M_ChangeMultiSample},
  {"Field Of View",             S_NUM,    m_null, G_X, G_Y+ 7*8, {"render_fov"}, 0, M_ChangeFOV},
  {"Sector Light Mode",         S_CHOICE, m_null, G_X, G_Y+ 8*8, {"gl_lightmode"}, 0, M_ChangeLightMode, gl_lightmodes},
  {"Allow Fog",                 S_YESNO,  m_null, G_X, G_Y+ 9*8, {"gl_fog"}, 0, M_ChangeAllowFog},
  {"Simple Shadows",            S_YESNO,  m_null, G_X, G_Y+10*8, {"gl_shadows"}},

  {"Paper Items",               S_YESNO,  m_null, G_X, G_Y+12*8, {"render_paperitems"}},
  {"Smooth sprite edges",       S_YESNO,  m_null, G_X, G_Y+13*8, {"gl_sprite_blend"}},
  {"Adjust Sprite Clipping",    S_CHOICE, m_null, G_X, G_Y+14*8, {"gl_spriteclip"}, 0, M_ChangeSpriteClip, gl_spriteclipmodes},
  {"Item out of Floor offset",  S_NUM,    m_null, G_X, G_Y+15*8, {"gl_sprite_offset"}, 0, M_ChangeSpriteClip},
  {"Health Bar Above Monsters", S_YESNO,  m_null, G_X, G_Y+16*8, {"health_bar"}},
#endif

  {"<- PREV",S_SKIP|S_PREV, m_null,KB_PREV, KB_Y+20*8, {gen_settings4}},
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {gen_settings6}},
  {0,S_SKIP|S_END,m_null}
};

setup_menu_t gen_settings6[] =
{
  {"EMULATION"                         ,S_SKIP|S_TITLE,m_null,G_X2,G_Y+ 1*8},
  {"WARN ON SPECHITS OVERFLOW"         ,S_YESNO     ,m_null,G_X2,G_Y+ 2*8, {"overrun_spechit_warn"}},
  {"TRY TO EMULATE IT"                 ,S_YESNO     ,m_null,G_X2,G_Y+ 3*8, {"overrun_spechit_emulate"}},
  {"WARN ON REJECT OVERFLOW"           ,S_YESNO     ,m_null,G_X2,G_Y+ 4*8, {"overrun_reject_warn"}},
  {"TRY TO EMULATE IT"                 ,S_YESNO     ,m_null,G_X2,G_Y+ 5*8, {"overrun_reject_emulate"}},
  {"WARN ON INTERCEPTS OVERFLOW"       ,S_YESNO     ,m_null,G_X2,G_Y+ 6*8, {"overrun_intercept_warn"}},
  {"TRY TO EMULATE IT"                 ,S_YESNO     ,m_null,G_X2,G_Y+ 7*8, {"overrun_intercept_emulate"}},
  {"WARN ON PLAYERINGAME OVERFLOW"     ,S_YESNO     ,m_null,G_X2,G_Y+ 8*8, {"overrun_playeringame_warn"}},
  {"TRY TO EMULATE IT"                 ,S_YESNO     ,m_null,G_X2,G_Y+ 9*8, {"overrun_playeringame_emulate"}},
  {"WARN ON DONUT OVERFLOW"            ,S_YESNO     ,m_null,G_X2,G_Y+10*8, {"overrun_donut_warn"}},
  {"TRY TO EMULATE IT"                 ,S_YESNO     ,m_null,G_X2,G_Y+11*8, {"overrun_donut_emulate"}},
  {"WARN ON MISSEDBACKSIDE OVERFLOW"   ,S_YESNO     ,m_null,G_X2,G_Y+12*8, {"overrun_missedbackside_warn"}},
  {"TRY TO EMULATE IT"                 ,S_YESNO     ,m_null,G_X2,G_Y+13*8, {"overrun_missedbackside_emulate"}},

  {"<- PREV",S_SKIP|S_PREV,m_null,KB_PREV,KB_Y+20*8, {gen_settings5}},
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {gen_settings7}},
  {0,S_SKIP|S_END,m_null}
};

static const char *jumpheights[] = {
  "No", "Low", "High", NULL};

setup_menu_t gen_settings7[] =
{
  {"COMPATIBILITY WITH COMMON MAPPING ERRORS" ,S_SKIP|S_TITLE,m_null,G_X2,G_Y+1*8},
  {"LINEDEFS W/O TAGS APPLY LOCALLY"   ,S_YESNO     ,m_null,G_X2,G_Y+2*8, {"comperr_zerotag"}},
  {"USE PASSES THRU ALL SPECIAL LINES" ,S_YESNO     ,m_null,G_X2,G_Y+3*8, {"comperr_passuse"}},
  {"WALK UNDER SOLID HANGING BODIES"   ,S_YESNO     ,m_null,G_X2,G_Y+4*8, {"comperr_hangsolid"}},
  {"FIX CLIPPING PROBLEMS IN LARGE LEVELS" ,S_YESNO ,m_null,G_X2,G_Y+5*8, {"comperr_blockmap"}},
  {"ALLOW JUMP"                        ,S_CHOICE    ,m_null,G_X2,G_Y+6*8, {"comperr_allowjump"}, 0, NULL, jumpheights},
  {"ALLOW VERTICAL AIMING"             ,S_YESNO     ,m_null,G_X2,G_Y+7*8, {"comperr_freeaim"}},

  {"<- PREV",S_SKIP|S_PREV,m_null,KB_PREV,KB_Y+20*8, {gen_settings6}},
#ifdef GL_DOOM
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {gen_settings8}},
#else
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {dsda_gen_settings}},
#endif
  {0,S_SKIP|S_END,m_null}
};

static const char *gltexfilters[] = {
  "None", "Linear", "Nearest Mipmap", "Linear Mipmap", "Bilinear", "Trilinear", NULL};

static const char *gltexfilters_anisotropics[] =
  {"Off", "2x", "4x", "8x", "16x", NULL};

setup_menu_t gen_settings8[] = { // General Settings screen4
#ifdef GL_DOOM
  {"Texture Options",  S_SKIP|S_TITLE,m_null,G_X,G_Y+ 1*8},
  {"Texture Filter Mode",        S_CHOICE, m_null, G_X, G_Y+2 *8, {"gl_texture_filter"}, 0, M_ChangeTextureParams, gltexfilters},
  {"Sprite Filter Mode",        S_CHOICE, m_null, G_X, G_Y+3 *8, {"gl_sprite_filter"}, 0, M_ChangeTextureParams, gltexfilters},
  {"Patch Filter Mode",          S_CHOICE, m_null, G_X, G_Y+4 *8, {"gl_patch_filter"}, 0, M_ChangeTextureParams, gltexfilters},
  {"Anisotropic filter",         S_CHOICE, m_null, G_X, G_Y+5 *8, {"gl_texture_filter_anisotropic"}, 0, M_ChangeTextureParams, gltexfilters_anisotropics},
  {"Texture format",             S_CHOICE, m_null, G_X, G_Y+6 *8, {"gl_tex_format_string"}, 0, M_ChangeTextureParams, gltexformats},

  {"Enable Colormaps",           S_YESNO, m_null, G_X,G_Y+ 8*8, {"gl_boom_colormaps"}, 0, M_ChangeAllowBoomColormaps},
  {"Enable Internal Hi-Res",     S_YESNO, m_null, G_X,G_Y+ 9*8, {"gl_texture_internal_hires"}, 0, M_ChangeTextureUseHires},
  {"Enable External Hi-Res",     S_YESNO, m_null, G_X,G_Y+10*8, {"gl_texture_external_hires"}, 0, M_ChangeTextureUseHires},
  {"Override PWAD's graphics with Hi-Res" ,S_YESNO|S_PRGWARN,m_null,G_X,G_Y+11*8, {"gl_hires_override_pwads"}, 0, M_ChangeTextureUseHires},

  {"Enable High Quality Resize", S_YESNO,  m_null, G_X, G_Y+13*8, {"gl_texture_hqresize"}, 0, M_ChangeTextureHQResize},
  {"Resize textures",            S_CHOICE, m_null, G_X, G_Y+14*8, {"gl_texture_hqresize_textures"}, 0, M_ChangeTextureHQResize, gl_hqresizemodes},
  {"Resize sprites",             S_CHOICE, m_null, G_X, G_Y+15*8, {"gl_texture_hqresize_sprites"}, 0, M_ChangeTextureHQResize, gl_hqresizemodes},
  {"Resize patches",             S_CHOICE, m_null, G_X, G_Y+16*8, {"gl_texture_hqresize_patches"}, 0, M_ChangeTextureHQResize, gl_hqresizemodes},

  {"Allow Detail Textures",      S_YESNO,  m_null, G_X, G_Y+18*8, {"gl_allow_detail_textures"}, 0, M_ChangeUseDetail},
  {"Blend Animations",           S_YESNO,  m_null, G_X, G_Y+19*8, {"gl_blend_animations"}},
#endif //GL_DOOM

  {"<- PREV",S_SKIP|S_PREV,m_null,KB_PREV,KB_Y+20*8, {gen_settings7}},
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {dsda_gen_settings}},
  {0,S_SKIP|S_END,m_null}
};

setup_menu_t dsda_gen_settings[] = {
  { "DSDA-Doom Settings", S_SKIP | S_TITLE, m_null, G_X, G_Y + 1 * 8 },
  { "Strict Mode", S_YESNO, m_null, G_X, G_Y + 2 * 8, { "dsda_strict_mode" }, 0, dsda_ChangeStrictMode },
  { "Cycle Ghost Colors", S_YESNO, m_null, G_X, G_Y + 3 * 8, { "dsda_cycle_ghost_colors" } },
  { "Automatic Key Frame Interval (s)", S_NUM, m_null, G_X, G_Y + 4 * 8, { "dsda_auto_key_frame_interval" } },
  { "Automatic Key Frame Depth", S_NUM | S_PRGWARN, m_null, G_X, G_Y + 5 * 8, { "dsda_auto_key_frame_depth" } },
  { "Use Extended Hud", S_YESNO, m_null, G_X, G_Y + 6 * 8, { "dsda_exhud" } },
  { "Wipe At Full Speed", S_YESNO, m_null, G_X, G_Y + 7 * 8, { "dsda_wipe_at_full_speed" } },
  { "Show Demo Attempts", S_YESNO, m_null, G_X, G_Y + 8 * 8, { "dsda_show_demo_attempts" } },
  { "Fine Sensitivity", S_NUM, m_null, G_X, G_Y + 9 * 8, { "dsda_fine_sensitivity" } },
  { "Hide Status Bar Horns", S_YESNO, m_null, G_X, G_Y + 10 * 8, { "dsda_hide_horns" } },
  { "Organize My Save Files", S_YESNO, m_null, G_X, G_Y + 11 * 8, { "dsda_organized_saves" } },
  { "Show Command Display (TAS)", S_YESNO, m_null, G_X, G_Y + 12 * 8, { "dsda_command_display" } },
  { "Command History", S_NUM, m_null, G_X, G_Y + 13 * 8, { "dsda_command_history_size" } },
  { "Hide Empty Commands", S_YESNO, m_null, G_X, G_Y + 14 * 8, { "dsda_hide_empty_commands" } },
  { "Show Coordinate Display (TAS)", S_YESNO, m_null, G_X, G_Y + 15 * 8, { "dsda_coordinate_display" } },
  { "Skip Quit Prompt", S_YESNO, m_null, G_X, G_Y + 16 * 8, { "dsda_skip_quit_prompt" } },
  { "Show Split Data", S_YESNO, m_null, G_X, G_Y + 17 * 8, { "dsda_show_split_data" } },
  { "Text File Author", S_NAME, m_null, G_X, G_Y + 18 * 8, { "dsda_player_name" } },

#ifdef GL_DOOM
  { "<- PREV", S_SKIP | S_PREV, m_null, KB_PREV, KB_Y + 20 * 8, { gen_settings8 } },
#else
  { "<- PREV", S_SKIP | S_PREV, m_null, KB_PREV, KB_Y + 20 * 8, { gen_settings7 } },
#endif
  { 0, S_SKIP | S_END, m_null }
};

void M_Trans(void) // To reset translucency after setting it in menu
{
  general_translucency = default_translucency; //e6y: Fix for "translucency won't change until you restart the engine"

  if (general_translucency)
    R_InitTranMap(0);
}

// To (un)set fullscreen video after menu changes
void M_ChangeFullScreen(void)
{
  V_ToggleFullscreen();
}

void M_ChangeVideoMode(void)
{
  V_ChangeScreenResolution();
}

void M_ChangeUseGLSurface(void)
{
  V_ChangeScreenResolution();
}

void M_ChangeDemoSmoothTurns(void)
{
  R_SmoothPlaying_Reset(NULL);
}

void M_ChangeTextureParams(void)
{
#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL)
  {
    gld_InitTextureParams();
    gld_FlushTextures();
  }
#endif
}

// Setting up for the General screen. Turn on flags, set pointers,
// locate the first item on the screen where the cursor is allowed to
// land.

void M_General(int choice)
{
  M_SetupNextMenu(&GeneralDef);

  setup_active = true;
  setup_screen = ss_gen;
  set_general_active = true;
  setup_select = false;
  default_verify = false;
  setup_gather = false;
  mult_screens_index = 0;
  current_setup_menu = gen_settings[0];
  set_menu_itemon = M_GetSetupMenuItemOn();
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}

// The drawing part of the General Setup initialization. Draw the
// background, title, instruction line, and items.

void M_DrawGeneral(void)
{
  menuactive = mnact_full;

  M_DrawBackground(g_menu_flat, 0); // Draw background

  // proff/nicolas 09/20/98 -- changed for hi-res
  M_DrawTitle(114, 2, "M_GENERL", CR_DEFAULT, "GENERAL", g_cr_gold);
  M_DrawInstructions();
  M_DrawScreenItems(current_setup_menu);

  // If the Reset Button has been selected, an "Are you sure?" message
  // is overlayed across everything else.

  if (default_verify)
    M_DrawDefVerify();
}

/////////////////////////////
//
// The Messages table.

#define M_X 230
#define M_Y  39

// killough 11/98: enumerated

enum {
  mess_color_play,
  mess_timer,
  mess_color_chat,
  mess_chat_timer,
  mess_color_review,
  mess_timed,
  mess_hud_timer,
  mess_lines,
  mess_scrollup,
  mess_background,
};

setup_menu_t mess_settings1[];

setup_menu_t* mess_settings[] =
{
  mess_settings1,
  NULL
};

setup_menu_t mess_settings1[] =  // Messages screen
{
  {"Message Color During Play", S_CRITEM, m_null, M_X,
   M_Y + mess_color_play*8, {"hudcolor_mesg"}},

  {"Chat Message Color", S_CRITEM, m_null, M_X,
   M_Y + mess_color_chat*8, {"hudcolor_chat"}},

  {"Message Review Color", S_CRITEM, m_null, M_X,
   M_Y + mess_color_review*8, {"hudcolor_list"}},

  {"Number of Review Message Lines", S_NUM, m_null,  M_X,
   M_Y + mess_lines*8, {"hud_msg_lines"}},

  {"Message Background",  S_YESNO,  m_null,  M_X,
   M_Y + mess_background*8, {"hud_list_bgon"}},

  // Button for resetting to defaults
  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},

  // Final entry

  {0,S_SKIP|S_END,m_null}
};


// Setting up for the Messages screen. Turn on flags, set pointers,
// locate the first item on the screen where the cursor is allowed to
// land.

void M_Messages(int choice)
{
  M_SetupNextMenu(&MessageDef);

  setup_active = true;
  setup_screen = ss_mess;
  set_mess_active = true;
  setup_select = false;
  default_verify = false;
  setup_gather = false;
  mult_screens_index = 0;
  current_setup_menu = mess_settings[0];
  set_menu_itemon = M_GetSetupMenuItemOn();
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}


// The drawing part of the Messages Setup initialization. Draw the
// background, title, instruction line, and items.

void M_DrawMessages(void)
{
  menuactive = mnact_full;

  M_DrawBackground(g_menu_flat, 0); // Draw background

  // CPhipps - patch drawing updated
  M_DrawTitle(103, 2, "M_MESS", CR_DEFAULT, "MESSAGES", g_cr_gold);
  M_DrawInstructions();
  M_DrawScreenItems(current_setup_menu);
  if (default_verify)
    M_DrawDefVerify();
}


/////////////////////////////
//
// The Chat Strings table.

#define CS_X 20
#define CS_Y (31+8)

setup_menu_t chat_settings1[];

setup_menu_t* chat_settings[] =
{
  chat_settings1,
  NULL
};

setup_menu_t chat_settings1[] =  // Chat Strings screen
{
  {"1",S_CHAT,m_null,CS_X,CS_Y+ 1*8, {"chatmacro1"}},
  {"2",S_CHAT,m_null,CS_X,CS_Y+ 2*8, {"chatmacro2"}},
  {"3",S_CHAT,m_null,CS_X,CS_Y+ 3*8, {"chatmacro3"}},
  {"4",S_CHAT,m_null,CS_X,CS_Y+ 4*8, {"chatmacro4"}},
  {"5",S_CHAT,m_null,CS_X,CS_Y+ 5*8, {"chatmacro5"}},
  {"6",S_CHAT,m_null,CS_X,CS_Y+ 6*8, {"chatmacro6"}},
  {"7",S_CHAT,m_null,CS_X,CS_Y+ 7*8, {"chatmacro7"}},
  {"8",S_CHAT,m_null,CS_X,CS_Y+ 8*8, {"chatmacro8"}},
  {"9",S_CHAT,m_null,CS_X,CS_Y+ 9*8, {"chatmacro9"}},
  {"0",S_CHAT,m_null,CS_X,CS_Y+10*8, {"chatmacro0"}},

  // Button for resetting to defaults
  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},

  // Final entry
  {0,S_SKIP|S_END,m_null}

};

// Setting up for the Chat Strings screen. Turn on flags, set pointers,
// locate the first item on the screen where the cursor is allowed to
// land.

void M_ChatStrings(int choice)
{
  M_SetupNextMenu(&ChatStrDef);
  setup_active = true;
  setup_screen = ss_chat;
  set_chat_active = true;
  setup_select = false;
  default_verify = false;
  setup_gather = false;
  mult_screens_index = 0;
  current_setup_menu = chat_settings[0];
  set_menu_itemon = M_GetSetupMenuItemOn();
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}

// The drawing part of the Chat Strings Setup initialization. Draw the
// background, title, instruction line, and items.

void M_DrawChatStrings(void)
{
  menuactive = mnact_full;

  M_DrawBackground(g_menu_flat, 0); // Draw background

  // CPhipps - patch drawing updated
  M_DrawTitle(83, 2, "M_CHAT", CR_DEFAULT, "CHAT STRINGS", g_cr_gold);
  M_DrawInstructions();
  M_DrawScreenItems(current_setup_menu);

  // If the Reset Button has been selected, an "Are you sure?" message
  // is overlayed across everything else.

  if (default_verify)
    M_DrawDefVerify();
}

/////////////////////////////
//
// General routines used by the Setup screens.
//

static dboolean shiftdown = false; // phares 4/10/98: SHIFT key down or not

// phares 4/17/98:
// M_SelectDone() gets called when you have finished entering your
// Setup Menu item change.

static void M_SelectDone(setup_menu_t* ptr)
{
  ptr->m_flags &= ~S_SELECT;
  ptr->m_flags |= S_HILITE;
  S_StartSound(NULL,g_sfx_itemup);
  setup_select = false;
  colorbox_active = false;
  if (print_warning_about_changes)     // killough 8/15/98
    print_warning_about_changes--;
}

// phares 4/21/98:
// Array of setup screens used by M_ResetDefaults()

static setup_menu_t **setup_screens[] =
{
  keys_settings,
  weap_settings,
  stat_settings,
  auto_settings,
  mess_settings,
  chat_settings,
  gen_settings,      // killough 10/98
};

// phares 4/19/98:
// M_ResetDefaults() resets all values for a setup screen to default values
//
// killough 10/98: rewritten to fix bugs and warn about pending changes

static void M_ResetDefaults(void)
{
  int i; //e6y

  default_t *dp;
  int warn = 0;

  // Look through the defaults table and reset every variable that
  // belongs to the group we're interested in.
  //
  // killough: However, only reset variables whose field in the
  // current setup screen is the same as in the defaults table.
  // i.e. only reset variables really in the current setup screen.

  // e6y
  // Fixed crash while trying to read data past array end
  // All previous versions of prboom worked only by a lucky accident
  // old code: for (dp = defaults; dp->name; dp++)
  for (i = 0; i < numdefaults ; i++)
  {
    dp = &defaults[i];

    if (dp->setupscreen == setup_screen)
    {
      setup_menu_t **l, *p;
      for (l = setup_screens[setup_screen-1]; *l; l++)
        for (p = *l; !(p->m_flags & S_END); p++)
          if (p->m_flags & (S_INPUT | S_KEEP))
          {
            if (dp->identifier == p->input)
            {
              dsda_InputSet(dp->identifier, dp->input);

              goto end;
            }
          }
          else if ((p->m_flags & S_HASDEFPTR) && p->var.def == dp)
          {
            lprintf(LO_INFO, "Reset: %d %d\n", p->m_flags, i);
            if (IS_STRING(*dp))
            {
              union { const char **c; char **s; } u; // type punning via unions

              u.c = dp->location.ppsz;
              free(*(u.s));
              *(u.c) = strdup(dp->defaultvalue.psz);
            }
            else
              *dp->location.pi = dp->defaultvalue.i;

            if (p->action)
              p->action();

            goto end;
          }
      end:;
    }
  }

  if (warn)
    warn_about_changes(warn);
}

//
// M_InitDefaults()
//
// killough 11/98:
//
// This function converts all setup menu entries consisting of cfg
// variable names, into pointers to the corresponding default[]
// array entry. var.name becomes converted to var.def.
//

static void M_InitDefaults(void)
{
  setup_menu_t *const *p, *t;
  default_t *dp;
  int i;
  for (i = 0; i < ss_max-1; i++)
  {
    for (p = setup_screens[i]; *p; p++)
    {
      for (t = *p; !(t->m_flags & S_END); t++)
      {
        if (t->m_flags & S_HASDEFPTR)
        {
          if (!(dp = M_LookupDefault(t->var.name)))
            I_Error("M_InitDefaults: Couldn't find config variable %s", t->var.name);
          else
            (t->var.def = dp)->setup_menu = t;
        }
      }
    }
  }
}

//
// End of Setup Screens.
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//
// Start of Extended HELP screens               // phares 3/30/98
//
// The wad designer can define a set of extended HELP screens for their own
// information display. These screens should be 320x200 graphic lumps
// defined in a separate wad. They should be named "HELP01" through "HELP99".
// "HELP01" is shown after the regular BOOM Dynamic HELP screen, and ENTER
// and BACKSPACE keys move the player through the HELP set.
//
// Rather than define a set of menu definitions for each of the possible
// HELP screens, one definition is used, and is altered on the fly
// depending on what HELPnn lumps the game finds.

// phares 3/30/98:
// Extended Help Screen variables

int extended_help_count;   // number of user-defined help screens found
int extended_help_index;   // index of current extended help screen

menuitem_t ExtHelpMenu[] =
{
  {1,"",M_ExtHelpNextScreen,0}
};

menu_t ExtHelpDef =
{
  1,             // # of menu items
  &ReadDef1,     // previous menu
  ExtHelpMenu,   // menuitem_t ->
  M_DrawExtHelp, // drawing routine ->
  330,181,       // x,y
  0              // lastOn
};

// M_ExtHelpNextScreen establishes the number of the next HELP screen in
// the series.

void M_ExtHelpNextScreen(int choice)
{
  choice = 0;
  if (++extended_help_index > extended_help_count)
    {

      // when finished with extended help screens, return to Main Menu

      extended_help_index = 1;
      M_SetupNextMenu(&MainDef);
    }
}

// phares 3/30/98:
// Routine to look for HELPnn screens and create a menu
// definition structure that defines extended help screens.

void M_InitExtendedHelp(void)

{
  int index,i;
  char namebfr[] = { "HELPnn"} ;

  extended_help_count = 0;
  for (index = 1 ; index < 100 ; index++) {
    namebfr[4] = index/10 + 0x30;
    namebfr[5] = index%10 + 0x30;
    i = W_CheckNumForName(namebfr);
    if (i == -1) {
      if (extended_help_count) {
        /* The Extended Help menu is accessed using the
         * Help hotkey (F1) or the "Read This!" menu item.
         *
         * If Extended Help screens are present, use the
         * Extended Help routine when either the F1 Help Menu
         * or the "Read This!" menu items are accessed.
         *
         * See also: https://www.doomworld.com/forum/topic/111465-boom-extended-help-screens-an-undocumented-feature/
         */
          HelpMenu[0].routine = M_ExtHelp;
        if (gamemode == commercial) {
          ExtHelpDef.prevMenu  = &ReadDef1; /* previous menu */
          ReadMenu1[0].routine = M_ExtHelp;
        } else {
          ExtHelpDef.prevMenu  = &ReadDef2; /* previous menu */
          ReadMenu2[0].routine = M_ExtHelp;
        }
      }
      return;
    }
    extended_help_count++;
  }

}

// Initialization for the extended HELP screens.

void M_ExtHelp(int choice)
{
  choice = 0;
  extended_help_index = 1; // Start with first extended help screen
  M_SetupNextMenu(&ExtHelpDef);
}

// Initialize the drawing part of the extended HELP screens.

void M_DrawExtHelp(void)
{
  char namebfr[10] = { "HELPnn" }; // CPhipps - make it local & writable

  inhelpscreens = true;              // killough 5/1/98
  namebfr[4] = extended_help_index/10 + 0x30;
  namebfr[5] = extended_help_index%10 + 0x30;
  // CPhipps - patch drawing updated
  V_DrawNamePatch(0, 0, 0, namebfr, CR_DEFAULT, VPT_STRETCH);
}

//
// End of Extended HELP screens               // phares 3/30/98
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//
// Dynamic HELP screen                     // phares 3/2/98
//
// Rather than providing the static HELP screens from DOOM and its versions,
// BOOM provides the player with a dynamic HELP screen that displays the
// current settings of major key bindings.
//
// The Dynamic HELP screen is defined in a manner similar to that used for
// the Setup Screens above.
//
// M_GetKeyString finds the correct string to represent the key binding
// for the current item being drawn.

int M_GetKeyString(int c,int offset)
{
  const char* s;

  if (c >= 33 && c <= 126) {

    // The '=', ',', and '.' keys originally meant the shifted
    // versions of those keys, but w/o having to shift them in
    // the game. Any actions that are mapped to these keys will
    // still mean their shifted versions. Could be changed later
    // if someone can come up with a better way to deal with them.

    if (c == '=')      // probably means the '+' key?
      c = '+';
    else if (c == ',') // probably means the '<' key?
      c = '<';
    else if (c == '.') // probably means the '>' key?
      c = '>';
    menu_buffer[offset++] = c; // Just insert the ascii key
    menu_buffer[offset] = 0;

  } else {

    // Retrieve 4-letter (max) string representing the key

    // cph - Keypad keys, general code reorganisation to
    //  make this smaller and neater.
    if ((0x100 <= c) && (c < 0x200)) {
      if (c == KEYD_KEYPADENTER)
  s = "PADE";
      else {
  strcpy(&menu_buffer[offset], "PAD");
  offset+=4;
  menu_buffer[offset-1] = c & 0xff;
  menu_buffer[offset] = 0;
      }
    } else if ((KEYD_F1 <= c) && (c < KEYD_F10)) {
      menu_buffer[offset++] = 'F';
      menu_buffer[offset++] = '1' + c - KEYD_F1;
      menu_buffer[offset]   = 0;
    } else {
      switch(c) {
      case KEYD_TAB:      s = "TAB";  break;
      case KEYD_ENTER:      s = "ENTR"; break;
      case KEYD_ESCAPE:     s = "ESC";  break;
      case KEYD_SPACEBAR:   s = "SPAC"; break;
      case KEYD_BACKSPACE:  s = "BACK"; break;
      case KEYD_RCTRL:      s = "CTRL"; break;
      case KEYD_LEFTARROW:  s = "LARR"; break;
      case KEYD_UPARROW:    s = "UARR"; break;
      case KEYD_RIGHTARROW: s = "RARR"; break;
      case KEYD_DOWNARROW:  s = "DARR"; break;
      case KEYD_RSHIFT:     s = "SHFT"; break;
      case KEYD_RALT:       s = "ALT";  break;
      case KEYD_CAPSLOCK:   s = "CAPS"; break;
      case KEYD_SCROLLLOCK: s = "SCRL"; break;
      case KEYD_HOME:       s = "HOME"; break;
      case KEYD_PAGEUP:     s = "PGUP"; break;
      case KEYD_END:        s = "END";  break;
      case KEYD_PAGEDOWN:   s = "PGDN"; break;
      case KEYD_INSERT:     s = "INST"; break;
      case KEYD_DEL:        s = "DEL"; break;
      case KEYD_F10:        s = "F10";  break;
      case KEYD_F11:        s = "F11";  break;
      case KEYD_F12:        s = "F12";  break;
      case KEYD_PAUSE:      s = "PAUS"; break;
      case KEYD_MWHEELDOWN: s = "MWDN"; break;
      case KEYD_MWHEELUP:   s = "MWUP"; break;
      case KEYD_PRINTSC:    s = "PRSC"; break;
      case 0:               s = "NONE"; break;
      default:              s = "JUNK"; break;
      }

      if (s) { // cph - Slight code change
  strcpy(&menu_buffer[offset],s); // string to display
  offset += strlen(s);
      }
    }
  }
  return offset;
}

//
// The Dynamic HELP screen table.

#define KT_X1 283
#define KT_X2 172
#define KT_X3  87

#define KT_Y1   2
#define KT_Y2 118
#define KT_Y3 102

setup_menu_t helpstrings[] =  // HELP screen strings
{
  {"SCREEN"      ,S_SKIP|S_TITLE,m_null,KT_X1,KT_Y1},
  {"HELP"        ,S_SKIP|S_INPUT,m_null,KT_X1,KT_Y1+ 1*8,{0},dsda_input_help},
  {"MENU"        ,S_SKIP|S_INPUT,m_null,KT_X1,KT_Y1+ 2*8,{0},dsda_input_escape},
  {"SETUP"       ,S_SKIP|S_INPUT,m_null,KT_X1,KT_Y1+ 3*8,{0},dsda_input_setup},
  {"PAUSE"       ,S_SKIP|S_INPUT,m_null,KT_X1,KT_Y1+ 4*8,{0},dsda_input_pause},
  {"AUTOMAP"     ,S_SKIP|S_INPUT,m_null,KT_X1,KT_Y1+ 5*8,{0},dsda_input_map},
  {"SOUND VOLUME",S_SKIP|S_INPUT,m_null,KT_X1,KT_Y1+ 6*8,{0},dsda_input_soundvolume},
  {"HUD"         ,S_SKIP|S_INPUT,m_null,KT_X1,KT_Y1+ 7*8,{0},dsda_input_hud},
  {"MESSAGES"    ,S_SKIP|S_INPUT,m_null,KT_X1,KT_Y1+ 8*8,{0},dsda_input_messages},
  {"GAMMA FIX"   ,S_SKIP|S_INPUT,m_null,KT_X1,KT_Y1+ 9*8,{0},dsda_input_gamma},
  {"SPY"         ,S_SKIP|S_INPUT,m_null,KT_X1,KT_Y1+10*8,{0},dsda_input_spy},
  {"LARGER VIEW" ,S_SKIP|S_INPUT,m_null,KT_X1,KT_Y1+11*8,{0},dsda_input_zoomin},
  {"SMALLER VIEW",S_SKIP|S_INPUT,m_null,KT_X1,KT_Y1+12*8,{0},dsda_input_zoomout},
  {"SCREENSHOT"  ,S_SKIP|S_INPUT,m_null,KT_X1,KT_Y1+13*8,{0},dsda_input_screenshot},

  {"AUTOMAP"     ,S_SKIP|S_TITLE,m_null,KT_X1,KT_Y2},
  {"FOLLOW MODE" ,S_SKIP|S_INPUT,m_null,KT_X1,KT_Y2+ 1*8,{0},dsda_input_map_follow},
  {"ZOOM IN"     ,S_SKIP|S_INPUT,m_null,KT_X1,KT_Y2+ 2*8,{0},dsda_input_map_zoomin},
  {"ZOOM OUT"    ,S_SKIP|S_INPUT,m_null,KT_X1,KT_Y2+ 3*8,{0},dsda_input_map_zoomout},
  {"MARK PLACE"  ,S_SKIP|S_INPUT,m_null,KT_X1,KT_Y2+ 4*8,{0},dsda_input_map_mark},
  {"CLEAR MARKS" ,S_SKIP|S_INPUT,m_null,KT_X1,KT_Y2+ 5*8,{0},dsda_input_map_clear},
  {"FULL/ZOOM"   ,S_SKIP|S_INPUT,m_null,KT_X1,KT_Y2+ 6*8,{0},dsda_input_map_gobig},
  {"GRID"        ,S_SKIP|S_INPUT,m_null,KT_X1,KT_Y2+ 7*8,{0},dsda_input_map_grid},
  {"ROTATE"      ,S_SKIP|S_INPUT,m_null,KT_X1,KT_Y2+ 8*8,{0},dsda_input_map_rotate},
  {"OVERLAY"     ,S_SKIP|S_INPUT,m_null,KT_X1,KT_Y2+ 9*8,{0},dsda_input_map_overlay},

  {"WEAPONS"     ,S_SKIP|S_TITLE,m_null,KT_X3,KT_Y1},
  {"FIST"        ,S_SKIP|S_INPUT,m_null,KT_X3,KT_Y1+ 1*8,{0},dsda_input_weapon1},
  {"PISTOL"      ,S_SKIP|S_INPUT,m_null,KT_X3,KT_Y1+ 2*8,{0},dsda_input_weapon2},
  {"SHOTGUN"     ,S_SKIP|S_INPUT,m_null,KT_X3,KT_Y1+ 3*8,{0},dsda_input_weapon3},
  {"CHAINGUN"    ,S_SKIP|S_INPUT,m_null,KT_X3,KT_Y1+ 4*8,{0},dsda_input_weapon4},
  {"ROCKET"      ,S_SKIP|S_INPUT,m_null,KT_X3,KT_Y1+ 5*8,{0},dsda_input_weapon5},
  {"PLASMA"      ,S_SKIP|S_INPUT,m_null,KT_X3,KT_Y1+ 6*8,{0},dsda_input_weapon6},
  {"BFG 9000"    ,S_SKIP|S_INPUT,m_null,KT_X3,KT_Y1+ 7*8,{0},dsda_input_weapon7},
  {"CHAINSAW"    ,S_SKIP|S_INPUT,m_null,KT_X3,KT_Y1+ 8*8,{0},dsda_input_weapon8},
  {"SSG"         ,S_SKIP|S_INPUT,m_null,KT_X3,KT_Y1+ 9*8,{0},dsda_input_weapon9},
  {"BEST"        ,S_SKIP|S_INPUT,m_null,KT_X3,KT_Y1+10*8,{0},dsda_input_toggleweapon},
  {"FIRE"        ,S_SKIP|S_INPUT,m_null,KT_X3,KT_Y1+11*8,{0},dsda_input_fire},

  {"MOVEMENT"    ,S_SKIP|S_TITLE,m_null,KT_X3,KT_Y3},
  {"FORWARD"     ,S_SKIP|S_INPUT,m_null,KT_X3,KT_Y3+ 1*8,{0},dsda_input_forward},
  {"BACKWARD"    ,S_SKIP|S_INPUT,m_null,KT_X3,KT_Y3+ 2*8,{0},dsda_input_backward},
  {"TURN LEFT"   ,S_SKIP|S_INPUT,m_null,KT_X3,KT_Y3+ 3*8,{0},dsda_input_turnleft},
  {"TURN RIGHT"  ,S_SKIP|S_INPUT,m_null,KT_X3,KT_Y3+ 4*8,{0},dsda_input_turnright},
  {"RUN"         ,S_SKIP|S_INPUT,m_null,KT_X3,KT_Y3+ 5*8,{0},dsda_input_speed},
  {"STRAFE LEFT" ,S_SKIP|S_INPUT,m_null,KT_X3,KT_Y3+ 6*8,{0},dsda_input_strafeleft},
  {"STRAFE RIGHT",S_SKIP|S_INPUT,m_null,KT_X3,KT_Y3+ 7*8,{0},dsda_input_straferight},
  {"STRAFE"      ,S_SKIP|S_INPUT,m_null,KT_X3,KT_Y3+ 8*8,{0},dsda_input_strafe},
  {"AUTORUN"     ,S_SKIP|S_INPUT,m_null,KT_X3,KT_Y3+ 9*8,{0},dsda_input_autorun},
  {"180 TURN"    ,S_SKIP|S_INPUT,m_null,KT_X3,KT_Y3+10*8,{0},dsda_input_reverse},
  {"USE"         ,S_SKIP|S_INPUT,m_null,KT_X3,KT_Y3+11*8,{0},dsda_input_use},

  {"GAME"        ,S_SKIP|S_TITLE,m_null,KT_X2,KT_Y1},
  {"SAVE"        ,S_SKIP|S_INPUT,m_null,KT_X2,KT_Y1+ 1*8,{0},dsda_input_savegame},
  {"LOAD"        ,S_SKIP|S_INPUT,m_null,KT_X2,KT_Y1+ 2*8,{0},dsda_input_loadgame},
  {"QUICKSAVE"   ,S_SKIP|S_INPUT,m_null,KT_X2,KT_Y1+ 3*8,{0},dsda_input_quicksave},
  {"END GAME"    ,S_SKIP|S_INPUT,m_null,KT_X2,KT_Y1+ 4*8,{0},dsda_input_endgame},
  {"QUICKLOAD"   ,S_SKIP|S_INPUT,m_null,KT_X2,KT_Y1+ 5*8,{0},dsda_input_quickload},
  {"QUIT"        ,S_SKIP|S_INPUT,m_null,KT_X2,KT_Y1+ 6*8,{0},dsda_input_quit},

  // Final entry

  {0,S_SKIP|S_END,m_null}
};

#define SPACEWIDTH 4

/* cph 2006/08/06
 * M_DrawString() is the old M_DrawMenuString, except that it is not tied to
 * menu_buffer - no reason to force all the callers to write into one array! */

static void M_DrawString(int cx, int cy, int color, const char* ch)
{
  int   w;
  int   c;

  while (*ch) {
    c = *ch++;         // get next char
    c = toupper(c) - HU_FONTSTART;
    if (c < 0 || c> HU_FONTSIZE)
      {
      cx += SPACEWIDTH;    // space
      continue;
      }
    w = g_menu_font[c].width;
    if (cx + w > 320)
      break;

    // V_DrawpatchTranslated() will draw the string in the
    // desired color, colrngs[color]

    // CPhipps - patch drawing updated
    V_DrawNumPatch(cx, cy, 0, g_menu_font[c].lumpnum, color, VPT_STRETCH | VPT_TRANS);
    // The screen is cramped, so trim one unit from each
    // character so they butt up against each other.
    cx += w + g_menu_font_spacing;
  }
}

// M_DrawMenuString() draws the string in menu_buffer[]

static void M_DrawMenuString(int cx, int cy, int color)
{
    M_DrawString(cx, cy, color, menu_buffer);
}

// M_GetPixelWidth() returns the number of pixels in the width of
// the string, NOT the number of chars in the string.

static int M_GetPixelWidth(const char* ch)
{
  int len = 0;
  int c;

  while (*ch) {
    c = *ch++;    // pick up next char
    c = toupper(c) - HU_FONTSTART;
    if (c < 0 || c > HU_FONTSIZE)
      {
      len += SPACEWIDTH;   // space
      continue;
      }
    len += g_menu_font[c].width;
    len += g_menu_font_spacing;
  }
  len -= g_menu_font_spacing; // replace what you took away on the last char only
  return len;
}

static void M_DrawStringCentered(int cx, int cy, int color, const char* ch)
{
    M_DrawString(cx - M_GetPixelWidth(ch)/2, cy, color, ch);
}

//
// M_DrawHelp
//
// This displays the help screen

void M_DrawHelp (void)
{
  const int helplump = W_CheckNumForName("HELP");

  menuactive = mnact_full;

  if (helplump >= 0 && lumpinfo[helplump].source != source_iwad)
  {
    V_FillBorder(-1, 0);
    V_DrawNumPatch(0, 0, 0, helplump, CR_DEFAULT, VPT_STRETCH);
  }
  else
  {
    M_DrawBackground(g_menu_flat, 0);
    M_DrawScreenItems(helpstrings);
  }
}

//
// End of Dynamic HELP screen                // phares 3/2/98
//
////////////////////////////////////////////////////////////////////////////

enum {
  prog,
  prog_stub,
  prog_stub1,
  prog_stub2,
  adcr
};

enum {
  cr_prog=0,
  cr_adcr=2,
};

#define CR_S 9
#define CR_X 20
#define CR_X2 50
#define CR_Y 32
#define CR_SH 9

setup_menu_t cred_settings[]={

  {"Programmers",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X, CR_Y + CR_S*prog + CR_SH*cr_prog},
  {"Florian 'Proff' Schulze",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(prog+1) + CR_SH*cr_prog},
  {"Colin Phipps",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(prog+2) + CR_SH*cr_prog},
  {"Neil Stevens",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(prog+3) + CR_SH*cr_prog},
  {"Andrey Budko",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(prog+4) + CR_SH*cr_prog},

  {"Additional Credit To",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X, CR_Y + CR_S*adcr + CR_SH*cr_adcr},
  {"id Software for DOOM",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+1)+CR_SH*cr_adcr},
  {"TeamTNT for BOOM",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+2)+CR_SH*cr_adcr},
  {"Lee Killough for MBF",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+3)+CR_SH*cr_adcr},
  {"The DOSDoom-Team for DOSDOOM",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+4)+CR_SH*cr_adcr},
  {"Randy Heit for ZDOOM",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+5)+CR_SH*cr_adcr},
  {"Michael 'Kodak' Ryssen for DOOMGL",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+6)+CR_SH*cr_adcr},
  {"Jess Haas for lSDLDoom",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+7) + CR_SH*cr_adcr},
  {"all others who helped (see AUTHORS file)",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+8)+CR_SH*cr_adcr},

  {0,S_SKIP|S_END,m_null}
};

void M_DrawCredits(void)     // killough 10/98: credit screen
{
  const int creditlump = W_CheckNumForName("CREDIT");

  if (heretic)
  {
    V_DrawRawScreen("CREDIT");
    return;
  }

  inhelpscreens = true;
  if (creditlump >= 0 && lumpinfo[creditlump].source != source_iwad)
  {
    V_FillBorder(-1, 0);
    V_DrawNumPatch(0, 0, 0, creditlump, CR_DEFAULT, VPT_STRETCH);
  }
  else
  {
    // Use V_DrawBackground here deliberately to force drawing a background
    V_DrawBackground(gamemode==shareware ? "CEIL5_1" : "MFLR8_4", 0);
    M_DrawTitle(81, 9, "PRBOOM", g_cr_gold, PACKAGE_NAME " v" PACKAGE_VERSION, g_cr_gold);
    M_DrawScreenItems(cred_settings);
  }
}

static int M_IndexInChoices(const char *str, const char **choices) {
  int i = 0;

  while (*choices != NULL) {
    if (!strcmp(str, *choices))
      return i;
    i++;
    choices++;
  }
  return 0;
}

// [FG] support more joystick and mouse buttons

static inline int GetButtons(const unsigned int max, int data)
{
  int i;

  for (i = 0; i < max; ++i)
  {
    if (data & (1 << i))
    {
      return i;
    }
  }

  return -1;
}

/////////////////////////////////////////////////////////////////////////////
//
// M_Responder
//
// Examines incoming keystrokes and button pushes and determines some
// action based on the state of the system.
//

dboolean M_Responder (event_t* ev) {
  int    ch, action;
  int    i;
  int s_input;
  static int joywait   = 0;
  static int mousewait = 0;

  ch = MENU_NULL; // will be changed to a legit char if we're going to use it here
  action = MENU_NULL; // differentiate between action caused by a binding and input ch

  // Process joystick input

  if (ev->type == ev_joystick && joywait < I_GetTime()) {
    if (ev->data3 == -1)
    {
      action = MENU_UP;                                // phares 3/7/98
      ch = 0;
      joywait = I_GetTime() + 5;
    }
    else if (ev->data3 == 1)
    {
      action = MENU_DOWN;                              // phares 3/7/98
      ch = 0;
      joywait = I_GetTime() + 5;
    }

    if (ev->data2 == -1)
    {
      action = MENU_LEFT;                              // phares 3/7/98
      ch = 0;
      joywait = I_GetTime() + 2;
    }
    else if (ev->data2 == 1)
    {
      action = MENU_RIGHT;                             // phares 3/7/98
      ch = 0;
      joywait = I_GetTime() + 2;
    }

    if (ev->data1&1)
    {
      action = MENU_ENTER;                             // phares 3/7/98
      ch = 0;
      joywait = I_GetTime() + 5;
    }

    if (ev->data1&2)
    {
      action = MENU_BACKSPACE;                         // phares 3/7/98
      ch = 0;
      joywait = I_GetTime() + 5;
    }

    // phares 4/4/98:
    // Handle joystick buttons 3+, and allow them to pass down
    // to where key binding can eat them.

    if (setup_active && set_keybnd_active) {
      if (ev->data1 >> 2)
      {
        ch = 0; // meaningless, just to get you past the check for -1
        joywait = I_GetTime() + 5;
      }
    }
  }
  else {
   // Process mouse input
    if (ev->type == ev_mouse && mousewait < I_GetTime()) {
      if (ev->data1&1)
      {
        action = MENU_ENTER;                           // phares 3/7/98
        ch = 0;
        mousewait = I_GetTime() + 15;
      }

      if (ev->data1&2)
      {
        action = MENU_BACKSPACE;                       // phares 3/7/98
        ch = 0;
        mousewait = I_GetTime() + 15;
      }

      // phares 4/4/98:
      // Handle mouse buttons 3+, and allow it to pass down
      if (ev->data1 >> 2)
      {
        ch = 0; // meaningless, just to get you past the check for -1
        mousewait = I_GetTime() + 15;
      }
    }
    else
      // Process keyboard input
      if (ev->type == ev_keydown)
      {
        ch = ev->data1;
                                      // phares 4/11/98:
        if (ch == KEYD_RSHIFT)        // For chat string processing, need
          shiftdown = true;           // to know when shift key is up or
      }                               // down so you can get at the !,#,
      else if (ev->type == ev_keyup)  // etc. keys. Keydowns are allowed
        if (ev->data1 == KEYD_RSHIFT) // past this point, but keyups aren't
          shiftdown = false;          // so we need to note the difference
  }                                   // here using the 'shiftdown' dboolean.

  if (dsda_InputActivated(dsda_input_menu_left))
  {
    action = MENU_LEFT;
  }
  else if (dsda_InputActivated(dsda_input_menu_right))
  {
    action = MENU_RIGHT;
  }
  else if (dsda_InputActivated(dsda_input_menu_up))
  {
    action = MENU_UP;
  }
  else if (dsda_InputActivated(dsda_input_menu_down))
  {
    action = MENU_DOWN;
  }
  else if (dsda_InputActivated(dsda_input_menu_backspace))
  {
    action = MENU_BACKSPACE;
  }
  else if (dsda_InputActivated(dsda_input_menu_enter))
  {
    action = MENU_ENTER;
  }
  else if (dsda_InputActivated(dsda_input_menu_escape))
  {
    action = MENU_ESCAPE;
  }
  else if (dsda_InputActivated(dsda_input_menu_clear))
  {
    action = MENU_CLEAR;
  }

  if (
    menuactive &&
    currentMenu == &dsda_ConsoleDef &&
    (ch != MENU_NULL || action != MENU_NULL) &&
    action != MENU_ESCAPE
  )
  {
    dsda_UpdateConsole(ch, action);
    return true;
  }

  // Save Game string input

  if (saveStringEnter && (ch != MENU_NULL || action != MENU_NULL)) {
    if (action == MENU_BACKSPACE)                            // phares 3/7/98
    {
      if (saveCharIndex > 0)
      {
        saveCharIndex--;
        savegamestrings[saveSlot][saveCharIndex] = 0;
      }
    }
    else if (action == MENU_ESCAPE)                    // phares 3/7/98
    {
      saveStringEnter = 0;
      strcpy(&savegamestrings[saveSlot][0],saveOldString);
    }
    else if (action == MENU_ENTER)                     // phares 3/7/98
    {
      saveStringEnter = 0;
      if (savegamestrings[saveSlot][0])
        M_DoSave(saveSlot);
    }
    else if (ch > 0)
    {
      ch = toupper(ch);
      if (ch >= 32 && ch <= 127 &&
          saveCharIndex < SAVESTRINGSIZE-1 &&
          M_StringWidth(savegamestrings[saveSlot]) < (SAVESTRINGSIZE-2)*8)
      {
        savegamestrings[saveSlot][saveCharIndex++] = ch;
        savegamestrings[saveSlot][saveCharIndex] = 0;
      }
    }
    return true;
  }

  if (
    menuactive &&
    (currentMenu == &LoadDef || currentMenu == &SaveDef) &&
    !saveStringEnter
  )
  {
    int diff = 0;

    if (action == MENU_LEFT)
      diff = -1;
    else if (action == MENU_RIGHT)
      diff = 1;

    if (diff)
    {
      save_page += diff;
      if (save_page < 0)
        save_page = save_page_limit - 1;
      else if (save_page >= save_page_limit)
        save_page = 0;

      M_ReadSaveStrings();
    }
  }

  // Take care of any messages that need input

  if (messageToPrint && ch != MENU_NULL) {
    if (messageNeedsInput == true &&
        !(ch == ' ' || ch == 'n' || ch == 'y' || ch == KEYD_ESCAPE)) // phares
      return false;

    menuactive = messageLastMenuActive;
    messageToPrint = 0;
    if (messageRoutine)
      messageRoutine(ch);

    menuactive = mnact_inactive;
    S_StartSound(NULL,g_sfx_swtchx);
    return true;
  }

  // killough 2/22/98: add support for screenshot key:
  if (dsda_InputActivated(dsda_input_screenshot))
  {
    G_ScreenShot ();
    // Don't eat the keypress in this case. See sf bug #1843280.
  }

  if (heretic && F_BlockingInput())
    return false;

  // If there is no active menu displayed...

  if (!menuactive) {                                           // phares
    if (dsda_InputActivated(dsda_input_autorun))               //  V
    {
      autorun = !autorun;
      return true;
    }

    if (ch == KEYD_F1)
    {
      M_StartControlPanel ();

      currentMenu = &HelpDef;         // killough 10/98: new help screen

      itemOn = 0;
      S_StartSound(NULL,g_sfx_swtchn);
      return true;
    }

    if (dsda_InputActivated(dsda_input_savegame))
    {
      M_StartControlPanel();
      S_StartSound(NULL,g_sfx_swtchn);
      M_SaveGame(0);
      return true;
    }

    if (dsda_InputActivated(dsda_input_loadgame))
    {
      M_StartControlPanel();
      S_StartSound(NULL,g_sfx_swtchn);
      M_LoadGame(0);
      return true;
    }

    if (dsda_InputActivated(dsda_input_soundvolume))
    {
      M_StartControlPanel ();
      currentMenu = &SoundDef;
      itemOn = sfx_vol;
      S_StartSound(NULL,g_sfx_swtchn);
      return true;
    }

    if (dsda_InputActivated(dsda_input_quicksave))
    {
      S_StartSound(NULL,g_sfx_swtchn);
      M_QuickSave();
      return true;
    }

    if (dsda_InputActivated(dsda_input_endgame))
    {
      S_StartSound(NULL,g_sfx_swtchn);
      M_EndGame(0);
      return true;
    }

    // Toggle messages
    if (dsda_InputActivated(dsda_input_messages))
    {
      M_ChangeMessages(0);
      S_StartSound(NULL,g_sfx_swtchn);
      return true;
    }

    if (dsda_InputActivated(dsda_input_quickload))
    {
      S_StartSound(NULL,g_sfx_swtchn);
      M_QuickLoad();
      return true;
    }

    if (dsda_InputActivated(dsda_input_quit))
    {
      if (!dsda_SkipQuitPrompt())
        S_StartSound(NULL,g_sfx_swtchn);
      M_QuitDOOM(0);
      return true;
    }

    if (dsda_InputActivated(dsda_input_console) && !dsda_StrictMode())
    {
      if (dsda_OpenConsole())
        S_StartSound(NULL,g_sfx_swtchn);
      return true;
    }

    // Toggle gamma
    if (dsda_InputActivated(dsda_input_gamma))
    {
//e6y
#ifdef GL_DOOM
      if (V_GetMode() == VID_MODEGL && gl_hardware_gamma)
      {
        static char str[200];
        useglgamma++;
        if (useglgamma > MAX_GLGAMMA)
          useglgamma = 0;
        sprintf(str, "Gamma correction level %d", useglgamma);
        players[consoleplayer].message = str;

        gld_SetGammaRamp(useglgamma);
      }
      else
#endif
      {
        usegamma++;
        if (usegamma > 4)
          usegamma = 0;
        players[consoleplayer].message =
          usegamma == 0 ? s_GAMMALVL0 :
          usegamma == 1 ? s_GAMMALVL1 :
          usegamma == 2 ? s_GAMMALVL2 :
          usegamma == 3 ? s_GAMMALVL3 :
          s_GAMMALVL4;
        V_SetPalette(0);
        return true;
      }
    }

    if (dsda_InputActivated(dsda_input_zoomout))
    {
      if ((automapmode & am_active) || chat_on)
        return false;
      M_SizeDisplay(0);
      S_StartSound(NULL,g_sfx_stnmov);
      return true;
    }

    if (dsda_InputActivated(dsda_input_zoomin))
    {                                   // jff 2/23/98
      if ((automapmode & am_active) || chat_on)     // allow
        return false;                   // key_hud==key_zoomin
      M_SizeDisplay(1);                                             //  ^
      S_StartSound(NULL,g_sfx_stnmov);                              //  |
      return true;                                                  // phares
    }

    //e6y
    if (dsda_InputActivated(dsda_input_speed_default) && (!netgame||demoplayback) && !dsda_StrictMode())
    {
      realtic_clock_rate = StepwiseSum(realtic_clock_rate, 0, speed_step, 3, 10000, 100);
      doom_printf("Game Speed %d", realtic_clock_rate);
      I_Init2();
      // Don't eat the keypress in this case.
      // return true;
    }
    if (dsda_InputActivated(dsda_input_speed_up) && (!netgame||demoplayback) && !dsda_StrictMode())
    {
      realtic_clock_rate = StepwiseSum(realtic_clock_rate, 1, speed_step, 3, 10000, 100);
      doom_printf("Game Speed %d", realtic_clock_rate);
      I_Init2();
      // Don't eat the keypress in this case.
      // return true;
    }
    if (dsda_InputActivated(dsda_input_speed_down) && (!netgame||demoplayback) && !dsda_StrictMode())
    {
      realtic_clock_rate = StepwiseSum(realtic_clock_rate, -1, speed_step, 3, 10000, 100);
      doom_printf("Game Speed %d", realtic_clock_rate);
      I_Init2();
      // Don't eat the keypress in this case.
      // return true;
    }
    if (dsda_InputActivated(dsda_input_nextlevel))
    {
      if (demoplayback && !doSkip && singledemo)
      {
        demo_stoponnext = true;
        G_SkipDemoStart();
        return true;
      }
      else
      {
        if (G_GotoNextLevel())
          return true;
      }
    }

    if (dsda_InputActivated(dsda_input_restart))
    {
      if (G_ReloadLevel())
        return true;
    }

    if (dsda_InputActivated(dsda_input_demo_endlevel))
    {
      if (demoplayback && !doSkip && singledemo)
      {
        demo_stoponend = true;
        G_SkipDemoStart();
        return true;
      }
    }

    if (dsda_InputActivated(dsda_input_demo_skip))
    {
      if (demoplayback && singledemo)
      {
        if (doSkip)
        {
          G_SkipDemoStop();
        }
        else
        {
          G_SkipDemoStart();
        }
        return true;
      }
    }

    if (dsda_InputActivated(dsda_input_store_quick_key_frame))
    {
      if (
        gamestate == GS_LEVEL &&
        gameaction == ga_nothing &&
        !dsda_StrictMode()
      ) dsda_StoreQuickKeyFrame();
      return true;
    }

    if (dsda_InputActivated(dsda_input_restore_quick_key_frame))
    {
      if (!dsda_StrictMode()) dsda_RestoreQuickKeyFrame();
      return true;
    }

    if (dsda_InputActivated(dsda_input_rewind))
    {
      if (!dsda_StrictMode()) dsda_RewindAutoKeyFrame();
      return true;
    }

    if (dsda_InputActivated(dsda_input_cycle_profile))
    {
      dsda_InputCycleProfile();
      doom_printf("Input Profile %d", dsda_input_profile);
      S_StartSound(NULL, g_sfx_swtchn);
      return true;
    }

    if (dsda_InputActivated(dsda_input_cycle_palette))
    {
      dsda_CyclePlayPal();
      doom_printf("Palette %s", dsda_PlayPalData()->lump_name);
      S_StartSound(NULL, g_sfx_swtchn);
      return true;
    }

    if (dsda_InputActivated(dsda_input_walkcamera))
    {
      if (demoplayback && gamestate == GS_LEVEL)
      {
        walkcamera.type = (walkcamera.type+1)%3;
        P_SyncWalkcam (true, (walkcamera.type!=2));
        R_ResetViewInterpolation ();
        if (walkcamera.type==0)
          R_SmoothPlaying_Reset(NULL);
        // Don't eat the keypress in this case.
        // return true;
      }
    }

#ifdef GL_DOOM
    if (V_GetMode() == VID_MODEGL)
    {
      if (dsda_InputActivated(dsda_input_showalive) && !dsda_StrictMode())
      {
        show_alive = (show_alive + 1) % 3;
        doom_printf("Show Alive Monsters %s",
          (show_alive ? (show_alive == 1 ? "(mode 1) on" : "(mode 2) on" ) : "off"));
      }
    }
#endif

    if (dsda_InputActivated(dsda_input_command_display) && !dsda_StrictMode())
    {
      dsda_command_display = !dsda_command_display;
      doom_printf("Command Display %s", dsda_command_display ? "on" : "off");
    }

    if (dsda_InputActivated(dsda_input_coordinate_display) && !dsda_StrictMode())
    {
      dsda_coordinate_display = !dsda_coordinate_display;
      doom_printf("Coordinate Display %s", dsda_coordinate_display ? "on" : "off");
    }

    if (dsda_InputActivated(dsda_input_strict_mode))
    {
      dsda_strict_mode = !dsda_strict_mode;
      doom_printf("Strict Mode %s", dsda_strict_mode ? "on" : "off");
    }

    if (dsda_InputActivated(dsda_input_mlook)) // mouse look
    {
      movement_mouselook = !movement_mouselook;
      M_ChangeMouseLook();
      doom_printf("Mouselook %s", movement_mouselook ? "on" : "off");
      // Don't eat the keypress in this case.
      // return true;
    }

    if (dsda_InputActivated(dsda_input_novert))
    {
      movement_mousenovert = !movement_mousenovert;
      doom_printf("Vertical Mouse Movement %s", movement_mousenovert ? "off" : "on");
      // Don't eat the keypress in this case.
      // return true;
    }

    if (dsda_InputActivated(dsda_input_hud))   // heads-up mode
    {
      if ((automapmode & am_active) || chat_on)    // jff 2/22/98
        return false;                  // HUD mode control
      if (screenSize<8)                // function on default F5
        while (screenSize<8 || !hud_displayed) // make hud visible
          M_SizeDisplay(1);            // when configuring it
      else
        {
        hud_displayed = 1;               //jff 3/3/98 turn hud on
        HU_NextHud();
        HU_MoveHud(true);                //jff 3/9/98 move it now to avoid glitch
        }
      return true;
    }

    /* killough 10/98: allow key shortcut into Setup menu */
    if (dsda_InputActivated(dsda_input_setup)) {
      M_StartControlPanel();
      S_StartSound(NULL,g_sfx_swtchn);
      M_SetupNextMenu(&SetupDef);
      return true;
    }
  }
  // Pop-up Main menu?

  if (!menuactive)
  {
    if (ch == KEYD_ESCAPE) // phares
    {
      M_StartControlPanel ();
      S_StartSound(NULL,g_sfx_swtchn);
      return true;
    }
    return false;
  }

  if (ch == MENU_NULL)
    return false; // we can't use the event here

  // phares 3/26/98 - 4/11/98:
  // Setup screen key processing

  if (setup_active) {
    setup_menu_t* ptr1= current_setup_menu + set_menu_itemon;
    setup_menu_t* ptr2 = NULL;

    // phares 4/19/98:
    // Catch the response to the 'reset to default?' verification
    // screen

    if (default_verify)
    {
      if (toupper(ch) == 'Y') {
        M_ResetDefaults();
        default_verify = false;
        M_SelectDone(ptr1);
      }
      else if (toupper(ch) == 'N') {
        default_verify = false;
        M_SelectDone(ptr1);
      }
      return true;
    }

    // Common processing for some items

    if (setup_select) { // changing an entry
      if (action == MENU_ESCAPE) // Exit key = no change
      {
        M_SelectDone(ptr1);                           // phares 4/17/98
        setup_gather = false;   // finished gathering keys, if any
        return true;
      }

      if (ptr1->m_flags & S_YESNO) // yes or no setting?
      {
        if (action == MENU_ENTER) {
          *ptr1->var.def->location.pi = !*ptr1->var.def->location.pi; // killough 8/15/98

          // phares 4/14/98:
          // If not in demoplayback, demorecording, or netgame,
          // and there's a second variable in var2, set that
          // as well

          // killough 8/15/98: add warning messages

          if (ptr1->m_flags & (S_LEVWARN | S_PRGWARN))
            warn_about_changes(ptr1->m_flags &    // killough 10/98
             (S_LEVWARN | S_PRGWARN));
          else
            M_UpdateCurrent(ptr1->var.def);

          if (ptr1->action)      // killough 10/98
            ptr1->action();

          //e6y
          #ifdef GL_DOOM
          {
            extern dboolean gl_arb_multitexture;
            if ((ptr1->m_flags&S_CANT_GL_ARB_MULTITEXTURE) && !gl_arb_multitexture)
              warn_about_changes(ptr1->m_flags & S_CANT_GL_ARB_MULTITEXTURE);
          }
          #endif
        }
        M_SelectDone(ptr1);                           // phares 4/17/98
        return true;
      }

      if (ptr1->m_flags & S_CRITEM)
      {
        if (action != MENU_ENTER)
        {
          ch -= 0x30; // out of ascii
          if (ch < 0 || ch > 9)
            return true; // ignore
          *ptr1->var.def->location.pi = ch;
        }
        if (ptr1->action)      // killough 10/98
          ptr1->action();
        M_SelectDone(ptr1);                      // phares 4/17/98
        return true;
      }

      if (ptr1->m_flags & S_NUM) // number?
      {
        if (setup_gather) { // gathering keys for a value?
          /* killough 10/98: Allow negatives, and use a more
           * friendly input method (e.g. don't clear value early,
           * allow backspace, and return to original value if bad
           * value is entered).
           */
          if (action == MENU_ENTER) {
            if (gather_count) {     // Any input?
              int value;

              gather_buffer[gather_count] = 0;
              value = atoi(gather_buffer);  // Integer value

              //e6y
              if ((ptr1->m_flags&S_CANT_GL_ARB_MULTISAMPLEFACTOR) && value%2!=0)
                warn_about_changes(ptr1->m_flags & S_CANT_GL_ARB_MULTISAMPLEFACTOR);
              else

              if ((ptr1->var.def->minvalue != UL &&
                   value < ptr1->var.def->minvalue) ||
                  (ptr1->var.def->maxvalue != UL &&
                   value > ptr1->var.def->maxvalue))
                warn_about_changes(S_BADVAL);
              else {
                *ptr1->var.def->location.pi = value;

                /* killough 8/9/98: fix numeric vars
                 * killough 8/15/98: add warning message
                 */
                if (ptr1->m_flags & (S_LEVWARN | S_PRGWARN))
                  warn_about_changes(ptr1->m_flags &
                   (S_LEVWARN | S_PRGWARN));
                else
                  M_UpdateCurrent(ptr1->var.def);

                if (ptr1->action)      // killough 10/98
                  ptr1->action();
              }
            }
            M_SelectDone(ptr1);     // phares 4/17/98
            setup_gather = false; // finished gathering keys
            return true;
          }

          if (action == MENU_BACKSPACE && gather_count) {
            gather_count--;
            return true;
          }

          if (gather_count >= MAXGATHER)
            return true;

          if (!isdigit(ch) && ch != '-')
            return true; // ignore

          /* killough 10/98: character-based numerical input */
          gather_buffer[gather_count++] = ch;
        }
        return true;
      }

      if (ptr1->m_flags & S_CHOICE) // selection of choices?
      {
        if (action == MENU_LEFT) {
          if (ptr1->var.def->type == def_int) {
            int value = *ptr1->var.def->location.pi;

            value = value - 1;
            if ((ptr1->var.def->minvalue != UL &&
                 value < ptr1->var.def->minvalue))
              value = ptr1->var.def->minvalue;
            if ((ptr1->var.def->maxvalue != UL &&
                 value > ptr1->var.def->maxvalue))
              value = ptr1->var.def->maxvalue;
            while (ptr1->selectstrings && ptr1->selectstrings[value][0] == '~')
              value--;
            if (*ptr1->var.def->location.pi != value)
              S_StartSound(NULL,g_sfx_menu);
            *ptr1->var.def->location.pi = value;
          }
          if (ptr1->var.def->type == def_str) {
            int old_value, value;

            old_value = M_IndexInChoices(*ptr1->var.def->location.ppsz,
                                         ptr1->selectstrings);
            value = old_value - 1;
            if (value < 0)
              value = 0;
            if (old_value != value)
              S_StartSound(NULL,g_sfx_menu);
            *ptr1->var.def->location.ppsz = ptr1->selectstrings[value];
          }
        }
        if (action == MENU_RIGHT) {
          if (ptr1->var.def->type == def_int) {
            int value = *ptr1->var.def->location.pi;

            value = value + 1;
            if ((ptr1->var.def->minvalue != UL &&
                 value < ptr1->var.def->minvalue))
              value = ptr1->var.def->minvalue;
            if ((ptr1->var.def->maxvalue != UL &&
                 value > ptr1->var.def->maxvalue))
              value = ptr1->var.def->maxvalue;
            while (ptr1->selectstrings && ptr1->selectstrings[value][0] == '~')
              value++;
            if (*ptr1->var.def->location.pi != value)
              S_StartSound(NULL,g_sfx_menu);
            *ptr1->var.def->location.pi = value;
          }
          if (ptr1->var.def->type == def_str) {
            int old_value, value;

            old_value = M_IndexInChoices(*ptr1->var.def->location.ppsz,
                                         ptr1->selectstrings);
            value = old_value + 1;
            if (ptr1->selectstrings[value] == NULL)
              value = old_value;
            if (old_value != value)
              S_StartSound(NULL,g_sfx_menu);
            *ptr1->var.def->location.ppsz = ptr1->selectstrings[value];
          }
        }
        if (action == MENU_ENTER) {
          // phares 4/14/98:
          // If not in demoplayback, demorecording, or netgame,
          // and there's a second variable in var2, set that
          // as well

          // killough 8/15/98: add warning messages

          if (ptr1->m_flags & (S_LEVWARN | S_PRGWARN))
            warn_about_changes(ptr1->m_flags &    // killough 10/98
             (S_LEVWARN | S_PRGWARN));
          else
            M_UpdateCurrent(ptr1->var.def);

          if (ptr1->action)      // killough 10/98
            ptr1->action();
          M_SelectDone(ptr1);                           // phares 4/17/98
        }
        return true;
      }
    }

    // Key Bindings

    s_input = (ptr1->m_flags & S_INPUT) ? ptr1->input : 0;

    if (set_keybnd_active) // on a key binding setup screen
      if (setup_select)    // incoming key or button gets bound
      {
        if (ev->type == ev_joystick)
        {
          setup_group group;
          dboolean search = true;

          if (!s_input)
            return true; // not a legal action here (yet)

          // see if the button is already bound elsewhere. if so, you
          // have to swap bindings so the action where it's currently
          // bound doesn't go dead. Since there is more than one
          // keybinding screen, you have to search all of them for
          // any duplicates. You're only interested in the items
          // that belong to the same group as the one you're changing.

          group  = ptr1->m_group;
          if ((ch = GetButtons(MAX_JOY_BUTTONS, ev->data1)) == -1)
            return true;
          for (i = 0 ; keys_settings[i] && search ; i++)
            for (ptr2 = keys_settings[i] ; !(ptr2->m_flags & S_END) ; ptr2++)
              if (ptr2->m_group == group && ptr1 != ptr2)
              {
                if (ptr2->m_flags & S_INPUT)
                  if (dsda_InputMatchJoyB(ptr2->input, ch))
                  {
                    dsda_InputRemoveJoyB(ptr2->input, ch);
                    search = false;
                    break;
                  }
              }
          dsda_InputAddJoyB(s_input, ch);
        }
        else if (ev->type == ev_mouse)
        {
          int i;
          setup_group group;
          dboolean search = true;

          if (!s_input)
            return true; // not a legal action here (yet)

          // see if the button is already bound elsewhere. if so, you
          // have to swap bindings so the action where it's currently
          // bound doesn't go dead. Since there is more than one
          // keybinding screen, you have to search all of them for
          // any duplicates. You're only interested in the items
          // that belong to the same group as the one you're changing.

          group  = ptr1->m_group;
          if ((ch = GetButtons(MAX_MOUSE_BUTTONS, ev->data1)) == -1)
            return true;
          for (i = 0 ; keys_settings[i] && search ; i++)
            for (ptr2 = keys_settings[i] ; !(ptr2->m_flags & S_END) ; ptr2++)
              if (ptr2->m_group == group && ptr1 != ptr2)
              {
                if (ptr2->m_flags & S_INPUT)
                  if (dsda_InputMatchMouseB(ptr2->input, ch))
                  {
                    dsda_InputRemoveMouseB(ptr2->input, ch);
                    search = false;
                    break;
                  }
              }
          dsda_InputAddMouseB(s_input, ch);
        }
        else  // keyboard key
        {
          int i;
          setup_group group;
          dboolean search = true;

          // see if 'ch' is already bound elsewhere. if so, you have
          // to swap bindings so the action where it's currently
          // bound doesn't go dead. Since there is more than one
          // keybinding screen, you have to search all of them for
          // any duplicates. You're only interested in the items
          // that belong to the same group as the one you're changing.

          // if you find that you're trying to swap with an action
          // that has S_KEEP set, you can't bind ch; it's already
          // bound to that S_KEEP action, and that action has to
          // keep that key.

          group  = ptr1->m_group;
          for (i = 0 ; keys_settings[i] && search ; i++)
            for (ptr2 = keys_settings[i] ; !(ptr2->m_flags & S_END) ; ptr2++)
              if (ptr2->m_group == group && ptr1 != ptr2)
              {
                if (ptr2->m_flags & (S_INPUT | S_KEEP))
                  if (dsda_InputMatchKey(ptr2->input, ch))
                  {
                    if (ptr2->m_flags & S_KEEP)
                      return true; // can't have it!
                    dsda_InputRemoveKey(ptr2->input, ch);
                    search = false;
                    break;
                  }
              }
          dsda_InputAddKey(s_input, ch);
        }

        M_SelectDone(ptr1);       // phares 4/17/98
        return true;
      }

    // Weapons

    if (set_weapon_active) // on the weapons setup screen
  if (setup_select) // changing an entry
    {
      if (action != MENU_ENTER)
        {
    ch -= '0'; // out of ascii
    if (ch < 1 || ch > 9)
      return true; // ignore

    // Plasma and BFG don't exist in shareware
    // killough 10/98: allow it anyway, since this
    // isn't the game itself, just setting preferences

    // see if 'ch' is already assigned elsewhere. if so,
    // you have to swap assignments.

    // killough 11/98: simplified

    for (i = 0; (ptr2 = weap_settings[i]); i++)
      for (; !(ptr2->m_flags & S_END); ptr2++)
        if (ptr2->m_flags & S_WEAP &&
      *ptr2->var.def->location.pi == ch && ptr1 != ptr2)
          {
      *ptr2->var.def->location.pi = *ptr1->var.def->location.pi;
      goto end;
          }
        end:
    *ptr1->var.def->location.pi = ch;
        }

      M_SelectDone(ptr1);       // phares 4/17/98
      return true;
    }

      // Automap

      if (set_auto_active) // on the automap setup screen
  if (setup_select) // incoming key
    {
      if (action == MENU_DOWN)
        {
    if (++color_palette_y == 16)
      color_palette_y = 0;
    S_StartSound(NULL,g_sfx_itemup);
    return true;
        }

      if (action == MENU_UP)
        {
    if (--color_palette_y < 0)
      color_palette_y = 15;
    S_StartSound(NULL,g_sfx_itemup);
    return true;
        }

      if (action == MENU_LEFT)
        {
    if (--color_palette_x < 0)
      color_palette_x = 15;
    S_StartSound(NULL,g_sfx_itemup);
    return true;
        }

      if (action == MENU_RIGHT)
        {
    if (++color_palette_x == 16)
      color_palette_x = 0;
    S_StartSound(NULL,g_sfx_itemup);
    return true;
        }

      if (action == MENU_ENTER)
        {
    *ptr1->var.def->location.pi = color_palette_x + 16*color_palette_y;
    M_SelectDone(ptr1);                         // phares 4/17/98
    colorbox_active = false;
    return true;
        }
    }

      // killough 10/98: consolidate handling into one place:
      if (setup_select &&
    set_general_active | set_chat_active | set_mess_active | set_status_active)
  {
    if (ptr1->m_flags & S_STRING) // creating/editing a string?
      {
        if (action == MENU_BACKSPACE) // backspace and DEL
    {
      if (chat_string_buffer[chat_index] == 0)
        {
          if (chat_index > 0)
      chat_string_buffer[--chat_index] = 0;
        }
      // shift the remainder of the text one char left
      else
        strcpy(&chat_string_buffer[chat_index],
         &chat_string_buffer[chat_index+1]);
    }
        else if (action == MENU_LEFT) // move cursor left
    {
      if (chat_index > 0)
        chat_index--;
    }
        else if (action == MENU_RIGHT) // move cursor right
    {
      if (chat_string_buffer[chat_index] != 0)
        chat_index++;
    }
        else if ((action == MENU_ENTER) ||
           (action == MENU_ESCAPE))
    {
      *ptr1->var.def->location.ppsz = chat_string_buffer;
      M_SelectDone(ptr1);   // phares 4/17/98
    }

        // Adding a char to the text. Has to be a printable
        // char, and you can't overrun the buffer. If the
        // chat string gets larger than what the screen can hold,
        // it is dealt with when the string is drawn (above).

        else if ((ch >= 32) && (ch <= 126))
    if ((chat_index+1) < CHAT_STRING_BFR_SIZE)
      {
        if (shiftdown)
          ch = shiftxform[ch];
        if (chat_string_buffer[chat_index] == 0)
          {
      chat_string_buffer[chat_index++] = ch;
      chat_string_buffer[chat_index] = 0;
          }
        else
          chat_string_buffer[chat_index++] = ch;
      }
        return true;
      }

    M_SelectDone(ptr1);       // phares 4/17/98
    return true;
  }

      // Not changing any items on the Setup screens. See if we're
      // navigating the Setup menus or selecting an item to change.

      if (action == MENU_DOWN)
  {
    ptr1->m_flags &= ~S_HILITE;     // phares 4/17/98
    do
      if (ptr1->m_flags & S_END)
        {
    set_menu_itemon = 0;
    ptr1 = current_setup_menu;
        }
      else
        {
    set_menu_itemon++;
    ptr1++;
        }
    while (ptr1->m_flags & S_SKIP);
    M_SelectDone(ptr1);         // phares 4/17/98
    return true;
  }

      if (action == MENU_UP)
  {
    ptr1->m_flags &= ~S_HILITE;     // phares 4/17/98
    do
      {
        if (set_menu_itemon == 0)
    do
      set_menu_itemon++;
    while(!((current_setup_menu + set_menu_itemon)->m_flags & S_END));
        set_menu_itemon--;
      }
    while((current_setup_menu + set_menu_itemon)->m_flags & S_SKIP);
    M_SelectDone(current_setup_menu + set_menu_itemon);         // phares 4/17/98
    return true;
  }

      if (action == MENU_CLEAR)
  {
    if (ptr1->m_flags & S_INPUT)
    {
      dsda_InputReset(ptr1->input);
    }

    return true;
  }

      if (action == MENU_ENTER)
  {
    int flags = ptr1->m_flags;

    // You've selected an item to change. Highlight it, post a new
    // message about what to do, and get ready to process the
    // change.
    //
    // killough 10/98: use friendlier char-based input buffer

    if (flags & S_NUM)
      {
        setup_gather = true;
        print_warning_about_changes = false;
        gather_count = 0;
      }
    else if (flags & S_COLOR)
      {
        int color = *ptr1->var.def->location.pi;

        if (color < 0 || color > 255) // range check the value
    color = 0;        // 'no show' if invalid

        color_palette_x = *ptr1->var.def->location.pi & 15;
        color_palette_y = *ptr1->var.def->location.pi >> 4;
        colorbox_active = true;
      }
    else if (flags & S_STRING)
      {
        // copy chat string into working buffer; trim if needed.
        // free the old chat string memory and replace it with
        // the (possibly larger) new memory for editing purposes
        //
        // killough 10/98: fix bugs, simplify

        chat_string_buffer = malloc(CHAT_STRING_BFR_SIZE);
        strncpy(chat_string_buffer,
          *ptr1->var.def->location.ppsz, CHAT_STRING_BFR_SIZE);

        // guarantee null delimiter
        chat_string_buffer[CHAT_STRING_BFR_SIZE-1] = 0;

        // set chat table pointer to working buffer
        // and free old string's memory.
        {
          union { const char **c; char **s; } u; // type punning via unions

          u.c = ptr1->var.def->location.ppsz;
          free(*(u.s));
          *(u.c) = chat_string_buffer;
        }
        chat_index = 0; // current cursor position in chat_string_buffer
      }
    else if (flags & S_RESET)
      default_verify = true;

    ptr1->m_flags |= S_SELECT;
    setup_select = true;
    S_StartSound(NULL,g_sfx_itemup);
    return true;
  }

      if ((action == MENU_ESCAPE) || (action == MENU_BACKSPACE))
  {
    M_SetSetupMenuItemOn(set_menu_itemon);
    if (action == MENU_ESCAPE) // Clear all menus
      M_ClearMenus();
    else // MENU_BACKSPACE = return to Setup Menu
      if (currentMenu->prevMenu)
        {
    currentMenu = currentMenu->prevMenu;
    itemOn = currentMenu->lastOn;
    S_StartSound(NULL,g_sfx_swtchn);
        }
    ptr1->m_flags &= ~(S_HILITE|S_SELECT);// phares 4/19/98
    setup_active = false;
    set_keybnd_active = false;
    set_weapon_active = false;
    set_status_active = false;
    set_auto_active = false;
    set_mess_active = false;
    set_chat_active = false;
    colorbox_active = false;
    default_verify = false;       // phares 4/19/98
    set_general_active = false;    // killough 10/98
    HU_Start();    // catch any message changes // phares 4/19/98
    S_StartSound(NULL,g_sfx_swtchx);
    return true;
  }

      // Some setup screens may have multiple screens.
      // When there are multiple screens, m_prev and m_next items need to
      // be placed on the appropriate screen tables so the user can
      // move among the screens using the left and right arrow keys.
      // The m_var1 field contains a pointer to the appropriate screen
      // to move to.

      if (action == MENU_LEFT)
  {
    ptr2 = ptr1;
    do
      {
        ptr2++;
        if (ptr2->m_flags & S_PREV)
    {
      ptr1->m_flags &= ~S_HILITE;
      mult_screens_index--;
      M_SetSetupMenuItemOn(set_menu_itemon);
      current_setup_menu = ptr2->var.menu;
      set_menu_itemon = M_GetSetupMenuItemOn();
      print_warning_about_changes = false; // killough 10/98
      while (current_setup_menu[set_menu_itemon++].m_flags&S_SKIP);
      current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
      S_StartSound(NULL,g_sfx_menu);  // killough 10/98
      return true;
    }
      }
    while (!(ptr2->m_flags & S_END));
  }

      if (action == MENU_RIGHT)
  {
    ptr2 = ptr1;
    do
      {
        ptr2++;
        if (ptr2->m_flags & S_NEXT)
    {
      ptr1->m_flags &= ~S_HILITE;
      mult_screens_index++;
      M_SetSetupMenuItemOn(set_menu_itemon);
      current_setup_menu = ptr2->var.menu;
      set_menu_itemon = M_GetSetupMenuItemOn();
      print_warning_about_changes = false; // killough 10/98
      while (current_setup_menu[set_menu_itemon++].m_flags&S_SKIP);
      current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
      S_StartSound(NULL,g_sfx_menu);  // killough 10/98
      return true;
    }
      }
    while (!(ptr2->m_flags & S_END));
  }

    } // End of Setup Screen processing

  // From here on, these navigation keys are used on the BIG FONT menus
  // like the Main Menu.

  if (action == MENU_DOWN)                             // phares 3/7/98
    {
      do
  {
    if (itemOn+1 > currentMenu->numitems-1)
      itemOn = 0;
    else
      itemOn++;
    S_StartSound(NULL,g_sfx_menu);
  }
      while(currentMenu->menuitems[itemOn].status==-1);
      return true;
    }

  if (action == MENU_UP)                               // phares 3/7/98
    {
      do
  {
    if (!itemOn)
      itemOn = currentMenu->numitems-1;
    else
      itemOn--;
    S_StartSound(NULL,g_sfx_menu);
  }
      while(currentMenu->menuitems[itemOn].status==-1);
      return true;
    }

  if (action == MENU_LEFT)                             // phares 3/7/98
    {
      if (currentMenu->menuitems[itemOn].routine &&
    currentMenu->menuitems[itemOn].status == 2)
  {
    S_StartSound(NULL,g_sfx_stnmov);
    currentMenu->menuitems[itemOn].routine(0);
  }
      return true;
    }

  if (action == MENU_RIGHT)                            // phares 3/7/98
    {
      if (currentMenu->menuitems[itemOn].routine &&
    currentMenu->menuitems[itemOn].status == 2)
  {
    S_StartSound(NULL,g_sfx_stnmov);
    currentMenu->menuitems[itemOn].routine(1);
  }
      return true;
    }

  if (action == MENU_ENTER)                            // phares 3/7/98
    {
      if (currentMenu->menuitems[itemOn].routine &&
    currentMenu->menuitems[itemOn].status)
  {
    currentMenu->lastOn = itemOn;
    if (currentMenu->menuitems[itemOn].status == 2)
      {
        currentMenu->menuitems[itemOn].routine(1);   // right arrow
        S_StartSound(NULL,g_sfx_stnmov);
      }
    else
      {
        currentMenu->menuitems[itemOn].routine(itemOn);
        S_StartSound(NULL,g_sfx_pistol);
      }
  }
      //jff 3/24/98 remember last skill selected
      // killough 10/98 moved to skill-specific functions
      return true;
    }

  if (action == MENU_ESCAPE)                           // phares 3/7/98
    {
      currentMenu->lastOn = itemOn;
      M_ClearMenus ();
      S_StartSound(NULL,g_sfx_swtchx);
      return true;
    }

  if (action == MENU_BACKSPACE)                        // phares 3/7/98
    {
      currentMenu->lastOn = itemOn;

      // phares 3/30/98:
      // add checks to see if you're in the extended help screens
      // if so, stay with the same menu definition, but bump the
      // index back one. if the index bumps back far enough ( == 0)
      // then you can return to the Read_Thisn menu definitions

      if (currentMenu->prevMenu)
  {
    if (currentMenu == &ExtHelpDef)
      {
        if (--extended_help_index == 0)
    {
      currentMenu = currentMenu->prevMenu;
      extended_help_index = 1; // reset
    }
      }
    else
      currentMenu = currentMenu->prevMenu;
    itemOn = currentMenu->lastOn;
    S_StartSound(NULL,g_sfx_swtchn);
  }
      return true;
    }

  else
    {
      for (i = itemOn+1;i < currentMenu->numitems;i++)
  if (currentMenu->menuitems[i].alphaKey == ch)
    {
      itemOn = i;
      S_StartSound(NULL,g_sfx_menu);
      return true;
    }
      for (i = 0;i <= itemOn;i++)
  if (currentMenu->menuitems[i].alphaKey == ch)
    {
      itemOn = i;
      S_StartSound(NULL,g_sfx_menu);
      return true;
    }
    }
  return false;
}

//
// End of M_Responder
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//
// General Routines
//
// This displays the Main menu and gets the menu screens rolling.
// Plus a variety of routines that control the Big Font menu display.
// Plus some initialization for game-dependant situations.

void M_StartControlPanel (void)
{
  // intro might call this repeatedly

  if (menuactive)
    return;

  //jff 3/24/98 make default skill menu choice follow -skill or defaultskill
  //from command line or config file
  //
  // killough 10/98:
  // Fix to make "always floating" with menu selections, and to always follow
  // defaultskill, instead of -skill.

  NewDef.lastOn = defaultskill - 1;

  // e6y
  // We need to remove the fourth episode for pre-ultimate complevels.
  // It is located here instead of M_Init() because of TNTCOMP cheat.
  if (!heretic && !EpiCustom)
  {
    EpiDef.numitems = ep_end;
    if (gamemode != commercial
      && (compatibility_level < ultdoom_compatibility
        || W_CheckNumForName(EpiDef.menuitems[ep4].name) == -1))
    {
      EpiDef.numitems--;
    }
  }

  default_verify = 0;                  // killough 10/98
  menuactive = mnact_float;
  currentMenu = &MainDef;         // JDC
  itemOn = currentMenu->lastOn;   // JDC
  print_warning_about_changes = false;   // killough 11/98
}

//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
// killough 9/29/98: Significantly reformatted source
//

void M_Drawer (void)
{
  inhelpscreens = false;

  // Horiz. & Vertically center string and print it.
  // killough 9/29/98: simplified code, removed 40-character width limit
  if (messageToPrint)
  {
    char* ms;
    char* p;
    int y;

    if (heretic) return MN_DrawMessage(messageString);

    /* cph - strdup string to writable memory */
    ms = strdup(messageString);
    p = ms;

    y = 100 - M_StringHeight(messageString)/2;
    while (*p)
    {
      char *string = p, c;
      while ((c = *p) && *p != '\n')
        p++;
      *p = 0;
      M_WriteText(160 - M_StringWidth(string)/2, y, string, CR_DEFAULT);
      y += g_menu_font[0].height;
      if ((*p = c))
        p++;
    }
    free(ms);
  }
  else if (menuactive)
  {
    int x,y,max,i;
    int lumps_missing = 0;

    menuactive = mnact_float; // Boom-style menu drawers will set mnact_full

    if (currentMenu->routine)
      currentMenu->routine();     // call Draw routine

    if (heretic) return MN_Drawer();

    // DRAW MENU

    x = currentMenu->x;
    y = currentMenu->y;
    max = currentMenu->numitems;

    for (i = 0; i < max; i++)
      if (currentMenu->menuitems[i].name[0])
        if (W_CheckNumForName(currentMenu->menuitems[i].name) < 0)
          lumps_missing++;

    if (lumps_missing == 0)
      for (i=0;i<max;i++)
      {
        if (currentMenu->menuitems[i].name[0])
          V_DrawNamePatch(x,y,0,currentMenu->menuitems[i].name,
              CR_DEFAULT, VPT_STRETCH);
        y += LINEHEIGHT;
      }
    else
      for (i = 0; i < max; i++)
      {
        const char *alttext = currentMenu->menuitems[i].alttext;
        if (alttext)
          M_WriteText(x, y+8-(M_StringHeight(alttext)/2), alttext, CR_DEFAULT);
        y += LINEHEIGHT;
      }

    // DRAW SKULL
    if (max > 0)
      // CPhipps - patch drawing updated
      V_DrawNamePatch(x + SKULLXOFF, currentMenu->y - 5 + itemOn*LINEHEIGHT,0,
          skullName[whichSkull], CR_DEFAULT, VPT_STRETCH);
  }
}

//
// M_ClearMenus
//
// Called when leaving the menu screens for the real world

void M_ClearMenus (void)
{
  menuactive = mnact_inactive;
  print_warning_about_changes = 0;     // killough 8/15/98
  default_verify = 0;                  // killough 10/98

  BorderNeedRefresh = true;

  // if (!netgame && usergame && paused)
  //     sendpause = true;
}

//
// M_SetupNextMenu
//
void M_SetupNextMenu(menu_t *menudef)
{
  currentMenu = menudef;
  itemOn = currentMenu->lastOn;

  BorderNeedRefresh = true;
}

/////////////////////////////
//
// M_Ticker
//
void M_Ticker (void)
{
  // The skull counter is also used for non-skull pointers
  if (--skullAnimCounter <= 0)
  {
    whichSkull ^= 1;
    skullAnimCounter = 8;
  }

  if (heretic) return MN_Ticker();
}

/////////////////////////////
//
// Message Routines
//

void M_StartMessage (const char* string,void* routine,dboolean input)
{
  messageLastMenuActive = menuactive;
  messageToPrint = 1;
  messageString = string;
  messageRoutine = routine;
  messageNeedsInput = input;
  menuactive = mnact_float;
  return;
}

void M_StopMessage(void)
{
  menuactive = messageLastMenuActive;
  messageToPrint = 0;
}

/////////////////////////////
//
// Thermometer Routines
//

//
// M_DrawThermo draws the thermometer graphic for Mouse Sensitivity,
// Sound Volume, etc.
//
// proff/nicolas 09/20/98 -- changed for hi-res
// CPhipps - patch drawing updated
//
void M_DrawThermo(int x,int y,int thermWidth,int thermDot )
{
  int          xx;
  int           i;
  char num[4];
  int horizScaler; //Used to allow more thermo range for mouse sensitivity.

  if (heretic) return MN_DrawSlider(x, y, thermWidth, thermDot);

  /*
   * Modification By Barry Mead to allow the Thermometer to have vastly
   * larger ranges. (the thermWidth parameter can now have a value as
   * large as 200.      Modified 1-9-2000  Originally I used it to make
   * the sensitivity range for the mouse better. It could however also
   * be used to improve the dynamic range of music and sound affect
   * volume controls for example.
   */
  thermWidth = (thermWidth > 200) ? 200 : thermWidth; //Clamp to 200 max
  horizScaler = (thermWidth > 23) ? (200 / thermWidth) : 8; //Dynamic range
  xx = x;
  V_DrawNamePatch(xx, y, 0, "M_THERML", CR_DEFAULT, VPT_STRETCH);
  xx += 8;
  for (i=0;i<thermWidth;i++)
    {
    V_DrawNamePatch(xx, y, 0, "M_THERMM", CR_DEFAULT, VPT_STRETCH);
    xx += horizScaler;
    }

  xx += (8 - horizScaler);  /* make the right end look even */

  V_DrawNamePatch(xx, y, 0, "M_THERMR", CR_DEFAULT, VPT_STRETCH);

  // [crispy] print the value
  snprintf(num, 4, "%3d", thermDot);
  strcpy(menu_buffer, num);
  M_DrawMenuString(xx + 12, y + 3, g_menu_cr_select);

  // [crispy]
  if (thermDot >= thermWidth)
  {
    thermDot = thermWidth - 1;
  }

  V_DrawNamePatch((x+8)+thermDot*horizScaler,y,0,"M_THERMO",CR_DEFAULT,VPT_STRETCH);
}

//
// Draw an empty cell in the thermometer
//

void M_DrawEmptyCell (menu_t* menu,int item)
{
  // CPhipps - patch drawing updated
  V_DrawNamePatch(menu->x - 10, menu->y+item*LINEHEIGHT - 1, 0,
      "M_CELL1", CR_DEFAULT, VPT_STRETCH);
}

//
// Draw a full cell in the thermometer
//

void M_DrawSelCell (menu_t* menu,int item)
{
  // CPhipps - patch drawing updated
  V_DrawNamePatch(menu->x - 10, menu->y+item*LINEHEIGHT - 1, 0,
      "M_CELL2", CR_DEFAULT, VPT_STRETCH);
}

/////////////////////////////
//
// String-drawing Routines
//

//
// Find string width from g_menu_font chars
//

int M_StringWidth(const char* string)
{
  int i, c, w = 0;
  for (i = 0;(size_t)i < strlen(string);i++)
    w += (c = toupper(string[i]) - HU_FONTSTART) < 0 || c >= HU_FONTSIZE ?
      4 : g_menu_font[c].width;
  return w;
}

//
//    Find string height from g_menu_font chars
//

int M_StringHeight(const char* string)
{
  int i, h, height = h = g_menu_font[0].height;
  for (i = 0;string[i];i++)            // killough 1/31/98
    if (string[i] == '\n')
      h += height;
  return h;
}

//
//    Write a string using the g_menu_font
//
void M_WriteText (int x,int y, const char* string, int cm)
{
  int   w;
  const char* ch;
  int   c;
  int   cx;
  int   cy;
  int   flags;

  ch = string;
  cx = x;
  cy = y;

  flags = VPT_STRETCH;
  if (cm != CR_DEFAULT)
    flags |= VPT_TRANS;

  while(1) {
    c = *ch++;
    if (!c)
      break;
    if (c == '\n') {
      cx = x;
      cy += 12;
      continue;
    }

    c = toupper(c) - HU_FONTSTART;
    if (c < 0 || c>= HU_FONTSIZE) {
      cx += 4;
      continue;
    }

    w = g_menu_font[c].width;
    if (cx+w > BASE_WIDTH)
      break;
    // proff/nicolas 09/20/98 -- changed for hi-res
    // CPhipps - patch drawing updated
    V_DrawNumPatch(cx, cy, 0, g_menu_font[c].lumpnum, cm, flags);
    cx+=w;
  }
}

void M_DrawTitle(int x, int y, const char *patch, int cm,
                 const char *alttext, int altcm)
{
  int lumpnum = W_CheckNumForName(patch);

  if (lumpnum >= 0)
  {
    int flags = VPT_STRETCH;
    if (cm != CR_DEFAULT)
      flags |= VPT_TRANS;
    V_DrawNumPatch(x, y, 0, lumpnum, cm, flags);
  }
  else
  {
    // patch doesn't exist, draw some text in place of it
    M_WriteText(160-(M_StringWidth(alttext)/2),
                y+8-(M_StringHeight(alttext)/2), // assumes patch height 16
                alttext, altcm);
  }
}

/////////////////////////////
//
// Initialization Routines to take care of one-time setup
//

// phares 4/08/98:
// M_InitHelpScreen() clears the weapons from the HELP
// screen that don't exist in this version of the game.

void M_InitHelpScreen(void)
{
  setup_menu_t* src;

  src = helpstrings;
  while (!(src->m_flags & S_END)) {

    if ((strncmp(src->m_text,"PLASMA",6) == 0) && (gamemode == shareware))
      src->m_flags = S_SKIP; // Don't show setting or item
    if ((strncmp(src->m_text,"BFG",3) == 0) && (gamemode == shareware))
      src->m_flags = S_SKIP; // Don't show setting or item
    if ((strncmp(src->m_text,"SSG",3) == 0) && (gamemode != commercial))
      src->m_flags = S_SKIP; // Don't show setting or item
    src++;
  }
}

//
// M_Init
//
void M_Init(void)
{
  if (heretic) MN_Init();

  M_InitDefaults();                // killough 11/98
  currentMenu = &MainDef;
  menuactive = mnact_inactive;
  itemOn = currentMenu->lastOn;
  whichSkull = 0;
  skullAnimCounter = 10;
  screenSize = screenblocks - 3;
  messageToPrint = 0;
  messageString = NULL;
  messageLastMenuActive = menuactive;
  quickSaveSlot = -1;

  // Here we could catch other version dependencies,
  //  like HELP1/2, and four episodes.

  switch(gamemode)
    {
    case commercial:
      // This is used because DOOM 2 had only one HELP
      //  page. I use CREDIT as second page now, but
      //  kept this hack for educational purposes.
      MainMenu[readthis] = MainMenu[quitdoom];
      MainDef.numitems--;
      MainDef.y += 8;
      if (!EpiCustom)
        NewDef.prevMenu = &MainDef;
      ReadDef1.routine = M_DrawReadThis1;
      ReadDef1.x = 330;
      ReadDef1.y = 165;
      ReadMenu1[0].routine = M_FinishReadThis;
      break;
    case registered:
      // Episode 2 and 3 are handled,
      //  branching to an ad screen.

      // killough 2/21/98: Fix registered Doom help screen
      // killough 10/98: moved to second screen, moved up to the top
      ReadDef2.y = 15;
      // fallthrough

    case shareware:
      // We need to remove the fourth episode.
      EpiDef.numitems--;
      break;
    case retail:
      // We are fine.
    default:
      break;
    }

  M_InitHelpScreen();   // init the help screen       // phares 4/08/98
  M_InitExtendedHelp(); // init extended help screens // phares 3/30/98

  //e6y
  M_ChangeSpeed();
  M_ChangeMaxViewPitch();
  M_ChangeMouseLook();
  M_ChangeMouseInvert();
  M_ChangeFOV();
#ifdef GL_DOOM
  M_ChangeSpriteClip();
  M_ChangeAllowBoomColormaps();
#endif

  M_ChangeDemoSmoothTurns();

  M_ChangeMapMultisamling();

  render_stretch_hud = render_stretch_hud_default;

  M_ChangeMIDIPlayer();
}

//
// End of General Routines
//
/////////////////////////////////////////////////////////////////////////////
