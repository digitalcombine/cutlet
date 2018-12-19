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

  cutlet::variable_ptr block(cutlet::interpreter &interp,
                             const cutlet::list &parameters);

  cutlet::variable_ptr ret(cutlet::interpreter &interp,
                           const cutlet::list &parameters);

  cutlet::variable_ptr print(cutlet::interpreter &interp,
                             const cutlet::list &parameters);

  cutlet::variable_ptr list(cutlet::interpreter &interp,
                            const cutlet::list &parameters);

  cutlet::variable_ptr sandbox(cutlet::interpreter &interp,
                               const cutlet::list &parameters);

  /** This class is a Cutlet variable that wraps a cutlet sandbox. It allows
   * access to the sandbox API within scripts and interpreters.
   */
  class sandbox_var : public cutlet::variable {
  public:
    sandbox_var(cutlet::sandbox_ptr sb);
    ~sandbox_var() noexcept;

    virtual cutlet::variable_ptr operator()(cutlet::variable_ptr self,
                                            cutlet::interpreter &interp,
                                            const cutlet::list &parameters);

  private:
    cutlet::sandbox_ptr _sandbox;
  };
}

#endif /* _CUTLET_BUILTIN_H */
