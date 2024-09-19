#ifndef physics_h_INCLUDED
#define physics_h_INCLUDED

#include "ecs.h"
#include "math.h"

// an Entity without linear motion is not movable (static)
ECS_DECLARE_COMPONENT(Physics2DMotion, {
  union {
    pga2d_Bivector value;
    pga2d_Bivector B;
  };
})

// an Entity without mass is not movable by physics (kinematic)
ECS_DECLARE_COMPONENT(Physics2DMass, {
  R value;
})

ECS_DECLARE_COMPONENT(Physics2DCollision, {
  uint32_t mask;
})

ECS_DECLARE_COMPONENT(Physics2DCircle, {
  R radius;
})

ECS_DECLARE_COMPONENT(Physics2DRectangle, {
  R width;
  R height;
})

ECS_DECLARE_COMPONENT(Physics2DGravity, {
  R value;
})

ECS_DECLARE_COMPONENT(Physics2DDampen, {
  R value;
})

ECS_DECLARE_COMPONENT(Physics2DBodyMotion, {
  union {
    pga2d_AntiBivector forque;
    pga2d_AntiBivector F;
  };
  pga2d_AntiBivector I;
})

#endif // physics_h_INCLUDED
