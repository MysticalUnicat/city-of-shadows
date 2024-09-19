#ifndef camera_h_INCLUDED
#define camera_h_INCLUDED

#include "ecs.h"
#include "math.h"

ECS_DECLARE_COMPONENT(Camera, {
  pga2d_Point viewport_min;
  pga2d_Point viewport_max;
  R zoom;
})

#endif // camera_h_INCLUDED
