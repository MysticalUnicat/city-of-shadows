#include "game_local.h"

enum Terrain {
  Terrain_Soil,
  Terrain_Sand,
  Terrain_LichenCoveredDirt,
  Terrain_Gravel,
  Terrain_ShallowWater,
  Terrain_DeepWater,
  #define STONE_TERRAIN(NAME, MAJOR) Terrain_ ## MAJOR ## NAME,
  STONE_KINDS(STONE_TERRAIN, Rough)
  STONE_KINDS(STONE_TERRAIN, Hewn)
  STONE_KINDS(STONE_TERRAIN, Polished)
  STONE_KINDS(STONE_TERRAIN, Solid)
  #undef STONE_TERRAIN
};

