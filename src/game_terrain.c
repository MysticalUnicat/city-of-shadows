#include "game_local.h"

#include <stdlib.h>

/* This terrain implementation uses hexes, with of course a 2d array of data for the cells.
 * 
 * each cell can represent unpassible mountains that need to be mined, properties of the dirt to change how it looks and behaves, water depth and moisture levels.
 *
 * it may be overkill but it fits in 32bits per hex cell and its easy to add/remove features i will need but this allows me to consider advanced features in the first pass to find future issues.
 *
 * anyway, lets get to some math with the hexagons used and which are used where...
 *
 * https://www.redblobgames.com/grids/hexagons/
 *
 * with an orientation of "flat-top" and an assumed size of 1, the height is sqrt(3) and the width is 2.
 *
 * the vertical spacing is sqrt(3) and the horizontal spacing is 3/2
 *
 * the coordinates is "odd-q" vertical layout.
 *
 * The way the terrain is rendered is not per hexagon, but the triangles between each 3 hexagon.
 *
 * So, the border hexes are partially rendered as well.
 *
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

  // damange for rock face, depth for water depth of oceans and stuff
  uint32_t damage_depth : 6;

  uint32_t unused : 2;
};

static_assert(sizeof(struct TerrainCell) == sizeof(uint32_t));

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

