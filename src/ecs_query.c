#include "ecs_local.h"
#include "log.h"

#include <stdbool.h>
#include <stdio.h>

#define MAX(A, B) ((A) > (B) ? (A) : (B))

ecs_Result ecs_create_query(
    const ecs_QueryCreateInfo * create_info
  , ecs_Query *               * query_ptr
) {
  ecs_Query * query;

  ALLOC(1, query);

  query->component_count = create_info->num_write_components + create_info->num_read_components;
  query->first_component_read = create_info->num_write_components;

  ALLOC(query->component_count, query->component);
  ALLOC(query->component_count, query->runtime);

  uint32_t ci = 0;
  for(uint32_t i = 0; i < create_info->num_write_components; i++) {
    query->component[ci] = create_info->write_components[i];
    ci++;
  }

  for(uint32_t i = 0; i < create_info->num_read_components; i++) {
    query->component[ci] = create_info->read_components[i];
    ci++;
  }

  return_if_ERROR(ecs_ComponentSet_init(&query->component_set, query->component_count, query->component));

  Vector(uint32_t) other = { 0 };
  Vector_set_capacity(&other, query->component_count + create_info->num_filters);

  for(uint32_t i = 0; i < query->component_count; i++) {
    bool optional = false;
    for(uint32_t j = 0; j < create_info->num_filters; j++) {
      if(create_info->filters[j].filter == ECS_FILTER_OPTIONAL && create_info->filters[j].component == query->component[i]) {
        optional = true;
        break;
      }
    }
    if(optional) {
      continue;
    }
    *Vector_push(&other) = query->component[i];
  }
  for(uint32_t j = 0; j < create_info->num_filters; j++) {
    if(create_info->filters[j].filter != ECS_FILTER_MODIFIED) {
      continue;
    }
    if(!ecs_ComponentSet_contains(&query->component_set, create_info->filters[j].component)) {
      *Vector_push(&other) = create_info->filters[j].component;
    }
  }
  return_if_ERROR(ecs_ComponentSet_init(&query->require_component_set, other.length, other.data));

  other.length = 0;
  for(uint32_t j = 0; j < create_info->num_filters; j++) {
    if(create_info->filters[j].filter == ECS_FILTER_EXCLUDE) {
      *Vector_push(&other) = create_info->filters[j].component;
    }
  }
  return_if_ERROR(ecs_ComponentSet_init(&query->exclude_component_set, other.length, other.data));
  
  Vector_free(&other);

  *query_ptr = query;

  return ECS_SUCCESS;
}

#if 0
static void _print_component_set(ecs_ComponentSet * set) {
  printf("{");
  for(uint32_t i = 0; i < set->count; i++) {
    printf("%s%u", i == 0 ? "" : ", ", set->index[i]);
  }
  putchar('}');
}
#endif

static ecs_Result ecs_update_query(
  ecs_Query * query
) {
  return_ERROR_INVALID_ARGUMENT_if(query == NULL);

  for(; query->last_archetype_tested < engine_ecs_archetype.length; query->last_archetype_tested++) {
    ecs_Archetype * archetype = &engine_ecs_archetype.data[query->last_archetype_tested];

    bool skip = false;

    if(!ecs_ComponentSet_is_subset(&archetype->components, &query->require_component_set)) {
      skip = true;
    }

    if(!skip && query->exclude_component_set.count > 0 && ecs_ComponentSet_intersects(&archetype->components, &query->exclude_component_set)) {
      skip = true;
    }

    if(skip) {
      continue;
    }

    if(query->archetype_length + 1 >= query->archetype_capacity) {
      uint32_t old_capacity = query->archetype_capacity;
      uint32_t new_capacity = query->archetype_length + 1;
      new_capacity += new_capacity >> 1;
      RELOC(                         old_capacity,                          new_capacity, query->archetype);
      RELOC(query->component_count * old_capacity, query->component_count * new_capacity, query->size_offset);
      query->archetype_capacity = new_capacity;
    }

    uint32_t index = query->archetype_length++;
    query->archetype[index] = query->last_archetype_tested;

    for(uint32_t k = 0; k < query->component_count; k++) {
      uint32_t archetype_component_index = ecs_ComponentSet_order_of(&archetype->components, query->component[k]);
      if(archetype_component_index != UINT32_MAX) {
        query->size_offset[index * query->component_count + k] = archetype->paged_soa.size_offset[archetype_component_index + 1];
      }
    }
  }

  return ECS_SUCCESS;
}

ecs_Result ecs_execute_query(
    ecs_Query    * query
  , ecs_QueryFunction cb
  , void *ud
) {
  return_ERROR_INVALID_ARGUMENT_if(query == NULL);
  return_ERROR_INVALID_ARGUMENT_if(cb == NULL);

  return_if_ERROR(ecs_update_query(query));

  uint8_t ** runtime = query->runtime;
  const uint32_t * size_offset = query->size_offset;
  const ecs_ArchetypeHandle * archetype_handle = query->archetype;
  for(uint32_t i = 0; i < query->archetype_length; i++) {
    const ecs_Archetype * archetype = &engine_ecs_archetype.data[*archetype_handle];

    for(uint32_t j = 0; j < archetype->paged_soa.num_pages; j++) {
      uint8_t * page = archetype->paged_soa.pages[j];
      uint32_t * live_count = (uint32_t *)page;
      if(page == NULL) {
        break;
      }
      const uint32_t * entity = (const uint32_t *)(page + sizeof(uint32_t));
      for(uint32_t c = 0, C = query->component_count; c < C; c++) {
        runtime[c] = size_offset[c] ? page + (size_offset[c] & 0xFFFF) : NULL;
      }
      for(uint32_t k = 0, K = *live_count; k < K; ) {
        if(*entity) {
          cb(ud, *entity, (void **)runtime);
          k++;
        }
        entity++;
        for(uint32_t c = 0, C = query->component_count; c < C; c++) {
          runtime[c] += size_offset[c] >> 16;
        }
      }
    }

    archetype_handle++;
    size_offset += query->component_count;
  }

  return ECS_SUCCESS;
}

void ecs_destroy_query(
   ecs_Query    * query
) {
  if(query == NULL) {
    return;
  }
  ecs_ComponentSet_free(&query->component_set);
  ecs_ComponentSet_free(&query->require_component_set);
  ecs_ComponentSet_free(&query->exclude_component_set);
  FREE(                           query->component_count, query->component);
  FREE(                          query->component_count, query->runtime);
  FREE( query->archetype_length                         , query->archetype);
  FREE( query->archetype_length * query->component_count, query->size_offset);
  FREE(                                                1, query);
}

