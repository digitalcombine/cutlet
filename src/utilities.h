/*                                                                  -*- c++ -*-
 * Copyright © 2018 Ron R Wills <ron@digitalcombine.ca>
 *
 * This file is part of Cutlet.
 *
 * Cutlet is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cutlet is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cutlet.  If not, see <http://www.gnu.org/licenses/>.
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
