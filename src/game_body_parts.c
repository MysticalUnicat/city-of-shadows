#include "game_local.h"

#define BODY_PART_FRACTION(BODY, SPECIES, NAME) ((float)(BODY->NAME) / (float)SPECIES ## _ ## NAME ## _MAX_HEALTH)

ECS_COMPONENT(AnimalBody,
  requires(BloodFiltration),
  requires(BloodPumping),
  requires(BloodOxygen),
  requires(Pain),
  requires(Consciousness),
  requires(Digestion),
  requires(Eating),
  requires(Hearing),
  requires(Manipulation),
  requires(Moving),
  requires(Sight),
  requires(Talking),

  requires(FoodNeed),
  requires(SleepNeed)
)

ECS_QUERY(animal_blood_filtration,
  write(BloodFiltration, blood_filtration),
  read(AnimalBody, body),
  action(
    float factor = 1.0f;
    factor *= 0.5f + BODY_PART_FRACTION(body, ANIMAL, left_kidney) * 0.5f;
    factor *= 0.5f + BODY_PART_FRACTION(body, ANIMAL, right_kidney) * 0.5f;
    factor *= BODY_PART_FRACTION(body, ANIMAL, liver);
    BloodFiltration_initialize_from_body(blood_filtration, factor);
  )
)

ECS_QUERY(animal_blood_pumping,
  write(BloodPumping, blood_pumping),
  read(AnimalBody, body),
  action(
    float factor = 1.0f;
    factor *= BODY_PART_FRACTION(body, ANIMAL, heart);
    BloodPumping_initialize_from_body(blood_pumping, factor);
  )
)

ECS_QUERY(animal_blood_oxygen,
  write(BloodOxygen, blood_oxygen),
  read(AnimalBody, body)
  action(
    float factor = 1.0f;
    factor *= BODY_PART_FRACTION(body, ANIMAL, left_lung);
    factor *= BODY_PART_FRACTION(body, ANIMAL, right_lung);
    factor *= BODY_PART_FRACTION(body, ANIMAL, neck);
    factor *= BODY_PART_FRACTION(body, ANIMAL, ribcage);
    factor *= BODY_PART_FRACTION(body, ANIMAL, sternum);
    BloodOxygen_initialize_from_body(blood_oxygen, factor);
  )
)

ECS_QUERY(animal_pain,
  write(Pain, pain),
  read(AnimalBody, body)
  action(
    float weight = 0.0f, factor = 0.0f;
#define PAIN_CALCULATION(NAME, BITS, MAX_HEALTH) weight += 1.0f; factor += BODY_PART_FRACTION(body, ANIMAL, NAME) * 1.0f;
    ANIMAL_BODY_PARTS(PAIN_CALCULATION)
#undef PAIN_CALCULATION
    Pain_initialize_from_body(pain, (weight - factor) / weight);
  )
)

ECS_QUERY(animal_consciousness,
  write(Consciousness, consciousness),
  read(AnimalBody, body),
  action(
    float factor = 1.0f;
    factor *= BODY_PART_FRACTION(body, ANIMAL, brain);
    Consciousness_initialize_from_body(consciousness, factor);
  )
)

ECS_QUERY(animal_digestion,
  write(Digestion, digestion),
  read(AnimalBody, body),
  action(
    float factor = 1.0f;
    factor *= 0.5f + BODY_PART_FRACTION(body, ANIMAL, stomach) * 0.5f;
    factor *= 0.5f + BODY_PART_FRACTION(body, ANIMAL, liver) * 0.5f;
    Digestion_initialize_from_body(digestion, factor);
  )
)

ECS_QUERY(animal_eating,
  write(Eating, eating),
  read(AnimalBody, body),
  action(
    float factor = 1.0f;
    factor *= 0.5f + BODY_PART_FRACTION(body, ANIMAL, tongue) * 0.5f;
    factor *= -0.5f + BODY_PART_FRACTION(body, ANIMAL, jaw) * 1.5f;
    Eating_initialize_from_body(eating, max(factor, 0));
  )
)

ECS_QUERY(animal_hearing,
  write(Hearing, hearing),
  read(AnimalBody, body),
  action(
    float factor = 1.0f;
    factor *= (body->left_ear_dominate ? 0.25f : 0.75f) + BODY_PART_FRACTION(body, ANIMAL, left_ear) * (body->left_ear_dominate ? 0.75f : 0.25f);
    factor *= (body->left_ear_dominate ? 0.75f : 0.25f) + BODY_PART_FRACTION(body, ANIMAL, right_ear) * (body->left_ear_dominate ? 0.25f : 0.75f);
    Hearing_initialize_from_body(hearing, factor);
  )
)

ECS_QUERY(animal_manipulation,
  write(Manipulation, manipulation),
  read(AnimalBody, body),
  action(
    float factor = 1.0f;
    factor *= 0.75f + BODY_PART_FRACTION(body, ANIMAL, left_shoulder) * 0.25f;
    factor *= 0.75f + BODY_PART_FRACTION(body, ANIMAL, left_clavicle) * 0.25f;
    factor *= 0.5f + BODY_PART_FRACTION(body, ANIMAL, left_arm) * 0.5f;
    factor *= 0.75f + BODY_PART_FRACTION(body, ANIMAL, left_humerus) * 0.25f;
    factor *= 0.75f + BODY_PART_FRACTION(body, ANIMAL, left_radius) * 0.25f;
    factor *= 0.75f + BODY_PART_FRACTION(body, ANIMAL, left_hand) * 0.25f;
    factor *= 0.9f + BODY_PART_FRACTION(body, ANIMAL, left_pinky) * 0.1f;
    factor *= 0.9f + BODY_PART_FRACTION(body, ANIMAL, left_ring_finger) * 0.1f;
    factor *= 0.9f + BODY_PART_FRACTION(body, ANIMAL, left_middle_finger) * 0.1f;
    factor *= 0.9f + BODY_PART_FRACTION(body, ANIMAL, left_index_finger) * 0.1f;
    factor *= 0.9f + BODY_PART_FRACTION(body, ANIMAL, left_thumb) * 0.1f;
    factor *= 0.75f + BODY_PART_FRACTION(body, ANIMAL, right_shoulder) * 0.25f;
    factor *= 0.75f + BODY_PART_FRACTION(body, ANIMAL, right_clavicle) * 0.25f;
    factor *= 0.5f + BODY_PART_FRACTION(body, ANIMAL, right_arm) * 0.5f;
    factor *= 0.75f + BODY_PART_FRACTION(body, ANIMAL, right_humerus) * 0.25f;
    factor *= 0.75f + BODY_PART_FRACTION(body, ANIMAL, right_radius) * 0.25f;
    factor *= 0.75f + BODY_PART_FRACTION(body, ANIMAL, right_hand) * 0.25f;
    factor *= 0.9f + BODY_PART_FRACTION(body, ANIMAL, right_pinky) * 0.1f;
    factor *= 0.9f + BODY_PART_FRACTION(body, ANIMAL, right_ring_finger) * 0.1f;
    factor *= 0.9f + BODY_PART_FRACTION(body, ANIMAL, right_middle_finger) * 0.1f;
    factor *= 0.9f + BODY_PART_FRACTION(body, ANIMAL, right_index_finger) * 0.1f;
    factor *= 0.9f + BODY_PART_FRACTION(body, ANIMAL, right_thumb) * 0.1f;
    Manipulation_initialize_from_body(manipulation, factor);
  )
)

ECS_QUERY(animal_moving,
  write(Moving, moving),
  read(AnimalBody, body),
  action(
    float factor = 1.0f;
    factor *= 0.0f + BODY_PART_FRACTION(body, ANIMAL, pelvis) * 1.0f;
    factor *= 0.0f + BODY_PART_FRACTION(body, ANIMAL, spine) * 1.0f;
    factor *= 0.5f + BODY_PART_FRACTION(body, ANIMAL, left_leg) * 0.5f;
    factor *= 0.75f + BODY_PART_FRACTION(body, ANIMAL, left_femur) * 0.25f;
    factor *= 0.75f + BODY_PART_FRACTION(body, ANIMAL, left_tibia) * 0.25f;
    factor *= 0.75f + BODY_PART_FRACTION(body, ANIMAL, left_foot) * 0.25f;
    factor *= 0.9f + BODY_PART_FRACTION(body, ANIMAL, left_little_toe) * 0.1f;
    factor *= 0.9f + BODY_PART_FRACTION(body, ANIMAL, left_fourth_toe) * 0.1f;
    factor *= 0.9f + BODY_PART_FRACTION(body, ANIMAL, left_middle_toe) * 0.1f;
    factor *= 0.9f + BODY_PART_FRACTION(body, ANIMAL, left_second_toe) * 0.1f;
    factor *= 0.9f + BODY_PART_FRACTION(body, ANIMAL, left_big_toe) * 0.1f;
    factor *= 0.5f + BODY_PART_FRACTION(body, ANIMAL, right_leg) * 0.5f;
    factor *= 0.75f + BODY_PART_FRACTION(body, ANIMAL, right_femur) * 0.25f;
    factor *= 0.75f + BODY_PART_FRACTION(body, ANIMAL, right_tibia) * 0.25f;
    factor *= 0.75f + BODY_PART_FRACTION(body, ANIMAL, right_foot) * 0.25f;
    factor *= 0.9f + BODY_PART_FRACTION(body, ANIMAL, right_little_toe) * 0.1f;
    factor *= 0.9f + BODY_PART_FRACTION(body, ANIMAL, right_fourth_toe) * 0.1f;
    factor *= 0.9f + BODY_PART_FRACTION(body, ANIMAL, right_middle_toe) * 0.1f;
    factor *= 0.9f + BODY_PART_FRACTION(body, ANIMAL, right_second_toe) * 0.1f;
    factor *= 0.9f + BODY_PART_FRACTION(body, ANIMAL, right_big_toe) * 0.1f;
    Moving_initialize_from_body(moving, factor);
  )
)

ECS_QUERY(animal_sight,
  write(Sight, sight),
  read(AnimalBody, body),
  action(
    float factor = 1.0f;
    factor *= (body->left_eye_dominate ? 0.25f : 0.75f) + BODY_PART_FRACTION(body, ANIMAL, left_eye) * (body->left_eye_dominate ? 0.75f : 0.25f);
    factor *= (body->left_eye_dominate ? 0.75f : 0.25f) + BODY_PART_FRACTION(body, ANIMAL, right_eye) * (body->left_eye_dominate ? 0.25f : 0.75f);
    Sight_initialize_from_body(sight, factor);
  )
)

ECS_QUERY(animal_talking,
  write(Talking, talking),
  read(AnimalBody, body),
  action(
    float factor = 1.0f;
    factor *= BODY_PART_FRACTION(body, ANIMAL, jaw);
    factor *= BODY_PART_FRACTION(body, ANIMAL, tongue);
    Talking_initialize_from_body(talking, factor);
  )
)

