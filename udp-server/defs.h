#ifndef DEFS_H
#define DEFS_H

// C assert //
#include <assert.h>

#ifdef RECD_DEBUG
#define ASSERT_MACRO(EXPR) assert(EXPR)
#else
#define ASSERT_MACRO(EXPR)
#endif


#endif // DEFS_H
