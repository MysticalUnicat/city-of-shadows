#ifndef game_local_h_INCLUDED
#define game_local_h_INCLUDED

#include "game.h"
#include "math.h"
#include "game_stat.h"
#include "game_tables.h"

/* story: cut down a tree and build a fire for warmth
 *
 * items:
 *   stone          - collected, common on the ground in undeveloped areas
 *   stone hand axe - axe level 1 (improvised)
 *   small wood     - a length of small radius wood, uncommon on the ground in undeveloped areas
 *
 * actions:
 *   pickup *                 - collect from the item from the world
 *   construct stone hand axe - requires 2x stone, costs 1x stone - axe level 1
 *   cut down small tree      - requires an axe at lease level 1, results in a large number of small wood
 *   construct fire           - costs 4x small wood
 * 
 */

// body parts -> body capacity -> action multipliers
//                ^        |
//                 \-------/

// body part
#define BODY_PART_STORAGE(NAME, BITS, MAX_HEALTH) uint32_t NAME : BITS;
#define BODY_PART_MAX_HEALTH_CONST(NAME, BITS, MAX_HEALTH, SPECIES) static const uint32_t SPECIES ## _ ## NAME ## _MAX_HEALTH = MAX_HEALTH;

// body capacity
ECS_DECLARE_COMPONENT(BloodFiltration, {
  GameStatI capacity;
});

ECS_DECLARE_COMPONENT(BloodPumping, {
  GameStatI capacity;
});

ECS_DECLARE_COMPONENT(BloodOxygen, {
  GameStatI capacity;
});

ECS_DECLARE_COMPONENT(Pain, {
  GameStatI capacity;
});

ECS_DECLARE_COMPONENT(Consciousness, {
  GameStatIA capacity;
  GameStatI  limit;
});

ECS_DECLARE_COMPONENT(Digestion, {
  GameStatI capacity;
});

ECS_DECLARE_COMPONENT(Eating, {
  GameStatI capacity;
});

ECS_DECLARE_COMPONENT(Hearing, {
  GameStatIA capacity;
})

ECS_DECLARE_COMPONENT(Manipulation, {
  GameStatIA capacity;
})

ECS_DECLARE_COMPONENT(Moving, {
  GameStatIA capacity;
})

ECS_DECLARE_COMPONENT(Sight, {
  GameStatIA capacity;
})

ECS_DECLARE_COMPONENT(Talking, {
  GameStatIA capacity;
})

static inline void BloodFiltration_initialize_from_body(struct BloodFiltration * blood_filtration, float body_part_factor) {
  GameStat_set_base(&blood_filtration->capacity, body_part_factor);
}

static inline float BloodFiltration_get(const struct BloodFiltration * blood_filtration) {
  float b = GameStat_get(&blood_filtration->capacity);
  return max(b, 0);
}

static inline void BloodPumping_initialize_from_body(struct BloodPumping * blood_pumping, float body_part_factor) {
  GameStat_set_base(&blood_pumping->capacity, body_part_factor);
}

static inline float BloodPumping_get(const struct BloodPumping * blood_pumping) {
  float b = GameStat_get(&blood_pumping->capacity);
  return max(b, 0);
}

static inline void BloodOxygen_initialize_from_body(struct BloodOxygen * blood_oxygen, float body_part_factor) {
  GameStat_set_base(&blood_oxygen->capacity, body_part_factor);
}

static inline float BloodOxygen_get(const struct BloodOxygen * blood_oxygen) {
  float b = GameStat_get(&blood_oxygen->capacity);
  return max(b, 0);
}

static inline void Pain_initialize_from_body(struct Pain * pain, float body_part_factor) {
  GameStat_set_base(&pain->capacity, body_part_factor);
}

static inline float Pain_get(const struct Pain * pain) {
  float b = GameStat_get(&pain->capacity);
  return max(b, 0);
}

static inline void Consciousness_initialize_from_body(struct Consciousness * consciousness, float body_part_factor) {
  GameStat_set_base(&consciousness->capacity, body_part_factor);
}

static inline float Consciousness_get(const struct Consciousness * consciousness) {
  float b = GameStat_get(&consciousness->capacity);
  float l = GameStat_get(&consciousness->limit);
  return max(min(b, l), 0);
}

static inline void Digestion_initialize_from_body(struct Digestion * digestion, float body_part_factor) {
  GameStat_set_base(&digestion->capacity, body_part_factor);
}

static inline void Eating_initialize_from_body(struct Eating * eating, float body_part_factor) {
  GameStat_set_base(&eating->capacity, body_part_factor);
}

static inline void Hearing_initialize_from_body(struct Hearing * hearing, float body_part_factor) {
  GameStat_set_base(&hearing->capacity, body_part_factor);
}

static inline void Manipulation_initialize_from_body(struct Manipulation * manipulation, float body_part_factor) {
  GameStat_set_base(&manipulation->capacity, body_part_factor);
}

static inline void Moving_initialize_from_body(struct Moving * moving, float body_part_factor) {
  GameStat_set_base(&moving->capacity, body_part_factor);
}

static inline void Sight_initialize_from_body(struct Sight * sight, float body_part_factor) {
  GameStat_set_base(&sight->capacity, body_part_factor);
}

static inline void Talking_initialize_from_body(struct Talking * talking, float body_part_factor) {
  GameStat_set_base(&talking->capacity, body_part_factor);
}

// body parts
ANIMAL_BODY_PARTS(BODY_PART_MAX_HEALTH_CONST, ANIMAL)

ECS_DECLARE_COMPONENT(AnimalBody, {
  ANIMAL_BODY_PARTS(BODY_PART_STORAGE)
  uint32_t left_ear_dominate : 1;
  uint32_t left_eye_dominate : 1;
})

// needs
ECS_DECLARE_COMPONENT(FoodNeed, {
  GameStat nutrition;
  GameStatI hunger_rate;
})

ECS_DECLARE_COMPONENT(SleepNeed, {
  GameStat rest;
  GameStatI rest_rate_multiplier;
  GameStatI sleep_fall_rate;
})

// skills
ECS_DECLARE_COMPONENT(LearningRate, {
  GameStatMI multiplier;
})

ECS_DECLARE_COMPONENT(AnimalsSkill, {
  GameStatA  level;
  GameStatIA confidence;
  GameStat   xp;
})

ECS_DECLARE_COMPONENT(ArtisticSkill, {
  GameStatA  level;
  GameStatIA confidence;
  GameStat   xp;
})

ECS_DECLARE_COMPONENT(ConstructionSkill, {
  GameStatA  level;
  GameStatIA confidence;
  GameStat   xp;
})

ECS_DECLARE_COMPONENT(CookingSkill, {
  GameStatA  level;
  GameStatIA confidence;
  GameStat   xp;
})

ECS_DECLARE_COMPONENT(CraftingSkill, {
  GameStatA  level;
  GameStatIA confidence;
  GameStat   xp;
})

ECS_DECLARE_COMPONENT(MedicalSkill, {
  GameStatA  level;
  GameStatIA confidence;
  GameStat   xp;
})

ECS_DECLARE_COMPONENT(MeleeSkill, {
  GameStatA  level;
  GameStatIA confidence;
  GameStat   xp;
})

ECS_DECLARE_COMPONENT(MiningSkill, {
  GameStatA  level;
  GameStatIA confidence;
  GameStat   xp;
})

ECS_DECLARE_COMPONENT(IntellectualSkill, {
  GameStatA  level;
  GameStatIA confidence;
  GameStat   xp;
})

ECS_DECLARE_COMPONENT(PlantsSkill, {
  GameStatA  level;
  GameStatIA confidence;
  GameStat   xp;
})

ECS_DECLARE_COMPONENT(ShootingSkill, {
  GameStatA  level;
  GameStatIA confidence;
  GameStat   xp;
})

ECS_DECLARE_COMPONENT(SocialSkill, {
  GameStatA  level;
  GameStatIA confidence;
  GameStat   xp;
})

#undef BODY_PART_STORAGE 
#undef BODY_PART_MAX_HEALTH_CONST

#endif // game_local_h_INCLUDED
