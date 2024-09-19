#include "draw.h"

#include "transform.h"
#include "input.h"
#include "ui.h"
#include "matrix.h"
#include "display.h"
#include "font.h"
#include "resource.h"

ECS_COMPONENT(DrawRectangle)
ECS_COMPONENT(DrawCircle)
ECS_COMPONENT(DrawText)
ECS_COMPONENT(Sprite)

// ====================================================================================================================
static void _draw_sprite(const struct LocalToWorld2D *t, const struct Sprite *s) {
  struct LoadedResource *res = LoadedResource_from_image(s->image);

  R hw = res->image.width / 2, hh = res->image.height / 2, bl = -hw, br = hw, bt = -hh, bb = hh;

  pga2d_Point box[] = {pga2d_point(br, bb), pga2d_point(br, bt), pga2d_point(bl, bt),
                             pga2d_point(bl, bb)};

  box[0] = pga2d_sandwich_bm(box[0], t->motor);
  box[1] = pga2d_sandwich_bm(box[1], t->motor);
  box[2] = pga2d_sandwich_bm(box[2], t->motor);
  box[3] = pga2d_sandwich_bm(box[3], t->motor);

  uint32_t *indexes;
  struct draw_ScreenVertex *vertexes;
  draw_ScreenVertex_begin_draw(6, &indexes, 4, &vertexes);

  vertexes[0] = (struct draw_ScreenVertex){.xy = {pga2d_point_x(box[0]), pga2d_point_y(box[0])},
                                      .rgba = {s->color.r, s->color.g, s->color.b, s->color.a},
                                      .st = {s->s1, s->t1}};
  vertexes[1] = (struct draw_ScreenVertex){.xy = {pga2d_point_x(box[1]), pga2d_point_y(box[1])},
                                      .rgba = {s->color.r, s->color.g, s->color.b, s->color.a},
                                      .st = {s->s1, s->t0}};
  vertexes[2] = (struct draw_ScreenVertex){.xy = {pga2d_point_x(box[2]), pga2d_point_y(box[2])},
                                      .rgba = {s->color.r, s->color.g, s->color.b, s->color.a},
                                      .st = {s->s0, s->t0}};
  vertexes[3] = (struct draw_ScreenVertex){.xy = {pga2d_point_x(box[3]), pga2d_point_y(box[3])},
                                      .rgba = {s->color.r, s->color.g, s->color.b, s->color.a},
                                      .st = {s->s0, s->t1}};

  indexes[0] = 0;
  indexes[1] = 1;
  indexes[2] = 2;
  indexes[3] = 0;
  indexes[4] = 2;
  indexes[5] = 3;

  draw_ScreenVertex_draw(&res->image, 0, 6);
  draw_ScreenVertex_end_draw();
}

ECS_QUERY(_draw_sprites
  , read(LocalToWorld2D, t)
  , read(Sprite, s)
  , action(
    (void)state;
    _draw_sprite(t, s);
  )
)

// ====================================================================================================================
static void _draw_rectangle_action(const struct LocalToWorld2D *t, const struct DrawRectangle *r) {
 R
      hw = r->width / 2
    , hh = r->height / 2
    , bl = -hw
    , br =  hw
    , bt = -hh
    , bb =  hh
    ;
  pga2d_Point box[] = {
      pga2d_point(br, bb)
    , pga2d_point(br, bt)
    , pga2d_point(bl, bt)
    , pga2d_point(bl, bb)
    };
  box[0] = pga2d_sandwich_bm(box[0], t->motor);
  box[1] = pga2d_sandwich_bm(box[1], t->motor);
  box[2] = pga2d_sandwich_bm(box[2], t->motor);
  box[3] = pga2d_sandwich_bm(box[3], t->motor);
  uint32_t * indexes;
  struct draw_ScreenVertex * vertexes;
  draw_ScreenVertex_begin_draw(6, &indexes, 4, &vertexes);
  vertexes[0] = (struct draw_ScreenVertex) { .xy[0] = pga2d_point_x(box[0]), .xy[1] = pga2d_point_y(box[0]), .rgba = { r->color.r, r->color.g, r->color.b, r->color.a } };
  vertexes[1] = (struct draw_ScreenVertex) { .xy[0] = pga2d_point_x(box[1]), .xy[1] = pga2d_point_y(box[1]), .rgba = { r->color.r, r->color.g, r->color.b, r->color.a } };
  vertexes[2] = (struct draw_ScreenVertex) { .xy[0] = pga2d_point_x(box[2]), .xy[1] = pga2d_point_y(box[2]), .rgba = { r->color.r, r->color.g, r->color.b, r->color.a } };
  vertexes[3] = (struct draw_ScreenVertex) { .xy[0] = pga2d_point_x(box[3]), .xy[1] = pga2d_point_y(box[3]), .rgba = { r->color.r, r->color.g, r->color.b, r->color.a } };
  indexes[0] = 0;
  indexes[1] = 1;
  indexes[2] = 2;
  indexes[3] = 0;
  indexes[4] = 2;
  indexes[5] = 3;
  draw_ScreenVertex_draw(NULL, 0, 6);
  draw_ScreenVertex_end_draw();
}

ECS_QUERY(_draw_rectangles
  , read(LocalToWorld2D, t)
  , read(DrawRectangle, r)
  , action(
    (void)state;
    _draw_rectangle_action(t, r);
  )
)

// ====================================================================================================================
static void _draw_circle(float x, float y, float radius, Color color) {
#define NUM_CIRCLE_SEGMENTS 32

  struct draw_ScreenVertex * vertexes;
  uint32_t * indexes;

  draw_ScreenVertex_begin_draw(3 * (NUM_CIRCLE_SEGMENTS - 2), &indexes, NUM_CIRCLE_SEGMENTS, &vertexes);

  for(uint32_t i = 0; i < NUM_CIRCLE_SEGMENTS; i++) {
    R angle = (R)i / NUM_CIRCLE_SEGMENTS * R_PI * 2;
    R s = R_sin(angle);
    R c = R_cos(angle);
    vertexes[i].xy[0] = x + s * radius;
    vertexes[i].xy[1] = y + c * radius;
    vertexes[i].rgba[0] = color.r;
    vertexes[i].rgba[1] = color.g;
    vertexes[i].rgba[2] = color.b;
    vertexes[i].rgba[3] = color.a;
    vertexes[i].st[0] = 0;
    vertexes[i].st[1] = 0;

    if(i >= 2) {
      indexes[(i - 2) * 3 + 0] = 0;
      indexes[(i - 2) * 3 + 1] = i - 1;
      indexes[(i - 2) * 3 + 2] = i - 0;
    }
  }

  draw_ScreenVertex_draw(NULL, 0, 3 * (NUM_CIRCLE_SEGMENTS - 2));
  draw_ScreenVertex_end_draw();

#undef NUM_CIRCLE_SEGMENTS
}

ECS_QUERY(_draw_circles
  , read(LocalToWorld2D, transform)
  , read(DrawCircle, c)
  , action(
    (void)state;
    _draw_circle(pga2d_point_x(transform->position), pga2d_point_y(transform->position), c->radius, c->color);
  )
)

// ====================================================================================================================
extern struct Font BreeSerif;

ECS_QUERY(_draw_text
  , read(LocalToWorld2D, w)
  , read(DrawText, t)
  , action(
    (void)state;
    Font_draw(&BreeSerif, t->text, pga2d_point_x(w->position), pga2d_point_y(w->position), t->size, 1.0, t->color);
  )
)

// ====================================================================================================================
static void _draw_frame_action(const struct LocalToWorld2D * transform, const struct Camera * camera, uint32_t r_width, uint32_t r_height) {
  int x = pga2d_point_x(camera->viewport_min) * r_width;
  int y = pga2d_point_y(camera->viewport_min) * r_height;
  int width = (pga2d_point_x(camera->viewport_max) * r_width) - x;
  int height = (pga2d_point_y(camera->viewport_max) * r_height) - y;

  glViewport(x, y, width, height);

  pga2d_Point center = pga2d_sandwich_bm(pga2d_point(0, 0), transform->motor);

  float offset_x = width / 2.0f;
  float offset_y = height / 2.0f;
  float target_x = pga2d_point_x(center);
  float target_y = pga2d_point_y(center);
  float rotation = 0;
  float transform_m[16];

  {
    float origin_m[16], rotation_m[16], scale_m[16], translation_m[16];
    matrix_translation(-target_x, -target_y, 0, origin_m);
    matrix_rotation_z(rotation * R_PI / 180.0f, rotation_m);
    matrix_scale(camera->zoom, camera->zoom, 1, scale_m);
    matrix_translation(offset_x, offset_y, 0, translation_m);
    matrix_multiply(scale_m, rotation_m, transform_m);
    matrix_multiply(origin_m, transform_m, transform_m);
    matrix_multiply(transform_m, translation_m, transform_m);
  }

  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  _draw_sprites();
  _draw_rectangles();
  _draw_circles();
  _draw_text();
}

ECS_QUERY(_draw_frame
  , read(LocalToWorld2D, transform)
  , read(Camera, camera)
  , state(uint32_t, width)
  , state(uint32_t, height)
  , pre(
    display_dimensions(&state->width, &state->height);

    glViewport(0, 0, state->width, state->height);
    glClearColor(0.9, 0.9, 0.9, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    gl_resetTemporaryBuffers();
  )
  , action(
    (void)state;
    _draw_frame_action(transform, camera, state->width, state->height);
  )
  , post(
    matrix_ortho(0, state->width, state->height, 0, -99999, 99999, draw_projection_matrix.uniform.data.mat);
    matrix_identity(draw_view_matrix.uniform.data.mat);
    glViewport(0, 0, state->width, state->height);

    ui_iterate();
  )
)

