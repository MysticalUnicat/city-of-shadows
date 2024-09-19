#include "game_local.h"

#include <stdlib.h>

/* grid that is data is composed of hexagons
 * 
 *         +-------+
 *        /         \
 *       /           \
 *      +             +-------+
 *       \           /         \
 *        \         /           \
 *         +-------+             +
 *        /         \           /
 *       /           \         /
 *      +             +-------+
 *       \           /
 *        \         /
 *         +-------+
 *
 * each hexagon has:
 *   terrain type (various dirts, various clay, various soil, various stone)
 *   2-bits called variation, this is added in a way during rendering that allows variation in cells picked to render but be stable when there are changes
 *
 * when rendering, we decompose the terrain into equallateral triangles using the centerpoint of the hexagons as the vertexes.
 * 
 *         +-------+
 *        /         \
 *       /           \
 *      +      +      +-------+
 *       \     |     /         \
 *        \    |    /           \
 *         +---|---+      +      +
 *        /    |    \           /
 *       /     |     \         /
 *      +      +      +-------+
 *       \           /
 *        \         /
 *         +-------+
 * 
 * each rendering triangle gets a variation number as a sum of hexagons variation values
 *
 * supporting any number of matching textures/features for that triangle.
 *
 * this is done with 4 textures, a mask (triangle) and three source textures (rectangle) to support any number of dirts, clays, soils, stones etc..
 * for example, if we choose to have 4 kinds (dirt, clay, water, stone), we would have 4**3 variation buckets (64)
 */

// features:
//   rock face = dirt == 0, walkable == 0, silt == 0, clay == 0
//   
struct TerrainCell {
  // rock faces, are not walkable
  uint32_t walkable     : 1;

  // human activity has happend here, usally mining, no rock face
  uint32_t developed    : 1;

  // randomized at the start
  uint32_t variation    : 2;

  // always filled, 16 types of stone
  uint32_t stone_kind   : 4;

  // histogram of the soil
  // if all 3 is 0, then the tile is stone. if walkable is 0 and developed is 0, its a rock face
  // kind of encodes the depth of the soil as digging/filtering actions only reduce these by 1
  // plant fertility determined by how equal they all are, with water included
  uint32_t dirt         : 4;
  uint32_t silt         : 4;
  uint32_t clay         : 4;

  uint32_t moisture     : 4;

  // liquid kind (4 kinds, water, alchohol, blood, vomit)
  uint32_t liquid_kind  : 2;

  // damange for rock face, depth for water depth of oceans and stuff
  uint32_t damage_depth : 6;
};

ECS_DECLARE_COMPONENT(Terrain, {
  uint32_t width;
  uint32_t height;
  struct TerrainCell * cells;
})

ECS_COMPONENT(Terrain)

static inline uint32_t mountain_noise(int64_t x, int64_t y) {
  // todo: real implementation
  return rand() % 1;
}

static inline uint32_t stone_noise(int64_t x, int64_t y) {
  // todo: real implementation
  return rand() % 15;
}

static inline uint32_t dirt_noise(int64_t x, int64_t y) {
  // todo: real implementation
  return rand() % 15;
}

static inline uint32_t silt_noise(int64_t x, int64_t y) {
  // todo: real implementation
  return rand() % 15;
}

static inline uint32_t clay_noise(int64_t x, int64_t y) {
  // todo: real implementation
  return rand() % 15;
}

static inline uint32_t moisture_noise(int64_t x, int64_t y) {
  // todo: real implementation
  return rand() % 15;
}

static inline uint32_t water_noise(int64_t x, int64_t y) {
  // todo: real implementation
  return rand() % 63;
}

void Terrain_create(struct Terrain * terrain, uint32_t width, uint32_t height, int64_t world_x, int64_t world_y) {
  terrain->width = width;
  terrain->width = height;
  struct TerrainCell * cells = memory_alloc(width * height * sizeof(*cells), alignof(*cells));

  terrain->cells = cells;

  for(uint32_t x = 0; x < width; x++) {
    int64_t wx = world_x + x;
    for(uint32_t y = 0; y < height; y++) {
      int64_t wy = world_y + y;
      uint32_t is_mountain = mountain_noise(wx, wy);
      cells[y * width + x].walkable = is_mountain ? 0 : 1;
      cells[y * width + x].developed = 0;
      cells[y * width + x].variation = rand() % 3; // todo: better random
      cells[y * width + x].stone_kind = stone_noise(wx, wy);
      cells[y * width + x].dirt = is_mountain ? 0 : dirt_noise(wx, wy);
      cells[y * width + x].silt = is_mountain ? 0 : silt_noise(wx, wy);
      cells[y * width + x].clay = is_mountain ? 0 : clay_noise(wx, wy);
      cells[y * width + x].moisture = is_mountain ? 0 : moisture_noise(wx, wy);
      cells[y * width + x].liquid_kind = 0;
      cells[y * width + x].damage_depth = is_mountain ? 0 : water_noise(wx, wy);
    }
  }
}

/*

the data, being a square grid, needs a transform to and from the hex reality.

our hexes are flat on top, and shift on odd.

[a ][b ][c ]
  [d ][e ][f ]
[g ][h ][i ]

3x3 to 8 triangles, 2 strips (height - 1), each strip is 4 triangles ((width - 1) * 2)

simplified: (height - 1) * (width - 1) * 2

= vertex shader

triangle_index: 0
row: 0
a = triangle_index >> 1
b = (triangle_index >> 1) + 1
d = (triangle_index >> 1) + width

triangle_index: 1
row: 0
d = (triangle_index >> 1) + width
b = (triangle_index >> 1) + 1
e = (triangle_index >> 1) + width + 1

triangle_index: 2
row: 0
b = triangle_index >> 1
c = (triangle_index >> 1) + 1
e = (triangle_index >> 1) + width

triangle_index: 3
row: 0
e = (triangle_index >> 1) + width
c = (triangle_index >> 1) + 1
f = (triangle_index >> 1) + width + 1

triangle_index: 4
row: 1
g = (triangle_index >> 1) + width + row;
d = (triangle_index >> 1) - row;
h = (triangle_index >> 1) + width + 1 - row;

uvec2 data_size = texture_size();
int width = data_size.x;
int height = data_size.y;
int n_columns = (width - 1) * 2;
int n_rows = (height - 1);

int bias = 0;

int triangle_index = vertex_index / 3 + bias;
int row = triangle_index / n_columns;
int column = triangle_index % n_columns;

if(row & 1) {
  if(column & 1) {
  } else {
    a_index = 
  }
} else {
  if(column & 1) {
    a_index = (triangle_index / 2) + width;
    b_index = (triangle_index / 2) + 1;
    c_index = (triangle_index / 2) + width + 1;
  } else {
    a_index = (triangle_index / 2);
    b_index = (triangle_index / 2) + 1;
    c_index = (triangle_index / 2) + width;
  }
}

ivec3 xi = ivec3(a_index, b_index, c_index) / width;
ivec3 yi = ivec3(a_index, b_index, c_index) % width;

vec3 x = vec3(xi);
vec3 y = vec3(yi) + vec3(yi & 1) * 0.5;

uint current_vertex = vertex_index % 3;
vec2 position = vec2(x[current_vertex], y[current_vertex]);

// use provoking vertexes and flat outputs to reduce memory throughput by a third
if(current_vertex == 0) {
  TerrainCell a = read_cell(a_index);
  TerrainCell b = read_cell(b_index);
  TerrainCell c = read_cell(c_index);

  uint variation = a.variation + b.variation + c.variation;

  // clay texture, sand texture, silt texture, soil texture, rock texture
}

= fragment shader

*/

