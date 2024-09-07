//
// Copyright(C) 2022 by Ryan Krafnick
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
//	DSDA UDMF
//

#include <cstring>
#include <vector>

extern "C" {
char *Z_StrdupLevel(const char *s);
void *Z_MallocLevel(size_t size);
}

#include "scanner.h"

#include "udmf.h"

std::vector<udmf_line_t> udmf_lines;
std::vector<udmf_side_t> udmf_sides;
std::vector<udmf_vertex_t> udmf_vertices;
std::vector<udmf_sector_t> udmf_sectors;
std::vector<udmf_thing_t> udmf_things;

static void dsda_SkipValue(Scanner &scanner) {
  if (scanner.CheckToken('=')) {
    while (scanner.TokensLeft()) {
      if (scanner.CheckToken(';'))
        break;

      scanner.GetNextToken();
    }

    return;
  }

  scanner.MustGetToken('{');
  {
    int brace_count = 1;

    while (scanner.TokensLeft()) {
      if (scanner.CheckToken('}')) {
        --brace_count;
      }
      else if (scanner.CheckToken('{')) {
        ++brace_count;
      }

      if (!brace_count)
        break;

      scanner.GetNextToken();
    }

    return;
  }
}

// The scanner drops the sign when scanning, and we need it back
static char* dsda_FloatString(Scanner &scanner) {
  if (scanner.decimal >= 0)
    return Z_StrdupLevel(scanner.string);

  char* buffer = (char*) Z_MallocLevel(strlen(scanner.string) + 2);
  buffer[0] = '-';
  buffer[1] = '\0';
  strcat(buffer, scanner.string);

  return buffer;
}

#define SCAN_INT(x)  { scanner.MustGetToken('='); \
                       scanner.MustGetInteger(); \
                       x = scanner.number; \
                       scanner.MustGetToken(';'); }

#define SCAN_FLOAT(x) { scanner.MustGetToken('='); \
                        scanner.MustGetFloat(); \
                        x = scanner.decimal; \
                        scanner.MustGetToken(';'); }

#define SCAN_FLAG(x, f) { scanner.MustGetToken('='); \
                          scanner.MustGetToken(TK_BoolConst); \
                          if (scanner.boolean) \
                            x |= f; \
                          scanner.MustGetToken(';'); }

#define SCAN_STRING_N(x, n) { scanner.MustGetToken('='); \
                              scanner.MustGetToken(TK_StringConst); \
                              strncpy(x, scanner.string, n); \
                              scanner.MustGetToken(';'); }

#define SCAN_STRING(x) { scanner.MustGetToken('='); \
                         scanner.MustGetToken(TK_StringConst); \
                         x = Z_StrdupLevel(scanner.string); \
                         scanner.MustGetToken(';'); }

#define SCAN_FLOAT_STRING(x) { scanner.MustGetToken('='); \
                               scanner.MustGetFloat(); \
                               x = dsda_FloatString(scanner); \
                               scanner.MustGetToken(';'); }

static void dsda_ParseUDMFLineDef(Scanner &scanner) {
  udmf_line_t line = { 0 };

  line.id = -1;
  line.sideback = -1;
  line.alpha = 1.0;

  scanner.MustGetToken('{');
  while (!scanner.CheckToken('}')) {
    scanner.MustGetToken(TK_Identifier);

    if (scanner.StringMatch("id")) {
      SCAN_INT(line.id);
    }
    else if (scanner.StringMatch("v1")) {
      SCAN_INT(line.v1);
    }
    else if (scanner.StringMatch("v2")) {
      SCAN_INT(line.v2);
    }
    else if (scanner.StringMatch("special")) {
      SCAN_INT(line.special);
    }
    else if (scanner.StringMatch("arg0")) {
      SCAN_INT(line.arg0);
    }
    else if (scanner.StringMatch("arg1")) {
      SCAN_INT(line.arg1);
    }
    else if (scanner.StringMatch("arg2")) {
      SCAN_INT(line.arg2);
    }
    else if (scanner.StringMatch("arg3")) {
      SCAN_INT(line.arg3);
    }
    else if (scanner.StringMatch("arg4")) {
      SCAN_INT(line.arg4);
    }
    else if (scanner.StringMatch("sidefront")) {
      SCAN_INT(line.sidefront);
    }
    else if (scanner.StringMatch("sideback")) {
      SCAN_INT(line.sideback);
    }
    else if (scanner.StringMatch("locknumber")) {
      SCAN_INT(line.locknumber);
    }
    else if (scanner.StringMatch("automapstyle")) {
      SCAN_INT(line.automapstyle);
    }
    else if (scanner.StringMatch("health")) {
      SCAN_INT(line.health);
    }
    else if (scanner.StringMatch("healthgroup")) {
      SCAN_INT(line.healthgroup);
    }
    else if (scanner.StringMatch("alpha")) {
      SCAN_FLOAT(line.alpha);
    }
    else if (scanner.StringMatch("blocking")) {
      SCAN_FLAG(line.flags, UDMF_ML_BLOCKING);
    }
    else if (scanner.StringMatch("blockmonsters")) {
      SCAN_FLAG(line.flags, UDMF_ML_BLOCKMONSTERS);
    }
    else if (scanner.StringMatch("twosided")) {
      SCAN_FLAG(line.flags, UDMF_ML_TWOSIDED);
    }
    else if (scanner.StringMatch("dontpegtop")) {
      SCAN_FLAG(line.flags, UDMF_ML_DONTPEGTOP);
    }
    else if (scanner.StringMatch("dontpegbottom")) {
      SCAN_FLAG(line.flags, UDMF_ML_DONTPEGBOTTOM);
    }
    else if (scanner.StringMatch("secret")) {
      SCAN_FLAG(line.flags, UDMF_ML_SECRET);
    }
    else if (scanner.StringMatch("blocksound")) {
      SCAN_FLAG(line.flags, UDMF_ML_SOUNDBLOCK);
    }
    else if (scanner.StringMatch("dontdraw")) {
      SCAN_FLAG(line.flags, UDMF_ML_DONTDRAW);
    }
    else if (scanner.StringMatch("mapped")) {
      SCAN_FLAG(line.flags, UDMF_ML_MAPPED);
    }
    else if (scanner.StringMatch("passuse")) {
      SCAN_FLAG(line.flags, UDMF_ML_PASSUSE);
    }
    else if (scanner.StringMatch("translucent")) {
      SCAN_FLAG(line.flags, UDMF_ML_TRANSLUCENT);
    }
    else if (scanner.StringMatch("jumpover")) {
      SCAN_FLAG(line.flags, UDMF_ML_JUMPOVER);
    }
    else if (scanner.StringMatch("blockfloaters")) {
      SCAN_FLAG(line.flags, UDMF_ML_BLOCKFLOATERS);
    }
    else if (scanner.StringMatch("playercross")) {
      SCAN_FLAG(line.flags, UDMF_ML_PLAYERCROSS);
    }
    else if (scanner.StringMatch("playeruse")) {
      SCAN_FLAG(line.flags, UDMF_ML_PLAYERUSE);
    }
    else if (scanner.StringMatch("monstercross")) {
      SCAN_FLAG(line.flags, UDMF_ML_MONSTERCROSS);
    }
    else if (scanner.StringMatch("monsteruse")) {
      SCAN_FLAG(line.flags, UDMF_ML_MONSTERUSE);
    }
    else if (scanner.StringMatch("impact")) {
      SCAN_FLAG(line.flags, UDMF_ML_IMPACT);
    }
    else if (scanner.StringMatch("playerpush")) {
      SCAN_FLAG(line.flags, UDMF_ML_PLAYERPUSH);
    }
    else if (scanner.StringMatch("monsterpush")) {
      SCAN_FLAG(line.flags, UDMF_ML_MONSTERPUSH);
    }
    else if (scanner.StringMatch("missilecross")) {
      SCAN_FLAG(line.flags, UDMF_ML_MISSILECROSS);
    }
    else if (scanner.StringMatch("repeatspecial")) {
      SCAN_FLAG(line.flags, UDMF_ML_REPEATSPECIAL);
    }
    else if (scanner.StringMatch("playeruseback")) {
      SCAN_FLAG(line.flags, UDMF_ML_PLAYERUSEBACK);
    }
    else if (scanner.StringMatch("anycross")) {
      SCAN_FLAG(line.flags, UDMF_ML_ANYCROSS);
    }
    else if (scanner.StringMatch("monsteractivate")) {
      SCAN_FLAG(line.flags, UDMF_ML_MONSTERACTIVATE);
    }
    else if (scanner.StringMatch("blockplayers")) {
      SCAN_FLAG(line.flags, UDMF_ML_BLOCKPLAYERS);
    }
    else if (scanner.StringMatch("blockeverything")) {
      SCAN_FLAG(line.flags, UDMF_ML_BLOCKEVERYTHING);
    }
    else if (scanner.StringMatch("firstsideonly")) {
      SCAN_FLAG(line.flags, UDMF_ML_FIRSTSIDEONLY);
    }
    else if (scanner.StringMatch("zoneboundary")) {
      SCAN_FLAG(line.flags, UDMF_ML_ZONEBOUNDARY);
    }
    else if (scanner.StringMatch("clipmidtex")) {
      SCAN_FLAG(line.flags, UDMF_ML_CLIPMIDTEX);
    }
    else if (scanner.StringMatch("wrapmidtex")) {
      SCAN_FLAG(line.flags, UDMF_ML_WRAPMIDTEX);
    }
    else if (scanner.StringMatch("midtex3d")) {
      SCAN_FLAG(line.flags, UDMF_ML_MIDTEX3D);
    }
    else if (scanner.StringMatch("midtex3dimpassible")) {
      SCAN_FLAG(line.flags, UDMF_ML_MIDTEX3DIMPASSIBLE);
    }
    else if (scanner.StringMatch("checkswitchrange")) {
      SCAN_FLAG(line.flags, UDMF_ML_CHECKSWITCHRANGE);
    }
    else if (scanner.StringMatch("blockprojectiles")) {
      SCAN_FLAG(line.flags, UDMF_ML_BLOCKPROJECTILES);
    }
    else if (scanner.StringMatch("blockuse")) {
      SCAN_FLAG(line.flags, UDMF_ML_BLOCKUSE);
    }
    else if (scanner.StringMatch("blocksight")) {
      SCAN_FLAG(line.flags, UDMF_ML_BLOCKSIGHT);
    }
    else if (scanner.StringMatch("blockhitscan")) {
      SCAN_FLAG(line.flags, UDMF_ML_BLOCKHITSCAN);
    }
    else if (scanner.StringMatch("transparent")) {
      SCAN_FLAG(line.flags, UDMF_ML_TRANSPARENT);
    }
    else if (scanner.StringMatch("revealed")) {
      SCAN_FLAG(line.flags, UDMF_ML_REVEALED);
    }
    else if (scanner.StringMatch("noskywalls")) {
      SCAN_FLAG(line.flags, UDMF_ML_NOSKYWALLS);
    }
    else if (scanner.StringMatch("drawfullheight")) {
      SCAN_FLAG(line.flags, UDMF_ML_DRAWFULLHEIGHT);
    }
    else if (scanner.StringMatch("damagespecial")) {
      SCAN_FLAG(line.flags, UDMF_ML_DAMAGESPECIAL);
    }
    else if (scanner.StringMatch("deathspecial")) {
      SCAN_FLAG(line.flags, UDMF_ML_DEATHSPECIAL);
    }
    else if (scanner.StringMatch("blocklandmonsters")) {
      SCAN_FLAG(line.flags, UDMF_ML_BLOCKLANDMONSTERS);
    }
    else if (scanner.StringMatch("moreids")) {
      SCAN_STRING(line.moreids);
    }
    else if (scanner.StringMatch("arg0str")) {
      SCAN_STRING(line.arg0str);
    }
    else {
      dsda_SkipValue(scanner);
    }
  }

  udmf_lines.push_back(line);
}

static void dsda_ParseUDMFSideDef(Scanner &scanner) {
  udmf_side_t side = { 0 };

  side.texturetop[0] = '-';
  side.texturebottom[0] = '-';
  side.texturemiddle[0] = '-';
  side.scalex_top = 1.f;
  side.scaley_top = 1.f;
  side.scalex_mid = 1.f;
  side.scaley_mid = 1.f;
  side.scalex_bottom = 1.f;
  side.scaley_bottom = 1.f;

  scanner.MustGetToken('{');
  while (!scanner.CheckToken('}')) {
    scanner.MustGetToken(TK_Identifier);

    if (scanner.StringMatch("offsetx")) {
      SCAN_INT(side.offsetx);
    }
    else if (scanner.StringMatch("offsety")) {
      SCAN_INT(side.offsety);
    }
    else if (scanner.StringMatch("sector")) {
      SCAN_INT(side.sector);
    }
    else if (scanner.StringMatch("light")) {
      SCAN_INT(side.light);
    }
    else if (scanner.StringMatch("light_top")) {
      SCAN_INT(side.light_top);
    }
    else if (scanner.StringMatch("light_mid")) {
      SCAN_INT(side.light_mid);
    }
    else if (scanner.StringMatch("light_bottom")) {
      SCAN_INT(side.light_bottom);
    }
    else if (scanner.StringMatch("scalex_top")) {
      SCAN_FLOAT(side.scalex_top);
    }
    else if (scanner.StringMatch("scaley_top")) {
      SCAN_FLOAT(side.scaley_top);
    }
    else if (scanner.StringMatch("scalex_mid")) {
      SCAN_FLOAT(side.scalex_mid);
    }
    else if (scanner.StringMatch("scaley_mid")) {
      SCAN_FLOAT(side.scaley_mid);
    }
    else if (scanner.StringMatch("scalex_bottom")) {
      SCAN_FLOAT(side.scalex_bottom);
    }
    else if (scanner.StringMatch("scaley_bottom")) {
      SCAN_FLOAT(side.scaley_bottom);
    }
    else if (scanner.StringMatch("offsetx_top")) {
      SCAN_FLOAT(side.offsetx_top);
    }
    else if (scanner.StringMatch("offsety_top")) {
      SCAN_FLOAT(side.offsety_top);
    }
    else if (scanner.StringMatch("offsetx_mid")) {
      SCAN_FLOAT(side.offsetx_mid);
    }
    else if (scanner.StringMatch("offsety_mid")) {
      SCAN_FLOAT(side.offsety_mid);
    }
    else if (scanner.StringMatch("offsetx_bottom")) {
      SCAN_FLOAT(side.offsetx_bottom);
    }
    else if (scanner.StringMatch("xscroll")) {
      SCAN_FLOAT(side.xscroll);
    }
    else if (scanner.StringMatch("yscroll")) {
      SCAN_FLOAT(side.yscroll);
    }
    else if (scanner.StringMatch("xscrolltop")) {
      SCAN_FLOAT(side.xscrolltop);
    }
    else if (scanner.StringMatch("yscrolltop")) {
      SCAN_FLOAT(side.yscrolltop);
    }
    else if (scanner.StringMatch("xscrollmid")) {
      SCAN_FLOAT(side.xscrollmid);
    }
    else if (scanner.StringMatch("yscrollmid")) {
      SCAN_FLOAT(side.yscrollmid);
    }
    else if (scanner.StringMatch("xscrollbottom")) {
      SCAN_FLOAT(side.xscrollbottom);
    }
    else if (scanner.StringMatch("yscrollbottom")) {
      SCAN_FLOAT(side.yscrollbottom);
    }
    else if (scanner.StringMatch("offsety_bottom")) {
      SCAN_FLOAT(side.offsety_bottom);
    }
    else if (scanner.StringMatch("lightabsolute")) {
      SCAN_FLAG(side.flags, UDMF_SF_LIGHTABSOLUTE);
    }
    else if (scanner.StringMatch("lightfog")) {
      SCAN_FLAG(side.flags, UDMF_SF_LIGHTFOG);
    }
    else if (scanner.StringMatch("nofakecontrast")) {
      SCAN_FLAG(side.flags, UDMF_SF_NOFAKECONTRAST);
    }
    else if (scanner.StringMatch("smoothlighting")) {
      SCAN_FLAG(side.flags, UDMF_SF_SMOOTHLIGHTING);
    }
    else if (scanner.StringMatch("clipmidtex")) {
      SCAN_FLAG(side.flags, UDMF_SF_CLIPMIDTEX);
    }
    else if (scanner.StringMatch("wrapmidtex")) {
      SCAN_FLAG(side.flags, UDMF_SF_WRAPMIDTEX);
    }
    else if (scanner.StringMatch("nodecals")) {
      SCAN_FLAG(side.flags, UDMF_SF_NODECALS);
    }
    else if (scanner.StringMatch("lightabsolute_top")) {
      SCAN_FLAG(side.flags, UDMF_SF_LIGHTABSOLUTETOP);
    }
    else if (scanner.StringMatch("lightabsolute_mid")) {
      SCAN_FLAG(side.flags, UDMF_SF_LIGHTABSOLUTEMID);
    }
    else if (scanner.StringMatch("lightabsolute_bottom")) {
      SCAN_FLAG(side.flags, UDMF_SF_LIGHTABSOLUTEBOTTOM);
    }
    else if (scanner.StringMatch("texturetop")) {
      SCAN_STRING_N(side.texturetop, 8);
    }
    else if (scanner.StringMatch("texturebottom")) {
      SCAN_STRING_N(side.texturebottom, 8);
    }
    else if (scanner.StringMatch("texturemiddle")) {
      SCAN_STRING_N(side.texturemiddle, 8);
    }
    else {
      dsda_SkipValue(scanner);
    }
  }

  udmf_sides.push_back(side);
}

static void dsda_ParseUDMFVertex(Scanner &scanner) {
  udmf_vertex_t vertex = { 0 };

  scanner.MustGetToken('{');
  while (!scanner.CheckToken('}')) {
    scanner.MustGetToken(TK_Identifier);

    if (scanner.StringMatch("x")) {
      SCAN_FLOAT_STRING(vertex.x);
    }
    else if (scanner.StringMatch("y")) {
      SCAN_FLOAT_STRING(vertex.y);
    }
    else {
      dsda_SkipValue(scanner);
    }
  }

  udmf_vertices.push_back(vertex);
}

static void dsda_ParseUDMFSector(Scanner &scanner) {
  udmf_sector_t sector = { 0 };

  sector.lightlevel = 160;
  sector.xscalefloor = 1.f;
  sector.yscalefloor = 1.f;
  sector.xscaleceiling = 1.f;
  sector.yscaleceiling = 1.f;
  sector.gravity = "1.0";
  sector.damageinterval = 32;

  scanner.MustGetToken('{');
  while (!scanner.CheckToken('}')) {
    scanner.MustGetToken(TK_Identifier);

    if (scanner.StringMatch("heightfloor")) {
      SCAN_INT(sector.heightfloor);
    }
    else if (scanner.StringMatch("heightceiling")) {
      SCAN_INT(sector.heightceiling);
    }
    else if (scanner.StringMatch("lightlevel")) {
      SCAN_INT(sector.lightlevel);
    }
    else if (scanner.StringMatch("special")) {
      SCAN_INT(sector.special);
    }
    else if (scanner.StringMatch("id")) {
      SCAN_INT(sector.id);
    }
    else if (scanner.StringMatch("lightfloor")) {
      SCAN_INT(sector.lightfloor);
    }
    else if (scanner.StringMatch("lightceiling")) {
      SCAN_INT(sector.lightceiling);
    }
    else if (scanner.StringMatch("damageamount")) {
      SCAN_INT(sector.damageamount);
    }
    else if (scanner.StringMatch("damageinterval")) {
      SCAN_INT(sector.damageinterval);
    }
    else if (scanner.StringMatch("leakiness")) {
      SCAN_INT(sector.leakiness);
    }
    else if (scanner.StringMatch("xpanningfloor")) {
      SCAN_FLOAT(sector.xpanningfloor);
    }
    else if (scanner.StringMatch("ypanningfloor")) {
      SCAN_FLOAT(sector.ypanningfloor);
    }
    else if (scanner.StringMatch("xpanningceiling")) {
      SCAN_FLOAT(sector.xpanningceiling);
    }
    else if (scanner.StringMatch("ypanningceiling")) {
      SCAN_FLOAT(sector.ypanningceiling);
    }
    else if (scanner.StringMatch("xscalefloor")) {
      SCAN_FLOAT(sector.xscalefloor);
    }
    else if (scanner.StringMatch("yscalefloor")) {
      SCAN_FLOAT(sector.yscalefloor);
    }
    else if (scanner.StringMatch("xscaleceiling")) {
      SCAN_FLOAT(sector.xscaleceiling);
    }
    else if (scanner.StringMatch("yscaleceiling")) {
      SCAN_FLOAT(sector.yscaleceiling);
    }
    else if (scanner.StringMatch("rotationfloor")) {
      SCAN_FLOAT(sector.rotationfloor);
    }
    else if (scanner.StringMatch("rotationceiling")) {
      SCAN_FLOAT(sector.rotationceiling);
    }
    else if (scanner.StringMatch("xscrollfloor")) {
      SCAN_FLOAT(sector.xscrollfloor);
    }
    else if (scanner.StringMatch("yscrollfloor")) {
      SCAN_FLOAT(sector.yscrollfloor);
    }
    else if (scanner.StringMatch("scrollfloormode")) {
      SCAN_INT(sector.scrollfloormode);
    }
    else if (scanner.StringMatch("xscrollceiling")) {
      SCAN_FLOAT(sector.xscrollceiling);
    }
    else if (scanner.StringMatch("yscrollceiling")) {
      SCAN_FLOAT(sector.yscrollceiling);
    }
    else if (scanner.StringMatch("scrollceilingmode")) {
      SCAN_INT(sector.scrollceilingmode);
    }
    else if (scanner.StringMatch("xthrust")) {
      SCAN_FLOAT_STRING(sector.xthrust);
    }
    else if (scanner.StringMatch("ythrust")) {
      SCAN_FLOAT_STRING(sector.ythrust);
    }
    else if (scanner.StringMatch("thrustgroup")) {
      SCAN_INT(sector.thrustgroup);
    }
    else if (scanner.StringMatch("thrustlocation")) {
      SCAN_INT(sector.thrustlocation);
    }
    else if (scanner.StringMatch("gravity")) {
      SCAN_FLOAT_STRING(sector.gravity);
    }
    else if (scanner.StringMatch("frictionfactor")) {
      SCAN_FLOAT_STRING(sector.frictionfactor);
    }
    else if (scanner.StringMatch("movefactor")) {
      SCAN_FLOAT_STRING(sector.movefactor);
    }
    else if (scanner.StringMatch("lightfloorabsolute")) {
      SCAN_FLAG(sector.flags, UDMF_SECF_LIGHTFLOORABSOLUTE);
    }
    else if (scanner.StringMatch("lightceilingabsolute")) {
      SCAN_FLAG(sector.flags, UDMF_SECF_LIGHTCEILINGABSOLUTE);
    }
    else if (scanner.StringMatch("silent")) {
      SCAN_FLAG(sector.flags, UDMF_SECF_SILENT);
    }
    else if (scanner.StringMatch("nofallingdamage")) {
      SCAN_FLAG(sector.flags, UDMF_SECF_NOFALLINGDAMAGE);
    }
    else if (scanner.StringMatch("dropactors")) {
      SCAN_FLAG(sector.flags, UDMF_SECF_DROPACTORS);
    }
    else if (scanner.StringMatch("norespawn")) {
      SCAN_FLAG(sector.flags, UDMF_SECF_NORESPAWN);
    }
    else if (scanner.StringMatch("hidden")) {
      SCAN_FLAG(sector.flags, UDMF_SECF_HIDDEN);
    }
    else if (scanner.StringMatch("waterzone")) {
      SCAN_FLAG(sector.flags, UDMF_SECF_WATERZONE);
    }
    else if (scanner.StringMatch("damageterraineffect")) {
      SCAN_FLAG(sector.flags, UDMF_SECF_DAMAGETERRAINEFFECT);
    }
    else if (scanner.StringMatch("damagehazard")) {
      SCAN_FLAG(sector.flags, UDMF_SECF_DAMAGEHAZARD);
    }
    else if (scanner.StringMatch("noattack")) {
      SCAN_FLAG(sector.flags, UDMF_SECF_NOATTACK);
    }
    else if (scanner.StringMatch("texturefloor")) {
      SCAN_STRING_N(sector.texturefloor, 8);
    }
    else if (scanner.StringMatch("textureceiling")) {
      SCAN_STRING_N(sector.textureceiling, 8);
    }
    else if (scanner.StringMatch("colormap")) {
      SCAN_STRING(sector.colormap);
    }
    else if (scanner.StringMatch("skyfloor")) {
      SCAN_STRING(sector.skyfloor);
    }
    else if (scanner.StringMatch("skyceiling")) {
      SCAN_STRING(sector.skyceiling);
    }
    else if (scanner.StringMatch("moreids")) {
      SCAN_STRING(sector.moreids);
    }
    else {
      dsda_SkipValue(scanner);
    }
  }

  udmf_sectors.push_back(sector);
}

static void dsda_ParseUDMFThing(Scanner &scanner) {
  udmf_thing_t thing = { 0 };

  thing.gravity = "1.0";
  thing.health = "1.0";
  thing.floatbobphase = -1;
  thing.alpha = 1.0;

  scanner.MustGetToken('{');
  while (!scanner.CheckToken('}')) {
    scanner.MustGetToken(TK_Identifier);

    if (scanner.StringMatch("id")) {
      SCAN_INT(thing.id);
    }
    else if (scanner.StringMatch("angle")) {
      SCAN_INT(thing.angle);
    }
    else if (scanner.StringMatch("type")) {
      SCAN_INT(thing.type);
    }
    else if (scanner.StringMatch("special")) {
      SCAN_INT(thing.special);
    }
    else if (scanner.StringMatch("arg0")) {
      SCAN_INT(thing.arg0);
    }
    else if (scanner.StringMatch("arg1")) {
      SCAN_INT(thing.arg1);
    }
    else if (scanner.StringMatch("arg2")) {
      SCAN_INT(thing.arg2);
    }
    else if (scanner.StringMatch("arg3")) {
      SCAN_INT(thing.arg3);
    }
    else if (scanner.StringMatch("arg4")) {
      SCAN_INT(thing.arg4);
    }
    else if (scanner.StringMatch("floatbobphase")) {
      SCAN_INT(thing.floatbobphase);
    }
    else if (scanner.StringMatch("x")) {
      SCAN_FLOAT_STRING(thing.x);
    }
    else if (scanner.StringMatch("y")) {
      SCAN_FLOAT_STRING(thing.y);
    }
    else if (scanner.StringMatch("height")) {
      SCAN_FLOAT_STRING(thing.height);
    }
    else if (scanner.StringMatch("gravity")) {
      SCAN_FLOAT_STRING(thing.gravity);
    }
    else if (scanner.StringMatch("health")) {
      SCAN_FLOAT_STRING(thing.health);
    }
    else if (scanner.StringMatch("scalex")) {
      SCAN_FLOAT(thing.scalex);
    }
    else if (scanner.StringMatch("scaley")) {
      SCAN_FLOAT(thing.scaley);
    }
    else if (scanner.StringMatch("scale")) {
      SCAN_FLOAT(thing.scale);
    }
    else if (scanner.StringMatch("alpha")) {
      SCAN_FLOAT(thing.alpha);
    }
    else if (scanner.StringMatch("skill1")) {
      SCAN_FLAG(thing.flags, UDMF_TF_SKILL1);
    }
    else if (scanner.StringMatch("skill2")) {
      SCAN_FLAG(thing.flags, UDMF_TF_SKILL2);
    }
    else if (scanner.StringMatch("skill3")) {
      SCAN_FLAG(thing.flags, UDMF_TF_SKILL3);
    }
    else if (scanner.StringMatch("skill4")) {
      SCAN_FLAG(thing.flags, UDMF_TF_SKILL4);
    }
    else if (scanner.StringMatch("skill5")) {
      SCAN_FLAG(thing.flags, UDMF_TF_SKILL5);
    }
    else if (scanner.StringMatch("ambush")) {
      SCAN_FLAG(thing.flags, UDMF_TF_AMBUSH);
    }
    else if (scanner.StringMatch("single")) {
      SCAN_FLAG(thing.flags, UDMF_TF_SINGLE);
    }
    else if (scanner.StringMatch("dm")) {
      SCAN_FLAG(thing.flags, UDMF_TF_DM);
    }
    else if (scanner.StringMatch("coop")) {
      SCAN_FLAG(thing.flags, UDMF_TF_COOP);
    }
    else if (scanner.StringMatch("friend")) {
      SCAN_FLAG(thing.flags, UDMF_TF_FRIEND);
    }
    else if (scanner.StringMatch("dormant")) {
      SCAN_FLAG(thing.flags, UDMF_TF_DORMANT);
    }
    else if (scanner.StringMatch("class1")) {
      SCAN_FLAG(thing.flags, UDMF_TF_CLASS1);
    }
    else if (scanner.StringMatch("class2")) {
      SCAN_FLAG(thing.flags, UDMF_TF_CLASS2);
    }
    else if (scanner.StringMatch("class3")) {
      SCAN_FLAG(thing.flags, UDMF_TF_CLASS3);
    }
    else if (scanner.StringMatch("standing")) {
      SCAN_FLAG(thing.flags, UDMF_TF_STANDING);
    }
    else if (scanner.StringMatch("strifeally")) {
      SCAN_FLAG(thing.flags, UDMF_TF_STRIFEALLY);
    }
    else if (scanner.StringMatch("translucent")) {
      SCAN_FLAG(thing.flags, UDMF_TF_TRANSLUCENT);
    }
    else if (scanner.StringMatch("invisible")) {
      SCAN_FLAG(thing.flags, UDMF_TF_INVISIBLE);
    }
    else if (scanner.StringMatch("countsecret")) {
      SCAN_FLAG(thing.flags, UDMF_TF_COUNTSECRET);
    }
    else if (scanner.StringMatch("arg0str")) {
      SCAN_STRING(thing.arg0str);
    }
    else {
      dsda_SkipValue(scanner);
    }
  }

  udmf_things.push_back(thing);
}

static void dsda_ParseUDMFIdentifier(Scanner &scanner) {
  scanner.MustGetToken(TK_Identifier);

  if (scanner.StringMatch("namespace")) {
    scanner.MustGetToken('=');
    scanner.MustGetToken(TK_StringConst);

    if (stricmp(scanner.string, "zdoom") && stricmp(scanner.string, "dsda"))
      scanner.ErrorF("Unknown UDMF namespace \"%s\"", scanner.string);

    scanner.MustGetToken(';');
  }
  else if (scanner.StringMatch("linedef")) {
    dsda_ParseUDMFLineDef(scanner);
  }
  else if (scanner.StringMatch("sidedef")) {
    dsda_ParseUDMFSideDef(scanner);
  }
  else if (scanner.StringMatch("vertex")) {
    dsda_ParseUDMFVertex(scanner);
  }
  else if (scanner.StringMatch("sector")) {
    dsda_ParseUDMFSector(scanner);
  }
  else if (scanner.StringMatch("thing")) {
    dsda_ParseUDMFThing(scanner);
  }
  else {
    dsda_SkipValue(scanner);
  }
}

udmf_t udmf;

void dsda_ParseUDMF(const unsigned char* buffer, size_t length, udmf_errorfunc err) {
  Scanner scanner((const char*) buffer, length);

  scanner.SetErrorCallback(err);

  udmf_lines.clear();
  udmf_sides.clear();
  udmf_vertices.clear();
  udmf_sectors.clear();
  udmf_things.clear();

  while (scanner.TokensLeft())
    dsda_ParseUDMFIdentifier(scanner);

  if (
    udmf_lines.empty() ||
    udmf_sides.empty() ||
    udmf_vertices.empty() ||
    udmf_sectors.empty() ||
    udmf_things.empty()
  )
    scanner.ErrorF("Insufficient UDMF data");

  udmf.lines = &udmf_lines[0];
  udmf.num_lines = udmf_lines.size();

  udmf.sides = &udmf_sides[0];
  udmf.num_sides = udmf_sides.size();

  udmf.vertices = &udmf_vertices[0];
  udmf.num_vertices = udmf_vertices.size();

  udmf.sectors = &udmf_sectors[0];
  udmf.num_sectors = udmf_sectors.size();

  udmf.things = &udmf_things[0];
  udmf.num_things = udmf_things.size();
}
