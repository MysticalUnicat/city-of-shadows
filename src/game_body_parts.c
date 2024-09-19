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

static inline void animal_blood_filtration(struct BloodFiltration * blood_filtration, const struct AnimalBody * body) {
  float factor = 1.0f;
  factor *= 0.5f + BODY_PART_FRACTION(body, ANIMAL, left_kidney) * 0.5f;
  factor *= 0.5f + BODY_PART_FRACTION(body, ANIMAL, right_kidney) * 0.5f;
  factor *= BODY_PART_FRACTION(body, ANIMAL, liver);
  BloodFiltration_initialize_from_body(blood_filtration, factor);
}

static inline void animal_blood_pumping(struct BloodPumping * blood_pumping, const struct AnimalBody * body) {
  float factor = 1.0f;
  factor *= BODY_PART_FRACTION(body, ANIMAL, heart);
  BloodPumping_initialize_from_body(blood_pumping, factor);
}

static inline void animal_blood_oxygen(struct BloodOxygen * blood_oxygen, const struct AnimalBody * body) {
  float factor = 1.0f;
  factor *= BODY_PART_FRACTION(body, ANIMAL, left_lung);
  factor *= BODY_PART_FRACTION(body, ANIMAL, right_lung);
  factor *= BODY_PART_FRACTION(body, ANIMAL, neck);
  factor *= BODY_PART_FRACTION(body, ANIMAL, ribcage);
  factor *= BODY_PART_FRACTION(body, ANIMAL, sternum);
  BloodOxygen_initialize_from_body(blood_oxygen, factor);
}

static inline void animal_pain(struct Pain * pain, const struct AnimalBody * body) {
  float weight = 0.0f, factor = 0.0f;
#define PAIN_CALCULATION(NAME, BITS, MAX_HEALTH) weight += 1.0f; factor += BODY_PART_FRACTION(body, ANIMAL, NAME) * 1.0f;
  ANIMAL_BODY_PARTS(PAIN_CALCULATION)
#undef PAIN_CALCULATION
  Pain_initialize_from_body(pain, (weight - factor) / weight);
}

static inline void animal_consciousness(struct Consciousness * consciousness, const struct AnimalBody * body) {
  float factor = 1.0f;
  factor *= BODY_PART_FRACTION(body, ANIMAL, brain);
  Consciousness_initialize_from_body(consciousness, factor);
}

static inline void animal_digestion(struct Digestion * digestion, const struct AnimalBody * body) {
  float factor = 1.0f;
  factor *= 0.5f + BODY_PART_FRACTION(body, ANIMAL, stomach) * 0.5f;
  factor *= 0.5f + BODY_PART_FRACTION(body, ANIMAL, liver) * 0.5f;
  Digestion_initialize_from_body(digestion, factor);
}

static inline void animal_eating(struct Eating * eating, const struct AnimalBody * body) {
  float factor = 1.0f;
  factor *= 0.5f + BODY_PART_FRACTION(body, ANIMAL, tongue) * 0.5f;
  factor *= -0.5f + BODY_PART_FRACTION(body, ANIMAL, jaw) * 1.5f;
  Eating_initialize_from_body(eating, max(factor, 0));
}

static inline void animal_hearing(struct Hearing * hearing, const struct AnimalBody * body) {
  float factor = 1.0f;
  factor *= (body->left_ear_dominate ? 0.25f : 0.75f) + BODY_PART_FRACTION(body, ANIMAL, left_ear) * (body->left_ear_dominate ? 0.75f : 0.25f);
  factor *= (body->left_ear_dominate ? 0.75f : 0.25f) + BODY_PART_FRACTION(body, ANIMAL, right_ear) * (body->left_ear_dominate ? 0.25f : 0.75f);
  Hearing_initialize_from_body(hearing, factor);
}

static inline void animal_manipulation(struct Manipulation * manipulation, const struct AnimalBody * body) {
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
}

static inline void animal_moving(struct Moving * moving, const struct AnimalBody * body) {
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
}

static inline void animal_sight(struct Sight * sight, const struct AnimalBody * body) {
  float factor = 1.0f;
  factor *= (body->left_eye_dominate ? 0.25f : 0.75f) + BODY_PART_FRACTION(body, ANIMAL, left_eye) * (body->left_eye_dominate ? 0.75f : 0.25f);
  factor *= (body->left_eye_dominate ? 0.75f : 0.25f) + BODY_PART_FRACTION(body, ANIMAL, right_eye) * (body->left_eye_dominate ? 0.25f : 0.75f);
  Sight_initialize_from_body(sight, factor);
}

static inline void animal_talking(struct Talking * talking, const struct AnimalBody * body) {
  float factor = 1.0f;
  factor *= BODY_PART_FRACTION(body, ANIMAL, jaw);
  factor *= BODY_PART_FRACTION(body, ANIMAL, tongue);
  Talking_initialize_from_body(talking, factor);
}

