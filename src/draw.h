#ifndef draw_h_INCLUDED
#define draw_h_INCLUDED

#include "gl.h"
#include "color.h"
#include "image.h"
#include "ecs.h"
#include "math.h"
#include "camera.h"

struct draw_ScreenVertex {
  float xy[2];
  float rgba[4];
  float st[2];
};

extern struct gl_ShaderResource draw_time;
extern struct gl_ShaderResource draw_model_matrix;
extern struct gl_ShaderResource draw_view_matrix;
extern struct gl_ShaderResource draw_model_view_matrix;
extern struct gl_ShaderResource draw_projection_matrix;
extern struct gl_ShaderResource draw_view_projection_matrix;
extern struct gl_ShaderResource draw_model_view_projection_matrix;

void draw_ScreenVertex_begin_draw(uint32_t num_indexes, uint32_t **indexes_ptr, uint32_t num_vertexes,
                                     struct draw_ScreenVertex **vertexes_ptr);
void draw_ScreenVertex_draw(struct GraphicsImage * image, uint32_t index_offset, uint32_t num_indexes);
void draw_ScreenVertex_end_draw(void);

ECS_DECLARE_COMPONENT(DrawRectangle, {
  float width;
  float height;
  Color color;
})

ECS_DECLARE_COMPONENT(DrawCircle, {
  R radius;
  Color color;
})

ECS_DECLARE_COMPONENT(DrawText, {
  const char *text;
  R size;
  Color color;
})

ECS_DECLARE_COMPONENT(Sprite, {
  struct Image *image;
  R s0, t0, s1, t1;
  Color color;
})

#endif // draw_h_INCLUDED
