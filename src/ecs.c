#include "ecs_local.h"

#include "memory.h"

struct ecs_global_Layer engine_ecs_layer;
struct ecs_global_Entity engine_ecs_entity;
struct ecs_global_Archetype engine_ecs_archetype;
struct ecs_global_Component engine_ecs_component;

ecs_Result ecs_initialize(void) {
  memory_clear(&engine_ecs_archetype, sizeof(engine_ecs_archetype));
  memory_clear(&engine_ecs_component, sizeof(engine_ecs_component));
  memory_clear(&engine_ecs_entity, sizeof(engine_ecs_entity));
  memory_clear(&engine_ecs_layer, sizeof(engine_ecs_layer));

  {
    ecs_EntityHandle e;
    ecs_create_entity(&e);
    ecs_ComponentSet cs;
    ecs_ComponentSet_init(&cs, 0, NULL);
    ecs_ArchetypeHandle a;
    ecs_resolve_archetype(cs, &a);
    ecs_set_entity_archetype(e, a);
  }

  return ECS_SUCCESS;
}

void ecs_shutdown(void) {
  if(engine_ecs_layer.capacity > 0) {
    Vector_free(&engine_ecs_layer.free_indexes);

    ecs_free(engine_ecs_layer.generation, sizeof(*engine_ecs_layer.generation) * engine_ecs_layer.capacity, alignof(*engine_ecs_layer.generation));

    for(uint32_t i = 0; i < engine_ecs_layer.length; i++) {
      Vector_free(&engine_ecs_layer.data[i].entities);
    }
    ecs_free(engine_ecs_layer.data, sizeof(*engine_ecs_layer.data) * engine_ecs_layer.capacity, alignof(*engine_ecs_layer.data));
  }

  for(uint32_t i = 0; i < engine_ecs_component.length; i++) {
    FREE(engine_ecs_component.data[i].num_required_components, engine_ecs_component.data[i].required_components);
  }
  Vector_free(&engine_ecs_component);

  FREE(engine_ecs_entity.capacity, engine_ecs_entity.generation);
  FREE(engine_ecs_entity.capacity, engine_ecs_entity.layer_index);
  FREE(engine_ecs_entity.capacity, engine_ecs_entity.archetype_index);
  FREE(engine_ecs_entity.capacity, engine_ecs_entity.archetype_code);

  for(uint32_t i = 0; i < engine_ecs_archetype.length; i++) {
    ecs_Archetype * archetype = &engine_ecs_archetype.data[i];
    ecs_ComponentSet_free(&archetype->components);
    Vector_free(&archetype->free_codes);
    PagedSOA_free(&archetype->paged_soa);
  }
  FREE(engine_ecs_archetype.capacity, engine_ecs_archetype.components_index);
  FREE(engine_ecs_archetype.capacity, engine_ecs_archetype.data);
}

