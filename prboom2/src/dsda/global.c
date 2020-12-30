//
// Copyright(C) 2020 by Ryan Krafnick
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
//	DSDA Global - define top level globals for doom vs heretic
//

#include <stdlib.h>
#include <string.h>

#include "m_argv.h"
#include "info.h"
#include "d_items.h"
#include "heretic/def.h"

#include "global.h"

state_t* states;
int num_states;

const char** sprnames;
int num_sprites;

mobjinfo_t* mobjinfo;
int num_mobj_types;

weaponinfo_t* weaponinfo;

int g_mt_player;

static void dsda_AllocateMobjInfo(int count) {
  num_mobj_types = count;
  
  mobjinfo = malloc(sizeof(mobjinfo_t) * num_mobj_types);
  memset(mobjinfo, 0, sizeof(mobjinfo_t) * num_mobj_types);
}

static void dsda_SetStates(state_t* state_list, int count) {
  states = state_list;
  num_states = count;
}

static void dsda_SetSpriteNames(const char** sprite_name_list, int count) {
  sprnames = sprite_name_list;
  num_sprites = count;
}

static void dsda_InitDoom(void) {
  int i;
  doom_mobjinfo_t* mobjinfo_p;

  dsda_AllocateMobjInfo(NUMMOBJTYPES);
  dsda_SetStates(doom_states, NUMSTATES);
  dsda_SetSpriteNames(doom_sprnames, NUMSPRITES);
  
  weaponinfo = doom_weaponinfo;
  
  g_mt_player = MT_PLAYER;

  // convert doom mobj types to shared type
  for (i = 0; i < NUMMOBJTYPES; ++i) {
    mobjinfo_p = &doom_mobjinfo[i];
    
    mobjinfo[i].doomednum    = mobjinfo_p->doomednum;
    mobjinfo[i].spawnstate   = mobjinfo_p->spawnstate;
    mobjinfo[i].spawnhealth  = mobjinfo_p->spawnhealth;
    mobjinfo[i].seestate     = mobjinfo_p->seestate;
    mobjinfo[i].seesound     = mobjinfo_p->seesound;
    mobjinfo[i].reactiontime = mobjinfo_p->reactiontime;
    mobjinfo[i].attacksound  = mobjinfo_p->attacksound;
    mobjinfo[i].painstate    = mobjinfo_p->painstate;
    mobjinfo[i].painchance   = mobjinfo_p->painchance;
    mobjinfo[i].painsound    = mobjinfo_p->painsound;
    mobjinfo[i].meleestate   = mobjinfo_p->meleestate;
    mobjinfo[i].missilestate = mobjinfo_p->missilestate;
    mobjinfo[i].deathstate   = mobjinfo_p->deathstate;
    mobjinfo[i].xdeathstate  = mobjinfo_p->xdeathstate;
    mobjinfo[i].deathsound   = mobjinfo_p->deathsound;
    mobjinfo[i].speed        = mobjinfo_p->speed;
    mobjinfo[i].radius       = mobjinfo_p->radius;
    mobjinfo[i].height       = mobjinfo_p->height;
    mobjinfo[i].mass         = mobjinfo_p->mass;
    mobjinfo[i].damage       = mobjinfo_p->damage;
    mobjinfo[i].activesound  = mobjinfo_p->activesound;
    mobjinfo[i].flags        = mobjinfo_p->flags;
    mobjinfo[i].raisestate   = mobjinfo_p->raisestate;
    mobjinfo[i].droppeditem  = mobjinfo_p->droppeditem;
    mobjinfo[i].crashstate   = 0; // not in doom
    mobjinfo[i].flags2       = 0; // not in doom
  }
}

static void dsda_InitHeretic(void) {
  int i;
  heretic_mobjinfo_t* mobjinfo_p;

  dsda_AllocateMobjInfo(HERETIC_NUMMOBJTYPES);
  dsda_SetStates(heretic_states, HERETIC_NUMSTATES);
  dsda_SetSpriteNames(heretic_sprnames, HERETIC_NUMSPRITES);
  
  // HERETIC_TODO: of course, 2 levels requires complete rework...
  weaponinfo = wpnlev1info;
  
  g_mt_player = HERETIC_MT_PLAYER;
  
  // convert heretic mobj types to shared type
  for (i = 0; i < HERETIC_NUMMOBJTYPES; ++i) {
    mobjinfo_p = &heretic_mobjinfo[i];
    
    mobjinfo[i].doomednum    = mobjinfo_p->doomednum;
    mobjinfo[i].spawnstate   = mobjinfo_p->spawnstate;
    mobjinfo[i].spawnhealth  = mobjinfo_p->spawnhealth;
    mobjinfo[i].seestate     = mobjinfo_p->seestate;
    mobjinfo[i].seesound     = mobjinfo_p->seesound;
    mobjinfo[i].reactiontime = mobjinfo_p->reactiontime;
    mobjinfo[i].attacksound  = mobjinfo_p->attacksound;
    mobjinfo[i].painstate    = mobjinfo_p->painstate;
    mobjinfo[i].painchance   = mobjinfo_p->painchance;
    mobjinfo[i].painsound    = mobjinfo_p->painsound;
    mobjinfo[i].meleestate   = mobjinfo_p->meleestate;
    mobjinfo[i].missilestate = mobjinfo_p->missilestate;
    mobjinfo[i].deathstate   = mobjinfo_p->deathstate;
    mobjinfo[i].xdeathstate  = mobjinfo_p->xdeathstate;
    mobjinfo[i].deathsound   = mobjinfo_p->deathsound;
    mobjinfo[i].speed        = mobjinfo_p->speed;
    mobjinfo[i].radius       = mobjinfo_p->radius;
    mobjinfo[i].height       = mobjinfo_p->height;
    mobjinfo[i].mass         = mobjinfo_p->mass;
    mobjinfo[i].damage       = mobjinfo_p->damage;
    mobjinfo[i].activesound  = mobjinfo_p->activesound;
    mobjinfo[i].flags        = mobjinfo_p->flags;
    mobjinfo[i].raisestate   = 0; // not in heretic
    mobjinfo[i].droppeditem  = 0; // not in heretic
    mobjinfo[i].crashstate   = mobjinfo_p->crashstate;
    mobjinfo[i].flags2       = mobjinfo_p->flags2;
  }
}

void dsda_InitGlobal(void) {
  heretic = M_CheckParm ("-heretic");
  
  heretic ? dsda_InitHeretic() : dsda_InitDoom();
}
