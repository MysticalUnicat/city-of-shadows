#ifndef gl_h_INCLUDED
#define gl_h_INCLUDED

#include "memory.h"
#include "cpp.h"

#include "glad.h"

#include <stdlib.h> // for ssize_t

#define GL_MAX_ATTRIBUTES 16
#define GL_MAX_BINDINGS 2
#define GL_MAX_UNIFORMS 16
#define GL_MAX_IMAGES 16
#define GL_MAX_BUFFERS 16

#define GL_VERTEX_BIT 0x01
#define GL_GEOMETRY_BIT 0x02
#define GL_TESSC_BIT 0x04
#define GL_TESSE_BIT 0x08
#define GL_FRAGMENT_BIT 0x10

#define GL_LOCAL_GROUP_SIZE_SUBGROUP_SIZE 0xFFFFFFFF

enum gl_Type {
  gl_Type_unused,
  gl_Type_float,
  gl_Type_float2,
  gl_Type_float3,
  gl_Type_float4,
  gl_Type_double,
  gl_Type_double2,
  gl_Type_double3,
  gl_Type_double4,
  gl_Type_int,
  gl_Type_int2,
  gl_Type_int3,
  gl_Type_int4,
  gl_Type_uint,
  gl_Type_uint2,
  gl_Type_uint3,
  gl_Type_uint4,
  gl_Type_bool,
  gl_Type_bool2,
  gl_Type_bool3,
  gl_Type_bool4,
  gl_Type_float2x2,
  gl_Type_float3x3,
  gl_Type_float4x4,
  gl_Type_float2x3,
  gl_Type_float2x4,
  gl_Type_float3x2,
  gl_Type_float3x4,
  gl_Type_float4x2,
  gl_Type_float4x3,
  gl_Type_double2x2,
  gl_Type_double3x3,
  gl_Type_double4x4,
  gl_Type_double2x3,
  gl_Type_double2x4,
  gl_Type_double3x2,
  gl_Type_double3x4,
  gl_Type_double4x2,
  gl_Type_double4x3,
  gl_Type_sampler1D,
  gl_Type_sampler2D,
  gl_Type_sampler3D,
  gl_Type_samplerCube,
  gl_Type_sampler1DShadow,
  gl_Type_sampler2DShadow,
  gl_Type_sampler1DArray,
  gl_Type_sampler2DArray,
  gl_Type_samplerCubeArray,
  gl_Type_sampler1DArrayShadow,
  gl_Type_sampler2DArrayShadow,
  gl_Type_sampler2DMultisample,
  gl_Type_sampler2DMultisampleArray,
  gl_Type_samplerCubeShadow,
  gl_Type_samplerCubeArrayShadow,
  gl_Type_samplerBuffer,
  gl_Type_sampler2DRect,
  gl_Type_sampler2DRectShadow,
  gl_Type_intSampler1D,
  gl_Type_intSampler2D,
  gl_Type_intSampler3D,
  gl_Type_intSamplerCube,
  gl_Type_intSampler1DArray,
  gl_Type_intSampler2DArray,
  gl_Type_intSamplerCubeMapArray,
  gl_Type_intSampler2DMultisample,
  gl_Type_intSampler2DMultisampleArray,
  gl_Type_intSamplerBuffer,
  gl_Type_intSampler2DRect,
  gl_Type_uintSampler1D,
  gl_Type_uintSampler2D,
  gl_Type_uintSampler3D,
  gl_Type_uintSamplerCube,
  gl_Type_uintSampler1DArray,
  gl_Type_uintSampler2DArray,
  gl_Type_uintSamplerCubeMapArray,
  gl_Type_uintSampler2DMultisample,
  gl_Type_uintSampler2DMultisampleArray,
  gl_Type_uintSamplerBuffer,
  gl_Type_uintSampler2DRect,
  gl_Type_image1D,
  gl_Type_image2D,
  gl_Type_image3D,
  gl_Type_image2DRect,
  gl_Type_imageCube,
  gl_Type_imageBuffer,
  gl_Type_image1DArray,
  gl_Type_image2DArray,
  gl_Type_imageCubeArray,
  gl_Type_image2DMultisample,
  gl_Type_image2DMultisampleArray,
  gl_Type_intImage1D,
  gl_Type_intImage2D,
  gl_Type_intImage3D,
  gl_Type_intImage2DRect,
  gl_Type_intImageCube,
  gl_Type_intImageBuffer,
  gl_Type_intImage1DArray,
  gl_Type_intImage2DArray,
  gl_Type_intImageCubeArray,
  gl_Type_intImage2DMultisample,
  gl_Type_intImage2DMultisampleArray,
  gl_Type_uintImage1D,
  gl_Type_uintImage2D,
  gl_Type_uintImage3D,
  gl_Type_uintImage2DRect,
  gl_Type_uintImageCube,
  gl_Type_uintImageBuffer,
  gl_Type_uintImage1DArray,
  gl_Type_uintImage2DArray,
  gl_Type_uintImageCubeArray,
  gl_Type_uintImage2DMultisample,
  gl_Type_uintImage2DMultisampleArray,
  gl_Type_atomicUint,
  gl_Type_inlineStructure,
  gl_Type_shaderStorageBuffer
};

struct gl_UniformData {
  const void *pointer;
  bool transpose;
  const struct gl_Buffer *buffer;
  union {
    float _float;
    float vec[4];
    float mat[16];

    int32_t _int;
    int32_t ivec[4];
    int32_t imat[16];

    uint32_t uint;
    uint32_t uvec[4];
    uint32_t umat[16];
  };
};

struct gl_ShaderSnippet {
  const struct gl_ShaderSnippet *
    requires[8];
  const char *code;

  /* internal */
  uint32_t structure_size;
  uint32_t emit_index;
};

struct gl_Shader {
  const struct gl_ShaderSnippet *
    requires[8];
  const char *code;

  /* internal */
  uint32_t object;
};

struct gl_ShaderResource {
  enum gl_Type type;
  const char *name;
  ssize_t count;
  union {
    struct {
      void (*prepare)(void);
      uint32_t prepare_draw_index;
      struct gl_UniformData data;
    } uniform;
    struct {
      struct gl_ShaderSnippet *snippet;
      union {
        struct gl_Buffer *buffer;
        struct gl_Buffer *buffers[GL_MAX_BUFFERS];
      };
    } block;
  };
};

// --------------------------------------------------------------------------------------------------------------------
// state setup for draw/compute
// - interface defined in C ensures GLSL and C expect the same thing

#define GL_PIPILINE_STATE_FIELDS                                                                                 \
  struct {                                                                                                             \
    uint32_t stage_bits;                                                                                               \
    enum gl_Type type;                                                                                           \
    const char *name;                                                                                                  \
    ssize_t count;                                                                                                     \
    const struct gl_ShaderSnippet *struture;                                                                     \
  } uniform[GL_MAX_UNIFORMS];                                                                                    \
  struct {                                                                                                             \
    uint32_t stage_bits;                                                                                               \
    struct gl_ShaderResource *resource;                                                                          \
  } global[GL_MAX_UNIFORMS];                                                                                     \
  struct {                                                                                                             \
    uint32_t stage_bits;                                                                                               \
    enum gl_Type type;                                                                                           \
    const char *name;                                                                                                  \
  } image[GL_MAX_IMAGES];                                                                                        \
  /* internal */                                                                                                       \
  uint32_t program_object;

struct gl_PipelineState {
  GL_PIPILINE_STATE_FIELDS
};

#define GL_STAGE_FIELDS                                                                                          \
  const char *source;                                                                                                  \
  /* internal */                                                                                                       \
  uint32_t shader_object;

struct gl_VertexStage {
  GL_STAGE_FIELDS

  int32_t primitive;

  struct {
    int32_t binding;
    memory_Format format;
    int32_t size;
    const char *name;
    uint32_t offset;
  } attribute[GL_MAX_ATTRIBUTES];

  struct {
    ssize_t stride;
    uint32_t divisor;
  } binding[GL_MAX_BINDINGS];

  // internal
  uint32_t vertex_array_object;
};

struct gl_DrawState {
  GL_PIPILINE_STATE_FIELDS

  uint32_t primitive;

  struct {
    uint32_t binding;
    memory_Format format;
    int32_t size;
    const char *name;
    uint32_t offset;
  } attribute[GL_MAX_ATTRIBUTES];

  struct {
    ssize_t stride;
    uint32_t divisor;
  } binding[GL_MAX_BINDINGS];

  const struct gl_Shader *vertex_shader;

  bool depth_test_enable;
  bool depth_mask;
  float depth_range_min;
  float depth_range_max;

  const struct gl_Shader *fragment_shader;

  bool blend_enable;
  uint32_t blend_src_factor;
  uint32_t blend_dst_factor;

  /* internal */
  uint32_t vertex_shader_object;
  uint32_t fragment_shader_object;
  uint32_t vertex_array_object;
};

struct gl_ComputeState {
  GL_PIPILINE_STATE_FIELDS

  const struct gl_Shader *shader;

  uint32_t local_group_x;
  uint32_t local_group_y;
  uint32_t local_group_z;

  /* internal */
  uint32_t shader_object;
};

// --------------------------------------------------------------------------------------------------------------------
// asset

struct gl_Buffer {
  enum {
    gl_Buffer_static,    // never changes, lives on the GPU
    gl_Buffer_temporary, // The buffer used to send information from the CPU to GPU once
    gl_Buffer_gpu,       // only lives on the GPU
    gl_Buffer_cpu,       // A buffer persantly mapped and bound, updated by the CPU many times
  } kind;
  uint32_t buffer;
  ssize_t size;
  void *mapping;
  uint32_t offset;
  bool dirty;
};

// --------------------------------------------------------------------------------------------------------------------
// asset reference collection needed for a draw/compute

#define GL_PIPILINE_ASSETS_FIELDS                                                                                \
  uint32_t image[GL_MAX_IMAGES];                                                                                 \
  struct gl_UniformData uniforms[GL_MAX_UNIFORMS];

struct gl_PipelineAssets {
  GL_PIPILINE_ASSETS_FIELDS
};

struct gl_DrawAssets {
  GL_PIPILINE_ASSETS_FIELDS

  const struct gl_Buffer *element_buffer;
  uint32_t element_buffer_offset;

  const struct gl_Buffer *vertex_buffers[GL_MAX_BINDINGS];
};

struct gl_ComputeAssets {
  GL_PIPILINE_ASSETS_FIELDS
};

// --------------------------------------------------------------------------------------------------------------------
// draw
void gl_initializeDrawState(const struct gl_DrawState *state);
uint32_t gl_applyDrawState(const struct gl_DrawState *state);
uint32_t gl_applyDrawAssets(const struct gl_DrawState *state, const struct gl_DrawAssets *assets);

void gl_drawArrays(const struct gl_DrawState *state, const struct gl_DrawAssets *assets,
                         int32_t first, ssize_t count, ssize_t instancecount, uint32_t baseinstance);

void gl_drawElements(const struct gl_DrawState *state, const struct gl_DrawAssets *assets,
                           ssize_t count, ssize_t instancecount, int32_t basevertex, uint32_t baseinstance);

void gl_drawElementsIndirect(const struct gl_DrawState *state, const struct gl_DrawAssets *assets,
                                   const struct gl_Buffer *indirect, ssize_t indirect_offset);

// --------------------------------------------------------------------------------------------------------------------
// compute
void gl_initializeComputeState(const struct gl_ComputeState *state);
uint32_t gl_applyComputeState(const struct gl_ComputeState *state);
uint32_t gl_applyComputeAssets(const struct gl_ComputeState *state,
                                     const struct gl_ComputeAssets *assets);

void gl_compute(const struct gl_ComputeState *state, const struct gl_ComputeAssets *assets,
                      uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z);

void gl_computeIndirect(const struct gl_ComputeState *state, const struct gl_ComputeAssets *assets,
                              const struct gl_Buffer *indirect, ssize_t indirect_offset);

// --------------------------------------------------------------------------------------------------------------------
// resource data
struct gl_Buffer gl_allocateStaticBuffer(uint32_t type, ssize_t size, const void *data);

struct gl_Buffer gl_allocateTemporaryBuffer(uint32_t type, ssize_t size);

struct gl_Buffer gl_allocateTemporaryBufferFrom(uint32_t type, ssize_t size, const void *data);

void *gl_updateBufferBegin(const struct gl_Buffer *buffer, intptr_t offset, ssize_t size);
void gl_updateBufferEnd(const struct gl_Buffer *buffer, intptr_t offset, ssize_t size);

uint32_t gl_flushBuffer(const struct gl_Buffer *buffer, uint32_t read_barrier_bits);

void gl_freeBuffer(const struct gl_Buffer *buffer);

void gl_resetTemporaryBuffers(void);

void gl_temporaryBufferStats(uint32_t type, uint32_t *total_allocated, uint32_t *used);

void gl_destroyBuffer(const struct gl_Buffer *buffer);

// --------------------------------------------------------------------------------------------------------------------
// resource
void gl_ShaderResource_prepare(const struct gl_ShaderResource *resource);

// --------------------------------------------------------------------------------------------------------------------
#define GL_SNIPPET_REQUIRE_require(NAME) &gl_##NAME##_snippet,
#define GL_SNIPPET_REQUIRE_code(CODE)
#define GL_SNIPPET_REQUIRE_string(CODE)
#define GL_SNIPPET_REQUIRE_(ITEM) CPP_CAT(GL_SNIPPET_REQUIRE_, ITEM)

#define GL_SNIPPET_CODE_require(NAME)
#define GL_SNIPPET_CODE_code(CODE) #CODE
#define GL_SNIPPET_CODE_string(CODE) CODE
#define GL_SNIPPET_CODE_(ITEM) CPP_CAT(GL_SNIPPET_CODE_, ITEM)

#define GL_DECLARE_SNIPPET(NAME, ...) extern struct gl_ShaderSnippet gl_##NAME##_snippet;

#define GL_IMPL_SNIPPET(NAME, ...)                                                                                \
  struct gl_ShaderSnippet gl_##NAME##_snippet = {                                                                      \
      .requires = {CPP_EVAL(CPP_MAP(GL_SNIPPET_REQUIRE_, __VA_ARGS__))},                              \
      .code = CPP_EVAL(CPP_MAP(GL_SNIPPET_CODE_, __VA_ARGS__))};

#define GL_SNIPPET(NAME, ...) GL_IMPL_SNIPPET(NAME, __VA_ARGS__)

// --------------------------------------------------------------------------------------------------------------------
#define GL_SHADER_INSTANCE_REQUIRE_require(NAME) &gl_##NAME##_snippet,
#define GL_SHADER_INSTANCE_REQUIRE_code(CODE)
#define GL_SHADER_INSTANCE_REQUIRE_string(CODE)
#define GL_SHADER_INSTANCE_REQUIRE_main(CODE)
#define GL_SHADER_INSTANCE_REQUIRE_(ITEM) CPP_CAT(GL_SHADER_INSTANCE_REQUIRE_, ITEM)

#define GL_SHADER_INSTANCE_CODE_require(NAME)
#define GL_SHADER_INSTANCE_CODE_code(CODE) #CODE
#define GL_SHADER_INSTANCE_CODE_string(CODE) CODE
#define GL_SHADER_INSTANCE_CODE_main(CODE) "void main() {" #CODE "}"
#define GL_SHADER_INSTANCE_CODE_(ITEM) CPP_CAT(GL_SHADER_INSTANCE_CODE_, ITEM)

#define GL_SHADER_INSTANCE(NAME, ...)                                                                                      \
  static struct gl_Shader NAME##_shader = {                                                                            \
      .requires = {CPP_EVAL(CPP_MAP(GL_SHADER_INSTANCE_REQUIRE_, __VA_ARGS__))},                               \
      .code = CPP_EVAL(CPP_MAP(GL_SHADER_INSTANCE_CODE_, __VA_ARGS__))};

// --------------------------------------------------------------------------------------------------------------------
#define GL_ALIGNED(X) __attribute__((aligned(X)))

#define GL_STRUCT_TO_C_require(NAME)
#define GL_STRUCT_TO_C_unorm8(NAME) uint8_t NAME[4] GL_ALIGNED(4);     // uint (4, 4) -> float
#define GL_STRUCT_TO_C_unorm8x2(NAME) uint8_t NAME[4] GL_ALIGNED(4);   // uint (4, 4) -> vec2
#define GL_STRUCT_TO_C_unorm8x3(NAME) uint8_t NAME[4] GL_ALIGNED(4);   // uint (4, 4) -> vec3
#define GL_STRUCT_TO_C_unorm8x4(NAME) uint8_t NAME[4] GL_ALIGNED(4);   // uint (4, 4) -> vec4
#define GL_STRUCT_TO_C_unorm16(NAME) uint16_t NAME[2] GL_ALIGNED(4);   // uint (4, 4) -> float
#define GL_STRUCT_TO_C_unorm16x2(NAME) uint16_t NAME[2] GL_ALIGNED(4); // uint (4, 4) -> vec2
#define GL_STRUCT_TO_C_unorm16x3(NAME) uint16_t NAME[4] GL_ALIGNED(8); // uvec2 (8, 8) -> vec3
#define GL_STRUCT_TO_C_unorm16x4(NAME) uint16_t NAME[4] GL_ALIGNED(8); // uvec2 (8, 8) -> vec4
#define GL_STRUCT_TO_C_snorm16(NAME) int16_t NAME[2] GL_ALIGNED(4);    // uint (4, 4) -> float
#define GL_STRUCT_TO_C_snorm16x2(NAME) int16_t NAME[2] GL_ALIGNED(4);  // uint (4, 4) -> vec2
#define GL_STRUCT_TO_C_snorm16x3(NAME) int16_t NAME[4] GL_ALIGNED(8);  // uvec2 (8, 8) -> vec3
#define GL_STRUCT_TO_C_snorm16x4(NAME) int16_t NAME[4] GL_ALIGNED(8);  // uvec2 (8, 8) -> vec4
#define GL_STRUCT_TO_C_uint32(NAME) uint32_t NAME GL_ALIGNED(4);       // uint (4, 4) -> uint
#define GL_STRUCT_TO_C_int32(NAME) int32_t NAME GL_ALIGNED(4);         // int (4, 4) -> int
#define GL_STRUCT_TO_C_float32(NAME) float NAME GL_ALIGNED(4);         // float (4, 4) -> float
#define GL_STRUCT_TO_C_float32x2(NAME) float NAME[2] GL_ALIGNED(8);    // vec2 (8, 8) -> vec2
#define GL_STRUCT_TO_C_float32x3(NAME) float NAME[3] GL_ALIGNED(16);   // vec3 (16, 16) -> vec3
#define GL_STRUCT_TO_C_float32x4(NAME) float NAME[4] GL_ALIGNED(16);   // vec4 (16, 16) -> vec4
#define GL_STRUCT_TO_C_struct(TYPE, NAME) struct gl_##TYPE NAME;
#define GL_STRUCT_TO_C_unsized_array(TYPE)
#define GL_STRUCT_TO_C_(ITEM) CPP_CAT(GL_STRUCT_TO_C_, ITEM)
#define GL_STRUCT_TO_C(NAME, ...)                                                                                 \
  struct gl_##NAME {                                                                                                   \
    CPP_EVAL(CPP_MAP(GL_STRUCT_TO_C_, __VA_ARGS__))                                                   \
  };

#define GL_STRUCT_TO_GLSL_PACKED_require(NAME) "//"
#define GL_STRUCT_TO_GLSL_PACKED_unorm8(NAME) "uint " #NAME
#define GL_STRUCT_TO_GLSL_PACKED_unorm8x2(NAME) "uint " #NAME
#define GL_STRUCT_TO_GLSL_PACKED_unorm8x3(NAME) "uint " #NAME
#define GL_STRUCT_TO_GLSL_PACKED_unorm8x4(NAME) "uint " #NAME
#define GL_STRUCT_TO_GLSL_PACKED_unorm16(NAME) "uint " #NAME
#define GL_STRUCT_TO_GLSL_PACKED_unorm16x2(NAME) "uint " #NAME
#define GL_STRUCT_TO_GLSL_PACKED_unorm16x3(NAME) "uvec2 " #NAME
#define GL_STRUCT_TO_GLSL_PACKED_unorm16x4(NAME) "uvec2 " #NAME
#define GL_STRUCT_TO_GLSL_PACKED_snorm16(NAME) "uint " #NAME
#define GL_STRUCT_TO_GLSL_PACKED_snorm16x2(NAME) "uint " #NAME
#define GL_STRUCT_TO_GLSL_PACKED_snorm16x3(NAME) "uvec2 " #NAME
#define GL_STRUCT_TO_GLSL_PACKED_snorm16x4(NAME) "uvec2 " #NAME
#define GL_STRUCT_TO_GLSL_PACKED_uint32(NAME) "uint " #NAME
#define GL_STRUCT_TO_GLSL_PACKED_int32(NAME) "int " #NAME
#define GL_STRUCT_TO_GLSL_PACKED_float32(NAME) "float " #NAME
#define GL_STRUCT_TO_GLSL_PACKED_float32x2(NAME) "vec2 " #NAME
#define GL_STRUCT_TO_GLSL_PACKED_float32x3(NAME) "vec3 " #NAME
#define GL_STRUCT_TO_GLSL_PACKED_float32x4(NAME) "vec4 " #NAME
#define GL_STRUCT_TO_GLSL_PACKED_struct(TYPE, NAME) #TYPE "Packed " #NAME
#define GL_STRUCT_TO_GLSL_PACKED_(ITEM) "  " CPP_CAT(GL_STRUCT_TO_GLSL_PACKED_, ITEM) ";\n"

#define GL_STRUCT_TO_GLSL_UNPACKED_require(NAME) "//"
#define GL_STRUCT_TO_GLSL_UNPACKED_unorm8(NAME) "float " #NAME
#define GL_STRUCT_TO_GLSL_UNPACKED_unorm8x2(NAME) "vec2 " #NAME
#define GL_STRUCT_TO_GLSL_UNPACKED_unorm8x3(NAME) "vec3 " #NAME
#define GL_STRUCT_TO_GLSL_UNPACKED_unorm8x4(NAME) "vec4 " #NAME
#define GL_STRUCT_TO_GLSL_UNPACKED_unorm16(NAME) "float " #NAME
#define GL_STRUCT_TO_GLSL_UNPACKED_unorm16x2(NAME) "vec2 " #NAME
#define GL_STRUCT_TO_GLSL_UNPACKED_unorm16x3(NAME) "vec3 " #NAME
#define GL_STRUCT_TO_GLSL_UNPACKED_unorm16x4(NAME) "vec4 " #NAME
#define GL_STRUCT_TO_GLSL_UNPACKED_snorm16(NAME) "float " #NAME
#define GL_STRUCT_TO_GLSL_UNPACKED_snorm16x2(NAME) "vec2 " #NAME
#define GL_STRUCT_TO_GLSL_UNPACKED_snorm16x3(NAME) "vec3 " #NAME
#define GL_STRUCT_TO_GLSL_UNPACKED_snorm16x4(NAME) "vec4 " #NAME
#define GL_STRUCT_TO_GLSL_UNPACKED_uint32(NAME) "uint " #NAME
#define GL_STRUCT_TO_GLSL_UNPACKED_int32(NAME) "int " #NAME
#define GL_STRUCT_TO_GLSL_UNPACKED_float32(NAME) "float " #NAME
#define GL_STRUCT_TO_GLSL_UNPACKED_float32x2(NAME) "vec2 " #NAME
#define GL_STRUCT_TO_GLSL_UNPACKED_float32x3(NAME) "vec3 " #NAME
#define GL_STRUCT_TO_GLSL_UNPACKED_float32x4(NAME) "vec4 " #NAME
#define GL_STRUCT_TO_GLSL_UNPACKED_struct(TYPE, NAME) #TYPE " " #NAME
#define GL_STRUCT_TO_GLSL_UNPACKED_(ITEM) "  " CPP_CAT(GL_STRUCT_TO_GLSL_UNPACKED_, ITEM) ";\n"

#define GL_STRUCT_TO_GLSL_PACK_require(NAME) "//"
#define GL_STRUCT_TO_GLSL_PACK_unorm8(NAME) "packUnorm4x8(vec4(unpack." #NAME ", 0, 0, 0))"
#define GL_STRUCT_TO_GLSL_PACK_unorm8x2(NAME) "packUnorm4x8(vec4(unpack." #NAME ", 0, 0))"
#define GL_STRUCT_TO_GLSL_PACK_unorm8x3(NAME) "packUnorm4x8(vec4(unpack." #NAME ", 0))"
#define GL_STRUCT_TO_GLSL_PACK_unorm8x4(NAME) "packUnorm4x8(unpack." #NAME ")"
#define GL_STRUCT_TO_GLSL_PACK_unorm16(NAME) "packUnorm2x16(vec2(unpack." #NAME ", 0))"
#define GL_STRUCT_TO_GLSL_PACK_unorm16x2(NAME) "packUnorm2x16(unpack." #NAME ")"
#define GL_STRUCT_TO_GLSL_PACK_unorm16x3(NAME)                                                                    \
  "uvec2(packUnorm2x16(unpack." #NAME ".xy), packUnorm2x16(vec2(unpack." #NAME ".z, 0)))"
#define GL_STRUCT_TO_GLSL_PACK_unorm16x4(NAME)                                                                    \
  "uvec2(packUnorm2x16(unpack." #NAME ".xy), packUnorm2x16(unpack." #NAME ".zw))"
#define GL_STRUCT_TO_GLSL_PACK_snorm16(NAME) "packSnorm2x16(vec2(unpack." #NAME ", 0))"
#define GL_STRUCT_TO_GLSL_PACK_snorm16x2(NAME) "packSnorm2x16(unpack." #NAME ")"
#define GL_STRUCT_TO_GLSL_PACK_snorm16x3(NAME)                                                                    \
  "uvec2(packSnorm2x16(unpack." #NAME ".xy), packSnorm2x16(vec2(unpack." #NAME ".z, 0)))"
#define GL_STRUCT_TO_GLSL_PACK_snorm16x4(NAME)                                                                    \
  "uvec2(packSnorm2x16(unpack." #NAME ".xy), packSnorm2x16(unpack." #NAME ".zw))"
#define GL_STRUCT_TO_GLSL_PACK_uint32(NAME) "unpack." #NAME
#define GL_STRUCT_TO_GLSL_PACK_int32(NAME) "unpack." #NAME
#define GL_STRUCT_TO_GLSL_PACK_float32(NAME) "unpack." #NAME
#define GL_STRUCT_TO_GLSL_PACK_float32x2(NAME) "unpack." #NAME
#define GL_STRUCT_TO_GLSL_PACK_float32x3(NAME) "unpack." #NAME
#define GL_STRUCT_TO_GLSL_PACK_float32x4(NAME) "unpack." #NAME
#define GL_STRUCT_TO_GLSL_PACK_struct(TYPE, NAME) #TYPE "_pack(unpack." #NAME ")"
#define GL_STRUCT_TO_GLSL_PACK_(ITEM) ",\n  " CPP_CAT(GL_STRUCT_TO_GLSL_PACK_, ITEM)

#define GL_STRUCT_TO_GLSL_UNPACK_require(NAME) "//"
#define GL_STRUCT_TO_GLSL_UNPACK_unorm8(NAME) "unpackUnorm4x8(pack." #NAME ").x"
#define GL_STRUCT_TO_GLSL_UNPACK_unorm8x2(NAME) "unpackUnorm4x8(pack." #NAME ").xy"
#define GL_STRUCT_TO_GLSL_UNPACK_unorm8x3(NAME) "unpackUnorm4x8(pack." #NAME ").xyz"
#define GL_STRUCT_TO_GLSL_UNPACK_unorm8x4(NAME) "unpackUnorm4x8(pack." #NAME ")"
#define GL_STRUCT_TO_GLSL_UNPACK_unorm16(NAME) "unpackUnorm2x16(pack." #NAME ").x"
#define GL_STRUCT_TO_GLSL_UNPACK_unorm16x2(NAME) "unpackUnorm2x16(pack." #NAME ")"
#define GL_STRUCT_TO_GLSL_UNPACK_unorm16x3(NAME)                                                                  \
  "vec3(unpackUnorm2x16(pack." #NAME ".x), unpackUnorm2x16(pack." #NAME ".y).x)"
#define GL_STRUCT_TO_GLSL_UNPACK_unorm16x4(NAME)                                                                  \
  "vec4(unpackUnorm2x16(pack." #NAME ".x), unpackUnorm2x16(pack." #NAME ".y))"
#define GL_STRUCT_TO_GLSL_UNPACK_snorm16(NAME) "unpackSnorm2x16(pack." #NAME ")"
#define GL_STRUCT_TO_GLSL_UNPACK_snorm16x2(NAME) "unpackSnorm2x16(pack." #NAME ")"
#define GL_STRUCT_TO_GLSL_UNPACK_snorm16x3(NAME)                                                                  \
  "vec3(unpackSnorm2x16(pack." #NAME ".x), unpackSnorm2x16(pack." #NAME ".y).x)"
#define GL_STRUCT_TO_GLSL_UNPACK_snorm16x4(NAME)                                                                  \
  "vec4(unpackSnorm2x16(pack." #NAME ".x), unpackSnorm2x16(pack." #NAME ".y))"
#define GL_STRUCT_TO_GLSL_UNPACK_uint32(NAME) "pack." #NAME
#define GL_STRUCT_TO_GLSL_UNPACK_int32(NAME) "pack." #NAME
#define GL_STRUCT_TO_GLSL_UNPACK_float32(NAME) "pack." #NAME
#define GL_STRUCT_TO_GLSL_UNPACK_float32x2(NAME) "pack." #NAME
#define GL_STRUCT_TO_GLSL_UNPACK_float32x3(NAME) "pack." #NAME
#define GL_STRUCT_TO_GLSL_UNPACK_float32x4(NAME) "pack." #NAME
#define GL_STRUCT_TO_GLSL_UNPACK_struct(TYPE, NAME) #TYPE "_unpack(pack." #NAME ")"
#define GL_STRUCT_TO_GLSL_UNPACK_(ITEM) ",\n  " CPP_CAT(GL_STRUCT_TO_GLSL_UNPACK_, ITEM)

// clang-format off
#define GL_STRUCT_TO_GLSL(NAME, ...)                                                                              \
  static const char gl_##NAME##_glsl[] =                                                                               \
    "struct " #NAME " {\n" CPP_EVAL(CPP_MAP(GL_STRUCT_TO_GLSL_UNPACKED_, __VA_ARGS__)) "};\n"         \
    "struct " #NAME "Packed {\n" CPP_EVAL(CPP_MAP(GL_STRUCT_TO_GLSL_PACKED_, __VA_ARGS__)) "};\n"     \
    #NAME "Packed " #NAME "_pack(in " #NAME " unpack) {\n"                                                             \
    "  return " #NAME "Packed(//"                                                                                      \
    CPP_EVAL(CPP_MAP(GL_STRUCT_TO_GLSL_PACK_, __VA_ARGS__))                                           \
    "  );\n"                                                                                                           \
    "}\n"                                                                                                              \
    #NAME " " #NAME "_unpack(in " #NAME "Packed pack) {\n"                                                             \
    "  return " #NAME "(//"                                                                                            \
    CPP_EVAL(CPP_MAP(GL_STRUCT_TO_GLSL_UNPACK_, __VA_ARGS__))                                         \
    "  );\n"                                                                                                           \
    "}\n";
// clang-format onX

#define GL_STRUCT_TO_DESC_require(NAME) &gl_##NAME##_snippet,
#define GL_STRUCT_TO_DESC_unorm8(NAME)
#define GL_STRUCT_TO_DESC_unorm8x2(NAME)
#define GL_STRUCT_TO_DESC_unorm8x3(NAME)
#define GL_STRUCT_TO_DESC_unorm8x4(NAME)
#define GL_STRUCT_TO_DESC_unorm16(NAME)
#define GL_STRUCT_TO_DESC_unorm16x2(NAME)
#define GL_STRUCT_TO_DESC_unorm16x3(NAME)
#define GL_STRUCT_TO_DESC_unorm16x4(NAME)
#define GL_STRUCT_TO_DESC_snorm16(NAME)
#define GL_STRUCT_TO_DESC_snorm16x2(NAME)
#define GL_STRUCT_TO_DESC_snorm16x3(NAME)
#define GL_STRUCT_TO_DESC_snorm16x4(NAME)
#define GL_STRUCT_TO_DESC_uint32(NAME)
#define GL_STRUCT_TO_DESC_int32(NAME)
#define GL_STRUCT_TO_DESC_float32(NAME)
#define GL_STRUCT_TO_DESC_float32x2(NAME)
#define GL_STRUCT_TO_DESC_float32x3(NAME)
#define GL_STRUCT_TO_DESC_float32x4(NAME)
#define GL_STRUCT_TO_DESC_struct(TYPE, NAME)
#define GL_STRUCT_TO_DESC_unsized_array(TYPE)
#define GL_STRUCT_TO_DESC_(ITEM) CPP_CAT(GL_STRUCT_TO_DESC_, ITEM)
#define GL_STRUCT_TO_DESC(NAME, ...)                                                                              \
  struct gl_ShaderSnippet gl_##NAME##_snippet = {                                                                      \
      .requires = {CPP_EVAL(CPP_MAP(GL_STRUCT_TO_DESC_, __VA_ARGS__)) NULL},                          \
      .code = gl_##NAME##_glsl,                                                                                        \
      .structure_size = sizeof(struct gl_##NAME)};

#define GL_DECLARE_STRUCT(NAME, ...)                                                                              \
  GL_STRUCT_TO_C(NAME, __VA_ARGS__)                                                                               \
  extern struct gl_ShaderSnippet gl_##NAME##_snippet;

#define GL_IMPL_STRUCT(NAME, ...)                                                                                 \
  GL_STRUCT_TO_GLSL(NAME, __VA_ARGS__)                                                                            \
  GL_STRUCT_TO_DESC(NAME, __VA_ARGS__)

#define GL_STRUCT(NAME, ...)                                                                                      \
  GL_STRUCT_TO_C(NAME, __VA_ARGS__)                                                                               \
  GL_STRUCT_TO_GLSL(NAME, __VA_ARGS__)                                                                            \
  GL_STRUCT_TO_DESC(NAME, __VA_ARGS__)

GL_DECLARE_STRUCT(DrawArraysIndirectCommand, uint32(count), uint32(instance_count), uint32(first),
                       uint32(base_instance))

GL_DECLARE_STRUCT(DrawElementsIndirectCommand, uint32(count), uint32(instance_count), uint32(first_index),
                       uint32(base_vertex), uint32(base_instance))

GL_DECLARE_STRUCT(DispatchIndirectCommand, uint32(num_groups_x), uint32(num_groups_y), uint32(num_groups_z))

#define GL_BLOCK_TO_C(NAME, ...) GL_STRUCT_TO_C(NAME, __VA_ARGS__)
#define GL_BLOCK_TO_DESC(NAME, ...) GL_STRUCT_TO_DESC(NAME, __VA_ARGS__)

#define GL_CAT(A, ...) GL_CAT_(A, ##__VA_ARGS__)
#define GL_CAT_(A, ...) A##__VA_ARGS__

#define GL_BLOCK_TO_GLSL_require(NAME) "  //"
#define GL_BLOCK_TO_GLSL_uint32(NAME) "  uint " #NAME
#define GL_BLOCK_TO_GLSL_int32(NAME) "  int " #NAME
#define GL_BLOCK_TO_GLSL_float32(NAME) "  float " #NAME
#define GL_BLOCK_TO_GLSL_float32x2(NAME) "  vec2 " #NAME
#define GL_BLOCK_TO_GLSL_float32x3(NAME) "  vec3 " #NAME
#define GL_BLOCK_TO_GLSL_float32x4(NAME) "  vec4 " #NAME
#define GL_BLOCK_TO_GLSL_struct(TYPE, NAME) "  " #TYPE " " #NAME
#define GL_BLOCK_TO_GLSL_unsized_array(TYPE) GL_CAT(GL_BLOCK_TO_GLSL_, TYPE) "[]"
#define GL_BLOCK_TO_GLSL_(ITEM) CPP_CAT(GL_BLOCK_TO_GLSL_, ITEM) ";\n"

#define GL_BLOCK_TO_GLSL(NAME, ...)                                                                               \
  static const char gl_##NAME##_glsl[] =                                                                               \
      "buffer " #NAME " {\n" CPP_EVAL(CPP_MAP(GL_BLOCK_TO_GLSL_, __VA_ARGS__)) "} ";

#define GL_DECLARE_BLOCK(NAME, ...)                                                                               \
  GL_BLOCK_TO_C(NAME, __VA_ARGS__)                                                                                \
  extern struct gl_ShaderSnippet gl_##NAME##_snippet;

#define GL_IMPL_BLOCK(NAME, ...)                                                                                  \
  GL_BLOCK_TO_GLSL(NAME, __VA_ARGS__)                                                                             \
  GL_BLOCK_TO_DESC(NAME, __VA_ARGS__)

#define GL_BLOCK(NAME, ...)                                                                                       \
  GL_BLOCK_TO_C(NAME, __VA_ARGS__)                                                                                \
  GL_BLOCK_TO_GLSL(NAME, __VA_ARGS__)                                                                             \
  GL_BLOCK_TO_DESC(NAME, __VA_ARGS__)

#endif // gl_h_INCLUDED
