#include "ecs_local.h"

#include "log.h"
#include "sort.h"

ecs_Result ecs_validate_entity_handle(ecs_EntityHandle entity,
                                      uint32_t *index_ptr) {
  uint32_t index = (uint32_t)(entity & 0xFFFFFFFF);
  uint32_t generation = (uint32_t)(entity >> 32);
  if (index >= engine_ecs_entity.length) {
    return ECS_ERROR_INVALID_ENTITY;
  }
  if (generation != engine_ecs_entity.generation[index]) {
    return ECS_ERROR_INVALID_ENTITY;
  }
  *index_ptr = index;
  return ECS_SUCCESS;
}

ecs_Result ecs_create_entity(ecs_EntityHandle *entity_ptr) {
  uint32_t index;

  if (engine_ecs_entity.free_indexes.length > 0) {
    index = *Vector_pop(&engine_ecs_entity.free_indexes);
  } else {
    index = engine_ecs_entity.length++;
  }

  if (engine_ecs_entity.length > engine_ecs_entity.capacity) {
    size_t old_capacity = engine_ecs_entity.capacity;
    size_t new_capacity = engine_ecs_entity.length + 1;
    new_capacity += new_capacity >> 1;
    RELOC(old_capacity, new_capacity, engine_ecs_entity.generation);
    RELOC(old_capacity, new_capacity, engine_ecs_entity.layer_index);
    RELOC(old_capacity, new_capacity,
          engine_ecs_entity.archetype_index);
    RELOC(old_capacity, new_capacity,
          engine_ecs_entity.archetype_code);
    engine_ecs_entity.capacity = new_capacity;
  }

  uint32_t generation = engine_ecs_entity.generation[index];

  *entity_ptr = ((uint64_t)generation << 32) | (uint64_t)index;

  return ECS_SUCCESS;
}

ecs_Result ecs_free_entity(uint32_t entity_id) {
  ++engine_ecs_entity.generation[entity_id];
  if (!Vector_space_for(&engine_ecs_entity.free_indexes, 
                        1)) {
    return ECS_ERROR_OUT_OF_MEMORY;
  }
  *Vector_push(&engine_ecs_entity.free_indexes) = entity_id;
  return ECS_SUCCESS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

static int _compar_component_index(const void *ap, const void *bp, void *ud) {
  uint32_t a = *(uint32_t *)ap;
  uint32_t b = *(uint32_t *)bp;
  return a - b;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
ecs_Result ecs_spawn(const ecs_EntitySpawnInfo *spawn_info,
                     ecs_EntityHandle *entities_ptr) {
  return_ERROR_INVALID_ARGUMENT_if(spawn_info == NULL);

  return_ERROR_INVALID_ARGUMENT_if(spawn_info->count == 0);

  uint32_t layer_index = UINT32_MAX;
  if (spawn_info->layer != ECS_INVALID_LAYER) {
    return_if_ERROR(
        ecs_validate_layer_handle(spawn_info->layer, &layer_index));
  }

  uint32_t archetype_index;
  {
    ecs_ComponentSet components;
    components.count = spawn_info->num_components;
    ALLOC(components.count, components.index);
    for (uint32_t i = 0; i < components.count; ++i) {
      components.index[i] = spawn_info->components[i].component;
    }
    sort_qsort(components.index, components.count, sizeof(*components.index),
          _compar_component_index, NULL);
    return_if_ERROR(
        ecs_resolve_archetype(components, &archetype_index));
  }
  ecs_Archetype *archetype = &engine_ecs_archetype.data[archetype_index];

  int free_out_entities = entities_ptr == NULL;
  if (free_out_entities) {
    ALLOC(spawn_info->count, entities_ptr);
  }

  for (uint32_t i = 0; i < spawn_info->count; i++) {
    ecs_EntityHandle entity;
    uint32_t entity_index;

    return_if_ERROR(ecs_create_entity(&entity));

    return_if_ERROR(
        ecs_validate_entity_handle(entity, &entity_index));

    if (layer_index != UINT32_MAX) {
      return_if_ERROR(
          ecs_set_entity_layer(entity_index, layer_index));
    }
    return_if_ERROR(
        ecs_set_entity_archetype(entity_index, archetype_index));

    entities_ptr[i] = entity;
  }

  for (uint32_t i = 0; i < spawn_info->num_components; i++) {
    ecs_EntitySpawnComponent spawn_component = spawn_info->components[i];

    uint32_t component_index = ecs_ComponentSet_order_of(
        &archetype->components, spawn_component.component);

    ASSERT(component_index != UINT32_MAX);

    uint32_t component_size, component_offset;
    PagedSOA_decode_column(&archetype->paged_soa, component_index + 1,
                           &component_size, &component_offset);

    const uint8_t *read = spawn_component.data;
    uint32_t stride =
        spawn_component.stride ? spawn_component.stride : component_size;

    for (uint32_t j = 0; j < spawn_info->count; j++) {
      uint32_t entity_index = (uint32_t)(entities_ptr[j] & 0xFFFFFFFF);
      uint32_t code = ENTITY_ARCHETYPE_CODE(entity_index);
      uint32_t page, index;
      PagedSOA_decode_code(&archetype->paged_soa, code, &page, &index);
      void *write = PagedSOA_raw_write(&archetype->paged_soa, page, index,
                                       component_size, component_offset);
      memcpy(write, read, component_size);
      read += stride;
    }
  }

  for (uint32_t i = 0; i < archetype->components.count; i++) {
    uint32_t j;
    for (j = 0;
         j < spawn_info->num_components &&
         spawn_info->components[j].component != archetype->components.index[i];
         j++)
      ;
    if (j < spawn_info->num_components) {
      continue;
    }
    uint32_t component_size, component_offset;
    PagedSOA_decode_column(&archetype->paged_soa, i + 1, &component_size,
                           &component_offset);
    for (j = 0; j < spawn_info->count; j++) {
      uint32_t entity_index = (uint32_t)(entities_ptr[j] & 0xFFFFFFFF);
      uint32_t code = ENTITY_ARCHETYPE_CODE(entity_index);
      uint32_t page, index;
      PagedSOA_decode_code(&archetype->paged_soa, code, &page, &index);
      void *write = PagedSOA_raw_write(&archetype->paged_soa, page, index,
                                       component_size, component_offset);
      memset(write, 0, component_size);
    }
  }

  for (uint32_t j = 0; j < spawn_info->count; j++) {
    uint32_t entity_index = (uint32_t)(entities_ptr[j] & 0xFFFFFFFF);
    ecs_init_components(entity_index, archetype_index);
  }

  if (free_out_entities) {
    FREE(spawn_info->count, entities_ptr);
  }

  return ECS_SUCCESS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

ecs_Result ecs_add_component_to_entity(ecs_EntityHandle entity,
                                       ecs_ComponentHandle component_handle,
                                       const void *data) {
  uint32_t entity_index;

  return_if_ERROR(ecs_validate_entity_handle(entity, &entity_index));
  return_ERROR_INVALID_ARGUMENT_if(component_handle >=
                                   engine_ecs_component.length);

  const ecs_Component *component = &engine_ecs_component.data[component_handle];
  return_ERROR_INVALID_ARGUMENT_if(component->non_null && data == NULL);

  ecs_Archetype *archetype = ENTITY_ARCHETYPE_DATA(entity_index);

  uint32_t component_index =
      ecs_ComponentSet_order_of(&archetype->components, component_handle);

  if (component_index != UINT32_MAX) {
    return ECS_ERROR_COMPONENT_EXISTS;
  }

  ecs_ArchetypeHandle new_archetype;
  {
    ecs_ComponentSet new_components;
    return_if_ERROR(ecs_ComponentSet_add(
        &new_components, &archetype->components, component_handle));

    return_if_ERROR(
        ecs_resolve_archetype(new_components, &new_archetype));

    // ecs_resolve_archetype 'consumes' new components
  }

  return_if_ERROR(ecs_set_entity_archetype(entity, new_archetype));

  component_index = ecs_ComponentSet_order_of(
      &engine_ecs_archetype.data[new_archetype].components, component_handle);

  if (data != NULL) {
    memcpy(ecs_write(entity_index, component_index), data,
           component->size);
  } else {
    memset(ecs_write(entity_index, component_index), 0,
           component->size);
  }

  ecs_init_component(entity_index, new_archetype, component_index);

  return ECS_SUCCESS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

ecs_Result
ecs_remove_component_from_entity(ecs_EntityHandle entity,
                                 ecs_ComponentHandle component_handle) {
  uint32_t entity_index;

  return_if_ERROR(ecs_validate_entity_handle(entity, &entity_index));
  return_ERROR_INVALID_ARGUMENT_if(component_handle >=
                                   engine_ecs_component.length);

  uint32_t archetype_index = ENTITY_ARCHETYPE_INDEX(entity_index);
  ecs_Archetype *archetype = &engine_ecs_archetype.data[archetype_index];

  uint32_t component_index =
      ecs_ComponentSet_order_of(&archetype->components, component_handle);

  if (component_index == UINT32_MAX) {
    return ECS_ERROR_COMPONENT_DOES_NOT_EXIST;
  }
  ecs_cleanup_component(entity_index, archetype_index,
                        component_index);

  ecs_ArchetypeHandle new_archetype;
  {
    ecs_ComponentSet new_components;
    return_if_ERROR(ecs_ComponentSet_remove(
        &new_components, &archetype->components, component_handle));

    return_if_ERROR(
        ecs_resolve_archetype(new_components, &new_archetype));

    // ecs_resolve_archetype 'consumes' new components
  }

  ecs_set_entity_archetype(entity, new_archetype);

  return ECS_SUCCESS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

ecs_Result ecs_read_entity_component(ecs_EntityHandle entity,
                                     ecs_ComponentHandle component_handle,
                                     const void **ptr) {
  uint32_t entity_index;

  return_if_ERROR(ecs_validate_entity_handle(entity, &entity_index));
  return_ERROR_INVALID_ARGUMENT_if(component_handle >=
                                   engine_ecs_component.length);

  ecs_Archetype *archetype = ENTITY_ARCHETYPE_DATA(entity_index);

  uint32_t component_index =
      ecs_ComponentSet_order_of(&archetype->components, component_handle);
  if (component_index == UINT32_MAX) {
    *ptr = NULL;
    return ECS_ERROR_COMPONENT_DOES_NOT_EXIST;
  }

  *ptr = ecs_write(entity_index, component_index);

  return ECS_SUCCESS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

ecs_Result ecs_write_entity_component(ecs_EntityHandle entity,
                                      ecs_ComponentHandle component_handle,
                                      void **ptr) {
  uint32_t entity_index;

  return_if_ERROR(ecs_validate_entity_handle(entity, &entity_index));
  return_ERROR_INVALID_ARGUMENT_if(component_handle >=
                                   engine_ecs_component.length);

  ecs_Archetype *archetype = ENTITY_ARCHETYPE_DATA(entity_index);

  uint32_t component_index =
      ecs_ComponentSet_order_of(&archetype->components, component_handle);
  if (component_index == UINT32_MAX) {
    *ptr = NULL;
    return ECS_ERROR_COMPONENT_DOES_NOT_EXIST;
  }

  *ptr = ecs_write(entity_index, component_index);

  return ECS_SUCCESS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

ecs_Result ecs_despawn(uint32_t count, const ecs_EntityHandle *entity) {
  uint32_t entity_index;

  return_ERROR_INVALID_ARGUMENT_if(count > engine_ecs_entity.length);
  return_ERROR_INVALID_ARGUMENT_if(entity == NULL);

  for (uint32_t i = 0; i < count; i++) {
    return_if_ERROR(
        ecs_validate_entity_handle(entity[i], &entity_index));

    ecs_cleanup_components(entity_index,
                           ENTITY_ARCHETYPE_INDEX(entity_index));
    ecs_unset_entity_layer(entity_index);
    ecs_unset_entity_archetype(entity_index);
    ecs_free_entity(entity_index);
  }

  return ECS_SUCCESS;
}

ecs_Result ecs_despawn_indexes(uint32_t count, const uint32_t *entity_indexes) {
  uint32_t entity_index;

  return_ERROR_INVALID_ARGUMENT_if(count > engine_ecs_entity.length);
  return_ERROR_INVALID_ARGUMENT_if(entity_indexes == NULL);

  for (uint32_t i = 0; i < count; i++) {
    entity_index = entity_indexes[i];

    ecs_cleanup_components(entity_index,
                           ENTITY_ARCHETYPE_INDEX(entity_index));
    ecs_unset_entity_layer(entity_index);
    ecs_unset_entity_archetype(entity_index);
    ecs_free_entity(entity_index);
  }

  return ECS_SUCCESS;
}
