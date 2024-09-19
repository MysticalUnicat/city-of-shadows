#include "draw.h"

#include "matrix.h"

enum Sampler { Sampler_NEAREST, Sampler_LINEAR, NUM_BACKEND_SAMPLERS };

static inline void _prepare_model_view(void) {
  matrix_multiply(draw_view_matrix.uniform.data.mat, draw_model_matrix.uniform.data.mat,
                     draw_model_view_matrix.uniform.data.mat);
}

static inline void _prepare_view_projection(void) {
  matrix_multiply(draw_projection_matrix.uniform.data.mat, draw_view_matrix.uniform.data.mat,
                     draw_view_projection_matrix.uniform.data.mat);
}

static inline void _prepare_model_view_projection(void) {
  gl_ShaderResource_prepare(&draw_view_projection_matrix);
  matrix_multiply(draw_view_projection_matrix.uniform.data.mat, draw_model_matrix.uniform.data.mat,
                     draw_model_view_projection_matrix.uniform.data.mat);
}

struct gl_ShaderResource draw_time = {.type = gl_Type_float, .name = "time"};
struct gl_ShaderResource draw_model_matrix = {.type = gl_Type_float4x4, .name = "model_matrix"};
struct gl_ShaderResource draw_view_matrix = {.type = gl_Type_float4x4, .name = "view_matrix"};
struct gl_ShaderResource draw_model_view_matrix = {
    .type = gl_Type_float4x4, .name = "model_view_matrix", .uniform.prepare = _prepare_model_view};
struct gl_ShaderResource draw_projection_matrix = {.type = gl_Type_float4x4, .name = "projection_matrix"};
struct gl_ShaderResource draw_view_projection_matrix = {
    .type = gl_Type_float4x4, .name = "view_projection_matrix", .uniform.prepare = _prepare_view_projection};
struct gl_ShaderResource draw_model_view_projection_matrix = {
    .type = gl_Type_float4x4, .name = "model_view_projection_matrix", .uniform.prepare = _prepare_model_view_projection};

GL_SHADER_INSTANCE(draw_ScreenVertex_vertex,
  code(
    layout(location = 0) out vec2 out_st;
    layout(location = 1) out vec4 out_rgba;
  ),
  main(
    gl_Position = u_view_projection_matrix * vec4(in_xy, 0, 1);
    out_st = in_st;
    out_rgba = in_rgba;
  )
)

GL_SHADER_INSTANCE(draw_ScreenVertex_fragment,
  code(
    layout(location = 0) in vec2 in_st;
    layout(location = 1) in vec4 in_rgba;

    layout(location = 0) out vec4 out_color;
  ),
  main(
    out_color = texture(u_img, in_st) * in_rgba;
  )
)

static struct gl_Buffer _ScreenVertex_element_buffer;
static struct gl_Buffer _ScreenVertex_vertexes_buffer;
static struct gl_DrawState _ScreenVertex_draw_state = {
  .primitive = GL_TRIANGLES,
  .attribute[0] = {0, memory_Format_Float32, 2, "xy", 0},
  .attribute[1] = {0, memory_Format_Float32, 4, "rgba", 8},
  .attribute[2] = {0, memory_Format_Float32, 2, "st", 24},
  .binding[0] = {sizeof(struct draw_ScreenVertex)},
  .global[0] = {GL_VERTEX_BIT, &draw_view_projection_matrix},
  .image[0] = {GL_FRAGMENT_BIT, gl_Type_sampler2D, "img"},
  .vertex_shader = &draw_ScreenVertex_vertex_shader,
  .fragment_shader = &draw_ScreenVertex_fragment_shader,
  .depth_range_min = 0,
  .depth_range_max = 1,
  .blend_enable = true,
  .blend_src_factor = GL_SRC_ALPHA,
  .blend_dst_factor = GL_ONE_MINUS_SRC_ALPHA};

void draw_ScreenVertex_begin_draw(uint32_t num_indexes, uint32_t **indexes_ptr, uint32_t num_vertexes,
                                     struct draw_ScreenVertex **vertexes_ptr) {
  _ScreenVertex_element_buffer =
      gl_allocateTemporaryBuffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * num_indexes);
  _ScreenVertex_vertexes_buffer =
      gl_allocateTemporaryBuffer(GL_ARRAY_BUFFER, sizeof(struct draw_ScreenVertex) * num_vertexes);

  *indexes_ptr = _ScreenVertex_element_buffer.mapping;
  *vertexes_ptr = _ScreenVertex_vertexes_buffer.mapping;
}

void draw_ScreenVertex_draw(struct GraphicsImage * image, uint32_t index_offset, uint32_t num_indexes) {
  gl_drawElements(&_ScreenVertex_draw_state, &(struct gl_DrawAssets){
    .image[0] = image->gl.image,
    .element_buffer = &_ScreenVertex_element_buffer,
    .element_buffer_offset = index_offset,
    .vertex_buffers[0] = &_ScreenVertex_vertexes_buffer
  }, num_indexes, 1, 0, 0);
}

void draw_ScreenVertex_end_draw(void) {
}

