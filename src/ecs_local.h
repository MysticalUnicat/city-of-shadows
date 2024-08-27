#pragma once

#include "ecs.h"
#include "vector.h"
#include "paged_soa.h"

#include <stdlib.h>
#include <stdalign.h>
#include <string.h>

#define UNUSED(X) (void)X
#define sizeof_alignof(X) sizeof(X), alignof(X)

typedef uint32_t ecs_ArchetypeHandle;

typedef struct ecs_Layer {
  uint32_t dirty : 1;
  uint32_t at_max : 1;
  uint32_t _reserved : 30;
  Vector(uint32_t) entities;
} ecs_Layer;

typedef struct ecs_Component {
  union {
    struct {
      uint32_t non_null : 1;
      uint32_t _flags_unused : 31;
    };
    uint32_t flags;
  };
  uint32_t size;
  uint32_t num_required_components;
  const ecs_ComponentHandle * required_components;
  ecs_ComponentInit init;
  ecs_ComponentCleanup cleanup;
  void * user_data;
} ecs_Component;

typedef struct ecs_ComponentSet {
  uint32_t count;
  uint32_t * index;
} ecs_ComponentSet;

#define TOTAL_BLOCK_SIZE (1 << 16)
#define BLOCK_DATA_SIZE (TOTAL_BLOCK_SIZE - (sizeof(uint32_t) * 2))

typedef struct ecs_Archetype {
  ecs_ComponentSet components;
  uint32_t         any_init : 1;
  uint32_t         any_cleanup : 1;
  uint32_t         _reserved : 30;
  Vector(uint32_t) free_codes;
  PagedSOA         paged_soa;
} ecs_Archetype;

struct ecs_global_Layer {
  Vector(uint32_t) free_indexes;
  uint32_t capacity;
  uint32_t length;
  uint32_t * generation;
  ecs_Layer * data;
};

struct ecs_global_Entity {
  Vector(uint32_t) free_indexes;
  uint32_t capacity;
  uint32_t length;
  uint32_t * generation;
  uint32_t * layer_index;
  uint32_t * archetype_index;
  uint32_t * archetype_code;
}; 

// archetypes are 'sorted' by component sets
// created as needed
struct ecs_global_Archetype {
  uint32_t capacity;
  uint32_t length;
  uint32_t * components_index;
  ecs_Archetype * data;
};

struct ecs_global_Component {
  uint32_t capacity;
  uint32_t length;
  ecs_Component * data;
};

extern struct ecs_global_Layer engine_ecs_layer;
extern struct ecs_global_Entity engine_ecs_entity;
extern struct ecs_global_Archetype engine_ecs_archetype;
extern struct ecs_global_Component  engine_ecs_component;

struct ecs_Query {
  ecs_ComponentSet component_set;
  uint32_t last_archetype_tested;

  ecs_ComponentSet require_component_set;
  ecs_ComponentSet exclude_component_set;

  uint32_t component_count;

  uint32_t first_component_read;

  ecs_ComponentHandle * component;
  uint8_t ** runtime;

  uint32_t archetype_capacity;
  uint32_t archetype_length;

  ecs_ArchetypeHandle * archetype;

  uint32_t * size_offset;
};

// ============================================================================
#define ENTITY_GENERATION(E)             engine_ecs_entity.generation[E]
#define ENTITy_LAYER_INDEX(E)            engine_ecs_entity.layer_index[E]
#define ENTITY_ARCHETYPE_INDEX(E)        engine_ecs_entity.archetype_index[E]
#define ENTITY_ARCHETYPE_CODE(E)         engine_ecs_entity.archetype_code[E]
#define ENTITY_ARCHETYPE_CODE_PAGE(E)    (ENTITY_ARCHETYPE_CODE(E) >> 16)
#define ENTITY_ARCHETYPE_CODE_INDEX(E)   (ENTITY_ARCHETYPE_CODE(E) & 0xFFFF)
#define ENTITY_ARCHETYPE_DATA(E)         (&engine_ecs_archetype.data[ENTITY_ARCHETYPE_INDEX(E)])
#define ENTITY_DATA_BLOCK(E)             ENTITY_ARCHETYPE_DATA(E)->paged_soa.pages[ENTITY_ARCHETYPE_CODE_PAGE(E)]
#define ENTITY_DATA_BLOCK_DATA(E)        ((void *)(ENTITY_DATA_BLOCK(E) + sizeof(uint32_t)))
#define ENTITY_DATA_ENTITY_INDEX(E)      ((uint32_t *)ENTITY_DATA_BLOCK_DATA(E))[ENTITY_ARCHETYPE_CODE_INDEX(E)]

static inline ecs_EntityHandle ecs_construct_entity_handle(uint32_t generation, uint32_t index) {
  return ((uint64_t)generation << 32) | (uint64_t)index;
}

static inline ecs_EntityHandle ecs_construct_entity_handle_index_only(uint32_t index) {
  return ecs_construct_entity_handle(ENTITY_GENERATION(index), index);
}

static inline void * ecs_raw_access(
    uint32_t             archetype_index
  , uint32_t             component_index
  , uint32_t             page
  , uint32_t             index
) {
  ecs_Archetype * archetype = &engine_ecs_archetype.data[archetype_index];
  uint32_t size, offset;
  PagedSOA_decode_column(&archetype->paged_soa, component_index + 1, &size, &offset);
  return PagedSOA_raw_write(&archetype->paged_soa, page, index, size, offset);
}

static inline void * ecs_write(
    uint32_t             entity_index
  , uint32_t             component_index
) {
  uint32_t archetype_index = ENTITY_ARCHETYPE_INDEX(entity_index);
  uint32_t page = ENTITY_ARCHETYPE_CODE_PAGE(entity_index);
  uint32_t index = ENTITY_ARCHETYPE_CODE_INDEX(entity_index);
  return ecs_raw_access(archetype_index, component_index, page, index);
}

// ============================================================================
#define return_if_ERROR(C) do { ecs_Result __r = C; if(__r < ECS_SUCCESS) return __r; } while(0)
#define return_ERROR_INVALID_ARGUMENT_if(X) do { if(X) { return ECS_ERROR_INVALID_ARGUMENT; } } while(0)
#define ASSERT(X) do { if(!(X)) { return ECS_ERROR_INVALID_ARGUMENT; } } while(0)

// ============================================================================
// memory.c
ecs_Result ecs_malloc(
    size_t               size
  , size_t               alignment
  , void *             * out_ptr
);

ecs_Result ecs_realloc(
    void               * ptr
  , size_t               old_size
  , size_t               new_size
  , size_t               alignment
  , void *             * out_ptr
);

void ecs_free(
    void               * ptr
  , size_t               size
  , size_t               alignment
);

#define ALLOC(C, P)          return_if_ERROR(ecs_malloc((C) * sizeof_alignof(*P), (void **)&P))
#define RELOC(oldC, newC, P) return_if_ERROR(ecs_realloc(P, (oldC) * sizeof(*P), (newC) * sizeof_alignof(*P), (void **)&P))
#define FREE(C, P)           ecs_free((void *)P, (C) * sizeof_alignof(*P))

// ============================================================================
// component.c
ecs_Result ecs_init_component(uint32_t entity_index, uint32_t archetype_index, uint32_t component_index);
ecs_Result ecs_init_components(uint32_t entity_index, uint32_t archetype_index);
ecs_Result ecs_cleanup_component(uint32_t entity_index, uint32_t archetype_index, uint32_t component_index);
ecs_Result ecs_cleanup_components(uint32_t entity_index, uint32_t archetype_index);

ecs_Result ecs_ComponentSet_init(
    ecs_ComponentSet          * set
  , uint32_t                          count
  , const ecs_ComponentHandle * components
);

ecs_Result ecs_ComponentSet_add(
    ecs_ComponentSet       * dst
  , const ecs_ComponentSet * src
  , ecs_ComponentHandle      component
);

ecs_Result ecs_ComponentSet_remove(
    ecs_ComponentSet * dst
  , const ecs_ComponentSet * src
  , ecs_ComponentHandle component
);

uint32_t ecs_ComponentSet_order_of(
    const ecs_ComponentSet * set
  , ecs_ComponentHandle component
);

int ecs_ComponentSet_contains(
    const ecs_ComponentSet * set
  , ecs_ComponentHandle      component
);

int ecs_ComponentSet_is_subset(
    const ecs_ComponentSet * a
  , const ecs_ComponentSet * b
);

int ecs_ComponentSet_intersects(
    const ecs_ComponentSet * a
  , const ecs_ComponentSet * b
);

ecs_Result ecs_ComponentSet_expand_required(
    ecs_ComponentSet * set
);

void ecs_ComponentSet_free(
    ecs_ComponentSet * set
);

// ============================================================================
// layer.c
ecs_Result ecs_validate_layer_handle(
   ecs_LayerHandle      layer
  , uint32_t                 * index_ptr
);

ecs_Result ecs_set_entity_layer(
   uint32_t             entity_index
  , uint32_t             layer_index
);

void ecs_unset_entity_layer(
   uint32_t             entity_index
);

// ============================================================================
// entity.c
ecs_Result ecs_validate_entity_handle(
   ecs_EntityHandle     entity
  , uint32_t                 * index_ptr
);

ecs_Result ecs_create_entity(
   ecs_EntityHandle * entity_ptr
);

ecs_Result ecs_free_entity(
   uint32_t             entity_id
);

// ============================================================================
// archetype.c
ecs_Result ecs_resolve_archetype(
   ecs_ComponentSet      components
  , ecs_ArchetypeHandle * out_ptr
);

ecs_Result ecs_unset_entity_archetype(
   uint32_t             entity_index
);

ecs_Result ecs_set_entity_archetype(
    uint32_t             entity_index
  , uint32_t             archetype_index
);
