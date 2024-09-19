#include "ui.h"
#include "format.h"
#include "draw.h"
#include "resource.h"
#include "display.h"
#include "font.h"

struct uilib_dstack_child {
  uint32_t hole;
  float weight;
};

struct uilib_named_scope {
  uint64_t hash;
  uint32_t query_capacity;
  float *query_rects;
  uint32_t slot_capacity;
  uint32_t *slot;
};

struct uilib_tree_scope {
  uint32_t named_scope_index;
  uint32_t font_index;
  float font_size;
  uint32_t font_color;

  float weight;

  bool handle_alignment;
  float align_x;
  float align_y;

  union {
    struct {
      uint32_t hole;
      uint32_t num_children;
    } stack;
    struct {
      bool vertical;
      uint32_t hole;
      uint32_t child_pmark;
      uint32_t child_mmark;
      float total_weight;
      uint32_t children_capacity;
      uint32_t children_length;
      struct uilib_dstack_child *children;
    } dstack;
  };

  void (*begin_child)(struct uilib *, struct uilib_tree_scope *);
  void (*end_child)(struct uilib *, struct uilib_tree_scope *);
  void (*end_scope)(struct uilib *, struct uilib_tree_scope *);
  void *user_data;
};

struct uilib {
  uilib_Input input;
  uilib_Output *output;

  uint32_t font_capacity;
  uint32_t font_length;
  struct uilib_Font *font;

  uint32_t named_scope_capacity;
  uint32_t named_scope_length;
  struct uilib_named_scope *named_scope;

  uint32_t tree_scope_capacity;
  uint32_t tree_scope_length;
  struct uilib_tree_scope *tree_scope;

  uint32_t program_capacity;
  uint32_t program_length;
  uint32_t *program;

  uint32_t *ip;

  uint32_t memory_capacity;
  uint32_t memory_length;
  uint32_t *memory;

  uint32_t *mp;

  //uint32_t vertexes_capacity;
  //uint32_t vertexes_length;
  //struct uilib_vertex *vertexes;

  //uint32_t indexes_capacity;
  //uint32_t indexes_length;
  //uint32_t *indexes;

  uint32_t groups_capacity;
  uint32_t groups_length;
  struct uilib_group *groups;

  uint32_t texture_id;

#ifdef UNICAT_UI_ENABLE_FREETYPE
  uint32_t (*freetype_texture_callback)(uint32_t length, void ** ptr);
#endif

  struct {
    uint32_t texture_id;
    uint32_t num_vertexes;
    uint32_t num_indexes;
    uint32_t num_groups;
  } stats;
};

enum {
  P_TERMINATE,
  P_EMPTY_FILL,
  P_COLOR_FILL,
  P_IMAGE,
  P_TEXT,
  P_BACKGROUND_COLOR,
  P_BORDER,
  P_STACK,
  P_HORIZONTAL_STACK,
  P_VERTICAL_STACK,
  P_SIZE,
  P_WIDTH,
  P_HEIGHT,
  P_ALIGN,
  P_ALIGN_X,
  P_ALIGN_Y,
  P_PADDING,
  P_QUERY,
};

// util ---------------------------------------------------------------------------------------------------------------
static inline bool uilib__grow(void **ptr, uint32_t element_size, uint32_t length, uint32_t *capacity) {
  uint32_t old_capacity = *capacity;
  if(length > old_capacity) {
    *capacity = length + 1;
    *capacity += *capacity >> 1;
    void * new_ptr = memory_realloc(*ptr, element_size * old_capacity, element_size * *capacity, 4);
    if(new_ptr == NULL) {
      return false;
    }
    *ptr = new_ptr;
  }
  return true;
}

// tree ---------------------------------------------------------------------------------------------------------------
static inline void uilib__tree__space_for(struct uilib *uilib, uint32_t count) {
  uilib__grow((void **)&uilib->tree_scope, sizeof(*uilib->tree_scope), uilib->tree_scope_length + count,
                  &uilib->tree_scope_capacity);
}

static inline struct uilib_tree_scope *uilib__tree__top_scope(struct uilib *uilib) {
  assert(uilib->tree_scope_length > 0);
  return &uilib->tree_scope[uilib->tree_scope_length - 1];
}

static inline void uilib__tree__begin_child(struct uilib *uilib) {
  if(uilib->tree_scope_length > 0) {
    struct uilib_tree_scope *scope = uilib__tree__top_scope(uilib);
    if(scope->begin_child) {
      scope->begin_child(uilib, scope);
    }
  }
}

static inline void uilib__tree__end_child(struct uilib *uilib) {
  if(uilib->tree_scope_length > 0) {
    struct uilib_tree_scope *scope = uilib__tree__top_scope(uilib);
    if(scope->end_child) {
      scope->end_child(uilib, scope);
    }
  }
}

static inline void uilib__tree__end_scope(struct uilib *uilib) {
  if(uilib->tree_scope_length > 0) {
    struct uilib_tree_scope *scope = uilib__tree__top_scope(uilib);
    uilib->tree_scope_length -= 1;
    if(scope->end_scope) {
      scope->end_scope(uilib, scope);
    }
  }
}

static inline void uilib__tree__begin_scope(struct uilib *uilib,
                                                void (*begin_child)(struct uilib *, struct uilib_tree_scope *),
                                                void (*end_child)(struct uilib *, struct uilib_tree_scope *),
                                                void (*end_scope)(struct uilib *, struct uilib_tree_scope *)) {
  uilib__tree__space_for(uilib, 1);
  struct uilib_tree_scope *parent_scope = uilib->tree_scope_length > 0 ? uilib__tree__top_scope(uilib) : NULL;
  uilib->tree_scope_length += 1;
  struct uilib_tree_scope *scope = uilib__tree__top_scope(uilib);
  if(parent_scope != NULL) {
    scope->named_scope_index = parent_scope->named_scope_index;
    scope->font_index = parent_scope->font_index;
    scope->font_size = parent_scope->font_size;
    scope->font_color = parent_scope->font_color;
    if(parent_scope->handle_alignment) {
      scope->handle_alignment = true;
      scope->align_x = parent_scope->align_x;
      scope->align_y = parent_scope->align_y;
    }
  } else {
    scope->named_scope_index = -1;
    scope->font_index = 0;
    scope->font_size = 16;
    scope->font_color = 0xFFFFFFFF;
  }
  scope->begin_child = begin_child;
  scope->end_child = end_child;
  scope->end_scope = end_scope;
}

// program ------------------------------------------------------------------------------------------------------------
static inline void uilib__program__space_for(struct uilib *uilib, uint32_t count) {
  uilib__grow((void **)&uilib->program, sizeof(*uilib->program), uilib->program_length + count, &uilib->program_capacity);
}

static inline void uilib__program__initialize(struct uilib *uilib) {
  uilib->program_length = 0;
}

static inline uint32_t uilib__program__mark(struct uilib *uilib) {
  return uilib->program_length;
}

static inline uint32_t uilib__program__mark_size(struct uilib *uilib, uint32_t mark) {
  return uilib->program_length - mark;
}

static inline void uilib__program__reserve(struct uilib *uilib, uint32_t space) {
  uilib__program__space_for(uilib, space);
  uilib->program_length += space;
}

static inline uint32_t uilib__program__dig(struct uilib *uilib, uint32_t space) {
  uint32_t result = uilib__program__mark(uilib);
  uilib__program__reserve(uilib, space);
  return result;
}

static inline void uilib__program__emit_u32(struct uilib *uilib, uint32_t value) {
  uilib__program__space_for(uilib, 1);
  uilib->program[uilib->program_length++] = value;
}

static inline void uilib__program__emit_f32(struct uilib *uilib, float value) {
  union {
    uint32_t u;
    float f;
  } flip = {.f = value};
  uilib__program__emit_u32(uilib, flip.u);
}

static inline void uilib__program__fill_u32(struct uilib *uilib, uint32_t hole, uint32_t value) {
  uilib->program[hole] = value;
}

static inline void uilib__program__fill_f32(struct uilib *uilib, uint32_t hole, float value) {
  union {
    uint32_t u;
    float f;
  } flip = {.f = value};
  uilib__program__fill_u32(uilib, hole, flip.u);
}

// ip -----------------------------------------------------------------------------------------------------------------
static inline void uilib__ip__initialize(struct uilib *uilib) {
  uilib->ip = uilib->program;
}

static inline void uilib__ip__advance(struct uilib *uilib, uint32_t count) {
  uilib->ip += count;
}

static inline uint32_t uilib__ip__read_u32(struct uilib *uilib) {
  return *uilib->ip++;
}

static inline float uilib__ip__read_f32(struct uilib *uilib) {
  union {
    uint32_t u;
    float f;
  } flip = {.u = uilib__ip__read_u32(uilib)};
  return flip.f;
}

// memory -------------------------------------------------------------------------------------------------------------
static inline void uilib__memory__space_for(struct uilib *uilib, uint32_t count) {
  uilib__grow((void **)&uilib->memory, sizeof(*uilib->memory), uilib->memory_length + count, &uilib->memory_capacity);
}

static inline void uilib__memory__initialize(struct uilib *uilib) {
  uilib->memory_length = 0;
}

static inline uint32_t uilib__memory__mark(struct uilib *uilib) {
  return uilib->memory_length;
}

static inline uint32_t uilib__memory__mark_size(struct uilib *uilib, uint32_t mark) {
  return uilib->memory_length - mark;
}

static inline void uilib__memory__reserve(struct uilib *uilib, uint32_t space) {
  uilib__memory__space_for(uilib, space);
  uilib->memory_length += space;
}

static inline uint32_t uilib__memory__dig(struct uilib *uilib, uint32_t space) {
  uint32_t result = uilib__memory__mark(uilib);
  uilib__memory__reserve(uilib, space);
  return result;
}

static inline void uilib__memory__fill_u32(struct uilib *uilib, uint32_t hole, uint32_t value) {
  assert(hole < uilib->memory_length);
  uilib->memory[hole] = value;
}

static inline void uilib__memory__fill_f32(struct uilib *uilib, uint32_t hole, float value) {
  union {
    uint32_t u;
    float f;
  } flip = {.f = value};
  uilib__memory__fill_u32(uilib, hole, flip.u);
}

// float --------------------------------------------------------------------------------------------------------------
static inline float uilib__float__min(float a, float b) {
  return a < b ? a : b;
}

static inline float uilib__float__max(float a, float b) {
  return a > b ? a : b;
}

static inline float uilib__float__clamp(float x, float min, float max) {
  return uilib__float__min(uilib__float__max(x, min), max);
}

static inline uint8_t uilib__float__to_color_channel(float x) {
  return (uint8_t)uilib__float__clamp(x * 255.0f, 0.0f, 255.0f);
}

static inline uint32_t uilib__color__pack(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  return ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | a;
}

static inline void uilib__color__unpack(uint32_t color, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a) {
  *r = (color >> 24) & 0xFF;
  *g = (color >> 16) & 0xFF;
  *b = (color >> 8) & 0xFF;
  *a = (color >> 0) & 0xFF;
}

// mp -----------------------------------------------------------------------------------------------------------------
static inline void uilib__mp__initialize(struct uilib *uilib) {
  uilib->mp = uilib->memory;
}

static inline void uilib__mp__advance(struct uilib *uilib, uint32_t count) {
  uilib->mp += count;
}

static inline uint32_t uilib__mp__read_u32(struct uilib *uilib) {
  return *uilib->mp++;
}

static inline float uilib__mp__read_f32(struct uilib *uilib) {
  union {
    uint32_t u;
    float f;
  } flip = {.u = uilib__mp__read_u32(uilib)};
  return flip.f;
}

// vertex -------------------------------------------------------------------------------------------------------------
static inline void uilib__vertex__initialize(struct uilib *uilib) {
  uilib->output->num_vertexes = 0;
}

static inline uint32_t uilib__vertex__emit(struct uilib *uilib, float x, float y, float s, float t, uint8_t r,
                                               uint8_t g, uint8_t b, uint8_t a) {
  float xy[] = {x, y};
  float rgba[] = {r, g, b, a};
  float st[] = {s, t};
  memory_SubBuffer_write(&uilib->output->xy_sub_buffer, uilib->output->num_vertexes, 1, memory_Format_Float32, 0, xy);
  memory_SubBuffer_write(&uilib->output->rgba_sub_buffer, uilib->output->num_vertexes, 1, memory_Format_Unorm8, 0, rgba);
  memory_SubBuffer_write(&uilib->output->st_sub_buffer, uilib->output->num_vertexes, 1, memory_Format_Float32, 0, st);
  return uilib->output->num_vertexes++;
}

// index --------------------------------------------------------------------------------------------------------------
static inline void uilib__index__initialize(struct uilib *uilib) {
  uilib->output->num_indexes = 0;
}

static inline uint32_t uilib__index__emit(struct uilib *uilib, uint32_t vertex_index) {
  // requires caller to have called uilib__index__space_for(uilib, N) before
  memory_SubBuffer_write(&uilib->output->index_sub_buffer, uilib->output->num_indexes, 1, memory_Format_Sint32, 0, &vertex_index);
  return uilib->output->num_indexes++;
}

// group --------------------------------------------------------------------------------------------------------------
static inline void uilib__group__initialize(struct uilib *uilib) {
  uilib->output->num_groups = 0;
}

static inline struct uilib_OutputGroup *uilib__group__top(struct uilib *uilib) {
  return &uilib->output->groups[uilib->groups_length - 1];
}

static inline void uilib__group__prepare(struct uilib *uilib) {
  struct uilib_OutputGroup *group = uilib__group__top(uilib);
  group->texture_id = uilib->texture_id;
  group->index = uilib->output->num_indexes;
  group->length = 0;
}

static inline void uilib__group__finalize(struct uilib *uilib) {
  struct uilib_OutputGroup *group = uilib__group__top(uilib);
  group->length = uilib->output->num_indexes - group->index;
}

static inline void uilib__group__advance(struct uilib *uilib) {
  if(uilib->groups_length > 0) {
    uilib__group__finalize(uilib);
  }
  uilib->groups_length++;
  uilib__group__prepare(uilib);
}

// render -------------------------------------------------------------------------------------------------------------
void uilib_set_texture(struct uilib *uilib, uint32_t texture_id) {
  if(uilib->texture_id != texture_id) {
    uilib->texture_id = texture_id;
    uilib__group__advance(uilib);
  }
}

void uilib_draw_rectangle(struct uilib *uilib, float x, float y, float width, float height,
                                                     float s0, float t0, float s1, float t1, uint8_t r, uint8_t g,
                                                     uint8_t b, uint8_t a) {
  float x0 = x;
  float y0 = y;
  float x1 = x + width;
  float y1 = y + height;
  uint32_t A = uilib__vertex__emit(uilib, x0, y0, s0, t0, r, g, b, a);
  uint32_t B = uilib__vertex__emit(uilib, x0, y1, s0, t1, r, g, b, a);
  uint32_t C = uilib__vertex__emit(uilib, x1, y1, s1, t1, r, g, b, a);
  uint32_t D = uilib__vertex__emit(uilib, x1, y0, s1, t0, r, g, b, a);
  uilib__index__emit(uilib, A);
  uilib__index__emit(uilib, B);
  uilib__index__emit(uilib, C);
  uilib__index__emit(uilib, A);
  uilib__index__emit(uilib, C);
  uilib__index__emit(uilib, D);
}

static inline void uilib__stats__initialize(uilib *uilib) {
  memory_clear(&uilib->stats, sizeof(uilib->stats));
  uilib->stats.texture_id = -1;
}

static inline void uilib__stats__set_texture(uilib *uilib, uint32_t texture_id) {
  if(uilib->stats.texture_id != texture_id) {
    uilib->stats.texture_id = texture_id;
    uilib->stats.num_groups += 1;
  }
}

static inline void uilib__stats__draw_rectangle(struct uilib *uilib) {
  uilib->stats.num_vertexes += 4;
  uilib->stats.num_indexes += 6;
}

// ====================================================================================================================
static inline void uilib__layout(struct uilib *, float, float, float, float, float *, float *);
static inline void uilib__render(struct uilib *, float, float, float, float);

// ====================================================================================================================
// empty fill
// program: r:float, g:float, b:float, a:float
//  memory: unused
static void uilib__empty_fill__layout(struct uilib *uilib, float min_width, float max_width, float min_height,
                                          float max_height, float *out_width, float *out_height) {
  *out_width = max_width;
  *out_height = max_height;
}

static void uilib__empty_fill__render(struct uilib *uilib, float x, float y, float width, float height) {}

void uilib_empty_fill(struct uilib *uilib) {
  uilib__tree__begin_child(uilib);
  uilib__program__emit_u32(uilib, P_EMPTY_FILL);
  uilib__tree__end_child(uilib);
}


// ====================================================================================================================
// color fill
// program: color:uint32_t
//  memory: unused
static void uilib__color_fill__layout(struct uilib *uilib, float min_width, float max_width, float min_height,
                                          float max_height, float *out_width, float *out_height) {
  uilib__ip__advance(uilib, 4);
  *out_width = max_width;
  *out_height = max_height;
}

static void uilib__color_fill__render(struct uilib *uilib, float x, float y, float width, float height) {
  uint32_t color = uilib__ip__read_u32(uilib);
  uint8_t r, g, b, a;
  uilib__color__unpack(color, &r, &g, &b, &a);
  uilib_set_texture(uilib, 0);
  uilib_draw_rectangle(uilib, x, y, width, height, 0, 0, 1, 1, r, g, b, a);
}

void uilib_color_fill(struct uilib *uilib, float _r, float _g, float _b, float _a) {
  uilib__tree__begin_child(uilib);
  uilib__program__emit_u32(uilib, P_COLOR_FILL);
  uint32_t r = uilib__float__to_color_channel(_r);
  uint32_t g = uilib__float__to_color_channel(_g);
  uint32_t b = uilib__float__to_color_channel(_b);
  uint32_t a = uilib__float__to_color_channel(_a);
  uilib__program__emit_u32(uilib, uilib__color__pack(r, g, b, a));
  uilib__tree__end_child(uilib);
  uilib__stats__set_texture(uilib, 0);
  uilib__stats__draw_rectangle(uilib);
}

// ====================================================================================================================
// image
// program: texture_id:uint32_t, width:float, height:float, s0:float, t0:float, s1:float, t1:float
static void uilib__image__layout(struct uilib *uilib, float min_width, float max_width, float min_height,
                                     float max_height, float *out_width, float *out_height) {
  (void)uilib__ip__read_u32(uilib);
  float width = uilib__ip__read_f32(uilib);
  float height = uilib__ip__read_f32(uilib);
  (void)uilib__ip__read_f32(uilib);
  (void)uilib__ip__read_f32(uilib);
  (void)uilib__ip__read_f32(uilib);
  (void)uilib__ip__read_f32(uilib);
  *out_width = uilib__float__clamp(width, min_width, max_width);
  *out_height = uilib__float__clamp(height, min_height, max_height);
}

static void uilib__image__render(struct uilib *uilib, float x, float y, float width, float height) {
  uint32_t texture_id = uilib__ip__read_u32(uilib);
  (void)uilib__ip__read_f32(uilib);
  (void)uilib__ip__read_f32(uilib);
  float s0 = uilib__ip__read_f32(uilib);
  float t0 = uilib__ip__read_f32(uilib);
  float s1 = uilib__ip__read_f32(uilib);
  float t1 = uilib__ip__read_f32(uilib);
  uilib_set_texture(uilib, texture_id);
  uilib_draw_rectangle(uilib, x, y, width, height, s0, t0, s1, t1, 255, 255, 255, 255);
}

void uilib_image(struct uilib *uilib, float width, float height, float s0, float t0,
                     float s1, float t1, uint32_t texture_id) {
  uilib__tree__begin_child(uilib);
  uilib__program__emit_u32(uilib, P_IMAGE);
  uilib__program__emit_u32(uilib, texture_id);
  uilib__program__emit_f32(uilib, width);
  uilib__program__emit_f32(uilib, height);
  uilib__program__emit_f32(uilib, s0);
  uilib__program__emit_f32(uilib, t0);
  uilib__program__emit_f32(uilib, s1);
  uilib__program__emit_f32(uilib, t1);
  uilib__tree__end_child(uilib);
  uilib__stats__set_texture(uilib, texture_id);
  uilib__stats__draw_rectangle(uilib);
}

// ====================================================================================================================
// font
void uilib_font_index(struct uilib *uilib, uint32_t index) {
  assert(index < uilib->font_length);
  assert(uilib->tree_scope_length > 0);
  uilib__tree__top_scope(uilib)->font_index = index;
}

void uilib_font_size(struct uilib *uilib, float size) {
  assert(uilib->tree_scope_length > 0);
  uilib__tree__top_scope(uilib)->font_size = size;
}

void uilib_font_color(struct uilib *uilib, Color _color) {
  assert(uilib->tree_scope_length > 0);
  uint8_t r = uilib__float__to_color_channel(_color.r);
  uint8_t g = uilib__float__to_color_channel(_color.g);
  uint8_t b = uilib__float__to_color_channel(_color.b);
  uint8_t a = uilib__float__to_color_channel(_color.a);
  uint32_t color = uilib__color__pack(r, g, b, a);
  uilib__tree__top_scope(uilib)->font_color = color;
}

// ====================================================================================================================
// text
// program: font_index:uint32_t, size:float, color:uint32_t, len:uint32_t, content:[(len + 3) / 4]uint32_t
static void uilib__text__layout(struct uilib *uilib, float min_width, float max_width, float min_height,
                                    float max_height, float *out_width, float *out_height) {
  uint32_t font_index = uilib__ip__read_u32(uilib);
  float font_size = uilib__ip__read_f32(uilib);
  (void)uilib__ip__read_u32(uilib);
  uint32_t len = uilib__ip__read_u32(uilib);
  const char *string = (const char *)uilib->ip;
  uilib__ip__advance(uilib, (len + 3) >> 2);
  struct uilib_Font *font = &uilib->font[font_index];
  uint32_t glyph = 0;
  float x = 0;
  float y = 0;
  float width = 0;
  float height = 0;
  for(; *string; ) {
    uint32_t string_advance;
    struct uilib_Font_GlyphInfo glyph_info;
    struct uilib_Font *glyph_font = font;
    while(glyph_font != NULL && !glyph_font->decode(glyph_font, string, &glyph, &string_advance, &glyph_info)) {
      glyph_font = glyph_font->fallback;
    }

    string += string_advance;
    float glyph_width = (glyph_info.x + glyph_info.width) * font_size;
    float glyph_height = (glyph_info.y + glyph_info.height) * font_size;
    width = uilib__float__max(width, x + glyph_width);
    height = uilib__float__max(height, y + glyph_height);
    x += glyph_info.advance_x * font_size;
    if(glyph_info.newline) {
      x = 0;
      y += font_size;
    }
  }
  *out_width = uilib__float__clamp(width, min_width, max_width);
  *out_height = uilib__float__clamp(height, min_height, max_height);

}

static void uilib__text__render(struct uilib *uilib, float x, float y, float width, float height) {
  uint32_t font_index = uilib__ip__read_u32(uilib);
  float font_size = uilib__ip__read_f32(uilib);
  uint32_t font_color = uilib__ip__read_u32(uilib);
  uint32_t len = uilib__ip__read_u32(uilib);
  const char *string = (const char *)uilib->ip;
  uilib__ip__advance(uilib, (len + 3) >> 2);
  struct uilib_Font *font = &uilib->font[font_index];
  uint32_t glyph = 0;
  float offset_x = 0;
  float offset_y = 0;
  uint8_t r = (font_color >> 24) & 0xFF;
  uint8_t g = (font_color >> 16) & 0xFF;
  uint8_t b = (font_color >> 8) & 0xFF;
  uint8_t a = (font_color >> 0) & 0xFF;
  for(; *string; ) {
    uint32_t string_advance;
    struct uilib_Font_GlyphInfo glyph_info;
    struct uilib_Font *glyph_font = font;
    while(glyph_font != NULL && !glyph_font->decode(glyph_font, string, &glyph, &string_advance, &glyph_info)) {
      glyph_font = glyph_font->fallback;
    }
    string += string_advance;
    uilib_set_texture(uilib, glyph_font->texture_id);
    uilib_draw_rectangle(uilib, x + offset_x + glyph_info.x, y + offset_y + glyph_info.y, glyph_info.width * font_size,
                                      glyph_info.height * font_size, glyph_info.s0, glyph_info.t0, glyph_info.s1, glyph_info.t1, r,
                                      g, b, a);
    offset_x += glyph_info.advance_x * font_size;
    if(glyph_info.newline) {
      offset_x = 0;
      offset_y += font_size;
    }
  }
}

void uilib_textv(struct uilib *uilib, const char *format, va_list ap) {
  struct uilib_tree_scope *scope = uilib__tree__top_scope(uilib);
  scope->handle_alignment = true;
  uilib__tree__begin_child(uilib);
  uilib__program__emit_u32(uilib, P_TEXT);
  uilib__program__emit_u32(uilib, scope->font_index);
  uilib__program__emit_f32(uilib, scope->font_size);
  uilib__program__emit_u32(uilib, scope->font_color);
  va_list ap1, ap2;
  va_copy(ap1, ap);
  va_copy(ap2, ap);
  int size = format_count_v(format, ap1);
  va_end(ap1);
  uilib__program__emit_u32(uilib, size);
  uilib__program__space_for(uilib, (size + 3) >> 2);
  char *mut_string = (char *)(uilib->program + uilib->program_length);
  format_string(mut_string, size + 1, format, ap2);
  va_end(ap2);
  uilib->program_length += (size + 3) >> 2;
  uilib__tree__end_child(uilib);

  // fix this?
  uint32_t string_advance;
  uint32_t font_index = scope->font_index;
  const char * string = mut_string;
  struct uilib_Font *font = &uilib->font[font_index];
  uint32_t glyph = 0;
  for(; *string; ) {
    struct uilib_Font_GlyphInfo glyph_info;
    struct uilib_Font *glyph_font = font;
    while(glyph_font != NULL && !glyph_font->decode(glyph_font, string, &glyph, &string_advance, &glyph_info)) {
      glyph_font = glyph_font->fallback;
    }
    string += string_advance;
    uilib__stats__set_texture(uilib, glyph_font->texture_id);
    uilib__stats__draw_rectangle(uilib);
  }
}

// ====================================================================================================================
// background color
// program: color:uint32_t
static inline void uilib__background_color__begin_child(struct uilib *uilib, struct uilib_tree_scope *scope) {
}

static inline void uilib__background_color__end_child(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__tree__end_scope(uilib);
}

static inline void uilib__background_color__end_scope(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__tree__end_child(uilib);
}

static inline void uilib__background_color__layout(struct uilib *uilib, float min_width, float max_width,
                                                       float min_height, float max_height, float *out_width,
                                                       float *out_height) {
  (void)uilib__ip__read_u32(uilib);
  uilib__layout(uilib, min_width, max_width, min_height, max_height, out_width, out_height);
}

static inline void uilib__background_color__render(struct uilib *uilib, float x, float y, float width,
                                                       float height) {
  uint32_t color = uilib__ip__read_u32(uilib);
  uint8_t r, g, b, a;
  uilib__color__unpack(color, &r, &g, &b, &a);
  uilib_set_texture(uilib, 0);
  uilib_draw_rectangle(uilib, x, y, width, height, 0, 0, 0, 0, r, g, b, a);
  uilib__render(uilib, x, y, width, height);
}

void uilib_background_color(struct uilib *uilib, float r, float g, float b, float a) {
  uilib__tree__begin_child(uilib);
  uilib__tree__begin_scope(uilib, uilib__background_color__begin_child, uilib__background_color__end_child,
                               uilib__background_color__end_scope);
  uint32_t color =
      uilib__color__pack(uilib__float__to_color_channel(r), uilib__float__to_color_channel(g),
                             uilib__float__to_color_channel(b), uilib__float__to_color_channel(a));
  uilib__program__emit_u32(uilib, P_BACKGROUND_COLOR);
  uilib__program__emit_u32(uilib, color);
}

// ====================================================================================================================
// border
// program: color:uint32_t
static inline void uilib__border__begin_child(struct uilib *uilib, struct uilib_tree_scope *scope) {}

static inline void uilib__border__end_child(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__tree__end_scope(uilib);
}

static inline void uilib__border__end_scope(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__tree__end_child(uilib);
}

static inline void uilib__border__layout(struct uilib *uilib, float min_width, float max_width, float min_height,
                                             float max_height, float *out_width, float *out_height) {
  float size = uilib__ip__read_f32(uilib);
  (void)uilib__ip__read_u32(uilib);
  float child_width;
  float child_height;
  uilib__layout(uilib, min_width, max_width, min_height, max_height, &child_width, &child_height);
  child_width += size * 2;
  child_height += size * 2;
  *out_width = uilib__float__clamp(child_width, min_width, max_width);
  *out_height = uilib__float__clamp(child_height, min_height, max_height);
}

static inline void uilib__border__render(struct uilib *uilib, float x, float y, float width, float height) {
  uint32_t color = uilib__ip__read_u32(uilib);
  uint8_t r, g, b, a;
  uilib__color__unpack(color, &r, &g, &b, &a);
  uilib_set_texture(uilib, 0);
  uilib_draw_rectangle(uilib, x, y, width, height, 0, 0, 0, 0, r, g, b, a);
  uilib__render(uilib, x, y, width, height);
}

void uilib_border(struct uilib *uilib, float size, float r, float g, float b, float a) {
  uilib__tree__begin_child(uilib);
  uilib__tree__begin_scope(uilib, uilib__background_color__begin_child, uilib__background_color__end_child,
                               uilib__background_color__end_scope);
  uint32_t color =
      uilib__color__pack(uilib__float__to_color_channel(r), uilib__float__to_color_channel(g),
                             uilib__float__to_color_channel(b), uilib__float__to_color_channel(a));
  uilib__program__emit_u32(uilib, P_BORDER);
  uilib__program__emit_f32(uilib, size);
  uilib__program__emit_u32(uilib, color);
}

// ====================================================================================================================
// stack
// program: len:uint32_t, [len]child
//  memory: [len](width:float, height:float, child)
static inline void uilib__stack__begin_child(struct uilib *uilib, struct uilib_tree_scope *scope) {
  scope->stack.num_children += 1;
}

static inline void uilib__stack__end_child(struct uilib *uilib, struct uilib_tree_scope *scope) {}

static inline void uilib__stack__end_scope(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__program__fill_u32(uilib, scope->stack.hole, scope->stack.num_children);
  uilib__tree__end_child(uilib);
}

static inline void uilib__stack__layout(struct uilib *uilib, float min_width, float max_width, float min_height,
                                            float max_height, float *out_width, float *out_height) {
  float max_child_width = min_width;
  float max_child_height = min_height;
  uint32_t len = uilib__ip__read_u32(uilib);
  for(uint32_t i = 0; i < len; i++) {
    uint32_t hole = uilib__memory__dig(uilib, 2);
    float child_width;
    float child_height;
    uilib__layout(uilib, min_width, max_width, min_height, max_height, &child_width, &child_height);
    uilib__memory__fill_f32(uilib, hole + 0, child_width);
    uilib__memory__fill_f32(uilib, hole + 1, child_height);
    max_child_width = uilib__float__max(max_child_width, child_width);
    max_child_height = uilib__float__max(max_child_height, child_height);
  }
  *out_width = uilib__float__min(max_child_width, max_width);
  *out_height = uilib__float__min(max_child_height, max_height);
}

static inline void uilib__stack__render(struct uilib *uilib, float x, float y, float width, float height) {
  uint32_t len = uilib__ip__read_u32(uilib);
  for(uint32_t i = 0; i < len; i++) {
    float child_width = uilib__mp__read_f32(uilib);
    float child_height = uilib__mp__read_f32(uilib);
    uilib__render(uilib, x, y, child_width, child_height);
  }
}

void uilib_begin_stack(struct uilib *uilib) {
  uilib__tree__begin_child(uilib);
  uilib__tree__begin_scope(uilib, uilib__stack__begin_child, uilib__stack__end_child,
                               uilib__stack__end_scope);
  uilib__program__emit_u32(uilib, P_STACK);
  struct uilib_tree_scope *scope = uilib__tree__top_scope(uilib);
  scope->stack.hole = uilib__program__dig(uilib, 1);
}

// ====================================================================================================================
// directional stack
// program: len:uint32_t, [len](isize:uint32_t, msize:uint32_t, weight:float, align:float, child)
//  memory: [len](width:float, height:float, child)
static inline void uilib__dstack__space_for(struct uilib *uilib, uint32_t count) {
  struct uilib_tree_scope *scope = uilib__tree__top_scope(uilib);
  uilib__grow((void **)&scope->dstack.children, sizeof(*scope->dstack.children),
                  scope->dstack.children_length + count, &scope->dstack.children_capacity);
}

static inline void uilib__dstack__begin_child(struct uilib *uilib, struct uilib_tree_scope *scope) {
  scope->dstack.child_pmark = uilib__program__mark(uilib);
  scope->dstack.child_mmark = uilib__memory__mark(uilib);
  uilib__dstack__space_for(uilib, 1);
  scope->dstack.children_length += 1;
  struct uilib_dstack_child *child = &scope->dstack.children[scope->dstack.children_length - 1];
  child->hole = uilib__program__dig(uilib, 3);
  child->weight = uilib__float__max(scope->weight, 0);
  scope->dstack.total_weight += child->weight;
  uilib__program__emit_f32(uilib, scope->dstack.vertical ? scope->align_x : scope->align_y);
  uilib__memory__reserve(uilib, 2);
  scope->weight = 0;
  scope->align_x = 0.5;
  scope->align_y = 0.5;
}

static inline void uilib__dstack__end_child(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uint32_t psize = uilib__program__mark_size(uilib, scope->dstack.child_pmark);
  uint32_t msize = uilib__memory__mark_size(uilib, scope->dstack.child_mmark);
  struct uilib_dstack_child *child = &scope->dstack.children[scope->dstack.children_length - 1];
  uilib__program__fill_u32(uilib, child->hole + 0, psize);
  uilib__program__fill_u32(uilib, child->hole + 1, msize);
}

static inline void uilib__dstack__end_scope(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__program__fill_u32(uilib, scope->dstack.hole, scope->dstack.children_length);

  for(uint32_t i = 0; i < scope->dstack.children_length; i++) {
    struct uilib_dstack_child *child = &scope->dstack.children[i];
    uilib__program__fill_f32(uilib, child->hole + 2, child->weight / scope->dstack.total_weight);
  }

  memory_free(scope->dstack.children, sizeof(*scope->dstack.children) * scope->dstack.children_capacity, 4);

  uilib__tree__end_child(uilib);
}

static inline void uilib__dstack__create(struct uilib *uilib, bool vertical) {
  uilib__tree__begin_child(uilib);
  uilib__tree__begin_scope(uilib, uilib__dstack__begin_child, uilib__dstack__end_child,
                               uilib__dstack__end_scope);
  uilib__program__emit_u32(uilib, vertical ? P_VERTICAL_STACK : P_HORIZONTAL_STACK);
  struct uilib_tree_scope *scope = uilib__tree__top_scope(uilib);
  scope->handle_alignment = true;
  scope->align_x = 0.5;
  scope->align_y = 0.5;
  scope->dstack.vertical = vertical;
  scope->dstack.hole = uilib__program__dig(uilib, 1);
  scope->dstack.total_weight = 0;
  scope->dstack.children_capacity = 0;
  scope->dstack.children_length = 0;
  scope->dstack.children = NULL;
}

static inline void uilib__dstack__layout(struct uilib *uilib, bool vertical, float min_width, float max_width,
                                             float min_height, float max_height, float *out_width, float *out_height) {
  uint32_t len = uilib__ip__read_u32(uilib);
  uint32_t *prepass_ip = uilib->ip;
  float max_child_width = min_width;
  float max_child_height = min_height;
  for(uint32_t pass = 0; pass < 2; pass++) {
    uilib->ip = prepass_ip;
    for(uint32_t i = 0; i < len; i++) {
      uint32_t psize = uilib__ip__read_u32(uilib);
      uint32_t msize = uilib__ip__read_u32(uilib);
      float weight = uilib__ip__read_f32(uilib);
      if(pass != (weight > 0 ? 1 : 0)) {
        uilib__ip__advance(uilib, psize - 3);
        uilib__mp__advance(uilib, msize);
        continue;
      }
      (void)uilib__ip__read_f32(uilib);
      float child_width;
      float child_height;
      uint32_t mhole = uilib__memory__dig(uilib, 2);
      uilib__layout(uilib, min_width, max_width, min_height, max_height, &child_width, &child_height);
      if(pass == 1) {
        if(!vertical) {
          child_width = weight * max_width;
        } else {
          child_height = weight * max_height;
        }
      }
      uilib__memory__fill_f32(uilib, mhole + 0, child_width);
      uilib__memory__fill_f32(uilib, mhole + 1, child_height);
      max_child_width = uilib__float__max(max_child_width, child_width);
      max_child_height = uilib__float__max(max_child_height, child_height);
      if(pass == 0) {
        if(vertical) {
          min_height = uilib__float__max(min_height - child_height, 0);
          max_height = uilib__float__max(max_height - child_height, 0);
        } else {
          min_width = uilib__float__max(min_width - child_width, 0);
          max_width = uilib__float__max(max_width - child_width, 0);
        }
      }
    }
  }
  *out_width = uilib__float__min(max_width, max_child_width);
  *out_height = uilib__float__min(max_height, max_child_height);
}

static inline void uilib__dstack__render(struct uilib *uilib, bool vertical, float x, float y, float width,
                                             float height) {
  uint32_t len = uilib__ip__read_u32(uilib);
  for(uint32_t i = 0; i < len; i++) {
    (void)uilib__ip__read_u32(uilib);
    (void)uilib__ip__read_u32(uilib);
    (void)uilib__ip__read_f32(uilib);
    float align = uilib__ip__read_f32(uilib);
    float child_width = uilib__mp__read_f32(uilib);
    float child_height = uilib__mp__read_f32(uilib);
    uilib__render(uilib, vertical ? x + uilib__float__max(width - child_width, 0) * align : x,
                      vertical ? y : y + uilib__float__max(height - child_height, 0) * align, child_width,
                      child_height);
    if(!vertical) {
      x += child_width;
    } else {
      y += child_height;
    }
  }
}

void uilib_begin_horizontal_stack(struct uilib *uilib) {
  uilib__dstack__create(uilib, false);
}

void uilib_begin_vertical_stack(struct uilib *uilib) {
  uilib__dstack__create(uilib, true);
}

// ====================================================================================================================
void uilib_stack_weight(struct uilib *uilib, float weight) {
  uilib__tree__top_scope(uilib)->weight = weight;
}

void uilib_end_stack(struct uilib *uilib) {
  uilib__tree__end_scope(uilib);
}

// ====================================================================================================================
// named scope
// program: child
//  memory: child
static inline void uilib__named_scope__begin_child(struct uilib *uilib, struct uilib_tree_scope *scope) {}

static inline void uilib__named_scope__end_child(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__tree__end_scope(uilib);
}

static inline void uilib__named_scope__end_scope(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__tree__end_child(uilib);
}

void uilib_named_scope(struct uilib *uilib, const char *format, ...) {
  uilib__tree__begin_child(uilib);
  uilib__tree__begin_scope(uilib, uilib__named_scope__begin_child, uilib__named_scope__end_child,
                               uilib__named_scope__end_scope);
  struct uilib_tree_scope *scope = uilib__tree__top_scope(uilib);
  va_list ap;
  va_start(ap, format);
  int size = format_count_v(format, ap);
  va_end(ap);
  char *buffer = alloca(size + 1);
  va_start(ap, format);
  format_string_v(buffer, size + 1, format, ap);
  va_end(ap);
  uint64_t hash =
      0xCBF29CE484222325ul ^
      (scope->named_scope_index < uilib->named_scope_length ? uilib->named_scope[scope->named_scope_index].hash : 0);
  while(*buffer) {
    hash ^= *buffer++;
    hash *= 0x00000100000001B3ul;
  }
  for(uint32_t i = 0; i < uilib->named_scope_length; i++) {
    if(uilib->named_scope[i].hash == hash) {
      scope->named_scope_index = i;
      return;
    }
  }
  uilib__grow((void **)&uilib->named_scope, sizeof(*uilib->named_scope), uilib->named_scope_length + 1,
                  &uilib->named_scope_capacity);
  uilib->named_scope[uilib->named_scope_length].hash = hash;
  uilib->named_scope[uilib->named_scope_length].query_capacity = 0;
  uilib->named_scope[uilib->named_scope_length].query_rects = NULL;
  uilib->named_scope[uilib->named_scope_length].slot_capacity = 0;
  uilib->named_scope[uilib->named_scope_length].slot = NULL;
  scope->named_scope_index = uilib->named_scope_length++;
}

// ====================================================================================================================
void uilib_set_uint32(struct uilib *uilib, uint32_t slot_index, uint32_t value) {
  struct uilib_tree_scope *scope = uilib__tree__top_scope(uilib);
  struct uilib_named_scope *named_scope = &uilib->named_scope[scope->named_scope_index];
  uilib__grow((void **)&named_scope->slot, sizeof(*named_scope->slot), slot_index, &named_scope->slot_capacity);
  named_scope->slot[slot_index] = value;
}

void uilib_set_float(struct uilib *uilib, uint32_t slot_index, float value) {
  union {
    uint32_t u;
    float f;
  } flip = {.f = value};
  uilib_set_uint32(uilib, slot_index, flip.u);
}

uint32_t uilib_get_uint32(struct uilib *uilib, uint32_t slot_index) {
  struct uilib_tree_scope *scope = uilib__tree__top_scope(uilib);
  struct uilib_named_scope *named_scope = &uilib->named_scope[scope->named_scope_index];
  if(slot_index < named_scope->slot_capacity) {
    return named_scope->slot[slot_index];
  } else {
    return 0;
  }
}

float uilib_get_float(struct uilib *uilib, uint32_t slot_index) {
  union {
    uint32_t u;
    float f;
  } flip = {.u = uilib_get_uint32(uilib, slot_index)};
  return flip.f;
}

// ====================================================================================================================
// size
// program: width:float, height:float, child
//  memory: child
static inline void uilib__size__begin_child(struct uilib *uilib, struct uilib_tree_scope *scope) {}

static inline void uilib__size__end_child(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__tree__end_scope(uilib);
}

static inline void uilib__size__end_scope(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__tree__end_child(uilib);
}

static inline void uilib__size__layout(struct uilib *uilib, float min_width, float max_width, float min_height,
                                           float max_height, float *out_width, float *out_height) {
  float width = uilib__ip__read_f32(uilib);
  float height = uilib__ip__read_f32(uilib);
  float unused_width;
  float unused_height;
  uilib__layout(uilib, width, width, height, height, &unused_width, &unused_height);
  *out_width = uilib__float__clamp(width, min_width, max_width);
  *out_height = uilib__float__clamp(height, min_height, max_height);
}

static inline void uilib__size__render(struct uilib *uilib, float x, float y, float width, float height) {
  (void)uilib__ip__read_f32(uilib);
  (void)uilib__ip__read_f32(uilib);
  uilib__render(uilib, x, y, width, height);
}

void uilib_size(struct uilib *uilib, float width, float height) {
  uilib__tree__begin_child(uilib);
  uilib__tree__begin_scope(uilib, uilib__size__begin_child, uilib__size__end_child,
                               uilib__size__end_scope);
  uilib__program__emit_u32(uilib, P_SIZE);
  uilib__program__emit_f32(uilib, width);
  uilib__program__emit_f32(uilib, height);
}

// ====================================================================================================================
// width
// program: width:float, child
//  memory: child
static inline void uilib__width__begin_child(struct uilib *uilib, struct uilib_tree_scope *scope) {}

static inline void uilib__width__end_child(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__tree__end_scope(uilib);
}

static inline void uilib__width__end_scope(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__tree__end_child(uilib);
}

static inline void uilib__width__layout(struct uilib *uilib, float min_width, float max_width, float min_height,
                                            float max_height, float *out_width, float *out_height) {
  float width = uilib__ip__read_f32(uilib);
  float unused_width;
  float height;
  uilib__layout(uilib, width, width, min_height, max_height, &unused_width, &height);
  *out_width = uilib__float__clamp(width, min_width, max_width);
  *out_height = uilib__float__clamp(height, min_height, max_height);
}

static inline void uilib__width__render(struct uilib *uilib, float x, float y, float width, float height) {
  (void)uilib__ip__read_f32(uilib);
  uilib__render(uilib, x, y, width, height);
}

void uilib_width(struct uilib *uilib, float width) {
  uilib__tree__begin_child(uilib);
  uilib__tree__begin_scope(uilib, uilib__width__begin_child, uilib__width__end_child,
                               uilib__width__end_scope);
  uilib__program__emit_u32(uilib, P_WIDTH);
  uilib__program__emit_f32(uilib, width);
}

// ====================================================================================================================
// height
// program: height:float, child
//  memory: child
static inline void uilib__height__begin_child(struct uilib *uilib, struct uilib_tree_scope *scope) {}

static inline void uilib__height__end_child(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__tree__end_scope(uilib);
}

static inline void uilib__height__end_scope(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__tree__end_child(uilib);
}

static inline void uilib__height__layout(struct uilib *uilib, float min_width, float max_width, float min_height,
                                             float max_height, float *out_width, float *out_height) {
  float height = uilib__ip__read_f32(uilib);
  float width;
  float unused_height;
  uilib__layout(uilib, min_width, max_width, height, height, &width, &unused_height);
  *out_width = uilib__float__clamp(width, min_width, max_width);
  *out_height = uilib__float__clamp(height, min_height, max_height);
}

static inline void uilib__height__render(struct uilib *uilib, float x, float y, float width, float height) {
  (void)uilib__ip__read_f32(uilib);
  uilib__render(uilib, x, y, width, height);
}

void uilib_height(struct uilib *uilib, float height) {
  uilib__tree__begin_child(uilib);
  uilib__tree__begin_scope(uilib, uilib__height__begin_child, uilib__height__end_child,
                               uilib__height__end_scope);
  uilib__program__emit_u32(uilib, P_HEIGHT);
  uilib__program__emit_f32(uilib, height);
}

// ====================================================================================================================
// align
// program: x:float, y:float, child
//  memory: width:float, height:float, child
static inline void uilib__align__begin_child(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__memory__reserve(uilib, 2);
}

static inline void uilib__align__end_child(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__tree__end_scope(uilib);
}

static inline void uilib__align__end_scope(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__tree__end_child(uilib);
}

static inline void uilib__align__layout(struct uilib *uilib, float min_width, float max_width, float min_height,
                                            float max_height, float *out_width, float *out_height) {
  (void)uilib__ip__read_f32(uilib);
  (void)uilib__ip__read_f32(uilib);
  float width;
  float height;
  uint32_t mhole = uilib__memory__dig(uilib, 2);
  uilib__layout(uilib, min_width, max_width, min_height, max_height, &width, &height);
  uilib__memory__fill_f32(uilib, mhole + 0, width);
  uilib__memory__fill_f32(uilib, mhole + 1, height);
  *out_width = max_width;
  *out_height = max_height;
}

static inline void uilib__align__render(struct uilib *uilib, float x, float y, float width, float height) {
  float align_x = uilib__ip__read_f32(uilib);
  float align_y = uilib__ip__read_f32(uilib);
  float child_width = uilib__mp__read_f32(uilib);
  float child_height = uilib__mp__read_f32(uilib);
  uilib__render(uilib, x + (width - child_width) * align_x, y + (height - child_height) * align_y, child_width,
                    child_height);
}

void uilib_align(struct uilib *uilib, float x, float y) {
  uilib__tree__begin_child(uilib);
  uilib__tree__begin_scope(uilib, uilib__align__begin_child, uilib__align__end_child,
                               uilib__align__end_scope);
  uilib__program__emit_u32(uilib, P_ALIGN);
  uilib__program__emit_f32(uilib, x);
  uilib__program__emit_f32(uilib, y);
}

// ====================================================================================================================
// alignx
// program: x:float, child
//  memory: width:float, child
static inline void uilib__alignx__begin_child(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__memory__reserve(uilib, 1);
}

static inline void uilib__alignx__end_child(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__tree__end_scope(uilib);
}

static inline void uilib__alignx__end_scope(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__tree__end_child(uilib);
}

static inline void uilib__alignx__layout(struct uilib *uilib, float min_width, float max_width, float min_height,
                                             float max_height, float *out_width, float *out_height) {
  (void)uilib__ip__read_f32(uilib);
  float width;
  float height;
  uint32_t mhole = uilib__memory__dig(uilib, 1);
  uilib__layout(uilib, min_width, max_width, min_height, max_height, &width, &height);
  uilib__memory__fill_f32(uilib, mhole, width);
  *out_width = max_width;
  *out_height = uilib__float__clamp(height, min_height, min_height);
}

static inline void uilib__alignx__render(struct uilib *uilib, float x, float y, float width, float height) {
  float align_x = uilib__ip__read_f32(uilib);
  float child_width = uilib__mp__read_f32(uilib);
  uilib__render(uilib, x + (width - child_width) * align_x, y, child_width, height);
}

void uilib_align_x(struct uilib *uilib, float x) {
  uilib__tree__begin_child(uilib);
  uilib__tree__begin_scope(uilib, uilib__alignx__begin_child, uilib__alignx__end_child,
                               uilib__alignx__end_scope);
  uilib__program__emit_u32(uilib, P_ALIGN_X);
  uilib__program__emit_f32(uilib, x);
}

// ====================================================================================================================
// aligny
// program: y:float, child
//  memory: height:float, child
static inline void uilib__aligny__begin_child(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__memory__reserve(uilib, 1);
}

static inline void uilib__aligny__end_child(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__tree__end_scope(uilib);
}

static inline void uilib__aligny__end_scope(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__tree__end_child(uilib);
}

static inline void uilib__aligny__layout(struct uilib *uilib, float min_width, float max_width, float min_height,
                                             float max_height, float *out_width, float *out_height) {
  (void)uilib__ip__read_f32(uilib);
  float width;
  float height;
  uint32_t mhole = uilib__memory__dig(uilib, 1);
  uilib__layout(uilib, min_width, max_width, min_height, max_height, &width, &height);
  uilib__memory__fill_f32(uilib, mhole, height);
  *out_width = uilib__float__clamp(width, min_width, min_width);
  *out_height = max_height;
}

static inline void uilib__aligny__render(struct uilib *uilib, float x, float y, float width, float height) {
  float align_y = uilib__ip__read_f32(uilib);
  float child_height = uilib__mp__read_f32(uilib);
  uilib__render(uilib, x, y + (height - child_height) * align_y, width, child_height);
}

void uilib_align_y(struct uilib *uilib, float y) {
  uilib__tree__begin_child(uilib);
  uilib__tree__begin_scope(uilib, uilib__aligny__begin_child, uilib__aligny__end_child,
                               uilib__aligny__end_scope);
  uilib__program__emit_u32(uilib, P_ALIGN_Y);
  uilib__program__emit_f32(uilib, y);
}

// ====================================================================================================================
// padding
// program: left:float, top:float, right:float, bottom:float, child
//  memory: child
static inline void uilib__padding__begin_child(struct uilib *uilib, struct uilib_tree_scope *scope) {
}

static inline void uilib__padding__end_child(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__tree__end_scope(uilib);
}

static inline void uilib__padding__end_scope(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__tree__end_child(uilib);
}

static inline void uilib__padding__layout(struct uilib *uilib, float min_width, float max_width, float min_height,
                                              float max_height, float *out_width, float *out_height) {
  float left = uilib__ip__read_f32(uilib);
  float top = uilib__ip__read_f32(uilib);
  float right = uilib__ip__read_f32(uilib);
  float bottom = uilib__ip__read_f32(uilib);
  float padding_width = left + right;
  float padding_height = top + bottom;
  float child_width;
  float child_height;
  float child_min_width = uilib__float__max(min_width - padding_width, 0);
  float child_max_width = uilib__float__max(max_width - padding_width, 0);
  float child_min_height = uilib__float__max(min_height - padding_height, 0);
  float child_max_height = uilib__float__max(max_height - padding_height, 0);
  uilib__layout(uilib, child_min_width, child_max_width, child_min_height, child_max_height, &child_width, &child_height);
  *out_width = uilib__float__clamp(child_width + padding_width, min_width, max_width);
  *out_height = uilib__float__clamp(child_height + padding_height, min_height, max_height);
}

static inline void uilib__padding__render(struct uilib *uilib, float x, float y, float width, float height) {
  float left = uilib__ip__read_f32(uilib);
  float top = uilib__ip__read_f32(uilib);
  float right = uilib__ip__read_f32(uilib);
  float bottom = uilib__ip__read_f32(uilib);
  float padding_width = left + right;
  float padding_height = top + bottom;
  uilib__render(uilib, x + left, y + top, width - padding_width, height - padding_height);
}

void uilib_padding(struct uilib *uilib, float left, float top, float right, float bottom) {
  uilib__tree__begin_child(uilib);
  uilib__tree__begin_scope(uilib, uilib__padding__begin_child, uilib__padding__end_child,
                               uilib__padding__end_scope);
  uilib__program__emit_u32(uilib, P_PADDING);
  uilib__program__emit_f32(uilib, left);
  uilib__program__emit_f32(uilib, top);
  uilib__program__emit_f32(uilib, right);
  uilib__program__emit_f32(uilib, bottom);
}

// ====================================================================================================================
// query
// program: named_scope:uint32_t, query_index:uint32_t, child
//  memory: child
static inline void uilib__query__begin_child(struct uilib *uilib, struct uilib_tree_scope *scope) {}

static inline void uilib__query__end_child(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__tree__end_scope(uilib);
}

static inline void uilib__query__end_scope(struct uilib *uilib, struct uilib_tree_scope *scope) {
  uilib__tree__end_child(uilib);
}

static inline void uilib__query__layout(struct uilib *uilib, float min_width, float max_width, float min_height,
                                            float max_height, float *out_width, float *out_height) {
  (void)uilib__ip__read_u32(uilib);
  (void)uilib__ip__read_u32(uilib);
  uilib__layout(uilib, min_width, max_width, min_height, max_height, out_width, out_height);
}

static inline void uilib__query__render(struct uilib *uilib, float x, float y, float width, float height) {
  uint32_t named_scope_index = uilib__ip__read_u32(uilib);
  uint32_t query_index = uilib__ip__read_u32(uilib);
  struct uilib_named_scope *named_scope = &uilib->named_scope[named_scope_index];
  named_scope->query_rects[query_index * 4 + 0] = x;
  named_scope->query_rects[query_index * 4 + 1] = y;
  named_scope->query_rects[query_index * 4 + 2] = width;
  named_scope->query_rects[query_index * 4 + 3] = height;
  uilib__render(uilib, x, y, width, height);
}

void uilib_query(struct uilib *uilib, uint32_t query_index, float *out_rect) {
  uilib__tree__begin_child(uilib);
  uilib__tree__begin_scope(uilib, uilib__query__begin_child, uilib__query__end_child,
                               uilib__query__end_scope);
  struct uilib_tree_scope *scope = uilib__tree__top_scope(uilib);
  uilib__program__emit_u32(uilib, P_QUERY);
  uilib__program__emit_u32(uilib, scope->named_scope_index);
  uilib__program__emit_u32(uilib, query_index);
  struct uilib_named_scope *named_scope = &uilib->named_scope[scope->named_scope_index];
  uilib__grow((void **)&named_scope->query_rects, sizeof(*named_scope->query_rects) * 4, query_index + 1,
                  &named_scope->query_capacity);
  out_rect[0] = named_scope->query_rects[query_index * 4 + 0];
  out_rect[1] = named_scope->query_rects[query_index * 4 + 1];
  out_rect[2] = named_scope->query_rects[query_index * 4 + 2];
  out_rect[3] = named_scope->query_rects[query_index * 4 + 3];
}

// ====================================================================================================================
static inline void uilib__layout__initialize(struct uilib *uilib) {
  uilib__ip__initialize(uilib);
  uilib__memory__initialize(uilib);
}

static inline void uilib__layout(struct uilib *uilib, float aw, float bw, float ah, float bh, float *oh,
                                     float *ow) {
  uint32_t code = uilib__ip__read_u32(uilib);
  switch(code) {
  case P_TERMINATE:
    break;
  case P_EMPTY_FILL:
    uilib__empty_fill__layout(uilib, aw, bw, ah, bh, oh, ow);
    break;
  case P_COLOR_FILL:
    uilib__color_fill__layout(uilib, aw, bw, ah, bh, oh, ow);
    break;
  case P_TEXT:
    uilib__text__layout(uilib, aw, bw, ah, bh, oh, ow);
    break;
  case P_IMAGE:
    uilib__image__layout(uilib, aw, bw, ah, bh, oh, ow);
    break;
  case P_BACKGROUND_COLOR:
    uilib__background_color__layout(uilib, aw, bw, ah, bh, oh, ow);
    break;
  case P_BORDER:
    uilib__border__layout(uilib, aw, bw, ah, bh, oh, ow);
    break;
  case P_STACK:
    uilib__stack__layout(uilib, aw, bw, ah, bh, oh, ow);
    break;
  case P_HORIZONTAL_STACK:
    uilib__dstack__layout(uilib, false, aw, bw, ah, bh, oh, ow);
    break;
  case P_VERTICAL_STACK:
    uilib__dstack__layout(uilib, true, aw, bw, ah, bh, oh, ow);
    break;
  case P_SIZE:
    uilib__size__layout(uilib, aw, bw, ah, bh, oh, ow);
    break;
  case P_WIDTH:
    uilib__width__layout(uilib, aw, bw, ah, bh, oh, ow);
    break;
  case P_HEIGHT:
    uilib__height__layout(uilib, aw, bw, ah, bh, oh, ow);
    break;
  case P_ALIGN:
    uilib__align__layout(uilib, aw, bw, ah, bh, oh, ow);
    break;
  case P_ALIGN_X:
    uilib__alignx__layout(uilib, aw, bw, ah, bh, oh, ow);
    break;
  case P_ALIGN_Y:
    uilib__aligny__layout(uilib, aw, bw, ah, bh, oh, ow);
    break;
  case P_PADDING:
    uilib__padding__layout(uilib, aw, bw, ah, bh, oh, ow);
    break;
  case P_QUERY:
    uilib__query__layout(uilib, aw, bw, ah, bh, oh, ow);
    break;
  }
}

static inline void uilib__render__initialize(struct uilib *uilib) {
  uilib__ip__initialize(uilib);
  uilib__mp__initialize(uilib);
  uilib__vertex__initialize(uilib);
  uilib__index__initialize(uilib);
  uilib__group__initialize(uilib);
  uilib->texture_id = 0;
  uilib__group__advance(uilib);
}

static inline void uilib__render(struct uilib *uilib, float x, float y, float w, float h) {
  uint32_t code = uilib__ip__read_u32(uilib);
  switch(code) {
  case P_TERMINATE:
    break;
  case P_EMPTY_FILL:
    uilib__empty_fill__render(uilib, x, y, w, h);
    break;
  case P_COLOR_FILL:
    uilib__color_fill__render(uilib, x, y, w, h);
    break;
  case P_TEXT:
    uilib__text__render(uilib, x, y, w, h);
    break;
  case P_IMAGE:
    uilib__image__render(uilib, x, y, w, h);
    break;
  case P_BACKGROUND_COLOR:
    uilib__background_color__render(uilib, x, y, w, h);
    break;
  case P_BORDER:
    uilib__border__render(uilib, x, y, w, h);
    break;
  case P_STACK:
    uilib__stack__render(uilib, x, y, w, h);
    break;
  case P_HORIZONTAL_STACK:
    uilib__dstack__render(uilib, false, x, y, w, h);
    break;
  case P_VERTICAL_STACK:
    uilib__dstack__render(uilib, true, x, y, w, h);
    break;
  case P_SIZE:
    uilib__size__render(uilib, x, y, w, h);
    break;
  case P_WIDTH:
    uilib__width__render(uilib, x, y, w, h);
    break;
  case P_HEIGHT:
    uilib__height__render(uilib, x, y, w, h);
    break;
  case P_ALIGN:
    uilib__align__render(uilib, x, y, w, h);
    break;
  case P_ALIGN_X:
    uilib__alignx__render(uilib, x, y, w, h);
    break;
  case P_ALIGN_Y:
    uilib__aligny__render(uilib, x, y, w, h);
    break;
  case P_PADDING:
    uilib__padding__render(uilib, x, y, w, h);
    break;
  case P_QUERY:
    uilib__query__render(uilib, x, y, w, h);
    break;
  }
}
// ====================================================================================================================
static inline bool uilib__dev_font__decode(struct uilib_Font *font, const char *string, uint32_t *inout_glyph,
                                               uint32_t *out_num_bytes, struct uilib_Font_GlyphInfo *out_info) {
  uint32_t glyph = *string;
  *inout_glyph = glyph;
  *out_num_bytes = 1;
  uint32_t row = glyph >> 4;
  uint32_t col = glyph & 15;
  out_info->newline = glyph == '\n';
  out_info->advance_x = 0.5f;
  out_info->x = 0.0f;
  out_info->y = 0.0f;
  out_info->width = 1.33f;
  out_info->height = 1.33f;
  if(row < 2 || glyph == ' ') {
    out_info->width = 0;
    out_info->height = 0;
  } else {
    row -= 2;
  }
  out_info->s0 = (float)(col + 0) / 16.0f;
  out_info->t0 = (float)(row + 0) / 16.0f;
  out_info->s1 = (float)(col + 1) / 16.0f;
  out_info->t1 = (float)(row + 1) / 16.0f;
  return true;
}

static struct uilib_Font uilib__dev_font = {
  .texture_id = 0,
  .decode = uilib__dev_font__decode,
};

// --------------------------------------------------------------------------------------------------------------------

#ifdef UNICAT_UI_ENABLE_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

#define MULTILINE_STRING(...) #__VA_ARGS__

const char * uilib_freetype_opengl_fragment_shader(uint32_t major, uint32_t minor) {
  return 
MULTILINE_STRING(
vec2 uilib_freetype_snorm16_decode2(uvec2 x) {
  return vec2(x) / 65535.0 - 0.5;
}

float uilib_freetype_draw_curve(vec2 p0, vec2 p1, vec2 p2, float inverse_pixel_width) {
  vec2 a = p0 - 2.0*p1 + p2;
  vec2 b = p0 - p1;
  vec2 c = p0;
  vec2 t;
  float si = b.y*b.y - a.y*c.y;
  float s = sqrt(max(si, 0.0));
  t = vec2(b.y - s, b.y + s) / a.y;
  vec2 x = (a.x*t - 2.0*b.x)*t + c.x;
  vec2 weight = clamp(x * inverse_pixel_width + 0.5, 0.0, 1.0) * step(0.0, t) * step(-1.0, -t);
  return dot(weight, vec2(1, -1)) * step(0.0, si);
}

float uilib_freetype_draw_layer(vec2 uv, vec2 inverse_pixel_width, uint curve_start, uint curve_length, bool super_sampling) {
  float alpha = 0.0;
  uint row = curve_start;
  uint end = row + curve_length;
  while(row < end) {
    uvec4 contour = uilib_freetype_user_font(row++);
    uint count = contour.x;
    vec2 a = uilib_freetype_snorm16_decode2(contour.zw) * scale - uv;
    for(uint i = 0u; i < count; i++) {
      uvec4 curve = uilib_freetype_user_font(row++);
      vec2 b = uilib_freetype_snorm16_decode2(curve.xy) * scale - uv;
      vec2 c = uilib_freetype_snorm16_decode2(curve.zw) * scale - uv;
      vec3 x = vec3(a.x, b.x, c.x);
      vec3 y = vec3(a.y, b.y, c.y);
      if(!all(lessThan(y, vec3(0))) && !all(greaterThan(y, vec3(0)))) {
        alpha += uilib_freetype_draw_curve(a, b, c, inverse_pixel_width.x);
      }
      if(super_sampling && !all(lessThan(x, vec3(0))) && !all(greaterThan(x, vec3(0)))) {
        alpha += uilib_freetype_draw_curve(vec2(a.y, -a.x), vec2(b.y, -b.x), vec2(c.y, -c.x), inverse_pixel_width.y);
      }
      a = c;
    }
  }
  return alpha * (float(!super_sampling)*0.5+0.5);
}

vec4 uilib_freetype_draw_glyph(vec4 default_color, vec2 uv, uint glyph_index, bool super_sampling) {
  vec2 inverse_pixel_width = 1.0 / fwidth(uv);
  uvec4 glyph = uilib_freetype_user_font(glyph_index);
  if(glyph.y == 1u) {
    float alpha = uilib_freetype_draw_layer(uv, inverse_pixel_width, glyph.z, glyph.w, super_sampling);
    return vec4(default_color.rgb, default_color.a * alpha);
  } else {
    vec4 color = vec4(0);
    for(uint i = 0u; i < glyph.y; i++) {
      uvec4 layer = uilib_freetype_user_font(glyph.x + i);
      float alpha = uilib_freetype_draw_layer(uv, inverse_pixel_width, glyph.z, glyph.w, super_sampling);
      
    }
    return color;
  }
}
  );
}

#undef MULTILINE_STRING

void uilib_freetype_texture_callback(struct uilib *uilib, uint32_t (*f)(uint32_t length, void ** ptr)) {
  uilib->freetype_texture_callback = f;
}

struct uilib_freetype__Counter {
  uint32_t num_points;
  uint32_t num_contours;
};

static int uilib_freetype__counter_move(const FT_Vector * to, void * user) {
  struct uilib_freetype__Counter *c = (struct uilib_freetype__Counter *)user;
  c->num_points += 1;
  c->num_contours += 1;
  return 0;
}

static int uilib_freetype__counter_line(const FT_Vector * to, void * user) {
  struct uilib_freetype__Counter *c = (struct uilib_freetype__Counter *)user;
  c->num_points += 2;
  return 0;
}

static int uilib_freetype__counter_conic(const FT_Vector * control, const FT_Vector * to, void * user) {
  struct uilib_freetype__Counter *c = (struct uilib_freetype__Counter *)user;
  c->num_points += 2;
  return 0;
}

static int uilib_freetype__counter_cubic(const FT_Vector * control1, const FT_Vector * control2, const FT_Vector * to, void * user) {
  struct uilib_freetype__Counter *c = (struct uilib_freetype__Counter *)user;
  c->num_points += 4;
  return 0;
}

struct uilib_freetype__Emitter {
  float scale;
  uint16_t * data;
  uint32_t contour_data_index_start;
  uint32_t data_index;
  float ax;
  float ay;
};

static void uilib_freetype__Emitter_flush(struct uilib_freetype__Emitter *e) {
  if(e->contour_data_index_start > 0) {
    e->data[e->contour_data_index_start * 4 + 0] = e->data_index - e->data[e->contour_data_index_start * 4 + 0];
  }
  e->contour_data_index_start = 0;
}

static void uilib_freetype__Emitter_contour(struct uilib_freetype__Emitter *e, float ax, float ay) {
  uilib_freetype__Emitter_flush(e);
  e->contour_data_index_start = e->data_index;
  e->data[e->data_index * 4 + 0] = 0;
  e->data[e->data_index * 4 + 1] = 0;
  e->data[e->data_index * 4 + 2] = (uint16_t)((int32_t)e->ax + 32768);
  e->data[e->data_index * 4 + 3] = (uint16_t)((int32_t)e->ay + 32768);
  e->data_index++;
  e->ax = ax;
  e->ay = ay;
}

static void uilib_freetype__Emitter_curve(struct uilib_freetype__Emitter *e, float bx, float by, float cx, float cy) {
  float x = e->ax - 2.0f*bx + cx;
  float y = e->ay - 2.0f*by + cy;
  if(fabs(x) < 0.00001f || fabs(y) < 0.00001f) {
    bx += (e->ax - cx) * 0.1f;
    by += (e->ay - cy) * 0.1f;
  }
  e->data[e->data_index * 4 + 0] = (uint16_t)((int32_t)bx + 32768);
  e->data[e->data_index * 4 + 1] = (uint16_t)((int32_t)by + 32768);
  e->data[e->data_index * 4 + 2] = (uint16_t)((int32_t)cx + 32768);
  e->data[e->data_index * 4 + 3] = (uint16_t)((int32_t)cy + 32768);
  e->data_index++;
  e->ax = cx;
  e->ay = cy;
}

static int uilib_freetype__emitter_move(const FT_Vector * to, void * user) {
  struct uilib_freetype__Emitter *e = (struct uilib_freetype__Emitter *)user;
  uilib_freetype__Emitter_contour(e, (float)to->x * e->scale, (float)to->y * e->scale);
  return 0;
}

static int uilib_freetype__emitter_line(const FT_Vector * to, void * user) {
  struct uilib_freetype__Emitter *e = (struct uilib_freetype__Emitter *)user;
  float cx = (float)to->x * e->scale;
  float cy = (float)to->y * e->scale;
  float bx = (e->ax + cx) * 0.5f;
  float by = (e->ay + cy) * 0.5f;
  uilib_freetype__Emitter_curve(e, bx, by, cx, cy);
  return 0;
}

static int uilib_freetype__emitter_conic(const FT_Vector * control, const FT_Vector * to, void * user) {
  struct uilib_freetype__Emitter *e = (struct uilib_freetype__Emitter *)user;
  float bx = (float)control->x * e->scale;
  float by = (float)control->y * e->scale;
  float cx = (float)to->x * e->scale;
  float cy = (float)to->y * e->scale;
  uilib_freetype__Emitter_curve(e, bx, by, cx, cy);
  return 0;

}

static int uilib_freetype__emitter_cubic(const FT_Vector * control1, const FT_Vector * control2, const FT_Vector * to, void * user) {
  struct uilib_freetype__Emitter *e = (struct uilib_freetype__Emitter *)user;
  // a      b   c  d   e
  // start c1 mid c2 end
  float control1x = (float)control1->x * e->scale;
  float control1y = (float)control1->y * e->scale;
  float control2x = (float)control2->x * e->scale;
  float control2y = (float)control2->y * e->scale;
  float ex = (float)to->x * e->scale;
  float ey = (float)to->y * e->scale;
  float bx = e->ax + (control1x - e->ax) * 0.75;
  float by = e->ax + (control1y - e->ay) * 0.75;
  float dx = ex + (control2x - ex) * 0.75;
  float dy = ey + (control2y - ey) * 0.75;
  float cx = (bx + dx) * 0.5;
  float cy = (by + dy) * 0.5;
  uilib_freetype__Emitter_curve(e, bx, by, cx, cy);
  uilib_freetype__Emitter_curve(e, dx, dy, ex, ey);
  return 0;
}

struct uilib_freetype_font_character {
  uint32_t charcode;
  uint32_t have_bold        : 1;
  uint32_t have_italic      : 1;
  uint32_t have_bold_italic : 1;
  uint32_t glyph_index      : 29;
};

struct uilib_freetype_font_glyph {
  float advance_x;
  float x;
  float y;
  float width;
  float height;
};

struct uilib_freetype_font {
  struct uilib_font root;

  struct uilib_freetype_font_character * character_map;
  uint32_t num_characters;

  struct uilib_freetype_font_glyph * glyph_map;
  uint32_t num_glyphs;
};

static int uilib_freetype_character_map_compare(const void * a, const void * b) {
  uint32_t charcode = *(const uint32_t *)a;
  const struct uilib_freetype_font_character * character = (const struct uilib_freetype_font_character *)b;
  return (int)((int64_t)character->charcode - (int64_t)a);
}

static bool uilib_freetype_font_decode(struct uilib_font *_font, const char *string, uint32_t *inout_glyph, uint32_t *out_num_bytes, struct uilib_font_glyph_info *out_info) {
  struct uilib_freetype_font * font = (struct uilib_freetype_font *)_font->user_data;
  
  uint32_t charcode = *string;
  struct uilib_freetype_font_character * character = bsearch(&charcode, font->character_map, font->num_characters, sizeof(*font->character_map), uilib_freetype_character_map_compare);
  *out_num_bytes = 1;

  if(character != NULL) {
    *inout_glyph = character - font->character_map;
  } else {
    return false;
  }

  out_info->newline == *string == '\n';
  out_info->advance_x = font->glyph_map[*inout_glyph].advance_x;
  out_info->x = font->glyph_map[*inout_glyph].x;
  out_info->y = font->glyph_map[*inout_glyph].y;
  out_info->width = font->glyph_map[*inout_glyph].width;
  out_info->height = font->glyph_map[*inout_glyph].height;
  out_info->s0 = 0.0f + *inout_glyph;
  out_info->t0 = 0.0f;
  out_info->s1 = 1.0f + *inout_glyph;
  out_info->t1 = 1.0f;
  return true;
}

struct uilib_font * uilib_freetype_create_font(struct uilib *uilib, const char ** filenames, uint32_t num_filenames) {
  FT_Library library;
  FT_Init_FreeType(&library);

  struct uilib_freetype_font * font = malloc(sizeof(*font));

  uint32_t num_faces = num_filenames;
  FT_Face * faces = malloc(sizeof(FT_Face) * num_faces);

  for(uint32_t i = 0; i < num_filenames; i++) {
    FT_New_Face(library, filenames[i], 0, &faces[i]);
  }

  struct {
    struct {
      uint32_t face;
      uint32_t glyph;
    } style[4];
  } *building_character_map = NULL;

  font->num_glyphs = 0;
  for(uint32_t charcode = ' '; charcode < 0x1FFFFF; charcode++) {
    uint32_t face_index = 0;
    FT_UInt glyph_index = 0;
    struct {
      uint32_t face;
      uint32_t glyph;
    } style[4] = {{-1,-1},{-1,-1},{-1,-1},{-1,-1}};
    for(; face_index < num_faces; face_index++) {
      glyph_index = FT_Get_Char_Index(faces[face_index], charcode);
      if(glyph_index != 0) {
        if(style[faces[face_index]->style_flags].face == -1) {
          style[faces[face_index]->style_flags].face = face_index;
          style[faces[face_index]->style_flags].glyph = glyph_index;
        }
      }
    }
    if(style[0].face == -1) {
      continue;
    }
    font->character_map = realloc(font->character_map, sizeof(*font->character_map) * (font->num_characters + 1));
    building_character_map = realloc(building_character_map, sizeof(*building_character_map) * (font->num_characters + 1));

    font->character_map[font->num_characters].charcode = charcode;
    font->character_map[font->num_characters].glyph_index = font->num_glyphs;
    font->character_map[font->num_characters].have_bold = style[1].face != -1;
    font->character_map[font->num_characters].have_italic = style[2].face != -1;
    font->character_map[font->num_characters].have_bold_italic = style[3].face != -1;
    building_character_map[font->num_characters].style[0].face = style[0].face;
    building_character_map[font->num_characters].style[0].glyph = style[0].glyph;
    building_character_map[font->num_characters].style[1].face = style[1].face;
    building_character_map[font->num_characters].style[1].glyph = style[1].glyph;
    building_character_map[font->num_characters].style[2].face = style[2].face;
    building_character_map[font->num_characters].style[2].glyph = style[2].glyph;
    building_character_map[font->num_characters].style[3].face = style[3].face;
    building_character_map[font->num_characters].style[3].glyph = style[3].glyph;

    font->num_glyphs += 1 + 
      font->character_map[font->num_characters].have_bold +
      font->character_map[font->num_characters].have_italic +
      font->character_map[font->num_characters].have_bold_italic;

    font->num_characters++;
  }


  struct uilib_freetype__Counter counter;
  memory_clear(&counter, sizeof(counter));

  FT_Outline_Funcs counter_funcs;
  memory_clear(&counter_funcs, sizeof(counter_funcs));
  counter_funcs.move_to = uilib_freetype__counter_move;
  counter_funcs.line_to = uilib_freetype__counter_line;
  counter_funcs.conic_to = uilib_freetype__counter_conic;
  counter_funcs.cubic_to = uilib_freetype__counter_cubic;

  for(uint32_t character_index = 0; character_index < font->num_characters; character_index++) {
    for(uint32_t style_index = 0; style_index < 4; style_index) {
      if(building_character_map[character_index].style[style_index].face != -1) {
        FT_Face face = faces[building_character_map[character_index].style[style_index].face];
        FT_Load_Glyph(
          face,
          building_character_map[character_index].style[style_index].glyph,
          FT_LOAD_NO_HINTING | FT_LOAD_NO_SCALE | FT_LOAD_LINEAR_DESIGN
        );
        FT_Outline_Decompose(&face->glyph->outline, &counter_funcs, &counter);

      }
    }
  }

  FT_Outline_Funcs emitter_funcs;
  memory_clear(&emitter_funcs, sizeof(emitter_funcs));
  emitter_funcs.move_to = uilib_freetype__emitter_move;
  emitter_funcs.line_to = uilib_freetype__emitter_line;
  emitter_funcs.conic_to = uilib_freetype__emitter_conic;
  emitter_funcs.cubic_to = uilib_freetype__emitter_cubic;

  uint32_t length = font->num_glyphs + counter.num_contours + (counter.num_points - counter.num_contours) * 2;

  struct uilib_freetype__Emitter emitter;
  memory_clear(&emitter, sizeof(emitter));
  uilib->freetype_texture_callback(length, (void **)&emitter.data);
  emitter.data_index = font->num_characters;

  font->glyph_map = malloc(sizeof(*font->glyph_map) * font->num_glyphs);

  uint32_t glyph_index = 0;
  for(uint32_t character_index = 0; character_index < font->num_characters; character_index++) {
    for(uint32_t style_index = 0; style_index < 4; style_index) {
      if(building_character_map[character_index].style[style_index].face != -1) {
        FT_Face face = faces[building_character_map[character_index].style[style_index].face];
        FT_Load_Glyph(
          face,
          building_character_map[character_index].style[style_index].glyph,
          FT_LOAD_NO_HINTING | FT_LOAD_NO_SCALE
        );
        emitter.scale = 65535.0f / (float)face->units_per_EM;
        FT_Outline_Decompose(&face->glyph->outline, &emitter_funcs, &emitter);
        uilib_freetype__Emitter_flush(&emitter);
        font->glyph_map[glyph_index].advance_x = (float)face->glyph->metrics.horiAdvance * emitter.scale;
        font->glyph_map[glyph_index].x = (float)face->glyph->metrics.horiBearingX * emitter.scale;
        font->glyph_map[glyph_index].y = (float)face->glyph->metrics.horiBearingY * emitter.scale;
        font->glyph_map[glyph_index].width = (float)face->glyph->metrics.width * emitter.scale;
        font->glyph_map[glyph_index].height = (float)face->glyph->metrics.height * emitter.scale;
      }
    }
  }

  for(uint32_t i = 0; i < num_faces; i++) {
    FT_Done_Face(faces[i]);
  }

  free(faces);
  free(building_character_map);

  font->root.texture_id = uilib->freetype_texture_callback(length, (void **)&emitter.data);
  font->root.decode = uilib_freetype_font_decode;
  font->root.user_data = font;
  return &font->root;
}

void uilib_freetype_free_font(struct uilib_font * font) {
}

#endif

// ====================================================================================================================

uilib_Result uilib_initialize(uilib **uilib_ptr) {
  struct uilib * uilib = memory_alloc(sizeof(struct uilib), alignof(struct uilib));
  *uilib_ptr = uilib;
  if(uilib == NULL) {
    return uilib_ErrorOutOfMemory;
  }
  memory_clear(uilib, sizeof(*uilib));
  return uilib_add_font(uilib, &uilib__dev_font, NULL);
}

void uilib_free(uilib *uilib) {
}

uilib_Result uilib_add_font(uilib *uilib, struct uilib_Font *font, uint32_t *out_result) {
  if(!uilib__grow((void **)&uilib->font, sizeof(*uilib->font), uilib->font_length + 1, &uilib->font_capacity)) {
    return uilib_ErrorOutOfMemory;
  }
  uint32_t index = uilib->font_length;
  uilib->font[index] = *font;
  uilib->font_length += 1;
  if(out_result != NULL) {
    *out_result = index;
  }
  return uilib_Success;
}

uilib_Result uilib_begin_frame(uilib *uilib, const uilib_Input *input) {
  uilib__program__initialize(uilib);
  uilib__memory__initialize(uilib);
  uilib__stats__initialize(uilib);
  uilib->input = *input;
  return uilib_Success;
}

void uilib_stats(uilib *uilib, uint32_t *num_vertexes, uint32_t *num_indexes, uint32_t *num_groups) {
  *num_vertexes = uilib->stats.num_vertexes;
  *num_indexes = uilib->stats.num_indexes;
  *num_groups = uilib->stats.num_groups;
}

uilib_Result uilib_end_frame(uilib *uilib, uilib_Output *output) {
  uilib->output = output;

  float width = uilib->input.screen_size.width;
  float height = uilib->input.screen_size.height;
  
  uilib__layout__initialize(uilib);
  uilib__layout(uilib, 0, width, 0, height, &width, &height);

  uilib__render__initialize(uilib);
  uilib__render(uilib, 0, 0, width, height);

  uilib__group__finalize(uilib);

  return uilib_Success;
}

#define UI_NUM_GROUPS   1024

static uilib * _ui = NULL;
static bool _ui_recording = false;
static uilib_OutputGroup _ui_groups[UI_NUM_GROUPS];

extern struct Font BreeSerif;

static inline void _ui_text_size(uilib *_ui, const char * buffer, float size, float _max_width, float * out_width, float * out_height) {
  (void)_ui;
  (void)_max_width;
  Font_measure(&BreeSerif, buffer, size, 0, out_width, out_height);
}

static inline void _ui_text_draw(uilib *_ui, const char * buffer, float x, float y, float _width, float size, Color color) {
  (void)_ui;
  (void)_width;
  Font_draw(&BreeSerif, buffer, x, y, size, 0, color);
}

static inline void _ui_start(void) {
  static uilib_Input input;

  if(_ui == NULL) {
    // TODO proper init/shutdown
    uilib_initialize(&_ui);
  }

  if(!_ui_recording) {
    uint32_t width, height;
    display_dimensions(&width, &height);
    
    input.screen_size.width = width;
    input.screen_size.height = height;
    input.text_size = _ui_text_size;
    input.text_draw = _ui_text_draw;

    uilib_begin_frame(_ui, &input);
    _ui_recording = true;

    uilib_begin_stack(_ui);
  }
}

void ui_image(struct Image * img) {
  _ui_start();
  struct LoadedResource * resource = LoadedResource_from_image(img);
  uilib_image(_ui, resource->image.width, resource->image.height, 0, 0, 1, 1, resource->id);
}

void ui_alignFractions(float x, float y) {
  _ui_start();
  uilib_align(_ui, x, y);
}

void ui_fontSize(R size) {
  _ui_start();
  uilib_font_size(_ui, size);
}

void ui_fontColor(Color color) {
  _ui_start();
  uilib_font_color(_ui, color);
}

void ui_text(const char * format, ...) {
  _ui_start();

  va_list ap;
  va_start(ap, format);
  uilib_textv(_ui, format, ap);
  va_end(ap);
}

void ui_vertical(void) {
  _ui_start();
  uilib_begin_vertical_stack(_ui);
}

void ui_horizontal(void) {
  _ui_start();
  uilib_begin_horizontal_stack(_ui);
}

void ui_stack(void) {
  _ui_start();
  uilib_begin_stack(_ui);
}

void ui_end(void) {
  _ui_start();
  uilib_end_stack(_ui);
}

void ui_iterate(void) {
  // _state_ui();

  if(!_ui_recording) {
    return;
  }

  uilib_end_stack(_ui); // end stack

  uint32_t num_vertexes = 0, num_indexes = 0, num_groups = 0;
  uilib_stats(_ui, &num_vertexes, &num_indexes, &num_groups);

  uint32_t * indexes;
  struct draw_ScreenVertex * vertexes;

  draw_ScreenVertex_begin_draw(num_indexes, &indexes, num_vertexes, &vertexes);

  uilib_Output output = {
      .num_groups = 0
    , .max_groups = UI_NUM_GROUPS
    , .groups = _ui_groups
    , .num_indexes = 0
    , .index_sub_buffer = (memory_SubBuffer) {
        .pointer = indexes
      , .count = num_indexes
      , .stride = sizeof(*indexes)
      , .type_format = memory_Format_Uint32
      , .type_length = 1
      }
    , .num_vertexes = 0
    , .xy_sub_buffer = (memory_SubBuffer) {
        .pointer = (uint8_t *)vertexes + offsetof(struct draw_ScreenVertex, xy)
      , .count = num_vertexes
      , .stride = sizeof(*vertexes)
      , .type_format = memory_Format_Float32
      , .type_length = 2
      }
    , .rgba_sub_buffer = (memory_SubBuffer) {
        .pointer = (uint8_t *)vertexes + offsetof(struct draw_ScreenVertex, rgba)
      , .count = num_vertexes
      , .stride = sizeof(*vertexes)
      , .type_format = memory_Format_Float32
      , .type_length = 4
      }
    , .st_sub_buffer = (memory_SubBuffer) {
        .pointer = (uint8_t *)vertexes + offsetof(struct draw_ScreenVertex, st)
      , .count = num_vertexes
      , .stride = sizeof(*vertexes)
      , .type_format = memory_Format_Float32
      , .type_length = 2
      }
    };
  
  uilib_end_frame(_ui, &output);
  _ui_recording = 0;

  for(uint32_t g = 0; g < output.num_groups; g++) {
    uint32_t length = _ui_groups[g].length;
    if(length == 0) {
      continue;
    }

    struct LoadedResource * material = LoadedResource_from_id(_ui_groups[g].texture_id);

    draw_ScreenVertex_draw(&material->image, _ui_groups[g].index, _ui_groups[g].length);
  }

  draw_ScreenVertex_end_draw();
}

