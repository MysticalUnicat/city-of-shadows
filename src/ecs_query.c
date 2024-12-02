#include "ecs_local.h"
#include "log.h"

#include <stdbool.h>
#include <stdio.h>

#define MAX(A, B) ((A) > (B) ? (A) : (B))

ecs_Result ecs_create_query(const ecs_QueryCreateInfo *create_info,
                            ecs_Query **query_ptr) {
  ecs_Query *query;

  ALLOC(1, query);

  query->component_count =
      create_info->num_write_components + create_info->num_read_components;
  query->first_component_read = create_info->num_write_components;

  ALLOC(query->component_count, query->component);
  ALLOC(query->component_count, query->runtime);

  uint32_t ci = 0;
  for (uint32_t i = 0; i < create_info->num_write_components; i++) {
    query->component[ci] = create_info->write_components[i];
    ci++;
  }
  return_if_ERROR(ecs_ComponentSet_init(
      &query->write_component_set, query->component_count, query->component));

  for (uint32_t i = 0; i < create_info->num_read_components; i++) {
    query->component[ci] = create_info->read_components[i];
    ci++;
  }
  return_if_ERROR(ecs_ComponentSet_init(
      &query->component_set, query->component_count, query->component));

  Vector(uint32_t) other = {0};
  Vector_set_capacity(&other,
                      query->component_count + create_info->num_filters);

  for (uint32_t i = 0; i < query->component_count; i++) {
    bool optional = false;
    for (uint32_t j = 0; j < create_info->num_filters; j++) {
      if (create_info->filters[j].filter == ECS_FILTER_OPTIONAL &&
          create_info->filters[j].component == query->component[i]) {
        optional = true;
        break;
      }
    }
    if (optional) {
      continue;
    }
    *Vector_push(&other) = query->component[i];
  }
  for (uint32_t j = 0; j < create_info->num_filters; j++) {
    if (create_info->filters[j].filter != ECS_FILTER_MODIFIED) {
      continue;
    }
    if (!ecs_ComponentSet_contains(&query->component_set,
                                   create_info->filters[j].component)) {
      *Vector_push(&other) = create_info->filters[j].component;
    }
  }
  return_if_ERROR(ecs_ComponentSet_init(&query->require_component_set,
                                        other.length, other.data));

  other.length = 0;
  for (uint32_t j = 0; j < create_info->num_filters; j++) {
    if (create_info->filters[j].filter == ECS_FILTER_EXCLUDE) {
      *Vector_push(&other) = create_info->filters[j].component;
    }
  }
  return_if_ERROR(ecs_ComponentSet_init(&query->exclude_component_set,
                                        other.length, other.data));

  other.length = 0;
  for (uint32_t j = 0; j < create_info->num_filters; j++) {
    if (create_info->filters[j].filter == ECS_FILTER_MODIFIED) {
      *Vector_push(&other) = create_info->filters[j].component;
    }
  }
  return_if_ERROR(ecs_ComponentSet_init(&query->modified_component_set,
                                        other.length, other.data));

  Vector_free(&other);

  *query_ptr = query;

  return ECS_SUCCESS;
}

static ecs_Result ecs_update_query(ecs_Query *query) {
  return_ERROR_INVALID_ARGUMENT_if(query == NULL);

  for (; query->last_archetype_tested < engine_ecs_archetype.length;
       query->last_archetype_tested++) {
    ecs_Archetype *archetype =
        &engine_ecs_archetype.data[query->last_archetype_tested];

    bool skip = false;

    if (!ecs_ComponentSet_is_subset(&archetype->components,
                                    &query->require_component_set)) {
      skip = true;
    }

    if (!skip && query->exclude_component_set.count > 0 &&
        ecs_ComponentSet_intersects(&archetype->components,
                                    &query->exclude_component_set)) {
      skip = true;
    }

    if (skip) {
      continue;
    }

    if (query->archetype_length + 1 >= query->archetype_capacity) {
      uint32_t old_capacity = query->archetype_capacity;
      uint32_t new_capacity = query->archetype_length + 1;
      new_capacity += new_capacity >> 1;
      RELOC(old_capacity, new_capacity, query->archetype);
      RELOC((1 + query->component_count) * old_capacity,
            (1 + query->component_count) * new_capacity, query->size_offset);
      RELOC(query->write_component_set.count * old_capacity,
            query->write_component_set.count * new_capacity,
            query->write_index);
      RELOC(query->modified_component_set.count * old_capacity,
            query->modified_component_set.count * new_capacity,
            query->modified_index);
      RELOC(query->modified_component_set.count * old_capacity,
            query->modified_component_set.count * new_capacity,
            query->modified_last_seen_write);
      query->archetype_capacity = new_capacity;
    }

    uint32_t index = query->archetype_length++;
    query->archetype[index] = query->last_archetype_tested;

    for (uint32_t k = 0; k < query->write_component_set.count; k++) {
      uint32_t archetype_component_index = ecs_ComponentSet_order_of(
          &archetype->components, query->write_component_set.index[k]);
      if (archetype_component_index != UINT32_MAX) {
        query->write_index[index * query->write_component_set.count + k] =
            archetype_component_index;
      }
    }

    for (uint32_t k = 0; k < query->modified_component_set.count; k++) {
      uint32_t archetype_component_index = ecs_ComponentSet_order_of(
          &archetype->components, query->modified_component_set.index[k]);
      if (archetype_component_index != UINT32_MAX) {
        query->modified_index[index * query->modified_component_set.count + k] =
            archetype_component_index;
      }
    }

    query->size_offset[index * query->component_count + 0] =
        archetype->paged_soa.size_offset[0];

    for (uint32_t k = 0; k < query->component_count; k++) {
      uint32_t archetype_component_index = ecs_ComponentSet_order_of(
          &archetype->components, query->component[k]);
      if (archetype_component_index != UINT32_MAX) {
        query->size_offset[index * query->component_count + (1 + k)] =
            archetype->paged_soa.size_offset[archetype_component_index + 1];
      }
    }
  }

  return ECS_SUCCESS;
}

ecs_Result ecs_execute_query(ecs_Query *query, ecs_QueryFunction cb, void *ud) {
  return_ERROR_INVALID_ARGUMENT_if(query == NULL);
  return_ERROR_INVALID_ARGUMENT_if(cb == NULL);

  return_if_ERROR(ecs_update_query(query));

  uint8_t **runtime = query->runtime;
  const uint32_t *size_offset = query->size_offset;
  const uint32_t *write_index = query->write_index;
  const uint32_t *modified_index = query->modified_index;
  uint32_t *modified_last_seen_write = query->modified_last_seen_write;
  const ecs_ArchetypeHandle *archetype_handle = query->archetype;
  for (uint32_t i = 0; i < query->archetype_length; i++) {
    const ecs_Archetype *archetype =
        &engine_ecs_archetype.data[*archetype_handle];

    // if the query watches modification, see if the archetype has changes
    bool skip = query->modified_component_set.count > 0;
    for(uint32_t k = 0; k < query->modified_component_set.count; k++) {
      if(modified_last_seen_write[k] != archetype->write[modified_index[k]]) {
        skip = false;
      }
      modified_last_seen_write[k] = archetype->write[modified_index[k]];
    }

    if(!skip) {
      // mark the archetype's component version
      for (uint32_t j = 0; j < query->write_component_set.count; j++) {
        archetype->write[write_index[j]] += 1;
      }

      for (uint32_t j = 0; j < archetype->paged_soa.num_pages; j++) {
        uint8_t *page = archetype->paged_soa.pages[j];
        uint32_t *live_count = (uint32_t *)page;
        uint32_t * page_last_seen_write = live_count + 1;
        if (page == NULL || *live_count == 0) {
          break;
        }

        // if the query watches modification, see if the page has changes
        skip = query->modified_component_set.count > 0;
        for(uint32_t k = 0; k < query->modified_component_set.count; k++) {
          if(page_last_seen_write[modified_index[k]] != modified_last_seen_write[k]) {
            skip = false;
            break;
          }
        }

        if(!skip) {
          // mark the page written with archetype's component version
          for (uint32_t k = 0; k < query->write_component_set.count; k++) {
            page_last_seen_write[write_index[k]] = archetype->write[write_index[k]];
          }

          const uint32_t *entity = (const uint32_t *)(page + (size_offset[0] & 0xFFFF));
          for (uint32_t c = 0, C = query->component_count; c < C; c++) {
            runtime[c] =
                size_offset[1 + c] ? page + (size_offset[1 + c] & 0xFFFF) : NULL;
          }
          for (uint32_t k = 0, K = *live_count; k < K;) {
            if (*entity) {
              cb(ud, *entity, (void **)runtime);
              k++;
            }
            entity++;
            for (uint32_t c = 0, C = query->component_count; c < C; c++) {
              runtime[c] += size_offset[1 + c] >> 16;
            }
          }
        }
      }
    }

    archetype_handle++;
    size_offset += 1 + query->component_count;
    write_index += query->write_component_set.count;
    modified_index += query->modified_component_set.count;
    modified_last_seen_write += query->modified_component_set.count;
  }

  return ECS_SUCCESS;
}

void ecs_destroy_query(ecs_Query *query) {
  if (query == NULL) {
    return;
  }
  ecs_ComponentSet_free(&query->component_set);
  ecs_ComponentSet_free(&query->require_component_set);
  ecs_ComponentSet_free(&query->exclude_component_set);
  FREE(query->component_count, query->component);
  FREE(query->component_count, query->runtime);
  FREE(query->archetype_length, query->archetype);
  FREE(query->archetype_length * (1 + query->component_count),
       query->size_offset);
  FREE(query->archetype_length * query->write_component_set.count,
       query->write_index);
  FREE(query->archetype_length * query->modified_component_set.count,
       query->modified_index);
  FREE(query->archetype_length * query->modified_component_set.count,
       query->modified_last_seen_write);
  FREE(1, query);
}
