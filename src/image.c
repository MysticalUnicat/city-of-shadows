#define STBI_NO_BMP
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_NO_LINEAR
//#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "image.h"
#include "gl.h"

void GraphicsImage_load(struct GraphicsImage *image, const char *path) {
  int width, height, channels;
  uint8_t *data = stbi_load(path, &width, &height, &channels, 0);

  if(data == NULL) {
    return;
  }

  if(channels != 3 && channels != 4) {
    free(data);
    return;
  }

  GLenum internal_format = channels == 3 ? GL_RGB8 : GL_RGBA8;
  GLenum external_format = channels == 3 ? GL_RGB : GL_RGBA;

  int levels = 1, w = width, h = height;
  while(w > 1 && h > 1) {
    levels++;
    w = w >> (w > 1);
    h = h >> (h > 1);
  }

  glCreateTextures(GL_TEXTURE_2D, 1, &image->gl.image);
  glTextureStorage2D(image->gl.image, levels, internal_format, width, height);
  glTextureSubImage2D(image->gl.image, 0, 0, 0, width, height, external_format, GL_UNSIGNED_BYTE, data);
  glGenerateTextureMipmap(image->gl.image);
  glTextureParameteri(image->gl.image, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTextureParameteri(image->gl.image, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  stbi_image_free(data);

  image->width = width;
  image->height = height;
  image->depth = 1;
  image->levels = levels;
  image->layers = 1;
  image->internal_format = internal_format;
}

void GraphicsImage_unload(struct GraphicsImage *image) {
}

