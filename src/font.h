#ifndef font_h_INCLUDED
#define font_h_INCLUDED

#include <stdint.h>

#include "image.h"
#include "color.h"

enum Font_AtlasType { Font_AtlasType_Bitmap, Font_AtlasType_SDF };

struct Font_Glyph {
  uint32_t codepoint;

  float advance;

  float plane_left;
  float plane_top;
  float plane_right;
  float plane_bottom;

  float atlas_left;
  float atlas_top;
  float atlas_right;
  float atlas_bottom;
};

struct Font {
  struct Image atlas;
  enum Font_AtlasType atlas_type;
  uint32_t num_glyphs;
  const struct Font_Glyph *glyphs;
};

const struct Font_Glyph *Font_findGlyph(const struct Font *font, const char *text);
void Font_measure(struct Font *font, const char *text, float size, float spacing, float *width, float *height);
void Font_draw(struct Font *font, const char *text, float x, float y, float size, float spacing, Color color);

#endif // font_h_INCLUDED
