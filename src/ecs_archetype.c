#include "ecs_local.h"

#include "sort.h"

static int _compare_components(
    uint32_t         a_num_components
  , const uint32_t * a_component_indexes
  , uint32_t         b_num_components
  , const uint32_t * b_component_indexes
) {
  if(a_num_components != b_num_components) {
    return (int)a_num_components - (int)b_num_components;
  }
  if(a_num_components > 0) {
    return memcmp(a_component_indexes, b_component_indexes, sizeof(*a_component_indexes) * a_num_components);
  } else {
    return 0;
  }
}

static int _sort(const void * ap, const void * bp, void * ud) {
  uint32_t a_index = *(uint32_t *)ap;
  uint32_t b_index = *(uint32_t *)bp;
  ecs_Archetype * a = &engine_ecs_archetype.data[a_index];
  ecs_Archetype * b = &engine_ecs_archetype.data[b_index];
  return _compare_components(a->components.count, a->components.index, b->components.count, b->components.index);
}

static int _search(const void * ap, const void * bp, void * ud) {
  const ecs_ComponentSet * components = (const ecs_ComponentSet *)ap;
  uint32_t archetype_index = *(uint32_t *)bp;
  const ecs_Archetype * archetype = &engine_ecs_archetype.data[archetype_index];
  return _compare_components(components->count, components->index, archetype->components.count, archetype->components.index);
}

ecs_Result ecs_resolve_archetype(
    ecs_ComponentSet      components
  , ecs_ArchetypeHandle * out_ptr
) {
  return_if_ERROR(ecs_ComponentSet_expand_required(&components));
  
  uint32_t * index_ptr = sort_bsearch(
      &components
    , engine_ecs_archetype.components_index
    , engine_ecs_archetype.length
    , sizeof(*engine_ecs_archetype.components_index)
    , _search, NULL
  );

  if(index_ptr != NULL) {
    *out_ptr = *index_ptr;
    ecs_ComponentSet_free(&components);
    return ECS_SUCCESS;
  }

  uint32_t archetype_index = engine_ecs_archetype.length++;

  if(engine_ecs_archetype.length > engine_ecs_archetype.capacity) {
    uint32_t old_capacity = engine_ecs_archetype.capacity;
    uint32_t new_capacity = engine_ecs_archetype.length + (engine_ecs_archetype.length >> 1);
    RELOC(old_capacity, new_capacity, engine_ecs_archetype.components_index);
    RELOC(old_capacity, new_capacity, engine_ecs_archetype.data);
    engine_ecs_archetype.capacity = new_capacity;
  }

  ecs_Archetype * archetype = &engine_ecs_archetype.data[archetype_index];

  archetype->components = components;

  size_t * sizes = memory_alloc(sizeof(size_t) * (1 + components.count), alignof(*sizes));

  sizes[0] = sizeof(uint32_t);
  for(uint32_t i = 0; i < components.count; i++) {
    sizes[i + 1] = engine_ecs_component.data[components.index[i]].size;

    archetype->any_init |= engine_ecs_component.data[components.index[i]].init != NULL;
    archetype->any_cleanup |= &engine_ecs_component.data[components.index[i]].cleanup != NULL;
  }

  PagedSOA_initialize(&archetype->paged_soa, sizeof(uint32_t), 1 + components.count, sizes);

  memory_free(sizes, sizeof(size_t) * (1 + components.count), alignof(*sizes));

  engine_ecs_archetype.components_index[archetype_index] = archetype_index;
  sort_qsort(
      engine_ecs_archetype.components_index
    , engine_ecs_archetype.length
    , sizeof(*engine_ecs_archetype.components_index)
    , _sort, NULL );

  *out_ptr = archetype_index;

  return ECS_SUCCESS;
}

static ecs_Result _allocate_code(
    uint32_t             archetype_index
  , uint32_t           * archetype_code
) {
  ecs_Archetype * archetype = &engine_ecs_archetype.data[archetype_index];

  uint32_t code;

  if(archetype->free_codes.length > 0) {
    code = *Vector_pop(&archetype->free_codes);
  } else {
    PagedSOA_space_for(&archetype->paged_soa, 1);
    code = PagedSOA_push(&archetype->paged_soa);
  }

  *(uint32_t *)PagedSOA_page(&archetype->paged_soa, code) += 1;

  *archetype_code = code;

  return ECS_SUCCESS;
}

static ecs_Result _free_code(
    uint32_t             archetype_index
  , uint32_t             code
) {
  ecs_Archetype * archetype = &engine_ecs_archetype.data[archetype_index];

  if(!Vector_space_for(&archetype->free_codes, 1)) {
    return ECS_ERROR_OUT_OF_MEMORY;
  }
  
  *Vector_push(&archetype->free_codes) = code;

  *(uint32_t *)PagedSOA_page(&archetype->paged_soa, code) -= 1;

  return ECS_SUCCESS;
}

ecs_Result ecs_unset_entity_archetype(
    uint32_t             entity_index
) {
  uint32_t archetype_index = ENTITY_ARCHETYPE_INDEX(entity_index);
  uint32_t archetype_code = ENTITY_ARCHETYPE_CODE(entity_index);
  
  ENTITY_DATA_ENTITY_INDEX(entity_index) = 0;
  ENTITY_ARCHETYPE_INDEX(entity_index) = 0;
  ENTITY_ARCHETYPE_CODE(entity_index) = 0;

  return _free_code(archetype_index, archetype_code);
}

ecs_Result ecs_set_entity_archetype(
    uint32_t             entity_index
  , uint32_t             archetype_index
) {
  ecs_Archetype * archetype = &engine_ecs_archetype.data[archetype_index];

  if(ENTITY_ARCHETYPE_INDEX(entity_index) == 0) {
    uint32_t code;
    return_if_ERROR(_allocate_code(archetype_index, &code));
    ENTITY_ARCHETYPE_INDEX(entity_index) = archetype_index;
    ENTITY_ARCHETYPE_CODE(entity_index) = code;
    ENTITY_DATA_ENTITY_INDEX(entity_index) = entity_index;
    return ECS_SUCCESS;
  }

  ENTITY_DATA_ENTITY_INDEX(entity_index) = 0;

  uint32_t old_archetype_index = ENTITY_ARCHETYPE_INDEX(entity_index);
  uint32_t old_code = ENTITY_ARCHETYPE_CODE(entity_index);
  ecs_Archetype * old_archetype = &engine_ecs_archetype.data[old_archetype_index];
  uint32_t old_block_index = old_code >> 16;
  uint32_t old_block_offset = old_code & 0xFFFF;

  return_if_ERROR(_free_code(old_archetype_index, old_code));

  uint32_t code;
  return_if_ERROR(_allocate_code(archetype_index, &code));
  uint32_t block_index = code >> 16;
  uint32_t block_offset = code & 0xFFFF;
  
  ENTITY_ARCHETYPE_INDEX(entity_index) = archetype_index;
  ENTITY_ARCHETYPE_CODE(entity_index) = code;
  ENTITY_DATA_ENTITY_INDEX(entity_index) = entity_index;

  uint32_t r = 0;
  uint32_t w = 0;

  while(w < archetype->components.count && r < old_archetype->components.count) {
    ecs_ComponentHandle c = archetype->components.index[w];

    while(r < old_archetype->components.count && old_archetype->components.index[r] < c) {
      r++;
    }

    if(r < old_archetype->components.count && old_archetype->components.index[r] == c) {
      void * old_data = ecs_raw_access(old_archetype_index, r, old_block_index, old_block_offset);
      void *     data = ecs_raw_access(    archetype_index, w,     block_index,     block_offset);
      memcpy(data, old_data, archetype->paged_soa.size_offset[w + 1] >> 16);
      
      r++;
    }

    w++;
  }
  
  return ECS_SUCCESS;
}

