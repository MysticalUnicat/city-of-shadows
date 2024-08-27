#ifndef __LIBRARY_CORE_INITIALIZATION_H__
#define __LIBRARY_CORE_INITIALIZATION_H__

// TODO: handle MIR environment

#define INIT_SECTION_PRIORITY(N) __attribute__((constructor(101 + N)))
#define INIT_SECTION             INIT_SECTION_PRIORITY(1)

#endif
