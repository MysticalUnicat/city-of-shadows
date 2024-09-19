#ifndef image_h_INCLUDED
#define image_h_INCLUDED

#include <stdint.h>

struct GraphicsImage {
  uint32_t width;
  uint32_t height;
  uint32_t depth;

  uint32_t levels;
  uint32_t layers;

  uint32_t internal_format;

  union {
    struct {
      uint32_t image;
    } gl;
    struct {
      uint64_t image;
      uint64_t memory;
      uint64_t imageview;
    } vk;
  };
};

void GraphicsImage_load(struct GraphicsImage *image, const char *path);
void GraphicsImage_unload(struct GraphicsImage *image);

struct Image {
  const char *path;
  uint32_t resource_id;
  struct LoadedResource *resource;
};

#endif // image_h_INCLUDED
