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

#include <cutlet.h>

#ifndef _CUTLET_BUILTIN_H
#define _CUTLET_BUILTIN_H

namespace builtin {

  cutlet::variable_ptr def(cutlet::interpreter &interp,
                           const cutlet::list &parameters);

  cutlet::variable_ptr import(cutlet::interpreter &interp,
                              const cutlet::list &parameters);

  cutlet::variable_ptr local(cutlet::interpreter &interp,
                             const cutlet::list &parameters);

  cutlet::variable_ptr global(cutlet::interpreter &interp,
                              const cutlet::list &parameters);

  cutlet::variable_ptr ret(cutlet::interpreter &interp,
                           const cutlet::list &parameters);

  cutlet::variable_ptr print(cutlet::interpreter &interp,
                             const cutlet::list &parameters);

  cutlet::variable_ptr list(cutlet::interpreter &interp,
                            const cutlet::list &parameters);

  cutlet::variable_ptr expr(cutlet::interpreter &interp,
                            const cutlet::list &parameters);
}

#endif /* _CUTLET_BUILTIN_H */
