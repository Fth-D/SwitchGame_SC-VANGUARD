#pragma once

// Platform detection
#if defined(NN_BUILD_CONFIG_OS_HORIZON) || defined(NX64) || defined(NX32)
  #define PLATFORM_NX    1
  #define PLATFORM_WIN64 0
#elif defined(_WIN64) || defined(WIN64) || defined(_WIN32)
  #define PLATFORM_NX    0
  #define PLATFORM_WIN64 1
#else
  #error "Unsupported platform"
#endif

// Logging / Assert
#if PLATFORM_NX
  #include <nn/nn_Assert.h>
  #include <nn/nn_Log.h>
  #define PAL_LOG       NN_LOG
  #define PAL_ASSERT    NN_ASSERT
  #define PAL_UNUSED(x) NN_UNUSED(x)
#else
  #include <cstdio>
  #include <cassert>
  #define PAL_LOG(fmt, ...)    printf(fmt, ##__VA_ARGS__)
  #define PAL_ASSERT(cond, ...) assert(cond)
  #define PAL_UNUSED(x) ((void)(x))
  #define NN_LOG        PAL_LOG
  #define NN_ASSERT     PAL_ASSERT
  #define NN_UNUSED(x)  PAL_UNUSED(x)
  #define NN_ASSERT_EQUAL(a, b) assert((a) == (b))
  #define NN_ABORT_UNLESS_RESULT_SUCCESS(r) ((void)(r))
#endif
