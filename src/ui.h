#ifndef ui_h_INCLUDED
#define ui_h_INCLUDED

#include <stdarg.h>

#include "color.h"
#include "memory.h"
#include "image.h"

#ifndef ALIAS_UI_INDEX_SIZE
#define ALIAS_UI_INDEX_SIZE 16
#endif

typedef struct uilib uilib;

typedef enum uilib_Result { uilib_Success, uilib_ErrorOutOfMemory } uilib_Result;

struct uilib_Font_GlyphInfo {
  bool newline;
  float advance_x;
  float x;
  float y;
  float width;
  float height;
  float s0;
  float t0;
  float s1;
  float t1;
};

struct uilib_Font {
  struct uilib_Font *fallback;
  uint32_t texture_id;
  bool (*decode)(struct uilib_Font *font, const char *string, uint32_t *inout_glyph, uint32_t *out_num_bytes,
                 struct uilib_Font_GlyphInfo *out_info);
  void *user_data;
};

typedef struct uilib_Size {
  float width;
  float height;
} uilib_Size;

typedef struct uilib_Constraint {
  uilib_Size min;
  uilib_Size max;
} uilib_Constraint;

void uilib_set_texture(struct uilib *uilib, uint32_t texture_id);
void uilib_draw_rectangle(struct uilib *uilib, float x, float y, float width, float height,
                                                     float s0, float t0, float s1, float t1, uint8_t r, uint8_t g,
                                                     uint8_t b, uint8_t a);

typedef void (*uilib_TextSizeFn)(uilib *uilib, const char *buffer, float size, float max_width, float *out_width, float *out_height);
typedef void (*uilib_TextDrawFn)(uilib *uilib, const char *buffer, float x, float y, float width, float size, Color color);

typedef struct uilib_Input {
  uilib_Size screen_size;

  uilib_TextSizeFn text_size;
  uilib_TextDrawFn text_draw;
} uilib_Input;

typedef struct uilib_OutputGroup {
  uint32_t texture_id;
  uint32_t index;
  uint32_t length;
} uilib_OutputGroup;

typedef struct uilib_Output {
  uint32_t num_groups;
  uint32_t max_groups;
  uilib_OutputGroup *groups;

  uint32_t num_indexes;
  memory_SubBuffer index_sub_buffer;

  uint32_t num_vertexes;
  memory_SubBuffer xy_sub_buffer;
  memory_SubBuffer rgba_sub_buffer;
  memory_SubBuffer st_sub_buffer;
} uilib_Output;

// lifetime
uilib_Result uilib_initialize(uilib **uilib_ptr);
void uilib_free(uilib *uilib);

// frame
uilib_Result uilib_begin_frame(uilib *uilib, const uilib_Input *input);
uilib_Result uilib_end_frame(uilib *uilib, uilib_Output *output);

uilib_Result uilib_add_font(struct uilib *uilib, struct uilib_Font *font, uint32_t *out_result);

// layouting
void uilib_align(uilib *uilib, float x, float y);
void uilib_align_x(uilib *uilib, float x);
void uilib_align_y(uilib *uilib, float y);

void uilib_size(uilib *uilib, float width, float height);
void uilib_width(uilib *uilib, float width);
void uilib_height(uilib *uilib, float height);

void uilib_margin(uilib *uilib, float left, float right, float top, float bottom);
void uilib_padding(uilib *uilib, float left, float right, float top, float bottom);

void uilib_begin_stack(uilib *uilib);
void uilib_begin_vertical_stack(uilib *uilib);
void uilib_begin_horizontal_stack(uilib *uilib);
void uilib_end_stack(uilib *uilib);
void uilib_stats(uilib *uilib, uint32_t *num_vertexes, uint32_t *num_indexes, uint32_t *num_groups);

// per scope parameters
void uilib_font_size(uilib *uilib, float size);
void uilib_font_color(uilib *uilib, Color color);

// create elements
void uilib_textv(uilib *uilib, const char *format, va_list ap);
void uilib_color_fill(uilib *uilib, float r, float g, float b, float a);
void uilib_image(uilib *uilib, float width, float height, float s0, float t0, float s1, float t1, uint32_t texture_id);

static inline void uilib_text(uilib *uilib, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  uilib_textv(uilib, format, ap);
  va_end(ap);
}

void ui_alignFractions(float x, float y);

static inline void ui_topLeft(void) { ui_alignFractions(0, 0); }
static inline void ui_top(void) { ui_alignFractions(0.5, 0); }
static inline void ui_topRight(void) { ui_alignFractions(1, 0); }
static inline void ui_left(void) { ui_alignFractions(0, 0.5); }
static inline void ui_center(void) { ui_alignFractions(0.5, 0.5); }
static inline void ui_right(void) { ui_alignFractions(1, 0.5); }
static inline void ui_bottomLeft(void) { ui_alignFractions(0, 1); }
static inline void ui_bottom(void) { ui_alignFractions(0.5, 1); }
static inline void ui_bottomRight(void) { ui_alignFractions(1, 1); }

void ui_image(struct Image *);

void ui_vertical(void);
void ui_horizontal(void);
void ui_stack(void);
void ui_end(void);

void ui_fontSize(R size);
void ui_fontColor(Color color);
void ui_text(const char *format, ...);

void ui_iterate(void);

#endif // ui_h_INCLUDED
