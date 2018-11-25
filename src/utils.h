/*                                                                 -*- c++ -*-
 * Copyright © Ron R Wills
 * All rights reserved
 */

#include <string>

#if defined (__linux__) || defined(__FreeBSD__)
#include <dlfcn.h>
#define SOEXT ".so"
typedef void *nativelib_t;
#elif defined (_WIN32) || defined (_WIN64)
#include <windows.h>
#define SOEXT ".dll"
typedef HMODULE nativelib_t;
#endif

#ifndef _CUTLET_UTILS_H
#define _CUTLET_UTILS_H

bool fexists(const std::string &filename);

#endif /* _CUTLET_UTILS_H */
