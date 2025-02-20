//-----------------------------------------------------------------------------
//
// Copyright 2017 Christoph Oelckers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/
//
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "umapinfo.h"
#include "scanner.h"

extern "C"
{
#include "m_misc.h"
#include "g_game.h"
#include "doomdef.h"
#include "doomstat.h"

#include "dsda/episode.h"
#include "dsda/name.h"

MapList Maps;
}

// -----------------------------------------------
//
//
// -----------------------------------------------

static void FreeMap(MapEntry *mape)
{
	if (mape->mapname) Z_Free(mape->mapname);
	if (mape->levelname) Z_Free(mape->levelname);
	if (mape->label) Z_Free(mape->label);
	if (mape->author) Z_Free(mape->author);
	if (mape->intertext) Z_Free(mape->intertext);
	if (mape->intertextsecret) Z_Free(mape->intertextsecret);
	if (mape->properties) Z_Free(mape->properties);
	if (mape->bossactions) Z_Free(mape->bossactions);
	mape->propertycount = 0;
	mape->mapname = NULL;
	mape->properties = NULL;
}


void FreeMapList()
{
	unsigned i;

	for(i = 0; i < Maps.mapcount; i++)
	{
		FreeMap(&Maps.maps[i]);
	}
	Z_Free(Maps.maps);
	Maps.maps = NULL;
	Maps.mapcount = 0;
}


void ReplaceString(char **pptr, const char *newstring)
{
	if (*pptr != NULL) Z_Free(*pptr);
	*pptr = Z_Strdup(newstring);
}

// -----------------------------------------------
//
// Parses a set of string and concatenates them
//
// -----------------------------------------------

static char *ParseMultiString(Scanner &scanner, int error)
{
	char *build = NULL;

	if (scanner.CheckToken(TK_Identifier))
	{
		if (scanner.StringMatch("clear"))
		{
			return Z_Strdup("-");	// this was explicitly deleted to override the default.
		}
		else
		{
			scanner.ErrorF("Either 'clear' or string constant expected");
		}
	}

	do
	{
		scanner.MustGetToken(TK_StringConst);
		if (build == NULL) build = Z_Strdup(scanner.string);
		else
		{
			size_t newlen = strlen(build) + strlen(scanner.string) + 2; // strlen for both the existing text and the new line, plus room for one \n and one \0
			build = (char*)Z_Realloc(build, newlen); // Prepare the destination memory for the below strcats
			strcat(build, "\n"); // Replace the existing text's \0 terminator with a \n
			strcat(build, scanner.string); // Concatenate the new line onto the existing text
		}
	} while (scanner.CheckToken(','));
	return build;
}

// -----------------------------------------------
//
// Parses a lump name. The buffer must be at least 9 characters.
//
// -----------------------------------------------

static int ParseLumpName(Scanner &scanner, char *buffer)
{
	scanner.MustGetToken(TK_StringConst);
	if (strlen(scanner.string) > 8)
	{
		scanner.ErrorF("String too long. Maximum size is 8 characters.");
		return 0;
	}
	strncpy(buffer, scanner.string, 8);
	buffer[8] = 0;
	M_Strupr(buffer);
	return 1;
}

// -----------------------------------------------
//
// Parses a standard property that is already known
// These do not get stored in the property list
// but in dedicated struct member variables.
//
// -----------------------------------------------

static int ParseStandardProperty(Scanner &scanner, MapEntry *mape)
{
	// find the next line with content.
	// this line is no property.

	scanner.MustGetToken(TK_Identifier);
	char *pname = Z_Strdup(scanner.string);
	scanner.MustGetToken('=');

	if (!stricmp(pname, "levelname"))
	{
		scanner.MustGetToken(TK_StringConst);
		ReplaceString(&mape->levelname, scanner.string);
	}
	else if (!stricmp(pname, "label"))
	{
		if (scanner.CheckToken(TK_Identifier))
		{
			if (scanner.StringMatch("clear")) ReplaceString(&mape->label, "-");
			else
			{
				scanner.ErrorF("Either 'clear' or string constant expected");
				return 0;
			}
		}
		else
		{
			scanner.MustGetToken(TK_StringConst);
	                ReplaceString(&mape->label, scanner.string);
	        }
	}
	else if (!stricmp(pname, "author"))
	{
		scanner.MustGetToken(TK_StringConst);
		ReplaceString(&mape->author, scanner.string);
	}
	else if (!stricmp(pname, "next"))
	{
		ParseLumpName(scanner, mape->nextmap);
		if (!G_ValidateMapName(mape->nextmap, NULL, NULL))
		{
			scanner.ErrorF("Invalid map name %s.", mape->nextmap);
			return 0;
		}
	}
	else if (!stricmp(pname, "nextsecret"))
	{
		ParseLumpName(scanner, mape->nextsecret);
		if (!G_ValidateMapName(mape->nextsecret, NULL, NULL))
		{
			scanner.ErrorF("Invalid map name %s", mape->nextsecret);
			return 0;
		}
	}
	else if (!stricmp(pname, "levelpic"))
	{
		ParseLumpName(scanner, mape->levelpic);
	}
	else if (!stricmp(pname, "skytexture"))
	{
		ParseLumpName(scanner, mape->skytexture);
	}
	else if (!stricmp(pname, "music"))
	{
		ParseLumpName(scanner, mape->music);
	}
	else if (!stricmp(pname, "endpic"))
	{
		ParseLumpName(scanner, mape->endpic);
	}
	else if (!stricmp(pname, "endcast"))
	{
		scanner.MustGetToken(TK_BoolConst);
		if (scanner.boolean) strcpy(mape->endpic, "$CAST");
		else strcpy(mape->endpic, "-");
	}
	else if (!stricmp(pname, "endbunny"))
	{
		scanner.MustGetToken(TK_BoolConst);
		if (scanner.boolean) strcpy(mape->endpic, "$BUNNY");
		else strcpy(mape->endpic, "-");
	}
	else if (!stricmp(pname, "endgame"))
	{
		scanner.MustGetToken(TK_BoolConst);
		if (scanner.boolean) strcpy(mape->endpic, "!");
		else strcpy(mape->endpic, "-");
	}
	else if (!stricmp(pname, "exitpic"))
	{
		ParseLumpName(scanner, mape->exitpic);
	}
	else if (!stricmp(pname, "enterpic"))
	{
		ParseLumpName(scanner, mape->enterpic);
	}
	else if (!stricmp(pname, "nointermission"))
	{
		scanner.MustGetToken(TK_BoolConst);
		mape->nointermission = scanner.boolean;
	}
	else if (!stricmp(pname, "partime"))
	{
		scanner.MustGetInteger();
		mape->partime = TICRATE * scanner.number;
	}
	else if (!stricmp(pname, "intertext"))
	{
		char *lname = ParseMultiString(scanner, 1);
		if (!lname) return 0;
		if (mape->intertext != NULL) Z_Free(mape->intertext);
		mape->intertext = lname;
	}
	else if (!stricmp(pname, "intertextsecret"))
	{
		char *lname = ParseMultiString(scanner, 1);
		if (!lname) return 0;
		if (mape->intertextsecret != NULL) Z_Free(mape->intertextsecret);
		mape->intertextsecret = lname;
	}
	else if (!stricmp(pname, "interbackdrop"))
	{
		ParseLumpName(scanner, mape->interbackdrop);
	}
	else if (!stricmp(pname, "intermusic"))
	{
		ParseLumpName(scanner, mape->intermusic);
	}
	else if (!stricmp(pname, "episode"))
	{
		if (scanner.CheckToken(TK_Identifier))
		{
			if (scanner.StringMatch("clear")) dsda_ClearEpisodes();
			else
			{
				scanner.ErrorF("Either 'clear' or string constant expected");
				return 0;
			}
		}
		else
		{
			char lumpname[9] = {0};
			char *alttext = NULL;
			char key = 0;

			ParseLumpName(scanner, lumpname);
			if (scanner.CheckToken(','))
			{
				scanner.MustGetToken(TK_StringConst);
				alttext = Z_Strdup(scanner.string);
				if (scanner.CheckToken(','))
				{
					scanner.MustGetToken(TK_StringConst);
					key = tolower(scanner.string[0]);
				}
			}

			dsda_AddEpisode(mape->mapname, alttext, lumpname, key, false);

			if (alttext) Z_Free(alttext);
		}
	}
	else if (!stricmp(pname, "bossaction"))
	{
		scanner.MustGetToken(TK_Identifier);
		int special, tag;
		if (scanner.StringMatch("clear"))
		{
			// mark level free of boss actions
			special = tag = -1;
			if (mape->bossactions) Z_Free(mape->bossactions);
			mape->bossactions = NULL;
			mape->numbossactions = -1;
		}
		else
		{
			int i;

			i = dsda_ActorNameToType(scanner.string);

			if (i == NAME_NOT_FOUND)
			{
				scanner.ErrorF("Unknown thing type %s", scanner.string);
				return 0;
			}

			scanner.MustGetToken(',');
			scanner.MustGetInteger();
			special = scanner.number;
			scanner.MustGetToken(',');
			scanner.MustGetInteger();
			tag = scanner.number;
			// allow no 0-tag specials here, unless a level exit.
			if (tag != 0 || special == 11 || special == 51 || special == 52 || special == 124)
			{
				if (mape->numbossactions == -1) mape->numbossactions = 1;
				else mape->numbossactions++;
				mape->bossactions = (struct BossAction *)Z_Realloc(mape->bossactions, sizeof(struct BossAction) * mape->numbossactions);
				mape->bossactions[mape->numbossactions - 1].type = i;
				mape->bossactions[mape->numbossactions - 1].special = special;
				mape->bossactions[mape->numbossactions - 1].tag = tag;
			}

		}
	}
	else
	{
		do
		{
			if (!scanner.CheckFloat()) scanner.GetNextToken();
			if (scanner.token > TK_BoolConst)
			{
				scanner.Error(TK_Identifier);
			}

		} while (scanner.CheckToken(','));
	}
	Z_Free(pname);
	return 1;
}

// -----------------------------------------------
//
// Parses a complete map entry
//
// -----------------------------------------------

static int ParseMapEntry(Scanner &scanner, MapEntry *val)
{
	val->mapname = NULL;
	val->propertycount = 0;
	val->properties = NULL;

	scanner.MustGetIdentifier("map");
	scanner.MustGetToken(TK_Identifier);
	if (!G_ValidateMapName(scanner.string, NULL, NULL))
	{
		scanner.ErrorF("Invalid map name %s", scanner.string);
		return 0;
	}

	ReplaceString(&val->mapname, scanner.string);
	scanner.MustGetToken('{');
	while(!scanner.CheckToken('}'))
	{
		ParseStandardProperty(scanner, val);
	}
	return 1;
}

// -----------------------------------------------
//
// Parses a complete UMAPINFO lump
//
// -----------------------------------------------

int ParseUMapInfo(const unsigned char *buffer, size_t length, umapinfo_errorfunc err)
{
	Scanner scanner((const char*)buffer, length);
	unsigned int i;

	scanner.SetErrorCallback(err);


	while (scanner.TokensLeft())
	{
		MapEntry parsed = { 0 };
		ParseMapEntry(scanner, &parsed);

		// Set default level progression here to simplify the checks elsewhere. Doing this lets us skip all normal code for this if nothing has been defined.
		if (parsed.endpic[0] && (strcmp(parsed.endpic, "-") != 0))
		{
			parsed.nextmap[0] = 0;
		}
		else if (!parsed.nextmap[0] && !parsed.endpic[0])
		{
			if (!stricmp(parsed.mapname, "MAP30")) strcpy(parsed.endpic, "$CAST");
			else if (!stricmp(parsed.mapname, "E1M8"))  strcpy(parsed.endpic, gamemode == retail? "CREDIT" : "HELP2");
			else if (!stricmp(parsed.mapname, "E2M8"))  strcpy(parsed.endpic, "VICTORY2");
			else if (!stricmp(parsed.mapname, "E3M8"))  strcpy(parsed.endpic, "$BUNNY");
			else if (!stricmp(parsed.mapname, "E4M8"))  strcpy(parsed.endpic, "ENDPIC");
			else if (gamemission == tc_chex && !stricmp(parsed.mapname, "E1M5"))  strcpy(parsed.endpic, "CREDIT");
			else
			{
				int ep, map;
				G_ValidateMapName(parsed.mapname, &ep, &map);
				map++;
				sprintf(parsed.nextmap, "%s", VANILLA_MAP_LUMP_NAME(ep, map));
			}
		}

		// Does this property already exist? If yes, replace it.
		for(i = 0; i < Maps.mapcount; i++)
		{
			if (!strcmp(parsed.mapname, Maps.maps[i].mapname))
			{
				FreeMap(&Maps.maps[i]);
				Maps.maps[i] = parsed;
				break;
			}
		}
		// Not found so create a new one.
		if (i == Maps.mapcount)
		{
			Maps.mapcount++;
			Maps.maps = (MapEntry*)Z_Realloc(Maps.maps, sizeof(MapEntry)*Maps.mapcount);
			Maps.maps[Maps.mapcount-1] = parsed;
		}

	}
	return 1;
}


MapProperty *FindProperty(MapEntry *map, const char *name)
{
	return NULL;
}
