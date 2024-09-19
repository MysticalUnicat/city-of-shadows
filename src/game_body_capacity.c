#include "game_local.h"

ECS_COMPONENT(BloodFiltration);
ECS_COMPONENT(BloodPumping);
ECS_COMPONENT(BloodOxygen);
ECS_COMPONENT(Pain);
ECS_COMPONENT(Consciousness);
ECS_COMPONENT(Digestion);
ECS_COMPONENT(Eating);
ECS_COMPONENT(Hearing)
ECS_COMPONENT(Manipulation)
ECS_COMPONENT(Moving)
ECS_COMPONENT(Sight)
ECS_COMPONENT(Talking)

#define CONSIOUSNESS_CAPACITY_FACTORS(X, ...) \
  X(filter,                     1, 0, 0, 0, ## __VA_ARGS__) \
  X(pumping,                    0, 1, 0, 0, ## __VA_ARGS__) \
  X(filter_pumping,             1, 1, 0, 0, ## __VA_ARGS__) \
  X(oxygen,                     0, 0, 1, 0, ## __VA_ARGS__) \
  X(filter_oxygen,              1, 0, 1, 0, ## __VA_ARGS__) \
  X(pumping_oxygen,             0, 1, 1, 0, ## __VA_ARGS__) \
  X(filter_pumping_oxygen,      1, 1, 1, 0, ## __VA_ARGS__) \
  X(pain,                       0, 0, 0, 1, ## __VA_ARGS__) \
  X(filter_pain,                1, 0, 0, 1, ## __VA_ARGS__) \
  X(pumping_pain,               0, 1, 0, 1, ## __VA_ARGS__) \
  X(filter_pumping_pain,        1, 1, 0, 1, ## __VA_ARGS__) \
  X(oxygen_pain,                0, 0, 1, 1, ## __VA_ARGS__) \
  X(filter_oxygen_pain,         1, 0, 1, 1, ## __VA_ARGS__) \
  X(pumping_oxygen_pain,        0, 1, 1, 1, ## __VA_ARGS__) \
  X(filter_pumping_oxygen_pain, 1, 1, 1, 1, ## __VA_ARGS__)

#define CONSIOUSNESS_impl(NAME, BLOOD_FILTRATION, BLOOD_PUMPING, OXYGEN, PAIN) \
static inline void consciousness_ ## NAME ## _factor(struct Consciousness * consciousness, const void ** read_only) { \
  float factor = GameStat_get_base(&consciousness->capacity); \
  CPP_IFF(BLOOD_FILTRATION)( \
    const struct BloodFiltration * blood_filtration = (const struct BloodFiltration *)*(read_only++); \
    factor *= 0.9f + BloodFiltration_get(blood_filtration) * 0.1f; \
  , ) \
  CPP_IFF(BLOOD_PUMPING)( \
    const struct BloodPumping * blood_pumping = (const struct BloodPumping *)*(read_only++); \
    factor *= 0.8f + BloodPumping_get(blood_pumping) * 0.2f; \
  , ) \
  CPP_IFF(OXYGEN)( \
    const struct BloodOxygen * blood_oxygen = (const struct BloodOxygen *)*(read_only++); \
    factor += 0.8f + BloodOxygen_get(blood_oxygen) * 0.2f; \
  , ) \
  CPP_IFF(PAIN)( \
    const struct Pain * pain = (const struct Pain *)*(read_only++); \
    factor *= max(-0.45f * Pain_get(pain) + 1.05f, 1.0f); \
  , ) \
  GameStat_set_base(&consciousness->capacity, factor); \
}

CONSIOUSNESS_CAPACITY_FACTORS(CONSIOUSNESS_impl)

static inline void manipulation_consciosness_factor(struct Manipulation * manipulation, const struct Consciousness * consciousness) {
  float factor = GameStat_get_base(&manipulation->capacity);
  factor *= Consciousness_get(consciousness);
  GameStat_set_base(&manipulation->capacity, factor);
}

#define MOVING_CAPACITY_FACTORS(X, ...) \
  X(consciousness,                1, 0, 0, ## __VA_ARGS__) \
  X(pumping,                      0, 1, 0, ## __VA_ARGS__) \
  X(consciousness_pumping,        1, 1, 0, ## __VA_ARGS__) \
  X(oxygen,                       0, 0, 1, ## __VA_ARGS__) \
  X(consciousness_oxygen,         1, 0, 1, ## __VA_ARGS__) \
  X(pumping_oxygen,               0, 1, 1, ## __VA_ARGS__) \
  X(consciousness_pumping_oxygen, 1, 1, 1, ## __VA_ARGS__)

#define MOVING_impl(NAME, CONSIOUSNESS, BLOOD_PUMPING, OXYGEN) \
static inline void moving_ ## NAME ## _factor(struct Moving * moving, const void ** read_only) { \
  float factor = GameStat_get_base(&moving->capacity); \
  CPP_IFF(CONSIOUSNESS)( \
    const struct Consciousness * consciousness = (const struct Consciousness *)*(read_only++); \
    factor *= Consciousness_get(consciousness); \
  , ) \
  CPP_IFF(BLOOD_PUMPING)( \
    const struct BloodPumping * blood_pumping = (const struct BloodPumping *)*(read_only++); \
    factor *= 0.8f + BloodPumping_get(blood_pumping) * 0.2f; \
  , ) \
  CPP_IFF(OXYGEN)( \
    const struct BloodOxygen * blood_oxygen = (const struct BloodOxygen *)*(read_only++); \
    factor += 0.8f + BloodOxygen_get(blood_oxygen) * 0.2f; \
  , ) \
  GameStat_set_base(&moving->capacity, factor); \
}

static inline void eating_consciousness_factor(struct Eating * eating, const struct Consciousness * consciousness) {
  float factor = GameStat_get_base(&eating->capacity);
  factor *= Consciousness_get(consciousness);
  GameStat_set_base(&eating->capacity, factor);
}

MOVING_CAPACITY_FACTORS(MOVING_impl)

static inline void talking_consciosness_factor(struct Talking * talking, const struct Consciousness * consciousness) {
  float factor = GameStat_get_base(&talking->capacity);
  factor *= Consciousness_get(consciousness);
  GameStat_set_base(&talking->capacity, factor);
}

