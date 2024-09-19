#pragma once

#include "string.h"
#include "image.h"
#include "font.h"
#include "inline_list.h"

enum ResourceType { ResourceType_invalid, ResourceType_image, ResourceType_font };

struct LoadedResource {
  InlineList list;

  enum ResourceType type;
  uint32_t id, gen;

  union {
    struct GraphicsImage image;
    struct Font font;
  };
};

void LoadedResource_free(struct LoadedResource * resource);
void LoadedResource_gc(void);
void LoadedResource_touch(struct LoadedResource * resource);
struct LoadedResource * LoadedResource_from_image(struct Image *img);
struct LoadedResource * LoadedResource_from_id(uint32_t id);
