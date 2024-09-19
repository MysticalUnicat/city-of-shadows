#ifndef __LIBRARY_ENGINE_ECS_H__
#define __LIBRARY_ENGINE_ECS_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "memory.h"
#include "assert.h"
#include "log.h"
#include "cpp.h"

typedef enum ecs_Result {
  ECS_SUCCESS = 0,                       ///< command successfully completed
  ECS_ERROR_INVALID_ARGUMENT = -1,       ///< an argument given to the command is invalid
  ECS_ERROR_OUT_OF_MEMORY = -2,          ///< a memory allocation has failed
  ECS_ERROR_DOES_NOT_EXIST = -3,         ///< a requested object does not exist
  ECS_ERROR_INVALID_ENTITY = -4,         ///< something inside is broken, should not happen
  ECS_ERROR_INVALID_LAYER = -5,          ///< something inside is broken, should not happen
  ECS_ERROR_COMPONENT_EXISTS = -6,
  ECS_ERROR_COMPONENT_DOES_NOT_EXIST = -7,
} ecs_Result;

typedef uint64_t ecs_LayerHandle;

#define ECS_INVALID_LAYER 0xFFFFFFFF

typedef struct ecs_LayerCreateInfo {
  uint32_t max_entities;
} ecs_LayerCreateInfo;

ecs_Result ecs_create_layer(const ecs_LayerCreateInfo *create_info,
                            ecs_LayerHandle *layer_handle_ptr);

typedef enum ecs_LayerDestroyFlags {
  ECS_LAYER_DESTROY_REMOVE_ENTITIES = 0x1,
} ecs_LayerDestroyFlags;

ecs_Result ecs_destroy_layer(const ecs_LayerHandle layer_handle,
                             ecs_LayerDestroyFlags flags);

typedef uint32_t ecs_ComponentHandle;

typedef uint64_t ecs_EntityHandle;

typedef void (*ecs_ComponentInit)(void *ud, ecs_EntityHandle entity,
                                    void **data);

typedef void (*ecs_ComponentCleanup)(void *ud, ecs_EntityHandle entity,
                                       void **data);

typedef enum ecs_ComponentCreateFlags {
  ECS_COMPONENT_CREATE_NOT_NULL = 0x1,
} ecs_ComponentCreateFlags;

typedef struct ecs_ComponentCreateInfo {
  ecs_ComponentCreateFlags flags;

  size_t size;

  uint32_t num_required_components;

  const ecs_ComponentHandle *required_components;

  ecs_ComponentInit init;
  ecs_ComponentCleanup cleanup;
  void * user_data;
} ecs_ComponentCreateInfo;

ecs_Result ecs_register_component(const ecs_ComponentCreateInfo *create_info,
                                  ecs_ComponentHandle *component_ptr);

typedef struct ecs_EntitySpawnComponent {
  ecs_ComponentHandle component;

  uint32_t stride;

  const void *data;
} ecs_EntitySpawnComponent;

typedef struct ecs_EntitySpawnInfo {
  ecs_LayerHandle layer;

  uint32_t count;

  uint32_t num_components;

  const ecs_EntitySpawnComponent *components;
} ecs_EntitySpawnInfo;

ecs_Result ecs_spawn(const ecs_EntitySpawnInfo *spawn_info,
                     ecs_EntityHandle *entities_ptr);

ecs_Result ecs_add_component_to_entity(ecs_EntityHandle entity,
                                       ecs_ComponentHandle component,
                                       const void *data);

ecs_Result ecs_remove_component_from_entity(ecs_EntityHandle entity,
                                            ecs_ComponentHandle component);

ecs_Result ecs_write_entity_component(ecs_EntityHandle entity,
                                      ecs_ComponentHandle component,
                                      void **out_ptr);

ecs_Result ecs_read_entity_component(ecs_EntityHandle entity,
                                     ecs_ComponentHandle component,
                                     const void **out_ptr);

ecs_Result ecs_despawn(uint32_t num_entities, const ecs_EntityHandle *entities);

typedef struct ecs_Query ecs_Query;

typedef void (*ecs_QueryFunction)(void *ud, ecs_EntityHandle entity, void **data);

typedef enum ecs_Filter {
  ECS_FILTER_EXCLUDE,
  ECS_FILTER_OPTIONAL,
  ECS_FILTER_MODIFIED
} ecs_Filter;

typedef struct ecs_QueryFilterCreateInfo {
  ecs_Filter filter;
  ecs_ComponentHandle component;
} ecs_QueryFilterCreateInfo;

typedef struct ecs_QueryCreateInfo {
  uint32_t num_write_components;
  const ecs_ComponentHandle *write_components;

  uint32_t num_read_components;
  const ecs_ComponentHandle *read_components;

  uint32_t num_filters;
  ecs_QueryFilterCreateInfo *filters;
} ecs_QueryCreateInfo;

ecs_Result ecs_create_query(const ecs_QueryCreateInfo *create_info,
                            ecs_Query **query_ptr);

ecs_Result ecs_execute_query(ecs_Query *query, ecs_QueryFunction cb, void *ud);

void ecs_destroy_query(ecs_Query *query);

// --------------------------------------------------------------------------------------------------------------------
#define ECS(F, ...) assert(ECS_SUCCESS == ecs_##F(__VA_ARGS__))

#define ECS_LAZY_GLOBAL(TYPE, IDENT, ...) \
  TYPE IDENT(void) { \
    static TYPE inner; \
    static int init = 0; \
    if(init == 0) { \
      TRACE(ecs, "making " #IDENT " ..."); \
      __VA_ARGS__ \
      init = 1; \
    } \
    return inner; \
  }

#define ECS_LAZY_GLOBAL_PTR(TYPE, IDENT, ...) \
  TYPE *IDENT(void) { \
    static TYPE inner; \
    static TYPE *__ptr = NULL; \
    if(__ptr == NULL) { \
      TRACE(ecs, "making " #IDENT " ..."); \
      __VA_ARGS__ \
      __ptr = &inner; \
    } \
    return __ptr; \
  }

#define ECS_DECLARE_COMPONENT(IDENT, ...) \
  struct IDENT __VA_ARGS__; \
  ecs_ComponentHandle IDENT##_component(void); \
  const struct IDENT *IDENT##_read(ecs_EntityHandle entity); \
  struct IDENT *IDENT##_write(ecs_EntityHandle entity);


#define CPP_EQ__ECS_COMPONENTrequires_requires(...) CPP_PROBE

#define ECS_COMPONENT_is_requires(X) CPP_EQ(ECS_COMPONENT, requires, X)

#define ECS_COMPONENT_emit(X) CPP_CAT(ECS_COMPONENT_emit_, X)
#define ECS_COMPONENT_emit_requires(NAME) NAME##_component(), 

#define ECS_COMPONENT_impl(IDENT, ...) \
  ECS_LAZY_GLOBAL(ecs_ComponentHandle, IDENT##_component, \
    ecs_ComponentHandle required_components[] = {CPP_FILTER_MAP(ECS_COMPONENT_is_requires, ECS_COMPONENT_emit, __VA_ARGS__)}; \
    ecs_ComponentCreateInfo create_info; \
		create_info.size = sizeof(struct IDENT); \
		create_info.num_required_components = (sizeof(required_components) / sizeof(ecs_ComponentHandle)); \
		create_info.required_components = required_components; \
    ECS(register_component, &create_info, &inner); \
  ) \
  const struct IDENT *IDENT##_read(ecs_EntityHandle entity) { \
    const struct IDENT *ptr; \
    ecs_read_entity_component(entity, IDENT##_component(), (const void **)&ptr); \
    return ptr; \
  } \
  struct IDENT *IDENT##_write(ecs_EntityHandle entity) { \
    struct IDENT *ptr; \
    ecs_write_entity_component(entity, IDENT##_component(), (void **)&ptr); \
    return ptr; \
  }
#define ECS_COMPONENT(IDENT, ...) CPP_EVAL(ECS_COMPONENT_impl(IDENT, ## __VA_ARGS__))

#define CPP_EQ__ECS_QUERYstate_state(...) CPP_PROBE
#define CPP_EQ__ECS_QUERYargument_argument(...) CPP_PROBE
#define CPP_EQ__ECS_QUERYpre_pre(...) CPP_PROBE
#define CPP_EQ__ECS_QUERYread_read(...) CPP_PROBE
#define CPP_EQ__ECS_QUERYwrite_write(...) CPP_PROBE
#define CPP_EQ__ECS_QUERYoptional_optional(...) CPP_PROBE
#define CPP_EQ__ECS_QUERYexclude_exclude(...) CPP_PROBE
#define CPP_EQ__ECS_QUERYmodified_modified(...) CPP_PROBE
#define CPP_EQ__ECS_QUERYaction_action(...) CPP_PROBE
#define CPP_EQ__ECS_QUERYpost_post(...) CPP_PROBE

#define ECS_QUERY_is_state(X) CPP_EQ(ECS_QUERY, state, X)
#define ECS_QUERY_is_argument(X) CPP_EQ(ECS_QUERY, argument, X)
#define ECS_QUERY_is_pre(X) CPP_EQ(ECS_QUERY, pre, X)
#define ECS_QUERY_is_read(X) CPP_EQ(ECS_QUERY, read, X)
#define ECS_QUERY_is_write(X) CPP_EQ(ECS_QUERY, write, X)
#define ECS_QUERY_is_filter(X) \
  CPP_OR(CPP_OR(CPP_EQ(ECS_QUERY, optional, X), CPP_EQ(ECS_QUERY, exclude, X)), \
               CPP_EQ(ECS_QUERY, modified, X))
#define ECS_QUERY_is_action(X) CPP_EQ(ECS_QUERY, action, X)
#define ECS_QUERY_is_post(X) CPP_EQ(ECS_QUERY, post, X)

#define ECS_QUERY_emit(X) CPP_CAT(ECS_QUERY_emit_, X)
#define ECS_QUERY_emit_state(TYPE, NAME, ...) TYPE NAME;
#define ECS_QUERY_emit_argument(TYPE, NAME, ...) TYPE NAME;
#define ECS_QUERY_emit_write(TYPE, NAME) struct TYPE *NAME = (struct TYPE *)data[__i++];
#define ECS_QUERY_emit_read(TYPE, NAME) const struct TYPE *NAME = (const struct TYPE *)data[__i++];
#define ECS_QUERY_emit_pre(...) __VA_ARGS__
#define ECS_QUERY_emit_action(...) __VA_ARGS__
#define ECS_QUERY_emit_post(...) __VA_ARGS__

#define ECS_QUERY_emit_create(X) CPP_CAT(ECS_QUERY_emit_create_, X)
#define ECS_QUERY_emit_create_read(TYPE, NAME) TYPE##_component(),
#define ECS_QUERY_emit_create_write(TYPE, NAME) TYPE##_component(),
#define ECS_QUERY_emit_create_optional(TYPE) {.component = TYPE##_component(), .filter = ECS_FILTER_OPTIONAL},
#define ECS_QUERY_emit_create_exclude(TYPE) {.component = TYPE##_component(), .filter = ECS_FILTER_EXCLUDE},
#define ECS_QUERY_emit_create_modified(TYPE) {.component = TYPE##_component(), .filter = ECS_FILTER_MODIFIED},

#define ECS_QUERY_emit_arg1(X) CPP_CAT(ECS_QUERY_emit_arg1_, X)
#define ECS_QUERY_emit_arg1_argument(TYPE, NAME) TYPE NAME,

#define ECS_QUERY_emit_arg2(X) CPP_CAT(ECS_QUERY_emit_arg2_, X)
#define ECS_QUERY_emit_arg2_argument(TYPE, NAME) state->NAME = NAME;

#define ECS_QUERY(NAME, ...) CPP_EVAL(ECS_QUERY_impl(NAME, __VA_ARGS__))
#define ECS_QUERY_impl(NAME, ...) \
  struct CPP_CAT(NAME, _state) { \
    ecs_Query *query; \
    CPP_FILTER_MAP(ECS_QUERY_is_state, ECS_QUERY_emit, __VA_ARGS__) \
    CPP_FILTER_MAP(ECS_QUERY_is_argument, ECS_QUERY_emit, __VA_ARGS__) \
  }; \
  static void CPP_CAT(NAME, _do)(void *ud, ecs_EntityHandle entity, \
                                       void **data) { \
    uint32_t __i = 0; \
    struct CPP_CAT(NAME, _state) *state = (struct CPP_CAT(NAME, _state) *)ud; \
    CPP_FILTER_MAP(ECS_QUERY_is_write, ECS_QUERY_emit, __VA_ARGS__) \
    CPP_FILTER_MAP(ECS_QUERY_is_read, ECS_QUERY_emit, __VA_ARGS__) \
    CPP_FILTER_MAP(ECS_QUERY_is_action, ECS_QUERY_emit, __VA_ARGS__) \
  } \
  void NAME( \
    CPP_FILTER_MAP(ECS_QUERY_is_argument, ECS_QUERY_emit_arg1, __VA_ARGS__) ... \
  ) { \
    static struct CPP_CAT(NAME, _state) _state = {0}; \
    static struct CPP_CAT(NAME, _state) *state = &_state; \
    CPP_FILTER_MAP(ECS_QUERY_is_argument, ECS_QUERY_emit_arg2, __VA_ARGS__) \
    if(state->query == NULL) { \
      ecs_ComponentHandle _rlist[] = { \
          CPP_FILTER_MAP(ECS_QUERY_is_read, ECS_QUERY_emit_create, __VA_ARGS__)}; \
      ecs_ComponentHandle _wlist[] = { \
          CPP_FILTER_MAP(ECS_QUERY_is_write, ECS_QUERY_emit_create, __VA_ARGS__)}; \
      ecs_QueryFilterCreateInfo _flist[] = { \
          CPP_FILTER_MAP(ECS_QUERY_is_filter, ECS_QUERY_emit_create, __VA_ARGS__)}; \
      ecs_create_query( \
                             &(ecs_QueryCreateInfo){.num_write_components = sizeof(_wlist) / sizeof(_wlist[0]), \
                                                          .write_components = _wlist, \
                                                          .num_read_components = sizeof(_rlist) / sizeof(_rlist[0]), \
                                                          .read_components = _rlist, \
                                                          .num_filters = sizeof(_flist) / sizeof(_flist[0]), \
                                                          .filters = _flist}, \
                             &state->query); \
    } \
    CPP_FILTER_MAP(ECS_QUERY_is_pre, ECS_QUERY_emit, __VA_ARGS__) \
    ecs_execute_query(state->query, CPP_CAT(NAME, _do), state); \
    CPP_FILTER_MAP(ECS_QUERY_is_post, ECS_QUERY_emit, __VA_ARGS__) \
  }

#endif
