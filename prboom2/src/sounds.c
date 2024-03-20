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
 * DESCRIPTION:
 *      Created by a sound utility.
 *      Kept as a sample, DOOM2 sounds.
 *
 *-----------------------------------------------------------------------------*/

// killough 5/3/98: reformatted

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doomtype.h"
#include "sounds.h"

//
// Information about all the music
//

musicinfo_t doom_S_music[] = {
  { 0 },
  { "e1m1", 0 },
  { "e1m2", 0 },
  { "e1m3", 0 },
  { "e1m4", 0 },
  { "e1m5", 0 },
  { "e1m6", 0 },
  { "e1m7", 0 },
  { "e1m8", 0 },
  { "e1m9", 0 },
  { "e2m1", 0 },
  { "e2m2", 0 },
  { "e2m3", 0 },
  { "e2m4", 0 },
  { "e2m5", 0 },
  { "e2m6", 0 },
  { "e2m7", 0 },
  { "e2m8", 0 },
  { "e2m9", 0 },
  { "e3m1", 0 },
  { "e3m2", 0 },
  { "e3m3", 0 },
  { "e3m4", 0 },
  { "e3m5", 0 },
  { "e3m6", 0 },
  { "e3m7", 0 },
  { "e3m8", 0 },
  { "e3m9", 0 },
  { "inter", 0 },
  { "intro", 0 },
  { "bunny", 0 },
  { "victor", 0 },
  { "introa", 0 },
  { "runnin", 0 },
  { "stalks", 0 },
  { "countd", 0 },
  { "betwee", 0 },
  { "doom", 0 },
  { "the_da", 0 },
  { "shawn", 0 },
  { "ddtblu", 0 },
  { "in_cit", 0 },
  { "dead", 0 },
  { "stlks2", 0 },
  { "theda2", 0 },
  { "doom2", 0 },
  { "ddtbl2", 0 },
  { "runni2", 0 },
  { "dead2", 0 },
  { "stlks3", 0 },
  { "romero", 0 },
  { "shawn2", 0 },
  { "messag", 0 },
  { "count2", 0 },
  { "ddtbl3", 0 },
  { "ampie", 0 },
  { "theda3", 0 },
  { "adrian", 0 },
  { "messg2", 0 },
  { "romer2", 0 },
  { "tense", 0 },
  { "shawn3", 0 },
  { "openin", 0 },
  { "evil", 0 },
  { "ultima", 0 },
  { "read_m", 0 },
  { "dm2ttl", 0 },
  { "dm2int", 0 },

  // custom music from MUSINFO lump
  { "musinfo", 0 }
};


//
// Information about all the sfx
//

sfxinfo_t doom_S_sfx[] = {
  // S_sfx[0] needs to be a dummy for odd reasons.
  { "dsnone", 0, 0, -1, 0, 0, 0, "" },
  { "dspistol", 64, 0, -1, 0, 0, 0, "" },
  { "dsshotgn", 64, 0, -1, 0, 0, 0, "" },
  { "dssgcock", 64, 0, -1, 0, 0, 0, "" },
  { "dsdshtgn", 64, 0, -1, 0, 0, 0, "" },
  { "dsdbopn", 64, 0, -1, 0, 0, 0, "" },
  { "dsdbcls", 64, 0, -1, 0, 0, 0, "" },
  { "dsdbload", 64, 0, -1, 0, 0, 0, "" },
  { "dsplasma", 64, 0, -1, 0, 0, 0, "" },
  { "dsbfg", 64, 0, -1, 0, 0, 0, "" },
  { "dssawup", 64, 0, -1, 0, 0, 0, "" },
  { "dssawidl", 118, 0, -1, 0, 0, 0, "" },
  { "dssawful", 64, 0, -1, 0, 0, 0, "" },
  { "dssawhit", 64, 0, -1, 0, 0, 0, "" },
  { "dsrlaunc", 64, 0, -1, 0, 0, 0, "" },
  { "dsrxplod", 70, 0, -1, 0, 0, 0, "" },
  { "dsfirsht", 70, 0, -1, 0, 0, 0, "" },
  { "dsfirxpl", 70, 0, -1, 0, 0, 0, "" },
  { "dspstart", 100, 0, -1, 0, 0, 0, "" },
  { "dspstop", 100, 0, -1, 0, 0, 0, "" },
  { "dsdoropn", 100, 0, -1, 0, 0, 0, "" },
  { "dsdorcls", 100, 0, -1, 0, 0, 0, "" },
  { "dsstnmov", 119, 0, -1, 0, 0, 0, "" },
  { "dsswtchn", 78, 0, -1, 0, 0, 0, "" },
  { "dsswtchx", 78, 0, -1, 0, 0, 0, "" },
  { "dsplpain", 96, 0, -1, 0, 0, 0, "" },
  { "dsdmpain", 96, 0, -1, 0, 0, 0, "" },
  { "dspopain", 96, 0, -1, 0, 0, 0, "" },
  { "dsvipain", 96, 0, -1, 0, 0, 0, "" },
  { "dsmnpain", 96, 0, -1, 0, 0, 0, "" },
  { "dspepain", 96, 0, -1, 0, 0, 0, "" },
  { "dsslop", 78, 0, -1, 0, 0, 0, "" },
  { "dsitemup", 78, 0, -1, 0, 0, 0, "" },
  { "dswpnup", 78, 0, -1, 0, 0, 0, "" },
  { "dsoof", 96, 0, -1, 0, 0, 0, "" },
  { "dstelept", 32, 0, -1, 0, 0, 0, "" },
  { "dsposit1", 98, 0, -1, 0, 0, 0, "" },
  { "dsposit2", 98, 0, -1, 0, 0, 0, "" },
  { "dsposit3", 98, 0, -1, 0, 0, 0, "" },
  { "dsbgsit1", 98, 0, -1, 0, 0, 0, "" },
  { "dsbgsit2", 98, 0, -1, 0, 0, 0, "" },
  { "dssgtsit", 98, 0, -1, 0, 0, 0, "" },
  { "dscacsit", 98, 0, -1, 0, 0, 0, "" },
  { "dsbrssit", 94, 0, -1, 0, 0, 0, "" },
  { "dscybsit", 92, 0, -1, 0, 0, 0, "" },
  { "dsspisit", 90, 0, -1, 0, 0, 0, "" },
  { "dsbspsit", 90, 0, -1, 0, 0, 0, "" },
  { "dskntsit", 90, 0, -1, 0, 0, 0, "" },
  { "dsvilsit", 90, 0, -1, 0, 0, 0, "" },
  { "dsmansit", 90, 0, -1, 0, 0, 0, "" },
  { "dspesit", 90, 0, -1, 0, 0, 0, "" },
  { "dssklatk", 70, 0, -1, 0, 0, 0, "" },
  { "dssgtatk", 70, 0, -1, 0, 0, 0, "" },
  { "dsskepch", 70, 0, -1, 0, 0, 0, "" },
  { "dsvilatk", 70, 0, -1, 0, 0, 0, "" },
  { "dsclaw", 70, 0, -1, 0, 0, 0, "" },
  { "dsskeswg", 70, 0, -1, 0, 0, 0, "" },
  { "dspldeth", 32, 0, -1, 0, 0, 0, "" },
  { "dspdiehi", 32, 0, -1, 0, 0, 0, "" },
  { "dspodth1", 70, 0, -1, 0, 0, 0, "" },
  { "dspodth2", 70, 0, -1, 0, 0, 0, "" },
  { "dspodth3", 70, 0, -1, 0, 0, 0, "" },
  { "dsbgdth1", 70, 0, -1, 0, 0, 0, "" },
  { "dsbgdth2", 70, 0, -1, 0, 0, 0, "" },
  { "dssgtdth", 70, 0, -1, 0, 0, 0, "" },
  { "dscacdth", 70, 0, -1, 0, 0, 0, "" },
  { "dsskldth", 70, 0, -1, 0, 0, 0, "" },
  { "dsbrsdth", 32, 0, -1, 0, 0, 0, "" },
  { "dscybdth", 32, 0, -1, 0, 0, 0, "" },
  { "dsspidth", 32, 0, -1, 0, 0, 0, "" },
  { "dsbspdth", 32, 0, -1, 0, 0, 0, "" },
  { "dsvildth", 32, 0, -1, 0, 0, 0, "" },
  { "dskntdth", 32, 0, -1, 0, 0, 0, "" },
  { "dspedth", 32, 0, -1, 0, 0, 0, "" },
  { "dsskedth", 32, 0, -1, 0, 0, 0, "" },
  { "dsposact", 120, 0, -1, 0, 0, 0, "" },
  { "dsbgact", 120, 0, -1, 0, 0, 0, "" },
  { "dsdmact", 120, 0, -1, 0, 0, 0, "" },
  { "dsbspact", 100, 0, -1, 0, 0, 0, "" },
  { "dsbspwlk", 100, 0, -1, 0, 0, 0, "" },
  { "dsvilact", 100, 0, -1, 0, 0, 0, "" },
  { "dsnoway", 78, 0, -1, 0, 0, 0, "" },
  { "dsbarexp", 60, 0, -1, 0, 0, 0, "" },
  { "dspunch", 64, 0, -1, 0, 0, 0, "" },
  { "dshoof", 70, 0, -1, 0, 0, 0, "" },
  { "dsmetal", 70, 0, -1, 0, 0, 0, "" },
  { "dschgun", 64, &doom_S_sfx[sfx_pistol], 150, 0, 0, 0, "" },
  { "dstink", 60, 0, -1, 0, 0, 0, "" },
  { "dsbdopn", 100, 0, -1, 0, 0, 0, "" },
  { "dsbdcls", 100, 0, -1, 0, 0, 0, "" },
  { "dsitmbk", 100, 0, -1, 0, 0, 0, "" },
  { "dsflame", 32, 0, -1, 0, 0, 0, "" },
  { "dsflamst", 32, 0, -1, 0, 0, 0, "" },
  { "dsgetpow", 60, 0, -1, 0, 0, 0, "" },
  { "dsbospit", 70, 0, -1, 0, 0, 0, "" },
  { "dsboscub", 70, 0, -1, 0, 0, 0, "" },
  { "dsbossit", 70, 0, -1, 0, 0, 0, "" },
  { "dsbospn", 70, 0, -1, 0, 0, 0, "" },
  { "dsbosdth", 70, 0, -1, 0, 0, 0, "" },
  { "dsmanatk", 70, 0, -1, 0, 0, 0, "" },
  { "dsmandth", 70, 0, -1, 0, 0, 0, "" },
  { "dssssit", 70, 0, -1, 0, 0, 0, "" },
  { "dsssdth", 70, 0, -1, 0, 0, 0, "" },
  { "dskeenpn", 70, 0, -1, 0, 0, 0, "" },
  { "dskeendt", 70, 0, -1, 0, 0, 0, "" },
  { "dsskeact", 70, 0, -1, 0, 0, 0, "" },
  { "dsskesit", 70, 0, -1, 0, 0, 0, "" },
  { "dsskeatk", 70, 0, -1, 0, 0, 0, "" },
  { "dsradio", 60, 0, -1, 0, 0, 0, "" },

  // killough 11/98: dog sounds
  { "dsdgsit", 98, 0, -1, 0, 0, 0, "" },
  { "dsdgatk", 70, 0, -1, 0, 0, 0, "" },
  { "dsdgact", 120, 0, -1, 0, 0, 0, "" },
  { "dsdgdth", 70, 0, -1, 0, 0, 0, "" },
  { "dsdgpain", 96, 0, -1, 0, 0, 0, "" },

  //e6y
  { "dssecret", 60, 0, -1, 0, 0, 0, "" },
  // Everything from here up to 500 is reserved for future use.

  // Free slots for DEHEXTRA. Priorities should be overridden by user.
  // There is a gap present to accomodate Eternity Engine - see their commit
  // @ https://github.com/team-eternity/eternity/commit/b8fb8f71 - which  means
  // I must use desginated initializers, or else supply an exact number of dummy
  // entries to pad it out. Not sure which would be uglier to maintain. -SH
  [500] = { "dsfre000", 127, 0, -1, 0, 0, 0, "" },
  [501] = { "dsfre001", 127, 0, -1, 0, 0, 0, "" },
  [502] = { "dsfre002", 127, 0, -1, 0, 0, 0, "" },
  [503] = { "dsfre003", 127, 0, -1, 0, 0, 0, "" },
  [504] = { "dsfre004", 127, 0, -1, 0, 0, 0, "" },
  [505] = { "dsfre005", 127, 0, -1, 0, 0, 0, "" },
  [506] = { "dsfre006", 127, 0, -1, 0, 0, 0, "" },
  [507] = { "dsfre007", 127, 0, -1, 0, 0, 0, "" },
  [508] = { "dsfre008", 127, 0, -1, 0, 0, 0, "" },
  [509] = { "dsfre009", 127, 0, -1, 0, 0, 0, "" },
  [510] = { "dsfre010", 127, 0, -1, 0, 0, 0, "" },
  [511] = { "dsfre011", 127, 0, -1, 0, 0, 0, "" },
  [512] = { "dsfre012", 127, 0, -1, 0, 0, 0, "" },
  [513] = { "dsfre013", 127, 0, -1, 0, 0, 0, "" },
  [514] = { "dsfre014", 127, 0, -1, 0, 0, 0, "" },
  [515] = { "dsfre015", 127, 0, -1, 0, 0, 0, "" },
  [516] = { "dsfre016", 127, 0, -1, 0, 0, 0, "" },
  [517] = { "dsfre017", 127, 0, -1, 0, 0, 0, "" },
  [518] = { "dsfre018", 127, 0, -1, 0, 0, 0, "" },
  [519] = { "dsfre019", 127, 0, -1, 0, 0, 0, "" },
  [520] = { "dsfre020", 127, 0, -1, 0, 0, 0, "" },
  [521] = { "dsfre021", 127, 0, -1, 0, 0, 0, "" },
  [522] = { "dsfre022", 127, 0, -1, 0, 0, 0, "" },
  [523] = { "dsfre023", 127, 0, -1, 0, 0, 0, "" },
  [524] = { "dsfre024", 127, 0, -1, 0, 0, 0, "" },
  [525] = { "dsfre025", 127, 0, -1, 0, 0, 0, "" },
  [526] = { "dsfre026", 127, 0, -1, 0, 0, 0, "" },
  [527] = { "dsfre027", 127, 0, -1, 0, 0, 0, "" },
  [528] = { "dsfre028", 127, 0, -1, 0, 0, 0, "" },
  [529] = { "dsfre029", 127, 0, -1, 0, 0, 0, "" },
  [530] = { "dsfre030", 127, 0, -1, 0, 0, 0, "" },
  [531] = { "dsfre031", 127, 0, -1, 0, 0, 0, "" },
  [532] = { "dsfre032", 127, 0, -1, 0, 0, 0, "" },
  [533] = { "dsfre033", 127, 0, -1, 0, 0, 0, "" },
  [534] = { "dsfre034", 127, 0, -1, 0, 0, 0, "" },
  [535] = { "dsfre035", 127, 0, -1, 0, 0, 0, "" },
  [536] = { "dsfre036", 127, 0, -1, 0, 0, 0, "" },
  [537] = { "dsfre037", 127, 0, -1, 0, 0, 0, "" },
  [538] = { "dsfre038", 127, 0, -1, 0, 0, 0, "" },
  [539] = { "dsfre039", 127, 0, -1, 0, 0, 0, "" },
  [540] = { "dsfre040", 127, 0, -1, 0, 0, 0, "" },
  [541] = { "dsfre041", 127, 0, -1, 0, 0, 0, "" },
  [542] = { "dsfre042", 127, 0, -1, 0, 0, 0, "" },
  [543] = { "dsfre043", 127, 0, -1, 0, 0, 0, "" },
  [544] = { "dsfre044", 127, 0, -1, 0, 0, 0, "" },
  [545] = { "dsfre045", 127, 0, -1, 0, 0, 0, "" },
  [546] = { "dsfre046", 127, 0, -1, 0, 0, 0, "" },
  [547] = { "dsfre047", 127, 0, -1, 0, 0, 0, "" },
  [548] = { "dsfre048", 127, 0, -1, 0, 0, 0, "" },
  [549] = { "dsfre049", 127, 0, -1, 0, 0, 0, "" },
  [550] = { "dsfre050", 127, 0, -1, 0, 0, 0, "" },
  [551] = { "dsfre051", 127, 0, -1, 0, 0, 0, "" },
  [552] = { "dsfre052", 127, 0, -1, 0, 0, 0, "" },
  [553] = { "dsfre053", 127, 0, -1, 0, 0, 0, "" },
  [554] = { "dsfre054", 127, 0, -1, 0, 0, 0, "" },
  [555] = { "dsfre055", 127, 0, -1, 0, 0, 0, "" },
  [556] = { "dsfre056", 127, 0, -1, 0, 0, 0, "" },
  [557] = { "dsfre057", 127, 0, -1, 0, 0, 0, "" },
  [558] = { "dsfre058", 127, 0, -1, 0, 0, 0, "" },
  [559] = { "dsfre059", 127, 0, -1, 0, 0, 0, "" },
  [560] = { "dsfre060", 127, 0, -1, 0, 0, 0, "" },
  [561] = { "dsfre061", 127, 0, -1, 0, 0, 0, "" },
  [562] = { "dsfre062", 127, 0, -1, 0, 0, 0, "" },
  [563] = { "dsfre063", 127, 0, -1, 0, 0, 0, "" },
  [564] = { "dsfre064", 127, 0, -1, 0, 0, 0, "" },
  [565] = { "dsfre065", 127, 0, -1, 0, 0, 0, "" },
  [566] = { "dsfre066", 127, 0, -1, 0, 0, 0, "" },
  [567] = { "dsfre067", 127, 0, -1, 0, 0, 0, "" },
  [568] = { "dsfre068", 127, 0, -1, 0, 0, 0, "" },
  [569] = { "dsfre069", 127, 0, -1, 0, 0, 0, "" },
  [570] = { "dsfre070", 127, 0, -1, 0, 0, 0, "" },
  [571] = { "dsfre071", 127, 0, -1, 0, 0, 0, "" },
  [572] = { "dsfre072", 127, 0, -1, 0, 0, 0, "" },
  [573] = { "dsfre073", 127, 0, -1, 0, 0, 0, "" },
  [574] = { "dsfre074", 127, 0, -1, 0, 0, 0, "" },
  [575] = { "dsfre075", 127, 0, -1, 0, 0, 0, "" },
  [576] = { "dsfre076", 127, 0, -1, 0, 0, 0, "" },
  [577] = { "dsfre077", 127, 0, -1, 0, 0, 0, "" },
  [578] = { "dsfre078", 127, 0, -1, 0, 0, 0, "" },
  [579] = { "dsfre079", 127, 0, -1, 0, 0, 0, "" },
  [580] = { "dsfre080", 127, 0, -1, 0, 0, 0, "" },
  [581] = { "dsfre081", 127, 0, -1, 0, 0, 0, "" },
  [582] = { "dsfre082", 127, 0, -1, 0, 0, 0, "" },
  [583] = { "dsfre083", 127, 0, -1, 0, 0, 0, "" },
  [584] = { "dsfre084", 127, 0, -1, 0, 0, 0, "" },
  [585] = { "dsfre085", 127, 0, -1, 0, 0, 0, "" },
  [586] = { "dsfre086", 127, 0, -1, 0, 0, 0, "" },
  [587] = { "dsfre087", 127, 0, -1, 0, 0, 0, "" },
  [588] = { "dsfre088", 127, 0, -1, 0, 0, 0, "" },
  [589] = { "dsfre089", 127, 0, -1, 0, 0, 0, "" },
  [590] = { "dsfre090", 127, 0, -1, 0, 0, 0, "" },
  [591] = { "dsfre091", 127, 0, -1, 0, 0, 0, "" },
  [592] = { "dsfre092", 127, 0, -1, 0, 0, 0, "" },
  [593] = { "dsfre093", 127, 0, -1, 0, 0, 0, "" },
  [594] = { "dsfre094", 127, 0, -1, 0, 0, 0, "" },
  [595] = { "dsfre095", 127, 0, -1, 0, 0, 0, "" },
  [596] = { "dsfre096", 127, 0, -1, 0, 0, 0, "" },
  [597] = { "dsfre097", 127, 0, -1, 0, 0, 0, "" },
  [598] = { "dsfre098", 127, 0, -1, 0, 0, 0, "" },
  [599] = { "dsfre099", 127, 0, -1, 0, 0, 0, "" },
  [600] = { "dsfre100", 127, 0, -1, 0, 0, 0, "" },
  [601] = { "dsfre101", 127, 0, -1, 0, 0, 0, "" },
  [602] = { "dsfre102", 127, 0, -1, 0, 0, 0, "" },
  [603] = { "dsfre103", 127, 0, -1, 0, 0, 0, "" },
  [604] = { "dsfre104", 127, 0, -1, 0, 0, 0, "" },
  [605] = { "dsfre105", 127, 0, -1, 0, 0, 0, "" },
  [606] = { "dsfre106", 127, 0, -1, 0, 0, 0, "" },
  [607] = { "dsfre107", 127, 0, -1, 0, 0, 0, "" },
  [608] = { "dsfre108", 127, 0, -1, 0, 0, 0, "" },
  [609] = { "dsfre109", 127, 0, -1, 0, 0, 0, "" },
  [610] = { "dsfre110", 127, 0, -1, 0, 0, 0, "" },
  [611] = { "dsfre111", 127, 0, -1, 0, 0, 0, "" },
  [612] = { "dsfre112", 127, 0, -1, 0, 0, 0, "" },
  [613] = { "dsfre113", 127, 0, -1, 0, 0, 0, "" },
  [614] = { "dsfre114", 127, 0, -1, 0, 0, 0, "" },
  [615] = { "dsfre115", 127, 0, -1, 0, 0, 0, "" },
  [616] = { "dsfre116", 127, 0, -1, 0, 0, 0, "" },
  [617] = { "dsfre117", 127, 0, -1, 0, 0, 0, "" },
  [618] = { "dsfre118", 127, 0, -1, 0, 0, 0, "" },
  [619] = { "dsfre119", 127, 0, -1, 0, 0, 0, "" },
  [620] = { "dsfre120", 127, 0, -1, 0, 0, 0, "" },
  [621] = { "dsfre121", 127, 0, -1, 0, 0, 0, "" },
  [622] = { "dsfre122", 127, 0, -1, 0, 0, 0, "" },
  [623] = { "dsfre123", 127, 0, -1, 0, 0, 0, "" },
  [624] = { "dsfre124", 127, 0, -1, 0, 0, 0, "" },
  [625] = { "dsfre125", 127, 0, -1, 0, 0, 0, "" },
  [626] = { "dsfre126", 127, 0, -1, 0, 0, 0, "" },
  [627] = { "dsfre127", 127, 0, -1, 0, 0, 0, "" },
  [628] = { "dsfre128", 127, 0, -1, 0, 0, 0, "" },
  [629] = { "dsfre129", 127, 0, -1, 0, 0, 0, "" },
  [630] = { "dsfre130", 127, 0, -1, 0, 0, 0, "" },
  [631] = { "dsfre131", 127, 0, -1, 0, 0, 0, "" },
  [632] = { "dsfre132", 127, 0, -1, 0, 0, 0, "" },
  [633] = { "dsfre133", 127, 0, -1, 0, 0, 0, "" },
  [634] = { "dsfre134", 127, 0, -1, 0, 0, 0, "" },
  [635] = { "dsfre135", 127, 0, -1, 0, 0, 0, "" },
  [636] = { "dsfre136", 127, 0, -1, 0, 0, 0, "" },
  [637] = { "dsfre137", 127, 0, -1, 0, 0, 0, "" },
  [638] = { "dsfre138", 127, 0, -1, 0, 0, 0, "" },
  [639] = { "dsfre139", 127, 0, -1, 0, 0, 0, "" },
  [640] = { "dsfre140", 127, 0, -1, 0, 0, 0, "" },
  [641] = { "dsfre141", 127, 0, -1, 0, 0, 0, "" },
  [642] = { "dsfre142", 127, 0, -1, 0, 0, 0, "" },
  [643] = { "dsfre143", 127, 0, -1, 0, 0, 0, "" },
  [644] = { "dsfre144", 127, 0, -1, 0, 0, 0, "" },
  [645] = { "dsfre145", 127, 0, -1, 0, 0, 0, "" },
  [646] = { "dsfre146", 127, 0, -1, 0, 0, 0, "" },
  [647] = { "dsfre147", 127, 0, -1, 0, 0, 0, "" },
  [648] = { "dsfre148", 127, 0, -1, 0, 0, 0, "" },
  [649] = { "dsfre149", 127, 0, -1, 0, 0, 0, "" },
  [650] = { "dsfre150", 127, 0, -1, 0, 0, 0, "" },
  [651] = { "dsfre151", 127, 0, -1, 0, 0, 0, "" },
  [652] = { "dsfre152", 127, 0, -1, 0, 0, 0, "" },
  [653] = { "dsfre153", 127, 0, -1, 0, 0, 0, "" },
  [654] = { "dsfre154", 127, 0, -1, 0, 0, 0, "" },
  [655] = { "dsfre155", 127, 0, -1, 0, 0, 0, "" },
  [656] = { "dsfre156", 127, 0, -1, 0, 0, 0, "" },
  [657] = { "dsfre157", 127, 0, -1, 0, 0, 0, "" },
  [658] = { "dsfre158", 127, 0, -1, 0, 0, 0, "" },
  [659] = { "dsfre159", 127, 0, -1, 0, 0, 0, "" },
  [660] = { "dsfre160", 127, 0, -1, 0, 0, 0, "" },
  [661] = { "dsfre161", 127, 0, -1, 0, 0, 0, "" },
  [662] = { "dsfre162", 127, 0, -1, 0, 0, 0, "" },
  [663] = { "dsfre163", 127, 0, -1, 0, 0, 0, "" },
  [664] = { "dsfre164", 127, 0, -1, 0, 0, 0, "" },
  [665] = { "dsfre165", 127, 0, -1, 0, 0, 0, "" },
  [666] = { "dsfre166", 127, 0, -1, 0, 0, 0, "" },
  [667] = { "dsfre167", 127, 0, -1, 0, 0, 0, "" },
  [668] = { "dsfre168", 127, 0, -1, 0, 0, 0, "" },
  [669] = { "dsfre169", 127, 0, -1, 0, 0, 0, "" },
  [670] = { "dsfre170", 127, 0, -1, 0, 0, 0, "" },
  [671] = { "dsfre171", 127, 0, -1, 0, 0, 0, "" },
  [672] = { "dsfre172", 127, 0, -1, 0, 0, 0, "" },
  [673] = { "dsfre173", 127, 0, -1, 0, 0, 0, "" },
  [674] = { "dsfre174", 127, 0, -1, 0, 0, 0, "" },
  [675] = { "dsfre175", 127, 0, -1, 0, 0, 0, "" },
  [676] = { "dsfre176", 127, 0, -1, 0, 0, 0, "" },
  [677] = { "dsfre177", 127, 0, -1, 0, 0, 0, "" },
  [678] = { "dsfre178", 127, 0, -1, 0, 0, 0, "" },
  [679] = { "dsfre179", 127, 0, -1, 0, 0, 0, "" },
  [680] = { "dsfre180", 127, 0, -1, 0, 0, 0, "" },
  [681] = { "dsfre181", 127, 0, -1, 0, 0, 0, "" },
  [682] = { "dsfre182", 127, 0, -1, 0, 0, 0, "" },
  [683] = { "dsfre183", 127, 0, -1, 0, 0, 0, "" },
  [684] = { "dsfre184", 127, 0, -1, 0, 0, 0, "" },
  [685] = { "dsfre185", 127, 0, -1, 0, 0, 0, "" },
  [686] = { "dsfre186", 127, 0, -1, 0, 0, 0, "" },
  [687] = { "dsfre187", 127, 0, -1, 0, 0, 0, "" },
  [688] = { "dsfre188", 127, 0, -1, 0, 0, 0, "" },
  [689] = { "dsfre189", 127, 0, -1, 0, 0, 0, "" },
  [690] = { "dsfre190", 127, 0, -1, 0, 0, 0, "" },
  [691] = { "dsfre191", 127, 0, -1, 0, 0, 0, "" },
  [692] = { "dsfre192", 127, 0, -1, 0, 0, 0, "" },
  [693] = { "dsfre193", 127, 0, -1, 0, 0, 0, "" },
  [694] = { "dsfre194", 127, 0, -1, 0, 0, 0, "" },
  [695] = { "dsfre195", 127, 0, -1, 0, 0, 0, "" },
  [696] = { "dsfre196", 127, 0, -1, 0, 0, 0, "" },
  [697] = { "dsfre197", 127, 0, -1, 0, 0, 0, "" },
  [698] = { "dsfre198", 127, 0, -1, 0, 0, 0, "" },
  [699] = { "dsfre199", 127, 0, -1, 0, 0, 0, "" },
};

#define DISAMBIGUATED_SFX(id, tag) { "", 0, &doom_S_sfx[id], 0, 0, 0, 0, tag }

sfxinfo_t doom_disambiguated_sfx[] = {
  DISAMBIGUATED_SFX(sfx_pistol, "weapons/pistol"),
  DISAMBIGUATED_SFX(sfx_pistol, "grunt/attack"),
  DISAMBIGUATED_SFX(sfx_pistol, "menu/choose"),
  DISAMBIGUATED_SFX(sfx_pistol, "intermission/tick"),

  DISAMBIGUATED_SFX(sfx_shotgn, "weapons/shotgf"),
  DISAMBIGUATED_SFX(sfx_shotgn, "shotguy/attack"),
  DISAMBIGUATED_SFX(sfx_shotgn, "chainguy/attack"),
  DISAMBIGUATED_SFX(sfx_shotgn, "spider/attack"),
  DISAMBIGUATED_SFX(sfx_shotgn, "wolfss/attack"),

  DISAMBIGUATED_SFX(sfx_sgcock, "weapons/shotgr"),
  DISAMBIGUATED_SFX(sfx_sgcock, "intermission/paststats"),
  DISAMBIGUATED_SFX(sfx_sgcock, "intermission/pastcoopstats"),

  DISAMBIGUATED_SFX(sfx_dshtgn, "weapons/sshotf"),

  DISAMBIGUATED_SFX(sfx_dbopn, "weapons/sshoto"),

  DISAMBIGUATED_SFX(sfx_dbcls, "weapons/sshotc"),

  DISAMBIGUATED_SFX(sfx_dbload, "weapons/sshotl"),

  DISAMBIGUATED_SFX(sfx_plasma, "weapons/plasmaf"),
  DISAMBIGUATED_SFX(sfx_plasma, "baby/attack"),

  DISAMBIGUATED_SFX(sfx_bfg, "weapons/bfgf"),

  DISAMBIGUATED_SFX(sfx_sawup, "weapons/sawup"),

  DISAMBIGUATED_SFX(sfx_sawidl, "weapons/sawidle"),

  DISAMBIGUATED_SFX(sfx_sawful, "weapons/sawfull"),

  DISAMBIGUATED_SFX(sfx_sawhit, "weapons/sawhit"),

  DISAMBIGUATED_SFX(sfx_rlaunc, "weapons/rocklf"),

  DISAMBIGUATED_SFX(sfx_rxplod, "weapons/bfgx"),

  DISAMBIGUATED_SFX(sfx_firsht, "baron/attack"),
  DISAMBIGUATED_SFX(sfx_firsht, "fatso/attack"),
  DISAMBIGUATED_SFX(sfx_firsht, "imp/attack"),
  DISAMBIGUATED_SFX(sfx_firsht, "caco/attack"),

  DISAMBIGUATED_SFX(sfx_firxpl, "weapons/plasmax"),
  DISAMBIGUATED_SFX(sfx_firxpl, "fatso/shotx"),
  DISAMBIGUATED_SFX(sfx_firxpl, "imp/shotx"),
  DISAMBIGUATED_SFX(sfx_firxpl, "caco/shotx"),
  DISAMBIGUATED_SFX(sfx_firxpl, "baron/shotx"),
  DISAMBIGUATED_SFX(sfx_firxpl, "skull/death"),
  DISAMBIGUATED_SFX(sfx_firxpl, "baby/shotx"),
  DISAMBIGUATED_SFX(sfx_firxpl, "brain/cubeboom"),

  DISAMBIGUATED_SFX(sfx_pstart, "plats/pt1_strt"),

  DISAMBIGUATED_SFX(sfx_pstop, "plats/pt1_stop"),
  DISAMBIGUATED_SFX(sfx_pstop, "menu/cursor"),

  DISAMBIGUATED_SFX(sfx_doropn, "doors/dr1_open"),

  DISAMBIGUATED_SFX(sfx_dorcls, "doors/dr1_clos"),

  DISAMBIGUATED_SFX(sfx_stnmov, "plats/pt1_mid"),
  DISAMBIGUATED_SFX(sfx_stnmov, "menu/change"),

  DISAMBIGUATED_SFX(sfx_swtchn, "switches/normbutn"),
  DISAMBIGUATED_SFX(sfx_swtchn, "menu/activate"),
  DISAMBIGUATED_SFX(sfx_swtchn, "menu/backup"),
  DISAMBIGUATED_SFX(sfx_swtchn, "menu/prompt"),

  DISAMBIGUATED_SFX(sfx_swtchx, "switches/exitbutn"),
  DISAMBIGUATED_SFX(sfx_swtchx, "menu/dismiss"),
  DISAMBIGUATED_SFX(sfx_swtchx, "menu/clear"),

  DISAMBIGUATED_SFX(sfx_plpain, "*pain100"),
  DISAMBIGUATED_SFX(sfx_plpain, "*pain75"),
  DISAMBIGUATED_SFX(sfx_plpain, "*pain50"),
  DISAMBIGUATED_SFX(sfx_plpain, "*pain25"),

  DISAMBIGUATED_SFX(sfx_dmpain, "demon/pain"),
  DISAMBIGUATED_SFX(sfx_dmpain, "spectre/pain"),
  DISAMBIGUATED_SFX(sfx_dmpain, "caco/pain"),
  DISAMBIGUATED_SFX(sfx_dmpain, "baron/pain"),
  DISAMBIGUATED_SFX(sfx_dmpain, "knight/pain"),
  DISAMBIGUATED_SFX(sfx_dmpain, "skull/pain"),
  DISAMBIGUATED_SFX(sfx_dmpain, "spider/pain"),
  DISAMBIGUATED_SFX(sfx_dmpain, "baby/pain"),
  DISAMBIGUATED_SFX(sfx_dmpain, "cyber/pain"),

  DISAMBIGUATED_SFX(sfx_popain, "grunt/pain"),
  DISAMBIGUATED_SFX(sfx_popain, "shotguy/pain"),
  DISAMBIGUATED_SFX(sfx_popain, "skeleton/pain"),
  DISAMBIGUATED_SFX(sfx_popain, "chainguy/pain"),
  DISAMBIGUATED_SFX(sfx_popain, "imp/pain"),
  DISAMBIGUATED_SFX(sfx_popain, "wolfss/pain"),

  DISAMBIGUATED_SFX(sfx_vipain, "vile/pain"),

  DISAMBIGUATED_SFX(sfx_mnpain, "fatso/pain"),

  DISAMBIGUATED_SFX(sfx_pepain, "pain/pain"),

  DISAMBIGUATED_SFX(sfx_slop, "*gibbed"),
  DISAMBIGUATED_SFX(sfx_slop, "misc/gibbed"),
  DISAMBIGUATED_SFX(sfx_slop, "vile/raise"),
  DISAMBIGUATED_SFX(sfx_slop, "intermission/pastdmstats"),

  DISAMBIGUATED_SFX(sfx_itemup, "misc/i_pkup"),
  DISAMBIGUATED_SFX(sfx_itemup, "misc/k_pkup"),
  DISAMBIGUATED_SFX(sfx_itemup, "misc/health_pkup"),
  DISAMBIGUATED_SFX(sfx_itemup, "misc/armor_pkup"),
  DISAMBIGUATED_SFX(sfx_itemup, "misc/ammo_pkup"),

  DISAMBIGUATED_SFX(sfx_wpnup, "misc/w_pkup"),

  DISAMBIGUATED_SFX(sfx_oof, "*grunt"),
  DISAMBIGUATED_SFX(sfx_oof, "*land"),
  DISAMBIGUATED_SFX(sfx_oof, "menu/invalid"),

  DISAMBIGUATED_SFX(sfx_telept, "misc/teleport"),
  DISAMBIGUATED_SFX(sfx_telept, "brain/spawn"),

  DISAMBIGUATED_SFX(sfx_posit1, "grunt/sight1"),
  DISAMBIGUATED_SFX(sfx_posit1, "shotguy/sight1"),
  DISAMBIGUATED_SFX(sfx_posit1, "chainguy/sight1"),

  DISAMBIGUATED_SFX(sfx_posit2, "grunt/sight2"),
  DISAMBIGUATED_SFX(sfx_posit2, "shotguy/sight2"),
  DISAMBIGUATED_SFX(sfx_posit2, "chainguy/sight2"),

  DISAMBIGUATED_SFX(sfx_posit3, "grunt/sight3"),
  DISAMBIGUATED_SFX(sfx_posit3, "shotguy/sight3"),
  DISAMBIGUATED_SFX(sfx_posit3, "chainguy/sight3"),

  DISAMBIGUATED_SFX(sfx_bgsit1, "imp/sight1"),

  DISAMBIGUATED_SFX(sfx_bgsit2, "imp/sight2"),

  DISAMBIGUATED_SFX(sfx_sgtsit, "demon/sight"),
  DISAMBIGUATED_SFX(sfx_sgtsit, "spectre/sight"),

  DISAMBIGUATED_SFX(sfx_cacsit, "caco/sight"),

  DISAMBIGUATED_SFX(sfx_brssit, "baron/sight"),

  DISAMBIGUATED_SFX(sfx_cybsit, "cyber/sight"),

  DISAMBIGUATED_SFX(sfx_spisit, "spider/sight"),

  DISAMBIGUATED_SFX(sfx_bspsit, "baby/sight"),

  DISAMBIGUATED_SFX(sfx_kntsit, "knight/sight"),

  DISAMBIGUATED_SFX(sfx_vilsit, "vile/sight"),

  DISAMBIGUATED_SFX(sfx_mansit, "fatso/sight"),

  DISAMBIGUATED_SFX(sfx_pesit, "pain/sight"),

  DISAMBIGUATED_SFX(sfx_sklatk, "skull/melee"),

  DISAMBIGUATED_SFX(sfx_sgtatk, "demon/melee"),
  DISAMBIGUATED_SFX(sfx_sgtatk, "spectre/melee"),

  DISAMBIGUATED_SFX(sfx_skepch, "skeleton/melee"),

  DISAMBIGUATED_SFX(sfx_vilatk, "vile/start"),

  DISAMBIGUATED_SFX(sfx_claw, "imp/melee"),
  DISAMBIGUATED_SFX(sfx_claw, "baron/melee"),

  DISAMBIGUATED_SFX(sfx_skeswg, "skeleton/swing"),

  DISAMBIGUATED_SFX(sfx_pldeth, "*death"),
  DISAMBIGUATED_SFX(sfx_pldeth, "intermission/cooptotal"),

  DISAMBIGUATED_SFX(sfx_pdiehi, "*xdeath"),

  DISAMBIGUATED_SFX(sfx_podth1, "grunt/death1"),
  DISAMBIGUATED_SFX(sfx_podth1, "shotguy/death1"),
  DISAMBIGUATED_SFX(sfx_podth1, "chainguy/death1"),

  DISAMBIGUATED_SFX(sfx_podth2, "grunt/death2"),
  DISAMBIGUATED_SFX(sfx_podth2, "shotguy/death2"),
  DISAMBIGUATED_SFX(sfx_podth2, "chainguy/death2"),

  DISAMBIGUATED_SFX(sfx_podth3, "grunt/death3"),
  DISAMBIGUATED_SFX(sfx_podth3, "shotguy/death3"),
  DISAMBIGUATED_SFX(sfx_podth3, "chainguy/death3"),

  DISAMBIGUATED_SFX(sfx_bgdth1, "imp/death1"),

  DISAMBIGUATED_SFX(sfx_bgdth2, "imp/death2"),

  DISAMBIGUATED_SFX(sfx_sgtdth, "demon/death"),
  DISAMBIGUATED_SFX(sfx_sgtdth, "spectre/death"),

  DISAMBIGUATED_SFX(sfx_cacdth, "caco/death"),

  DISAMBIGUATED_SFX(sfx_skldth, "misc/unused"),

  DISAMBIGUATED_SFX(sfx_brsdth, "baron/death"),

  DISAMBIGUATED_SFX(sfx_cybdth, "cyber/death"),

  DISAMBIGUATED_SFX(sfx_spidth, "spider/death"),

  DISAMBIGUATED_SFX(sfx_bspdth, "baby/death"),

  DISAMBIGUATED_SFX(sfx_vildth, "vile/death"),

  DISAMBIGUATED_SFX(sfx_kntdth, "knight/death"),

  DISAMBIGUATED_SFX(sfx_pedth, "pain/death"),

  DISAMBIGUATED_SFX(sfx_skedth, "skeleton/death"),

  DISAMBIGUATED_SFX(sfx_posact, "grunt/active"),
  DISAMBIGUATED_SFX(sfx_posact, "shotguy/active"),
  DISAMBIGUATED_SFX(sfx_posact, "fatso/active"),
  DISAMBIGUATED_SFX(sfx_posact, "chainguy/active"),
  DISAMBIGUATED_SFX(sfx_posact, "wolfss/active"),

  DISAMBIGUATED_SFX(sfx_bgact, "imp/active"),

  DISAMBIGUATED_SFX(sfx_dmact, "demon/active"),
  DISAMBIGUATED_SFX(sfx_dmact, "spectre/active"),
  DISAMBIGUATED_SFX(sfx_dmact, "caco/active"),
  DISAMBIGUATED_SFX(sfx_dmact, "baron/active"),
  DISAMBIGUATED_SFX(sfx_dmact, "knight/active"),
  DISAMBIGUATED_SFX(sfx_dmact, "skull/active"),
  DISAMBIGUATED_SFX(sfx_dmact, "spider/active"),
  DISAMBIGUATED_SFX(sfx_dmact, "cyber/active"),
  DISAMBIGUATED_SFX(sfx_dmact, "pain/active"),

  DISAMBIGUATED_SFX(sfx_bspact, "baby/active"),

  DISAMBIGUATED_SFX(sfx_bspwlk, "baby/walk"),

  DISAMBIGUATED_SFX(sfx_vilact, "vile/active"),

  DISAMBIGUATED_SFX(sfx_noway, "*usefail"),
  DISAMBIGUATED_SFX(sfx_noway, "misc/keytry"),

  DISAMBIGUATED_SFX(sfx_barexp, "weapons/rocklx"),
  DISAMBIGUATED_SFX(sfx_barexp, "vile/stop"),
  DISAMBIGUATED_SFX(sfx_barexp, "skeleton/tracex"),
  DISAMBIGUATED_SFX(sfx_barexp, "world/barrelx"),
  DISAMBIGUATED_SFX(sfx_barexp, "misc/brainexplode"),
  DISAMBIGUATED_SFX(sfx_barexp, "intermission/nextstage"),

  DISAMBIGUATED_SFX(sfx_punch, "*fist"),

  DISAMBIGUATED_SFX(sfx_hoof, "cyber/hoof"),

  DISAMBIGUATED_SFX(sfx_metal, "spider/walk"),

  DISAMBIGUATED_SFX(sfx_chgun, "weapons/chngun"), // -> chgun -> pistol

  DISAMBIGUATED_SFX(sfx_tink, "misc/chat2"),

  DISAMBIGUATED_SFX(sfx_bdopn, "doors/dr2_open"),

  DISAMBIGUATED_SFX(sfx_bdcls, "doors/dr2_clos"),

  DISAMBIGUATED_SFX(sfx_itmbk, "misc/spawn"),

  DISAMBIGUATED_SFX(sfx_flame, "vile/firecrkl"),

  DISAMBIGUATED_SFX(sfx_flamst, "vile/firestrt"),

  DISAMBIGUATED_SFX(sfx_getpow, "misc/p_pkup"),

  DISAMBIGUATED_SFX(sfx_bospit, "brain/spit"),

  DISAMBIGUATED_SFX(sfx_boscub, "brain/cube"),

  DISAMBIGUATED_SFX(sfx_bossit, "brain/sight"),

  DISAMBIGUATED_SFX(sfx_bospn, "brain/pain"),

  DISAMBIGUATED_SFX(sfx_bosdth, "brain/death"),

  DISAMBIGUATED_SFX(sfx_manatk, "fatso/raiseguns"),

  DISAMBIGUATED_SFX(sfx_mandth, "fatso/death"),

  DISAMBIGUATED_SFX(sfx_sssit, "wolfss/sight"),

  DISAMBIGUATED_SFX(sfx_ssdth, "wolfss/death"),

  DISAMBIGUATED_SFX(sfx_keenpn, "keen/pain"),

  DISAMBIGUATED_SFX(sfx_keendt, "keen/death"),

  DISAMBIGUATED_SFX(sfx_skeact, "skeleton/active"),

  DISAMBIGUATED_SFX(sfx_skesit, "skeleton/sight"),

  DISAMBIGUATED_SFX(sfx_skeatk, "skeleton/attack"),

  DISAMBIGUATED_SFX(sfx_radio, "misc/chat"),

  DISAMBIGUATED_SFX(sfx_dgsit, "dog/sight"),

  DISAMBIGUATED_SFX(sfx_dgatk, "dog/attack"),

  DISAMBIGUATED_SFX(sfx_dgact, "dog/active"),

  DISAMBIGUATED_SFX(sfx_dgdth, "dog/death"),

  DISAMBIGUATED_SFX(sfx_dgpain, "dog/pain"),

  DISAMBIGUATED_SFX(sfx_secret, "misc/secret"),
};
