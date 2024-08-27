#include "ecs_local.h"

#define MAX_ENTITIES_FOR_LINEAR_OPS 512 // number decided by dice roll

ecs_Result ecs_validate_layer_handle(
   ecs_LayerHandle      layer
  , uint32_t                 * index_ptr
) {
  uint32_t index = (uint32_t)(layer & 0xFFFFFFFF);
  uint32_t generation = (uint32_t)(layer >> 32);
  if(index >= engine_ecs_layer.length) {
    return ECS_ERROR_INVALID_LAYER;
  }
  if(generation != engine_ecs_layer.generation[index]) {
    return ECS_ERROR_INVALID_LAYER;
  }
  *index_ptr = index;
  return ECS_SUCCESS;
}

static int _compare_entity_index(const void * ap, const void * bp, void *ud) {
  uint32_t a = *(uint32_t *)ap;
  uint32_t b = *(uint32_t *)bp;
  return a - b;
}

static void _linear_remove_entity(ecs_Layer * data, uint32_t entity_index) {
  for(uint32_t i = 0; i < data->entities.length; i++) {
    if(data->entities.data[i] == entity_index) {
      Vector_remove_at(&data->entities, i);
      return;
    }
  }
  // the case where the entity is not found should not happen
  // TODO: record such case
}

void ecs_unset_entity_layer(
   uint32_t             entity_index
) {
  uint32_t old_layer_index = engine_ecs_entity.layer_index[entity_index];
  if(old_layer_index == 0) {
    return;
  }
  engine_ecs_entity.layer_index[entity_index] = 0;

  ecs_Layer * data = &engine_ecs_layer.data[old_layer_index];

  if(data->dirty && data->entities.length < MAX_ENTITIES_FOR_LINEAR_OPS) {
    _linear_remove_entity(data, entity_index);
    return;
  }

  if(data->dirty) {
    Vector_qsort(&data->entities, _compare_entity_index, NULL);
  }

  // the case where the entity is not found should not happen
  // TODO: record such case
  uint32_t * index_ptr = Vector_bsearch(&data->entities, &entity_index, _compare_entity_index, NULL);
  if(index_ptr != NULL) {
    if(data->entities.length < MAX_ENTITIES_FOR_LINEAR_OPS) {
      Vector_remove_at(&data->entities, index_ptr - data->entities.data);
    } else {
      Vector_swap_pop(&data->entities, *index_ptr);
      data->dirty = 1;
    }
  }
}

ecs_Result ecs_set_entity_layer(
   uint32_t             entity_index
  , uint32_t             layer_index
) {
  engine_ecs_entity.layer_index[entity_index] = layer_index;

  if(layer_index == 0) {
    return ECS_SUCCESS;
  }

  ecs_Layer * data = &engine_ecs_layer.data[layer_index];

  if(data->at_max && data->entities.length >= data->entities.capacity) {
    return ECS_ERROR_INVALID_ENTITY;
  }

  if(!Vector_space_for(&engine_ecs_layer.data[layer_index].entities, entity_index)) {
    return ECS_ERROR_OUT_OF_MEMORY;
  }
  *Vector_push(&engine_ecs_layer.data[layer_index].entities) = entity_index;

  return ECS_SUCCESS;
}

ecs_Result ecs_create_layer(
   const ecs_LayerCreateInfo * create_info
  , ecs_LayerHandle * layer_ptr
) {
  uint32_t index;

  return_ERROR_INVALID_ARGUMENT_if(create_info == NULL);
  return_ERROR_INVALID_ARGUMENT_if(layer_ptr == NULL);

  if(engine_ecs_layer.free_indexes.length > 0) {
    index = *Vector_pop(&engine_ecs_layer.free_indexes);
  } else {
    index = engine_ecs_layer.length++;
  }

  if(engine_ecs_layer.length > engine_ecs_layer.capacity) {
    uint32_t old_capacity = engine_ecs_layer.capacity;
    uint32_t new_capacity = engine_ecs_layer.length + (engine_ecs_layer.length >> 1);
    RELOC(old_capacity, new_capacity, engine_ecs_layer.generation);
    RELOC(old_capacity, new_capacity, engine_ecs_layer.data);
    engine_ecs_layer.capacity = new_capacity;
  }
 
  ecs_Layer * data = engine_ecs_layer.data + index;

  if(create_info->max_entities > 0) {
    data->at_max = 1;
    Vector_set_capacity(&data->entities, create_info->max_entities);
  }

  uint32_t generation = engine_ecs_layer.generation[index];

  *layer_ptr = ((uint64_t)generation << 32) | (uint64_t)index;

  return ECS_SUCCESS;
}

ecs_Result ecs_despawn_indexes(
   uint32_t             count
  , const uint32_t     * entity_indexes
);

ecs_Result ecs_destroy_layer(
   ecs_LayerHandle         layer
  , ecs_LayerDestroyFlags   flags
) {
  uint32_t layer_index;
  return_if_ERROR(ecs_validate_layer_handle(layer, &layer_index));
  ecs_Layer * data = &engine_ecs_layer.data[layer_index];
  if(flags & ECS_LAYER_DESTROY_REMOVE_ENTITIES) {
    return_if_ERROR(ecs_despawn_indexes(data->entities.length, data->entities.data));
  } else {
    // send them back to global layer
    for(uint32_t i = 0; i < data->entities.length; i++) {
      ecs_set_entity_layer(data->entities.data[i], 0);
    }
  }
  Vector_free(&data->entities);
  return ECS_SUCCESS;
}

