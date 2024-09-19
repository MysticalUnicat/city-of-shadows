#include "font.h"
#include "color.h"
#include "draw.h"
#include "resource.h"

#include <uchar.h>

const struct Font_Glyph *Font_findGlyph(const struct Font *font, const char *text) {
  // TODO handle utf8
  char32_t c = *text;

  // TODO bsearch
  for(uint32_t i = 0; i < font->num_glyphs; i++) {
    if(font->glyphs[i].codepoint == c) {
      return &font->glyphs[i];
    }
  }

  // return space
  return &font->glyphs[0];
}

void Font_measure(struct Font *font, const char *text, float size, float spacing, float *width, float *height) {
  *height = size;
  *width = 0;

  while(*text) {
    const struct Font_Glyph *glyph = Font_findGlyph(font, text);

    // TODO utf8 advance
    text++;

    (*width) += glyph->advance * size + spacing;
  }
}

#define FONT_DRAW_CHARACTERS_PER_BATCH 1024

void Font_draw(struct Font *font, const char *text, float x, float y, float size, float spacing, Color color) {
  struct draw_ScreenVertex *vertexes = NULL;
  uint32_t *indexes = NULL;
  uint32_t i = 0;

  size_t length = string_size(text);

  struct GraphicsImage *image = &LoadedResource_from_image(&font->atlas)->image;
  float s_scale = R_ONE / image->width;
  float t_scale = R_ONE / image->height;

  while(*text) {
    if(i == 0) {
      draw_ScreenVertex_begin_draw(6 * min(FONT_DRAW_CHARACTERS_PER_BATCH, length), &indexes,
                               4 * min(FONT_DRAW_CHARACTERS_PER_BATCH, length), &vertexes);
    }

    const struct Font_Glyph *glyph = Font_findGlyph(font, text);

    // TODO utf8 advance
    text++;

#define EMIT(I, V, H)                                                                                                  \
  vertexes[i * 4 + I].xy[0] = x + glyph->plane_##H * size;                                                             \
  vertexes[i * 4 + I].xy[1] = y + (1.0f - glyph->plane_##V) * size;                                                    \
  vertexes[i * 4 + I].rgba[0] = color.r;                                                                               \
  vertexes[i * 4 + I].rgba[1] = color.g;                                                                               \
  vertexes[i * 4 + I].rgba[2] = color.b;                                                                               \
  vertexes[i * 4 + I].rgba[3] = color.a;                                                                               \
  vertexes[i * 4 + I].st[0] = glyph->atlas_##H * s_scale;                                                              \
  vertexes[i * 4 + I].st[1] = 1.0f - (glyph->atlas_##V * t_scale);
    EMIT(0, bottom, left)
    EMIT(1, bottom, right)
    EMIT(2, top, right)
    EMIT(3, top, left)
#undef EMIT

    indexes[i * 6 + 0] = i * 4 + 0;
    indexes[i * 6 + 1] = i * 4 + 2;
    indexes[i * 6 + 2] = i * 4 + 1;
    indexes[i * 6 + 3] = i * 4 + 2;
    indexes[i * 6 + 4] = i * 4 + 0;
    indexes[i * 6 + 5] = i * 4 + 3;

    x += glyph->advance * size + spacing;
    i++;
    length--;

    if(i >= FONT_DRAW_CHARACTERS_PER_BATCH) {
      draw_ScreenVertex_draw(image, 0, 6 * FONT_DRAW_CHARACTERS_PER_BATCH);
      draw_ScreenVertex_end_draw();
      i = 0;
    }
  }

  if(i >= 0) {
    draw_ScreenVertex_draw(image, 0, 6 * i);
    draw_ScreenVertex_end_draw();
  }
}

