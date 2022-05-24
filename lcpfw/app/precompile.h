// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is used as a precompiled header for both C and C++ files. So
// any C++ headers must go in the __cplusplus block below.

#if defined(BUILD_PRECOMPILE_H_)
#error You shouldn't include the precompiled header file more than once.
#endif

#define BUILD_PRECOMPILE_H_

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <memory.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(__cplusplus)

#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iosfwd>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <numeric>
#include <ostream>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#endif  // __cplusplus


#if _MSC_VER
//#include <inttypes.h>
#endif

#include "base/logging.h"
#include "base/macros.h"
#include "base/task/post_task.h"

#ifdef _MSC_VER
    #ifdef APP_LIB_IMPLEMENTATION
        #ifdef __cplusplus
            #define APP_LIB_EXPORT extern "C" __declspec(dllexport)
        #else
            #define APP_LIB_EXPORT __declspec(dllexport)
        #endif
    #else
        #define APP_LIB_EXPORT __declspec(dllimport)
    #endif
#else
    #ifdef APP_LIB_IMPLEMENTATION
        #ifdef __cplusplus
            #define APP_LIB_EXPORT extern "C" __attribute__((visibility("default")))
        #else
            #define APP_LIB_EXPORT __attribute__((visibility("default")))
        #endif
    #else
        #define APP_LIB_EXPORT
    #endif
#endif

#define chSTR(x) #x
#define chSTR2(x) chSTR(x)
#define TODO_MSG(desc) message(__FILE__ "(" chSTR2(__LINE__) "):" #desc)