#ifndef __LIBRARY_CORE_REAL_H__
#define __LIBRARY_CORE_REAL_H__

#include <math.h>

static inline bool real_is_nan(float f) {
  return isnan(f);
}

#endif
