#include "ecs_local.h"

#include "sort.h"

ecs_Result ecs_register_component(const ecs_ComponentCreateInfo *create_info,
                                  ecs_ComponentHandle *component_ptr) {
  return_ERROR_INVALID_ARGUMENT_if(create_info == NULL);
  return_ERROR_INVALID_ARGUMENT_if(component_ptr == NULL);
  return_ERROR_INVALID_ARGUMENT_if(create_info->size == 0);
  return_ERROR_INVALID_ARGUMENT_if(create_info->num_required_components > 0 &&
                                   create_info->required_components == NULL);

  for (uint32_t i = 0; i < create_info->num_required_components; i++) {
    return_ERROR_INVALID_ARGUMENT_if(create_info->required_components[i] >=
                                     engine_ecs_component.length);
  }

  ecs_ComponentHandle *required_components = NULL;

  if (create_info->num_required_components > 0) {
    ALLOC(create_info->num_required_components, required_components);
    memcpy(required_components, create_info->required_components,
           create_info->num_required_components * sizeof(*required_components));
  }

  struct ecs_Component component_data = {
      .flags = create_info->flags,
      .size = create_info->size,
      .num_required_components = create_info->num_required_components,
      .required_components = required_components,
      .init = create_info->init,
      .cleanup = create_info->cleanup};

  if (!Vector_space_for(&engine_ecs_component, 1)) {
    return ECS_ERROR_OUT_OF_MEMORY;
  }

  *component_ptr = engine_ecs_component.length;

  *Vector_push(&engine_ecs_component) = component_data;

  return ECS_SUCCESS;
}

static inline void *ecs_raw_access(uint32_t archetype_index,
                                   uint32_t component_index, uint32_t page,
                                   uint32_t index);

static inline void *ecs_write(uint32_t entity_index, uint32_t component_index);

ecs_Result ecs_init_component(uint32_t entity_index, uint32_t archetype_index,
                              uint32_t component_index) {
  struct ecs_Component *component = &engine_ecs_component.data[component_index];

  if (component->init == NULL) {
    return ECS_SUCCESS;
  }

  uint32_t page = ENTITY_ARCHETYPE_CODE_PAGE(entity_index);
  uint32_t index = ENTITY_ARCHETYPE_CODE_INDEX(entity_index);

  void **pointers;

  ALLOC(1 + component->num_required_components, pointers);
  struct ecs_Archetype *archetype = &engine_ecs_archetype.data[archetype_index];

  pointers[0] = ecs_raw_access(entity_index, component_index, page, index);

  for (uint32_t i = 0; i < component->num_required_components; i++) {
    uint32_t rindex = ecs_ComponentSet_order_of(
        &archetype->components, component->required_components[i]);
    pointers[1 + i] = ecs_raw_access(entity_index, rindex, page, index);
  }

  component->init(component->user_data, ecs_construct_entity_handle_index_only(entity_index), pointers);

  FREE(1 + component->num_required_components, pointers);

  return ECS_SUCCESS;
}

ecs_Result ecs_init_components(uint32_t entity_index,
                               uint32_t archetype_index) {
  struct ecs_Archetype *archetype = &engine_ecs_archetype.data[archetype_index];

  if (!archetype->any_init) {
    return ECS_SUCCESS;
  }

  uint32_t page = ENTITY_ARCHETYPE_CODE_PAGE(entity_index);
  uint32_t index = ENTITY_ARCHETYPE_CODE_INDEX(entity_index);

  void **pointers1;
  void **pointers2;

  ALLOC(archetype->components.count, pointers1);
  ALLOC(archetype->components.count, pointers2);

  for (uint32_t i = 0; i < archetype->components.count; i++) {
    pointers1[i] = ecs_raw_access(entity_index, i, page, index);
  }

  for (uint32_t i = 0; i < archetype->components.count; i++) {
    uint32_t component_index = archetype->components.index[i];
    struct ecs_Component *component =
        &engine_ecs_component.data[component_index];
    if (component->init == NULL) {
      continue;
    }
    for (uint32_t j = 0; j < 1 + component->num_required_components; j++) {
      uint32_t rindex =
          j ? ecs_ComponentSet_order_of(&archetype->components,
                                        component->required_components[j - 1])
            : i;
      pointers2[j] = pointers1[rindex];
    }
    component->init(component->user_data, ecs_construct_entity_handle_index_only(entity_index),
                 pointers2);
  }

  FREE(archetype->components.count, pointers1);
  FREE(archetype->components.count, pointers2);

  return ECS_SUCCESS;
}

ecs_Result ecs_cleanup_component(uint32_t entity_index,
                                 uint32_t archetype_index,
                                 uint32_t component_index) {
  struct ecs_Component *component = &engine_ecs_component.data[component_index];

  if (component->cleanup == NULL) {
    return ECS_SUCCESS;
  }

  uint32_t page = ENTITY_ARCHETYPE_CODE_PAGE(entity_index);
  uint32_t index = ENTITY_ARCHETYPE_CODE_INDEX(entity_index);

  void **pointers;

  ALLOC(1 + component->num_required_components, pointers);
  struct ecs_Archetype *archetype = &engine_ecs_archetype.data[archetype_index];

  pointers[0] = ecs_raw_access(entity_index, component_index, page, index);

  for (uint32_t i = 0; i < component->num_required_components; i++) {
    uint32_t rindex = ecs_ComponentSet_order_of(
        &archetype->components, component->required_components[i]);
    pointers[1 + i] = ecs_raw_access(entity_index, rindex, page, index);
  }

  component->cleanup(component->user_data,
               ecs_construct_entity_handle_index_only(entity_index), pointers);

  FREE(1 + component->num_required_components, pointers);

  return ECS_SUCCESS;
}

ecs_Result ecs_cleanup_components(uint32_t entity_index,
                                  uint32_t archetype_index) {
  struct ecs_Archetype *archetype = &engine_ecs_archetype.data[archetype_index];

  if (!archetype->any_cleanup) {
    return ECS_SUCCESS;
  }

  uint32_t page = ENTITY_ARCHETYPE_CODE_PAGE(entity_index);
  uint32_t index = ENTITY_ARCHETYPE_CODE_INDEX(entity_index);

  void **pointers1;
  void **pointers2;

  ALLOC(archetype->components.count, pointers1);
  ALLOC(archetype->components.count, pointers2);

  for (uint32_t i = 0; i < archetype->components.count; i++) {
    pointers1[i] = ecs_raw_access(entity_index, i, page, index);
  }

  for (uint32_t i = 0; i < archetype->components.count; i++) {
    uint32_t component_index = archetype->components.index[i];
    struct ecs_Component *component =
        &engine_ecs_component.data[component_index];
    if (component->cleanup == NULL) {
      continue;
    }
    for (uint32_t j = 0; j < 1 + component->num_required_components; j++) {
      uint32_t rindex =
          j ? ecs_ComponentSet_order_of(&archetype->components,
                                        component->required_components[j - 1])
            : i;
      pointers2[j] = pointers1[rindex];
    }
    component->cleanup(component->user_data,
                 ecs_construct_entity_handle_index_only(entity_index),
                 pointers2);
  }

  FREE(archetype->components.count, pointers1);
  FREE(archetype->components.count, pointers2);

  return ECS_SUCCESS;
}

static int _compar_component_index(const void *ap, const void *bp, void *ud) {
  uint32_t a = *(uint32_t *)ap;
  uint32_t b = *(uint32_t *)bp;
  return a - b;
}

ecs_Result ecs_ComponentSet_init_0(ecs_ComponentSet *set, uint32_t a_count,
                                   const uint32_t *a_component_indexes,
                                   uint32_t b_count,
                                   const uint32_t *b_component_indexes) {
  set->count = a_count + b_count;
  set->index = NULL;
  if (set->count == 0) {
    return ECS_SUCCESS;
  }
  ALLOC(set->count, set->index);
  if (a_count > 0) {
    memcpy(set->index, a_component_indexes, a_count * sizeof(*set->index));
  }
  if (b_count > 0) {
    memcpy(set->index + a_count, b_component_indexes,
           b_count * sizeof(*set->index));
  }
  sort_qsort(set->index, set->count, sizeof(*set->index), _compar_component_index,
        NULL);
  return ECS_SUCCESS;
}

ecs_Result ecs_ComponentSet_init(ecs_ComponentSet *set, uint32_t count,
                                 const ecs_ComponentHandle *components) {
  // if aeComponent ever changes, need to translate to the indexes
  return ecs_ComponentSet_init_0(set, count, components, 0, NULL);
}

ecs_Result ecs_ComponentSet_add(ecs_ComponentSet *dst,
                                const ecs_ComponentSet *src,
                                ecs_ComponentHandle component) {
  // if aeComponent ever changes, need to translate to the indexes
  return ecs_ComponentSet_init_0(dst, src->count, src->index, 1, &component);
}

ecs_Result ecs_ComponentSet_remove(ecs_ComponentSet *dst,
                                   const ecs_ComponentSet *src,
                                   ecs_ComponentHandle component) {
  // if aeComponent ever changes, need to translate to the indexes
  uint32_t order = ecs_ComponentSet_order_of(src, component);
  if (order == UINT32_MAX) {
    return ecs_ComponentSet_init_0(dst, src->count, src->index, 0, NULL);
  } else {
    return ecs_ComponentSet_init_0(
        dst, order, src->index, src->count - order - 1, src->index + order + 1);
  }
}

uint32_t ecs_ComponentSet_order_of(const ecs_ComponentSet *set,
                                   ecs_ComponentHandle component) {
  // if aeComponent ever changes, need to translate to the indexes
  uint32_t *p = sort_bsearch(&component, set->index, set->count, sizeof(*set->index),
                        _compar_component_index, NULL);
  if (p == NULL) {
    return UINT32_MAX;
  }
  return (uint32_t)(p - set->index);
}

int ecs_ComponentSet_contains(const struct ecs_ComponentSet *set,
                              ecs_ComponentHandle component) {
  return ecs_ComponentSet_order_of(set, component) != UINT32_MAX;
}

int ecs_ComponentSet_is_subset(const ecs_ComponentSet *set,
                               const ecs_ComponentSet *subset) {
  for (uint32_t s = 0, ss = 0; ss < subset->count; ++ss) {
    while (s < set->count && set->index[s] < subset->index[ss])
      s++;
    if (s >= set->count) {
      return 0;
    }
    if (subset->index[ss] != set->index[s]) {
      return 0;
    }
  }
  return 1;
}

int ecs_ComponentSet_intersects(const ecs_ComponentSet *aset,
                                const ecs_ComponentSet *bset) {
  uint32_t ai = 0, bi = 0;
  for (; ai < aset->count; ai++) {
    while (bi < bset->count && bset->index[bi] < aset->index[ai])
      bi++;
    if (bi >= bset->count) {
      return 0;
    }
    if (bset->index[bi] == aset->index[ai]) {
      return 1;
    }
  }
  return 0;
}

ecs_Result ecs_ComponentSet_expand_required(ecs_ComponentSet *set) {
top:

  for (uint32_t i = 0; i < set->count; i++) {
    ecs_ComponentHandle c = set->index[i];

    if (engine_ecs_component.data[c].num_required_components > 0) {
      for (uint32_t j = 0;
           j < engine_ecs_component.data[c].num_required_components; j++) {
        ecs_ComponentHandle r =
            engine_ecs_component.data[c].required_components[j];

        if (!ecs_ComponentSet_contains(set, r)) {
          ecs_ComponentSet new_set;
          return_if_ERROR(ecs_ComponentSet_add(&new_set, set, r));
          ecs_ComponentSet_free(set);
          *set = new_set;
          goto top;
        }
      }
    }
  }

  return ECS_SUCCESS;
}

void ecs_ComponentSet_free(ecs_ComponentSet *set) {
  if (set->index != NULL) {
    FREE(set->count, set->index);
  }
}
