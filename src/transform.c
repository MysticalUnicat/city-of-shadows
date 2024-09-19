#include "transform.h"

ECS_COMPONENT(LocalToWorld2D)

ECS_COMPONENT(Transform2D, requires(LocalToWorld2D))

ECS_COMPONENT(Translation2D, requires(Transform2D))

ECS_COMPONENT(Rotation2D, requires(Transform2D))

ECS_COMPONENT(Parent2D)

ECS_QUERY(translation_query, read(Translation2D, translation), modified(Translation2D), exclude(Rotation2D), write(Transform2D, transform), action(
  transform->value = pga2d_translator(1, translation->value);
))

ECS_QUERY(rotation_query, exclude(Translation2D), read(Rotation2D, rotation), modified(Rotation2D), write(Transform2D, transform), action(
  transform->value = pga2d_rotor(rotation->value);
))

ECS_QUERY(translation_rotation_query, read(Translation2D, translation), modified(Translation2D), read(Rotation2D, rotation), modified(Rotation2D), write(Transform2D, transform), action(
  transform->value = pga2d_motor(rotation->value, 1, translation->value);
))

ECS_QUERY(parent_world_query, read(Transform2D, transform), write(LocalToWorld2D, local_to_world), modified(Transform2D), exclude(Parent2D), action(
  local_to_world->motor = transform->value;
  local_to_world->position = pga2d_sandwich_bm(pga2d_point(0, 0), local_to_world->motor);
  local_to_world->orientation = asinf(local_to_world->motor.e12) * 2;
))

ECS_QUERY(child_world_query, read(Transform2D, transform), read(Parent2D, parent), write(LocalToWorld2D, local_to_world), modified(Transform2D), modified(Parent2D), action(
  const struct LocalToWorld2D *parent_local_to_world = LocalToWorld2D_read(parent->value);
  local_to_world->motor = pga2d_mul_mm(parent_local_to_world->motor, transform->value);
  local_to_world->position = pga2d_sandwich_bm(pga2d_point(0, 0), local_to_world->motor);
  local_to_world->orientation = asinf(local_to_world->motor.e12) * 2;
))

void transform_update2d_serial(void) {
  translation_query();
  rotation_query();
  translation_rotation_query();
  parent_world_query();
  child_world_query();
}

