#ifndef __LIBRARY_ENGINE_ECS_H__
#define __LIBRARY_ENGINE_ECS_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "memory.h"

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

ecs_Result ecs_execute_query(ecs_Query *query, ecs_QueryFunction cb);

void ecs_destroy_query(ecs_Query *query);

#endif
