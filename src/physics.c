#include "physics.h"

#include "transform.h"

ECS_COMPONENT(Physics2DMotion, requires(Transform2D))

ECS_COMPONENT(Physics2DMass, requires(Physics2DBodyMotion))

ECS_COMPONENT(Physics2DCollision)

ECS_COMPONENT(Physics2DCircle)

ECS_COMPONENT(Physics2DRectangle)

ECS_COMPONENT(Physics2DGravity, requires(Physics2DBodyMotion))

ECS_COMPONENT(Physics2DDampen, requires(Physics2DBodyMotion))

ECS_COMPONENT(Physics2DBodyMotion, requires(Physics2DMotion))

ECS_QUERY(transform_motion, write(Transform2D, transform), read(Physics2DMotion, motion), exclude(Parent2D), argument(R, duration), action(
  pga2d_Motor M = transform->M;
  pga2d_Bivector B = motion->B;

  M = pga2d_sub(
      pga2d_m(M)
    , pga2d_mul(
        pga2d_s(state->duration / 2)
      , pga2d_mul_mb(M, B)
      )
    );

  R d = pga2d_norm(pga2d_m(M));
  transform->M = pga2d_mul(pga2d_s(1.0 / d), pga2d_m(M));
))

ECS_QUERY(circle_mass, write(Physics2DBodyMotion, body), read(Physics2DCircle, circle), read(Physics2DMass, mass), action(
  R r = circle->radius, x = R_PI * r*r*r*r / 4;
  body->I = pga2d_mul_sv(mass->value, ((pga2d_Vector) { .e0 = 1, .e1 = x, .e2 = x }));
))

ECS_QUERY(rectangle_mass, write(Physics2DBodyMotion, body), read(Physics2DRectangle, rectangle), read(Physics2DMass, mass), action(
  R w = rectangle->width
        , h = rectangle->height;
  body->I = pga2d_mul_sv(mass->value, ((pga2d_Vector) { .e0 = 1, .e1 = (h * w*w*w) / 12, .e2 = (w * h*h*h) / 12 }));
))

ECS_QUERY(force_gravity, write(Physics2DBodyMotion, body), read(Transform2D, position), read(Physics2DGravity, gravity), argument(pga2d_Bivector, gravity), action(
  body->forque = pga2d_add(
      pga2d_v(body->forque),
      pga2d_mul(pga2d_s(gravity->value),
                      pga2d_dual(pga2d_grade_2(pga2d_sandwich(
                          pga2d_b(state->gravity),
                          pga2d_reverse_m(position->value))))));
))

ECS_QUERY(force_dampen, write(Physics2DBodyMotion, body), read(Physics2DMotion, velocity), read(Physics2DDampen, dampen), action(
  body->forque = pga2d_sub(
      pga2d_v(body->forque),
      pga2d_mul(
          pga2d_s(dampen->value),
          pga2d_dual_b(velocity->value)));
))

ECS_QUERY(body_motion, write(Physics2DMotion, motion), write(Physics2DBodyMotion, body), argument(R, duration), action(
  pga2d_Bivector B = motion->B;
  pga2d_AntiBivector F = body->F;

  pga2d_Bivector dB = pga2d_undual(pga2d_sub(
      pga2d_v(F)
    , pga2d_commutator_product(pga2d_dual_b(B), pga2d_b(B))
  ));

  motion->value = pga2d_add(
      pga2d_b(B),
      pga2d_mul_sb(state->duration, dB));
  
  memory_clear(&body->forque, sizeof(body->forque));
))

void physics_update2d_serial_pre_transform(R duration) {
  transform_motion(duration);
  circle_mass();
  rectangle_mass();
}

void physics_update2d_serial_post_transform(R duration) {
  pga2d_Bivector gravity; // uninitialized
  
  force_gravity(gravity);
  force_dampen();
  body_motion(duration);
}
