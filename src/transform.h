#ifndef transform_h_INCLUDED
#define transform_h_INCLUDED

#include "math.h"
#include "ecs.h"

ECS_DECLARE_COMPONENT(Translation2D, {
  pga2d_Direction value;
})

ECS_DECLARE_COMPONENT(Rotation2D, {
  R value;
})

ECS_DECLARE_COMPONENT(Transform2D, {
  union {
    pga2d_Motor value;
    pga2d_Motor M;
  };
})

ECS_DECLARE_COMPONENT(LocalToWorld2D, {
  pga2d_Motor motor;

  pga2d_Point position;
  R orientation;
})

ECS_DECLARE_COMPONENT(Parent2D, {
  ecs_EntityHandle value;
})

ecs_ComponentHandle Translation2D_component(void);
ecs_ComponentHandle Rotation2D_component(void);
ecs_ComponentHandle Transform2D_component(void);
ecs_ComponentHandle LocalToWorld2D_component(void);
ecs_ComponentHandle Parent2D_component(void);

void transform_update2d_serial(void);

#endif // transform_h_INCLUDED
