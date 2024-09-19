#include "gl.h"
#include "log.h"
#include "math.h"
#include "format.h"

#include <stdlib.h> // for ssize_t

#define SOURCE_NAMESPACE core.gl

GL_IMPL_STRUCT(DrawArraysIndirectCommand, uint32(count), uint32(instance_count), uint32(first),
                    uint32(base_instance))

GL_IMPL_STRUCT(DrawElementsIndirectCommand, uint32(count), uint32(instance_count), uint32(first_index),
                    uint32(base_vertex), uint32(base_instance))

GL_IMPL_STRUCT(DispatchIndirectCommand, uint32(num_groups_x), uint32(num_groups_y), uint32(num_groups_z))

const char shader_prelude[] = "#version 460 core\n"
                              "#extension gl_KHR_shader_subgroup_arithmetic : enable\n"
                              "#extension gl_EXT_shader_explicit_arithmetic_types_int64 : enable\n"
                              "#define _Bool bool\n";

struct gl_TypeInfo {
  const char *name;
  GLenum target;
};

static struct gl_TypeInfo type_info[] = {
    [gl_Type_float] = {"float"},
    [gl_Type_float2] = {"vec2"},
    [gl_Type_float3] = {"vec3"},
    [gl_Type_float4] = {"vec4"},
    [gl_Type_int] = {"int"},
    [gl_Type_int2] = {"ivec2"},
    [gl_Type_int3] = {"ivec3"},
    [gl_Type_int4] = {"ivec4"},
    [gl_Type_uint] = {"uint"},
    [gl_Type_uint2] = {"uvec2"},
    [gl_Type_uint3] = {"uvec3"},
    [gl_Type_uint4] = {"uvec4"},
    [gl_Type_float2x2] = {"mat2"},
    [gl_Type_float3x3] = {"mat3"},
    [gl_Type_float4x4] = {"mat4"},
    [gl_Type_float2x3] = {"mat2x3"},
    [gl_Type_float3x2] = {"mat3x2"},
    [gl_Type_float2x4] = {"mat2x4"},
    [gl_Type_float4x2] = {"mat4x2"},
    [gl_Type_float3x4] = {"mat3x4"},
    [gl_Type_float4x3] = {"mat4x3"},
    [gl_Type_sampler1D] =
        {
            .name = "sampler1D",
            .target = GL_TEXTURE_1D,
        },
    [gl_Type_image1D] =
        {
            .name = "image1D",
            .target = GL_TEXTURE_1D,
        },
    [gl_Type_sampler1DShadow] =
        {
            .name = "sampler1DShadow",
            .target = GL_TEXTURE_1D_ARRAY,
        },
    [gl_Type_sampler1DArray] =
        {
            .name = "sampler1DArray",
            .target = GL_TEXTURE_1D_ARRAY,
        },
    [gl_Type_sampler1DArrayShadow] =
        {
            .name = "sampler1DArrayShadow",
            .target = GL_TEXTURE_1D_ARRAY,
        },
    [gl_Type_sampler2D] =
        {
            .name = "sampler2D",
            .target = GL_TEXTURE_2D,
        },
};

struct TemporaryBuffer {
  GLuint buffer;
  GLenum type;
  GLsizei size;
  GLsizei offset;
  void *mapped_memory;
};

static struct {
  uint32_t num_temporary_buffers;
  struct TemporaryBuffer *temporary_buffers;

  char *script_builder_ptr;
  uint32_t script_builder_cap;
  uint32_t script_builder_len;

  uint32_t draw_index;
  uint32_t emit_index;
} _ = {0, 0, 0, 0};

static void script_builder_init(void) {
  _.script_builder_len = 0;
  _.emit_index++;
}

static void script_builder_add(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  uint32_t len = format_count(format, ap);
  va_end(ap);
  if(_.script_builder_len + len + 1 > _.script_builder_cap) {
    uint32_t old_cap = _.script_builder_cap;
    _.script_builder_cap = _.script_builder_len + len + 2;
    _.script_builder_cap += _.script_builder_cap >> 1;
    _.script_builder_ptr = memory_realloc(_.script_builder_ptr, sizeof(*_.script_builder_ptr) * old_cap, sizeof(*_.script_builder_ptr) * _.script_builder_cap, 1);
  }
  va_start(ap, format);
  format_string(_.script_builder_ptr + _.script_builder_len, _.script_builder_cap - _.script_builder_len, format, ap);
  _.script_builder_len += len;
  _.script_builder_ptr[_.script_builder_len] = 0;
  va_end(ap);
}

static void script_builder_add_shader(const struct gl_ShaderSnippet *shader);

static void script_builder_add_shader_requisites(const struct gl_ShaderSnippet *shader) {
  for(uint32_t i = 0; i < 8; i++) {
    if(shader->requires[i] == NULL)
      break;
    script_builder_add_shader(shader->requires[i]);
  }
}

static void script_builder_add_shader(const struct gl_ShaderSnippet *shader) {
  if(shader->emit_index == _.emit_index)
    return;
  script_builder_add_shader_requisites(shader);
  script_builder_add("%s\n", shader->code);
  *(uint32_t *)(&shader->emit_index) = _.emit_index;
}

static void script_builder_add_vertex_format(const struct gl_DrawState *draw_state) {
  for(int i = 0; i < GL_MAX_ATTRIBUTES; i++) {
    if(draw_state->attribute[i].format == 0)
      break;

    script_builder_add("layout(location=%i) in ", i);
    const char *type_name, *vec_name;
    switch(draw_state->attribute[i].format) {
    case memory_Format_Uint8:
    case memory_Format_Uint16:
    case memory_Format_Uint32:
    case memory_Format_Uint64:
      type_name = "uint";
      vec_name = "uvec";
      break;
    case memory_Format_Sint8:
    case memory_Format_Sint16:
    case memory_Format_Sint32:
    case memory_Format_Sint64:
      type_name = "int";
      vec_name = "ivec";
      break;
    case memory_Format_Unorm8:
    case memory_Format_Unorm16:
    case memory_Format_Snorm8:
    case memory_Format_Snorm16:
    case memory_Format_Uscaled8:
    case memory_Format_Uscaled16:
    case memory_Format_Sscaled8:
    case memory_Format_Sscaled16:
    case memory_Format_Urgb8:
    case memory_Format_Float32:
      type_name = "float";
      vec_name = "vec";
      break;
    case memory_Format_Float16:
      type_name = "half_float";
      vec_name = "hvec";
      break;
    case memory_Format_Float64:
      type_name = "double";
      vec_name = "dvec";
      break;
    default:
      ERROR(SOURCE_NAMESPACE, "invalid memory format");
      return;
    }
    if(draw_state->attribute[i].size > 1) {
      script_builder_add("%s%i in_%s;\n", vec_name, draw_state->attribute[i].size, draw_state->attribute[i].name);
    } else {
      script_builder_add("%s in_%s;\n", type_name, draw_state->attribute[i].name);
    }
  }
}

static void script_builder_add_uniform_format(const struct gl_PipelineState *pipeline_state, GLbitfield stage_bit) {
  GLuint uniform_location = 0;
  GLuint shader_storage_buffer_binding = 0;

  GLuint location, binding;

  for(uint32_t i = 0; i < GL_MAX_UNIFORMS; i++) {
    if((stage_bit && !pipeline_state->uniform[i].stage_bits) || pipeline_state->uniform[i].type == gl_Type_unused)
      continue;
    GLuint location = uniform_location++;
    if((pipeline_state->uniform[i].stage_bits & stage_bit) != stage_bit)
      continue;
    script_builder_add("layout(location=%i) uniform %s u_%s;\n", location,
                       type_info[pipeline_state->uniform[i].type].name, pipeline_state->uniform[i].name);
  }

  for(uint32_t i = 0; i < GL_MAX_UNIFORMS; i++) {
    if((stage_bit && !pipeline_state->global[i].stage_bits) || pipeline_state->global[i].resource == NULL)
      continue;
    uint32_t count = max(pipeline_state->global[i].resource->count, 1);
    if(pipeline_state->global[i].resource->type == gl_Type_shaderStorageBuffer) {
      binding = shader_storage_buffer_binding;
      shader_storage_buffer_binding += count;
    } else {
      location = uniform_location++;
    }
    if((pipeline_state->global[i].stage_bits & stage_bit) != stage_bit)
      continue;
    if(pipeline_state->global[i].resource->type == gl_Type_shaderStorageBuffer) {
      script_builder_add_shader_requisites(pipeline_state->global[i].resource->block.snippet);

      script_builder_add("layout(std430, binding=%i) %s u_%s", binding,
                         pipeline_state->global[i].resource->block.snippet->code,
                         pipeline_state->global[i].resource->name);
    } else {
      script_builder_add("layout(location=%i) uniform %s u_%s", location,
                         type_info[pipeline_state->global[i].resource->type].name,
                         pipeline_state->global[i].resource->name);
    }
    if(count > 1) {
      script_builder_add("[%i];\n", count);
    } else {
      script_builder_add(";\n");
    }
  }
}

static void script_builder_add_images_format(const struct gl_PipelineState *pipeline_state, GLbitfield stage_bit) {
  for(uint32_t i = 0; i < GL_MAX_IMAGES; i++) {
    if(!(pipeline_state->image[i].stage_bits & stage_bit))
      continue;
    script_builder_add("layout(binding=%i) uniform %s u_%s;\n", i, type_info[pipeline_state->image[i].type].name,
                       pipeline_state->image[i].name);
  }
}

static void _gl_compileShader(GLuint shader) {
  glCompileShader(shader);

  GLint status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if(status != GL_TRUE) {
    GLint length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

    GLchar *info_log = memory_alloc(length + 1, 1);
    glGetShaderInfoLog(shader, length + 1, NULL, info_log);

    ERROR(SOURCE_NAMESPACE, "shader compile error: %s", info_log);

    memory_free(info_log, length + 1, 1);
  }
}

static void _gl_linkProgram(GLuint program) {
  glLinkProgram(program);
  GLint status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if(status != GL_TRUE) {
    GLint length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

    GLchar *info_log = memory_alloc(length + 1, 1);
    glGetProgramInfoLog(program, length + 1, NULL, info_log);

    ERROR(SOURCE_NAMESPACE, "program link error: %s", info_log);

    memory_free(info_log, length + 1, 1);
  }
}

void gl_initializeDrawState(const struct gl_DrawState *state) {
  if(state->vertex_shader != NULL && state->vertex_shader_object == 0) {
    script_builder_init();
    script_builder_add("%s", shader_prelude);
    script_builder_add_vertex_format(state);
    script_builder_add_uniform_format((const struct gl_PipelineState *)state, GL_VERTEX_BIT);
    script_builder_add_images_format((const struct gl_PipelineState *)state, GL_VERTEX_BIT);

    script_builder_add_shader_requisites((const struct gl_ShaderSnippet *)state->vertex_shader);
    script_builder_add("%s", state->vertex_shader->code);

    GLuint shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader, 1, (const char *const *)&_.script_builder_ptr, (const GLint *)&_.script_builder_len);
    _gl_compileShader(shader);
    *(GLuint *)(&state->vertex_shader_object) = shader;
  }

  if(state->fragment_shader != NULL && state->fragment_shader_object == 0) {
    script_builder_init();
    script_builder_add("%s", shader_prelude);
    script_builder_add_uniform_format((const struct gl_PipelineState *)state, GL_FRAGMENT_BIT);
    script_builder_add_images_format((const struct gl_PipelineState *)state, GL_FRAGMENT_BIT);

    script_builder_add_shader_requisites((const struct gl_ShaderSnippet *)state->fragment_shader);
    script_builder_add("%s", state->fragment_shader->code);

    GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shader, 1, (const char *const *)&_.script_builder_ptr, (const GLint *)&_.script_builder_len);
    _gl_compileShader(shader);
    *(GLuint *)(&state->fragment_shader_object) = shader;
  }

  if(state->vertex_shader_object != 0 && state->fragment_shader_object != 0 && state->program_object == 0) {
    GLuint program = glCreateProgram();
    glAttachShader(program, state->vertex_shader_object);
    glAttachShader(program, state->fragment_shader_object);
    _gl_linkProgram(program);
    *(GLuint *)(&state->program_object) = program;
  }

  if(state->attribute[0].format != 0 && state->vertex_array_object == 0) {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    for(int i = 0; i < GL_MAX_ATTRIBUTES; i++) {
      if(state->attribute[i].format == 0)
        break;
      glEnableVertexArrayAttrib(vao, i);
      switch(state->attribute[i].format) {
      case memory_Format_Uint8:
        glVertexArrayAttribIFormat(vao, i, state->attribute[i].size, GL_UNSIGNED_BYTE, state->attribute[i].offset);
        break;
      case memory_Format_Uint16:
        glVertexArrayAttribIFormat(vao, i, state->attribute[i].size, GL_UNSIGNED_SHORT, state->attribute[i].offset);
        break;
      case memory_Format_Uint32:
        glVertexArrayAttribIFormat(vao, i, state->attribute[i].size, GL_UNSIGNED_INT, state->attribute[i].offset);
        break;
      case memory_Format_Uint64:
        //
        break;
      case memory_Format_Sint8:
        glVertexArrayAttribIFormat(vao, i, state->attribute[i].size, GL_BYTE, state->attribute[i].offset);
        break;
      case memory_Format_Sint16:
        glVertexArrayAttribIFormat(vao, i, state->attribute[i].size, GL_SHORT, state->attribute[i].offset);
        break;
      case memory_Format_Sint32:
        glVertexArrayAttribIFormat(vao, i, state->attribute[i].size, GL_INT, state->attribute[i].offset);
        break;
      case memory_Format_Sint64:
        //
        break;
      case memory_Format_Unorm8:
        glVertexArrayAttribFormat(vao, i, state->attribute[i].size, GL_UNSIGNED_BYTE, GL_TRUE,
                                  state->attribute[i].offset);
        break;
      case memory_Format_Unorm16:
        glVertexArrayAttribFormat(vao, i, state->attribute[i].size, GL_UNSIGNED_SHORT, GL_TRUE,
                                  state->attribute[i].offset);
        break;
      case memory_Format_Snorm8:
        glVertexArrayAttribFormat(vao, i, state->attribute[i].size, GL_BYTE, GL_TRUE, state->attribute[i].offset);
        break;
      case memory_Format_Snorm16:
        glVertexArrayAttribFormat(vao, i, state->attribute[i].size, GL_SHORT, GL_TRUE, state->attribute[i].offset);
        break;
      case memory_Format_Uscaled8:
        glVertexArrayAttribFormat(vao, i, state->attribute[i].size, GL_UNSIGNED_BYTE, GL_FALSE,
                                  state->attribute[i].offset);
        break;
      case memory_Format_Uscaled16:
        glVertexArrayAttribFormat(vao, i, state->attribute[i].size, GL_UNSIGNED_SHORT, GL_FALSE,
                                  state->attribute[i].offset);
        break;
      case memory_Format_Sscaled8:
        glVertexArrayAttribFormat(vao, i, state->attribute[i].size, GL_BYTE, GL_FALSE, state->attribute[i].offset);
        break;
      case memory_Format_Sscaled16:
        glVertexArrayAttribFormat(vao, i, state->attribute[i].size, GL_SHORT, GL_FALSE, state->attribute[i].offset);
        break;
      case memory_Format_Urgb8:
        //
        break;
      case memory_Format_Float16:
        glVertexArrayAttribFormat(vao, i, state->attribute[i].size, GL_HALF_FLOAT, GL_FALSE,
                                  state->attribute[i].offset);
        break;
      case memory_Format_Float32:
        glVertexArrayAttribFormat(vao, i, state->attribute[i].size, GL_FLOAT, GL_FALSE, state->attribute[i].offset);
        break;
      case memory_Format_Float64:
        glVertexArrayAttribLFormat(vao, i, state->attribute[i].size, GL_DOUBLE, state->attribute[i].offset);
        break;
      default:
        ERROR(SOURCE_NAMESPACE, "invalid alias memory format");
        break;
      }
      glVertexArrayAttribBinding(vao, i, state->attribute[i].binding);
    }
    *(GLuint *)(&state->vertex_array_object) = vao;
  }
}

static GLbitfield apply_pipeline_state(const struct gl_PipelineState *state) {
  static struct gl_PipelineState current_pipeline_state;

  if(current_pipeline_state.program_object != state->program_object) {
    glUseProgram(state->program_object);
    current_pipeline_state.program_object = state->program_object;
  }

  return 0;
}

GLbitfield gl_applyDrawState(const struct gl_DrawState *state) {
  static struct gl_DrawState current_draw_state;

  GLbitfield barriers = apply_pipeline_state((const struct gl_PipelineState *)state);

  if(current_draw_state.vertex_array_object != state->vertex_array_object) {
    glBindVertexArray(state->vertex_array_object);
    current_draw_state.vertex_array_object = state->vertex_array_object;
  }

  if(current_draw_state.depth_test_enable != state->depth_test_enable) {
    if(state->depth_test_enable) {
      glEnable(GL_DEPTH_TEST);
      current_draw_state.depth_test_enable = true;
    } else {
      glDisable(GL_DEPTH_TEST);
      current_draw_state.depth_test_enable = false;
    }
  }

  if(current_draw_state.depth_mask != state->depth_mask) {
    if(state->depth_mask) {
      glDepthMask(GL_TRUE);
      current_draw_state.depth_mask = true;
    } else {
      glDepthMask(GL_FALSE);
      current_draw_state.depth_mask = false;
    }
  }

  if(current_draw_state.depth_range_min != state->depth_range_min ||
     current_draw_state.depth_range_max != state->depth_range_max) {
    glDepthRange(state->depth_range_min, state->depth_range_max);
    current_draw_state.depth_range_min = state->depth_range_min;
    current_draw_state.depth_range_max = state->depth_range_max;
  }

  if(state->blend_enable) {
    if(!current_draw_state.blend_enable || current_draw_state.blend_src_factor != state->blend_src_factor ||
       current_draw_state.blend_dst_factor != state->blend_dst_factor) {
      glEnable(GL_BLEND);
      glBlendFunc(state->blend_src_factor, state->blend_dst_factor);
      current_draw_state.blend_enable = true;
      current_draw_state.blend_src_factor = state->blend_src_factor;
      current_draw_state.blend_dst_factor = state->blend_dst_factor;
    }
  } else if(current_draw_state.blend_enable) {
    glDisable(GL_BLEND);
    current_draw_state.blend_enable = false;
  }

  return barriers;
}

static inline void gl_applyUniform(GLuint location, enum gl_Type type, GLsizei count,
                                    const struct gl_UniformData *data) {
  count = count || 1;
  if(data->pointer) {
    switch(type) {
    case gl_Type_unused:
      break;
    case gl_Type_float:
      glUniform1fv(location, count, data->pointer);
      break;
    case gl_Type_float2:
      glUniform2fv(location, count, data->pointer);
      break;
    case gl_Type_float3:
      glUniform3fv(location, count, data->pointer);
      break;
    case gl_Type_float4:
      glUniform4fv(location, count, data->pointer);
      break;
    case gl_Type_int:
      glUniform1iv(location, count, data->pointer);
      break;
    case gl_Type_int2:
      glUniform2iv(location, count, data->pointer);
      break;
    case gl_Type_int3:
      glUniform3iv(location, count, data->pointer);
      break;
    case gl_Type_int4:
      glUniform4iv(location, count, data->pointer);
      break;
    case gl_Type_uint:
      glUniform1uiv(location, count, data->pointer);
      break;
    case gl_Type_uint2:
      glUniform2uiv(location, count, data->pointer);
      break;
    case gl_Type_uint3:
      glUniform3uiv(location, count, data->pointer);
      break;
    case gl_Type_uint4:
      glUniform4uiv(location, count, data->pointer);
      break;
    case gl_Type_float2x2:
      glUniformMatrix2fv(location, count, data->transpose, data->pointer);
      break;
    case gl_Type_float3x3:
      glUniformMatrix3fv(location, count, data->transpose, data->pointer);
      break;
    case gl_Type_float4x4:
      glUniformMatrix4fv(location, count, data->transpose, data->pointer);
      break;
    case gl_Type_float2x3:
      glUniformMatrix2x3fv(location, count, data->transpose, data->pointer);
      break;
    case gl_Type_float3x2:
      glUniformMatrix3x2fv(location, count, data->transpose, data->pointer);
      break;
    case gl_Type_float2x4:
      glUniformMatrix2x4fv(location, count, data->transpose, data->pointer);
      break;
    case gl_Type_float4x2:
      glUniformMatrix4x2fv(location, count, data->transpose, data->pointer);
      break;
    case gl_Type_float3x4:
      glUniformMatrix3x4fv(location, count, data->transpose, data->pointer);
      break;
    case gl_Type_float4x3:
      glUniformMatrix4x3fv(location, count, data->transpose, data->pointer);
      break;
    default:
      ERROR(SOURCE_NAMESPACE, "invalid OpenGL data type");
    }
  } else {
    switch(type) {
    case gl_Type_unused:
      break;
    case gl_Type_float:
      glUniform1f(location, data->_float);
      break;
    case gl_Type_float2:
      glUniform2f(location, data->vec[0], data->vec[1]);
      break;
    case gl_Type_float3:
      glUniform3f(location, data->vec[0], data->vec[1], data->vec[2]);
      break;
    case gl_Type_float4:
      glUniform4f(location, data->vec[0], data->vec[1], data->vec[2], data->vec[3]);
      break;
    case gl_Type_int:
      glUniform1i(location, data->_int);
      break;
    case gl_Type_int2:
      glUniform2i(location, data->ivec[0], data->ivec[1]);
      break;
    case gl_Type_int3:
      glUniform3i(location, data->ivec[0], data->ivec[1], data->ivec[2]);
      break;
    case gl_Type_int4:
      glUniform4i(location, data->ivec[0], data->ivec[1], data->ivec[2], data->ivec[3]);
      break;
    case gl_Type_uint:
      glUniform1ui(location, data->uint);
      break;
    case gl_Type_uint2:
      glUniform2ui(location, data->uvec[0], data->uvec[1]);
      break;
    case gl_Type_uint3:
      glUniform3ui(location, data->uvec[0], data->uvec[1], data->uvec[2]);
      break;
    case gl_Type_uint4:
      glUniform4ui(location, data->uvec[0], data->uvec[1], data->uvec[2], data->uvec[3]);
      break;
    case gl_Type_float2x2:
      glUniformMatrix2fv(location, count, data->transpose, data->mat);
      break;
    case gl_Type_float3x3:
      glUniformMatrix3fv(location, count, data->transpose, data->mat);
      break;
    case gl_Type_float4x4:
      glUniformMatrix4fv(location, count, data->transpose, data->mat);
      break;
    case gl_Type_float2x3:
      glUniformMatrix2x3fv(location, count, data->transpose, data->mat);
      break;
    case gl_Type_float3x2:
      glUniformMatrix3x2fv(location, count, data->transpose, data->mat);
      break;
    case gl_Type_float2x4:
      glUniformMatrix2x4fv(location, count, data->transpose, data->mat);
      break;
    case gl_Type_float4x2:
      glUniformMatrix4x2fv(location, count, data->transpose, data->mat);
      break;
    case gl_Type_float3x4:
      glUniformMatrix3x4fv(location, count, data->transpose, data->mat);
      break;
    case gl_Type_float4x3:
      glUniformMatrix4x3fv(location, count, data->transpose, data->mat);
      break;
    default:
      ERROR(SOURCE_NAMESPACE, "invalid OpenGL data type");
    }
  }
}

static inline GLbitfield apply_pipeline_assets(const struct gl_PipelineState *state,
                                               const struct gl_PipelineAssets *assets, bool compute) {
  static struct gl_PipelineAssets current_assets;

  GLuint uniform_location = 0;
  GLuint shader_storage_buffer_binding = 0;
  GLuint location, binding;

  GLbitfield barriers = 0;

  for(uint32_t i = 0; i < GL_MAX_IMAGES; i++) {
    if(assets != NULL && assets->image[i]) {
      if(current_assets.image[i] != assets->image[i]) {
        if(type_info[state->image[i].type].target == GL_SAMPLER) {
          glBindSampler(i, assets->image[i]);
        } else {
          glActiveTexture(GL_TEXTURE0 + i);
          glBindTexture(type_info[state->image[i].type].target, assets->image[i]);
        }
      } else {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, assets->image[i]);
      }
      current_assets.image[i] = assets->image[i];
    }
  }

  for(uint32_t i = 0; i < GL_MAX_UNIFORMS; i++) {
    if((!compute && !state->uniform[i].stage_bits) || state->uniform[i].type == gl_Type_unused)
      continue;
    GLuint location = uniform_location++;
    if(assets != NULL)
      gl_applyUniform(location, state->uniform[i].type, state->uniform[i].count, &assets->uniforms[i]);
  }

  for(uint32_t i = 0; i < GL_MAX_UNIFORMS; i++) {
    if((!compute && !state->global[i].stage_bits) || state->global[i].resource == NULL)
      continue;
    uint32_t count = max(state->global[i].resource->count, 1);
    if(state->global[i].resource->type == gl_Type_shaderStorageBuffer) {
      binding = shader_storage_buffer_binding;
      shader_storage_buffer_binding += count;
    } else {
      location = uniform_location++;
    }
    if(state->global[i].resource->type == gl_Type_shaderStorageBuffer) {
      if(count > 1) {
        for(uint32_t j = 0; j < count; j++) {
          barriers |= gl_flushBuffer(state->global[i].resource->block.buffers[j], GL_SHADER_STORAGE_BARRIER_BIT);
          glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding + j, state->global[i].resource->block.buffers[j]->buffer);
        }
      } else {
        barriers |= gl_flushBuffer(state->global[i].resource->block.buffer, GL_SHADER_STORAGE_BARRIER_BIT);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, state->global[i].resource->block.buffer->buffer);
      }
    } else {
      gl_ShaderResource_prepare(state->global[i].resource);
      gl_applyUniform(location, state->global[i].resource->type, state->global[i].resource->count,
                       &state->global[i].resource->uniform.data);
    }
  }

  return barriers;
}

GLbitfield gl_applyDrawAssets(const struct gl_DrawState *state, const struct gl_DrawAssets *assets) {
  //static struct gl_DrawAssets current_draw_assets;

  GLbitfield barriers =
      apply_pipeline_assets((const struct gl_PipelineState *)state, (const struct gl_PipelineAssets *)assets, false);

  if(assets != NULL) {
    if(assets->element_buffer != NULL) {
      barriers |= gl_flushBuffer(assets->element_buffer, GL_ELEMENT_ARRAY_BARRIER_BIT);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, assets->element_buffer->buffer);
    }

    for(int i = 0; i < GL_MAX_BINDINGS; i++) {
      if(assets->vertex_buffers[i] == NULL)
        break;
      barriers |= gl_flushBuffer(assets->vertex_buffers[i], GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
      if(assets->vertex_buffers[i]->kind == gl_Buffer_temporary) {
        glBindVertexBuffer(i, assets->vertex_buffers[i]->buffer, assets->vertex_buffers[i]->offset,
                           state->binding[i].stride);
      } else {
        glBindVertexBuffer(i, assets->vertex_buffers[i]->buffer, 0, state->binding[i].stride);
      }
    }
  }

  return barriers;
}

static void apply_barriers(GLbitfield barriers) {
  if(barriers)
    glMemoryBarrier(barriers);
}

void gl_drawArrays(const struct gl_DrawState *state, const struct gl_DrawAssets *assets, int32_t first, ssize_t count,
                    ssize_t instancecount, uint32_t baseinstance) {
  gl_initializeDrawState(state);
  GLbitfield barriers = gl_applyDrawState(state);
  barriers |= gl_applyDrawAssets(state, assets);

  apply_barriers(barriers);

  glDrawArraysInstancedBaseInstance(state->primitive, first, count, instancecount, baseinstance);

  _.draw_index++;
}

void gl_drawElements(const struct gl_DrawState *state, const struct gl_DrawAssets *assets, ssize_t count,
                      ssize_t instancecount, int32_t basevertex, uint32_t baseinstance) {
  gl_initializeDrawState(state);
  GLbitfield barriers = gl_applyDrawState(state);
  barriers |= gl_applyDrawAssets(state, assets);

  apply_barriers(barriers);

  glDrawElementsInstancedBaseVertexBaseInstance(
      state->primitive, count, GL_UNSIGNED_INT,
      (void *)((assets->element_buffer->kind == gl_Buffer_temporary ? assets->element_buffer->offset : 0) +
          sizeof(uint32_t) * assets->element_buffer_offset),
      instancecount, basevertex, baseinstance);

  _.draw_index++;
}

void gl_drawElementsIndirect(const struct gl_DrawState *state, const struct gl_DrawAssets *assets,
                               const struct gl_Buffer *indirect, ssize_t indirect_offset) {
  gl_initializeDrawState(state);
  GLbitfield barriers = gl_applyDrawState(state);
  barriers |= gl_applyDrawAssets(state, assets);

  barriers |= gl_flushBuffer(indirect, GL_COMMAND_BARRIER_BIT);

  apply_barriers(barriers);

  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect->buffer);
  glDrawElementsIndirect(state->primitive, GL_UNSIGNED_INT, (const void *)(uintptr_t)indirect_offset);

  _.draw_index++;
}

struct gl_Buffer gl_allocateStaticBuffer(uint32_t type, ssize_t size, const void *data) {
  struct gl_Buffer result;
  result.kind = gl_Buffer_static;
  glCreateBuffers(1, &result.buffer);
  glNamedBufferStorage(result.buffer, size, data, 0);
  result.size = size;
  return result;
}

struct gl_Buffer gl_allocateTemporaryBuffer(uint32_t type, ssize_t size) {
again:
  for(uint32_t i = 0; i < _.num_temporary_buffers; i++) {
    if(_.temporary_buffers[i].type == type && _.temporary_buffers[i].size - _.temporary_buffers[i].offset > size) {
      struct gl_Buffer result;
      result.kind = gl_Buffer_temporary;
      result.buffer = _.temporary_buffers[i].buffer;
      result.size = size;
      result.mapping = (void *)((GLbyte *)_.temporary_buffers[i].mapped_memory + _.temporary_buffers[i].offset);
      result.offset = _.temporary_buffers[i].offset;
      result.dirty = true;
      _.temporary_buffers[i].offset += size;
      return result;
    }
  }

  // 16 << 20); // 16mb TODO make a cvar
  GLsizei block_size = 4 << 20;
  GLsizei allocation_size = ((size + block_size) / block_size) * block_size;

  _.temporary_buffers = memory_realloc(_.temporary_buffers, sizeof(*_.temporary_buffers) * _.num_temporary_buffers, sizeof(*_.temporary_buffers) * (_.num_temporary_buffers + 1), 4);
  glCreateBuffers(1, &_.temporary_buffers[_.num_temporary_buffers].buffer);
  glNamedBufferStorage(_.temporary_buffers[_.num_temporary_buffers].buffer, allocation_size, NULL,
                       GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT);
  _.temporary_buffers[_.num_temporary_buffers].type = type;
  _.temporary_buffers[_.num_temporary_buffers].size = allocation_size;
  _.temporary_buffers[_.num_temporary_buffers].offset = 0;
  _.temporary_buffers[_.num_temporary_buffers].mapped_memory =
      glMapNamedBufferRange(_.temporary_buffers[_.num_temporary_buffers].buffer, 0, allocation_size,
                            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
  _.num_temporary_buffers++;

  goto again;
}

struct gl_Buffer gl_allocateTemporaryBufferFrom(uint32_t type, ssize_t size, const void *ptr) {
  struct gl_Buffer result = gl_allocateTemporaryBuffer(type, size);
  memory_copy(result.mapping, size, ptr, size);
  return result;
}

static void activate_buffer(const struct gl_Buffer *buffer) {
  if(buffer->buffer != 0)
    return;
  switch(buffer->kind) {
  case gl_Buffer_static:
    break;
  case gl_Buffer_temporary:
    break;
  case gl_Buffer_cpu:
    glCreateBuffers(1, (GLuint *)&buffer->buffer);
    glNamedBufferStorage(buffer->buffer, buffer->size, NULL, GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT);
    *(void **)&buffer->mapping = glMapNamedBufferRange(
        buffer->buffer, 0, buffer->size, GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
    break;
  case gl_Buffer_gpu:
    glCreateBuffers(1, (GLuint *)&buffer->buffer);
    glNamedBufferStorage(buffer->buffer, buffer->size, NULL, 0);
    break;
  }
}

void *gl_updateBufferBegin(const struct gl_Buffer *buffer, intptr_t offset, ssize_t size) {
  (void)offset;
  (void)size;
  if(buffer->kind == gl_Buffer_cpu) {
    activate_buffer(buffer);
    return buffer->mapping;
  }
  return NULL;
}

void gl_updateBufferEnd(const struct gl_Buffer *buffer, intptr_t offset, ssize_t size) {
  if(buffer->kind == gl_Buffer_cpu) {
    glFlushMappedNamedBufferRange(buffer->buffer, offset, size);
    *(bool *)&buffer->dirty = true;
  }
}

GLbitfield gl_flushBuffer(const struct gl_Buffer *buffer, GLbitfield read_barrier_bits) {
  switch(buffer->kind) {
  case gl_Buffer_static:
    break;
  case gl_Buffer_temporary:
    if(buffer->dirty) {
      glFlushMappedNamedBufferRange(buffer->buffer, buffer->offset, buffer->size);
      *(bool *)&buffer->dirty = false;
      return GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT;
    }
    break;
  case gl_Buffer_cpu:
    activate_buffer(buffer);
    if(buffer->dirty) {
      *(bool *)&buffer->dirty = false;
      return GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT;
    }
    break;
  case gl_Buffer_gpu:
    activate_buffer(buffer);
    if(buffer->dirty) {
      *(bool *)&buffer->dirty = false;
      return read_barrier_bits;
    }
    break;
  }

  return 0;
}

void gl_freeBuffer(const struct gl_Buffer *buffer) {
  switch(buffer->kind) {
  case gl_Buffer_static:
    glDeleteBuffers(1, &buffer->buffer);
    break;
  default:
    break;
  }
}

void gl_resetTemporaryBuffers(void) {
  for(uint32_t i = 0; i < _.num_temporary_buffers; i++) {
    _.temporary_buffers[i].offset = 0;
  }
}

void gl_temporaryBufferStats(GLenum type, uint32_t *total_allocated, uint32_t *used) {
  *total_allocated = 0;
  *used = 0;

  for(uint32_t i = 0; i < _.num_temporary_buffers; i++) {
    if(_.temporary_buffers[i].type == type) {
      *total_allocated += _.temporary_buffers[i].size;
      *used += _.temporary_buffers[i].offset;
    }
  }
}

void gl_ShaderResource_prepare(const struct gl_ShaderResource *resource) {
  if(resource->uniform.prepare_draw_index != _.draw_index) {
    *(uint32_t *)&resource->uniform.prepare_draw_index = _.draw_index;
    if(resource->uniform.prepare != NULL) {
      resource->uniform.prepare();
    }
  }
}

#ifndef GL_SUBGROUP_SIZE
#define GL_SUBGROUP_SIZE 0x9532
#endif

void gl_initializeComputeState(const struct gl_ComputeState *state) {
  if(state->shader != NULL && state->shader_object == 0) {
    script_builder_init();
    script_builder_add("%s", shader_prelude);

    if(state->local_group_x == GL_LOCAL_GROUP_SIZE_SUBGROUP_SIZE) {
      glGetIntegerv(GL_SUBGROUP_SIZE, (GLint *)&state->local_group_x);
    }
    if(state->local_group_y == GL_LOCAL_GROUP_SIZE_SUBGROUP_SIZE) {
      glGetIntegerv(GL_SUBGROUP_SIZE, (GLint *)&state->local_group_y);
    }
    if(state->local_group_z == GL_LOCAL_GROUP_SIZE_SUBGROUP_SIZE) {
      glGetIntegerv(GL_SUBGROUP_SIZE, (GLint *)&state->local_group_z);
    }

    script_builder_add("layout(local_size_x=%i, local_size_y=%i, local_size_z=%i) in;\n",
                       max(state->local_group_x, 1), max(state->local_group_y, 1),
                       max(state->local_group_z, 1));
    script_builder_add_uniform_format((const struct gl_PipelineState *)state, 0);
    script_builder_add_images_format((const struct gl_PipelineState *)state, 0);
    script_builder_add_shader_requisites((const struct gl_ShaderSnippet *)state->shader);
    script_builder_add("%s", state->shader->code);

    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(shader, 1, (const char *const *)&_.script_builder_ptr, (const GLint *)&_.script_builder_len);
    _gl_compileShader(shader);
    *(GLuint *)(&state->shader_object) = shader;
  }

  if(state->shader_object != 0 && state->program_object == 0) {
    GLuint program = glCreateProgram();
    glAttachShader(program, state->shader_object);
    _gl_linkProgram(program);
    *(GLuint *)(&state->program_object) = program;
  }
}

GLbitfield gl_applyComputeState(const struct gl_ComputeState *state) {
  return apply_pipeline_state((const struct gl_PipelineState *)state);
}

GLbitfield gl_applyComputeAssets(const struct gl_ComputeState *state, const struct gl_ComputeAssets *assets) {
  //static struct gl_ComputeAssets current_compute_assets;

  GLbitfield barriers =
      apply_pipeline_assets((const struct gl_PipelineState *)state, (const struct gl_PipelineAssets *)assets, true);

  if(assets != NULL) {
  }

  return barriers;
}

void gl_compute(const struct gl_ComputeState *state, const struct gl_ComputeAssets *assets, uint32_t num_groups_x,
                uint32_t num_groups_y, uint32_t num_groups_z) {
  gl_initializeComputeState(state);
  GLbitfield barriers = gl_applyComputeState(state);
  barriers |= gl_applyComputeAssets(state, assets);

  apply_barriers(barriers);

  glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
}

void gl_computeIndirect(const struct gl_ComputeState *state, const struct gl_ComputeAssets *assets,
                         const struct gl_Buffer *indirect, ssize_t indirect_offset) {
  gl_initializeComputeState(state);
  GLbitfield barriers = gl_applyComputeState(state);
  barriers |= gl_applyComputeAssets(state, assets);

  barriers |= gl_flushBuffer(indirect, GL_COMMAND_BARRIER_BIT);

  apply_barriers(barriers);

  glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, indirect->buffer);
  glDispatchComputeIndirect(indirect_offset);
}

