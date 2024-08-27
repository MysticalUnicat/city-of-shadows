#ifndef __LIBRARY_CORE_ASSERT_H__
#define __LIBRARY_CORE_ASSERT_H__

// TODO: MIR version

#include <assert.h>

#define ASSERT(A, ...) assert(A, ## __VA_ARGS__)

#endif
