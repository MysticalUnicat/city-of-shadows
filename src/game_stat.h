#ifndef game_stat_h_INCLUDED
#define game_stat_h_INCLUDED

#include "cpp.h"
#include "cpp_type.h"
#include "fixed_point.h"

#define GAME_STAT_TYPE_impl(NAME, API_BASE_TYPE, INTERNAL_BASE_TYPE, INTERNAL_SCALE_TYPE, ADD, INC, MOR) \
  typedef struct { \
    INTERNAL_BASE_TYPE base; \
    CPP_IFF(ADD)(INTERNAL_BASE_TYPE add;, ) \
    CPP_IFF(INC)(INTERNAL_SCALE_TYPE increase;, ) \
    CPP_IFF(MOR)(INTERNAL_SCALE_TYPE more;, ) \
    CPP_IFF(CPP_OR(CPP_OR(ADD, INC), MOR))(INTERNAL_BASE_TYPE memo;, ) \
  } NAME; \
  static inline void NAME ## __reset(NAME * stat, API_BASE_TYPE amount) { \
    stat->base = CPP_TYPE_CONVERT(API_BASE_TYPE, INTERNAL_BASE_TYPE)(amount); \
    CPP_IFF(ADD)(stat->add = (INTERNAL_BASE_TYPE)(0);, ) \
    CPP_IFF(INC)(stat->increase = CPP_TYPE_CONVERT(float, INTERNAL_SCALE_TYPE)(0);, ) \
    CPP_IFF(MOR)(stat->more = CPP_TYPE_CONVERT(float, INTERNAL_SCALE_TYPE)(1);, ) \
    CPP_IFF(CPP_OR(CPP_OR(ADD, INC), MOR))(stat->memo = (INTERNAL_BASE_TYPE)(0);, ) \
  } \
  static inline API_BASE_TYPE NAME ## __get_base(const NAME * stat) { \
    return CPP_TYPE_CONVERT(INTERNAL_BASE_TYPE, API_BASE_TYPE)(stat->base); \
  } \
  static inline void NAME ## __set_base(NAME * stat, API_BASE_TYPE amount) { \
    stat->base = CPP_TYPE_CONVERT(API_BASE_TYPE, INTERNAL_BASE_TYPE)(amount); \
    CPP_IFF(CPP_OR(CPP_OR(ADD, INC), MOR))(stat->memo = (INTERNAL_BASE_TYPE)(0);, ) \
  } \
  static inline void NAME ## __add(NAME * stat, API_BASE_TYPE amount) { \
    CPP_IFF(ADD)(stat->add += CPP_TYPE_CONVERT(API_BASE_TYPE, INTERNAL_BASE_TYPE)(amount);, ) \
    CPP_IFF(ADD)(stat->memo = (INTERNAL_BASE_TYPE)(0);, ) \
  } \
  static inline void NAME ## __subtract(NAME * stat, API_BASE_TYPE amount) { \
    CPP_IFF(ADD)(stat->add -= CPP_TYPE_CONVERT(API_BASE_TYPE, INTERNAL_BASE_TYPE)(amount);, ) \
    CPP_IFF(ADD)(stat->memo = (INTERNAL_BASE_TYPE)(0);, ) \
  } \
  static inline void NAME ## __increase(NAME * stat, float amount) { \
    CPP_IFF(INC)(stat->increase += CPP_TYPE_CONVERT(float, INTERNAL_SCALE_TYPE)(amount);, ) \
    CPP_IFF(INC)(stat->memo = (INTERNAL_BASE_TYPE)(0);, ) \
  } \
  static inline void NAME ## __decrease(NAME * stat, float amount) { \
    CPP_IFF(INC)(stat->increase -= CPP_TYPE_CONVERT(float, INTERNAL_SCALE_TYPE)(amount);, ) \
    CPP_IFF(INC)(stat->memo = (INTERNAL_BASE_TYPE)(0);, ) \
  } \
  static inline void NAME ## __more(NAME * stat, float amount) { \
    CPP_IFF(MOR)(stat->more = CPP_TYPE_CONVERT(float, INTERNAL_SCALE_TYPE)(CPP_TYPE_CONVERT(INTERNAL_SCALE_TYPE, float)(stat->more) * (1 + amount));, ) \
    CPP_IFF(MOR)(stat->memo = (INTERNAL_BASE_TYPE)(0);, ) \
  } \
  static inline void NAME ## __less(NAME * stat, float amount) { \
    CPP_IFF(MOR)(stat->more = CPP_TYPE_CONVERT(float, INTERNAL_SCALE_TYPE)(CPP_TYPE_CONVERT(INTERNAL_SCALE_TYPE, float)(stat->more) / (1 + amount));, ) \
    CPP_IFF(MOR)(stat->memo = (INTERNAL_BASE_TYPE)(0);, ) \
  } \
  static inline API_BASE_TYPE NAME ## __get(const NAME * stat) { \
    CPP_IFF(CPP_OR(CPP_OR(ADD, INC), MOR))( \
      if(stat->memo == (INTERNAL_BASE_TYPE)(0)) { \
        *(INTERNAL_BASE_TYPE *)&stat->memo = CPP_TYPE_CONVERT(float, INTERNAL_BASE_TYPE)(CPP_TYPE_CONVERT(INTERNAL_SCALE_TYPE, float)(stat->base \
          CPP_IFF(ADD)(+ stat->add, ) \
        ) \
          CPP_IFF(INC)(* (1.0f + CPP_TYPE_CONVERT(INTERNAL_SCALE_TYPE, float)(stat->increase)), ) \
          CPP_IFF(MOR)(* CPP_TYPE_CONVERT(INTERNAL_SCALE_TYPE, float)(stat->more), ) \
        ); \
      } \
      return CPP_TYPE_CONVERT(INTERNAL_BASE_TYPE, API_BASE_TYPE)(stat->memo); \
    , return CPP_TYPE_CONVERT(INTERNAL_BASE_TYPE, API_BASE_TYPE)(stat->base);) \
  }

#define GAME_STAT_DATA_TYPES(X, NAME, ...) \
  X(NAME, float, float, float, ## __VA_ARGS__) \
  X(NAME ## _U8, uint8_t, uint8_t, FixedPoint4, ## __VA_ARGS__) \
  X(NAME ## _U16, uint16_t, uint16_t, FixedPoint8, ## __VA_ARGS__) \
  X(NAME ## _S16, float, FixedPoint8, FixedPoint8, ## __VA_ARGS__)

#define GAME_STAT_FEATURES(NEXT, X, ...) \
  NEXT(X, GameStat, 0, 0, 0, ## __VA_ARGS__) \
  NEXT(X, GameStatA, 1, 0, 0, ## __VA_ARGS__) \
  NEXT(X, GameStatI, 0, 1, 0, ## __VA_ARGS__) \
  NEXT(X, GameStatIA, 1, 1, 0, ## __VA_ARGS__) \
  NEXT(X, GameStatM, 0, 0, 1, ## __VA_ARGS__) \
  NEXT(X, GameStatMA, 1, 0, 1, ## __VA_ARGS__) \
  NEXT(X, GameStatMI, 0, 1, 1, ## __VA_ARGS__) \
  NEXT(X, GameStatMIA, 1, 1, 1, ## __VA_ARGS__)

#define GAME_STAT_VARIETY(X, ...) GAME_STAT_FEATURES(GAME_STAT_DATA_TYPES, X, ## __VA_ARGS__)

GAME_STAT_VARIETY(GAME_STAT_TYPE_impl)

#define GAME_STAT_GENERIC_impl(NAME, API_BASE_TYPE, INTERNAL_BASE_TYPE, INTERNAL_SCALE_TYPE, ADD, INC, MOR, SUFFIX) \
	NAME *: NAME ## __ ## SUFFIX,

#define GAME_STAT_GENERIC_CONST_impl(NAME, API_BASE_TYPE, INTERNAL_BASE_TYPE, INTERNAL_SCALE_TYPE, ADD, INC, MOR, SUFFIX) \
	NAME *: NAME ## __ ## SUFFIX, \
	const NAME *: NAME ## __ ## SUFFIX,

#define GAME_STAT_GENERIC(SUFFIX) GAME_STAT_VARIETY(GAME_STAT_GENERIC_impl, SUFFIX) default: (void(*)(...))NULL
#define GAME_STAT_GENERIC_CONST(SUFFIX) GAME_STAT_VARIETY(GAME_STAT_GENERIC_CONST_impl, SUFFIX) default: (void(*)(...))NULL

#define GameStat_reset(S, AMOUNT) _Generic((S), GAME_STAT_GENERIC(reset))(S, AMOUNT)
#define GameStat_get_base(S) _Generic((S), GAME_STAT_GENERIC(get_base))(S)
#define GameStat_set_base(S, AMOUNT) _Generic((S), GAME_STAT_GENERIC(set_base))(S, AMOUNT)
#define GameStat_add(S, AMOUNT) _Generic((S), GAME_STAT_GENERIC(add))(S, AMOUNT)
#define GameStat_subtract(S, AMOUNT) _Generic((S), GAME_STAT_GENERIC(subtract))(S, AMOUNT)
#define GameStat_increase(S, AMOUNT) _Generic((S), GAME_STAT_GENERIC(increase))(S, AMOUNT)
#define GameStat_decrease(S, AMOUNT) _Generic((S), GAME_STAT_GENERIC(decrease))(S, AMOUNT)
#define GameStat_more(S, AMOUNT) _Generic((S), GAME_STAT_GENERIC(more))(S, AMOUNT) 
#define GameStat_less(S, AMOUNT) _Generic((S), GAME_STAT_GENERIC(less))(S, AMOUNT) 
#define GameStat_get(S) _Generic((S), GAME_STAT_GENERIC_CONST(get))(S)

#endif // game_stat_h_INCLUDED
