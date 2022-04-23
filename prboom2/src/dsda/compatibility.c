//
// Copyright(C) 2021 by Ryan Krafnick
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
//	DSDA Compatibility
//

#include "md5.h"
#include "doomdata.h"
#include "doomstat.h"
#include "doomtype.h"
#include "w_wad.h"
#include "lprintf.h"

#include "compatibility.h"

typedef struct {
  const char* cksum_string;
  const signed char options[];
} dsda_compatibility_t;

static const dsda_compatibility_t eternal_doom_25 = {
  "6da6fcba8089161bdec6a1d3f6c8d60f",
  { comp_stairs, comp_vile, -1, -1 }
};

static const dsda_compatibility_t doomsday_of_uac_e1m8 = {
  "32fc3115a3162b623f0d0f4e7dee6861",
  { comp_666, -1, -1 }
};

static const dsda_compatibility_t hell_revealed_map19 = {
  "811a0c97777a198bc9b2bb558cb46e6a",
  { comp_pain, -1, -1 }
};

static const dsda_compatibility_t roger_ritenour_phobos_map03 = {
  "8fa29398776146189396aa1ac6bb9e13",
  { comp_floors, -1, -1 }
};

static const dsda_compatibility_t hell_revealed_map26 = {
  "145c4dfcf843f2b92c73036ba0e1d98a",
  { comp_vile, -1, -1 }
};

static const dsda_compatibility_t hell_to_pay_map14 = {
  "5379c080299eb961792b50ad96821543",
  { comp_vile, -1, -1 }
};

static const dsda_compatibility_t hell_to_pay_map22 = {
  "7837b5334a277f107515d649bcefb682",
  { comp_vile, -1, -1 }
};

static const dsda_compatibility_t icarus_map24 = {
  "2eeb1e12fa9f9545de9d99990a4a78e5",
  { comp_vile, -1, -1 }
};

static const dsda_compatibility_t plutonia2_map32 = {
  "65a53a09a09525ae42ea210bf879cd37",
  { comp_vile, -1, -1 }
};

static const dsda_compatibility_t requiem_map23 = {
  "2499cf9a9351be9bc4e9c66fc9f291a7",
  { comp_vile, -1, -1 }
};

static const dsda_compatibility_t the_waterfront_map01 = {
  "3ca5493feff2e27bfd4181e6c4a3c2bf",
  { comp_vile, -1, -1 }
};

static const dsda_compatibility_t gather2_map05_and_darkside_map01 = {
  "cbdfefac579a62de8f1b48ca4a09d381",
  { comp_vile, -1, -1 }
};

static const dsda_compatibility_t reverie_map18 = {
  "c7a2fafb0afb2632c50ad625cdb50e51",
  { comp_vile, -1, -1 }
};

static const dsda_compatibility_t project_x_map14 = {
  "9e5724bc6135aa6f86ee54fd4d91f1e2",
  { comp_vile, -1, -1 }
};

static const dsda_compatibility_t archie_map01 = {
  "01899825ffeae016d39c02a7da4b218f",
  { comp_vile, -1, -1 }
};

static const dsda_compatibility_t seej_map01 = {
  "1d9f3afdc2517c2e450491ed13896712",
  { comp_vile, -1, -1 }
};

static const dsda_compatibility_t sixpack2_map02 = {
  "0ae745a3ab86d15fb2fb74489962c421",
  { comp_vile, -1, -1 }
};

static const dsda_compatibility_t squadron_417_map21 = {
  "2ea635c6b6aec76b6bc77448dab22f9a",
  { comp_vile, -1, -1 }
};

static const dsda_compatibility_t mayhem_2013_map05 = {
  "1e998262ee319b7d088e01de782e6b41",
  { comp_vile, -1, -1 }
};

static const dsda_compatibility_t imps_are_ghost_gods_map01 = {
  "a81e2734f735a82720d8e0f1442ba0c9",
  { comp_vile, -1, -1 }
};

static const dsda_compatibility_t confinement_map31 = {
  "aad7502cb39bc050445e17b15f72356f",
  { comp_vile, -1, -1 }
};

static const dsda_compatibility_t conf256b_map07 = {
  "5592ea1ca3b8ee0dbb2cb352aaa00911",
  { comp_pain, -1, -1 }
};

static const dsda_compatibility_t conf256b_map12 = {
  "cecedae33b970f2bf7f8b8631da0c8dd",
  { comp_vile, -1, -1 }
};

static const dsda_compatibility_t sunlust_map30 = {
  "41efe03223e41935849f64114c5cb471",
  { comp_telefrag, -1, -1 }
};

static const dsda_compatibility_t tnt_map30 = {
  "42b68b84ff8e55f264c31e6f4cfea82d",
  { comp_stairs, -1, -1 }
};

static const dsda_compatibility_t intercep2_map03 = {
  "86587e4f8c8086991c8fc5c1ccfd30b9",
  { -1, comp_ledgeblock, -1 }
};

static const dsda_compatibility_t skulltiverse_map02 = {
  "b3fa4a18b31bd96e724f9aab101776a1",
  { -1, comp_ledgeblock, -1 }
};

static const dsda_compatibility_t* entry_0[] = {
  &archie_map01,
  &sixpack2_map02,
  NULL
};

static const dsda_compatibility_t* entry_1[] = {
  &hell_revealed_map26,
  &seej_map01,
  &mayhem_2013_map05,
  NULL
};

static const dsda_compatibility_t* entry_2[] = {
  &icarus_map24,
  &requiem_map23,
  &squadron_417_map21,
  NULL
};

static const dsda_compatibility_t* entry_3[] = {
  &doomsday_of_uac_e1m8,
  &the_waterfront_map01,
  NULL
};

static const dsda_compatibility_t* entry_4[] = {
  &sunlust_map30,
  &tnt_map30,
  NULL
};

static const dsda_compatibility_t* entry_5[] = {
  &hell_to_pay_map14,
  &conf256b_map07,
  NULL
};

static const dsda_compatibility_t* entry_6[] = {
  &eternal_doom_25,
  &plutonia2_map32,
  NULL
};

static const dsda_compatibility_t* entry_7[] = {
  &hell_to_pay_map22,
  NULL
};

static const dsda_compatibility_t* entry_8[] = {
  &hell_revealed_map19,
  &roger_ritenour_phobos_map03,
  &intercep2_map03,
  NULL
};

static const dsda_compatibility_t* entry_9[] = {
  &project_x_map14,
  NULL
};

static const dsda_compatibility_t* entry_a[] = {
  &imps_are_ghost_gods_map01,
  &confinement_map31,
  NULL
};

static const dsda_compatibility_t* entry_b[] = {
  &skulltiverse_map02,
  NULL
};

static const dsda_compatibility_t* entry_c[] = {
  &gather2_map05_and_darkside_map01,
  &reverie_map18,
  &conf256b_map12,
  NULL
};

static const dsda_compatibility_t* entry_d[] = {
  NULL
};

static const dsda_compatibility_t* entry_e[] = {
  NULL
};

static const dsda_compatibility_t* entry_f[] = {
  NULL
};

// This is clumsy but I couldn't figure out how to inline everything
static const dsda_compatibility_t** level_compatibilities[16] = {
  entry_0,
  entry_1,
  entry_2,
  entry_3,
  entry_4,
  entry_5,
  entry_6,
  entry_7,
  entry_8,
  entry_9,
  entry_a,
  entry_b,
  entry_c,
  entry_d,
  entry_e,
  entry_f,
};

static void dsda_MD5UpdateLump(int lump, struct MD5Context *md5)
{
  MD5Update(md5, W_CacheLumpNum(lump), W_LumpLength(lump));
  W_UnlockLumpNum(lump);
}

static void dsda_GetLevelCheckSum(int lump, byte cksum[16])
{
  struct MD5Context md5;

  MD5Init(&md5);

  dsda_MD5UpdateLump(lump + ML_LABEL, &md5);
  dsda_MD5UpdateLump(lump + ML_THINGS, &md5);
  dsda_MD5UpdateLump(lump + ML_LINEDEFS, &md5);
  dsda_MD5UpdateLump(lump + ML_SIDEDEFS, &md5);
  dsda_MD5UpdateLump(lump + ML_SECTORS, &md5);

  // ML_BEHAVIOR when it becomes applicable to comp options

  MD5Final(cksum, &md5);
}


// For casual players that aren't careful about setting complevels,
//   this function will apply comp options to automatically fix some issues
//   that appear when playing wads in mbf21 (since this is the default).
void dsda_ApplyLevelCompatibility(int lump) {
  unsigned int i;
  byte cksum[16];
  char cksum_string[33];
  const dsda_compatibility_t** level_compatibility;

  if (demorecording || demoplayback || !mbf21) return;

  dsda_GetLevelCheckSum(lump, cksum);

  for (i = 0; i < 16; i++)
    sprintf(&cksum_string[i * 2], "%02x", cksum[i]);
  cksum_string[32] = '\0';

  lprintf(LO_INFO, "Level checksum: %s\n", cksum_string);

  if (cksum_string[0] >= 'a')
    i = cksum_string[0] - 'a' + 10;
  else
    i = cksum_string[0] - '0';

  level_compatibility = level_compatibilities[i];

  while (*level_compatibility) {
    if (!strncmp((*level_compatibility)->cksum_string, cksum_string, 32)) {
      const signed char* option;

      for (option = (*level_compatibility)->options; *option != -1; option++) {
        comp[*option] = 1;
        lprintf(LO_INFO, "Automatically setting comp option %d on\n", *option);
      }

      for (option++; *option != -1; option++) {
        comp[*option] = 0;
        lprintf(LO_INFO, "Automatically setting comp option %d off\n", *option);
      }

      return;
    }

    level_compatibility++;
  }
}
