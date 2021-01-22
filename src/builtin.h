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

#include <cutlet>

#ifndef _CUTLET_BUILTIN_H
#define _CUTLET_BUILTIN_H

namespace builtin {

  cutlet::variable::pointer def(cutlet::interpreter &interp,
                                const cutlet::list &parameters);

  cutlet::variable::pointer incl(cutlet::interpreter &interp,
                                 const cutlet::list &parameters);

  cutlet::variable::pointer import(cutlet::interpreter &interp,
                                   const cutlet::list &parameters);

  cutlet::variable::pointer local(cutlet::interpreter &interp,
                                  const cutlet::list &parameters);

  cutlet::variable::pointer global(cutlet::interpreter &interp,
                                   const cutlet::list &parameters);

  cutlet::variable::pointer uplevel(cutlet::interpreter &interp,
                                    const cutlet::list &parameters);

  cutlet::variable::pointer ret(cutlet::interpreter &interp,
                                const cutlet::list &parameters);

  cutlet::variable::pointer print(cutlet::interpreter &interp,
                                  const cutlet::list &parameters);

  cutlet::variable::pointer list(cutlet::interpreter &interp,
                                 const cutlet::list &parameters);

  cutlet::variable::pointer sandbox(cutlet::interpreter &interp,
                                    const cutlet::list &parameters);

  /** This class is a Cutlet variable that wraps a cutlet sandbox. It allows
   * access to the sandbox API within scripts and interpreters.
   */
  class sandbox_var : public cutlet::variable {
  public:
    sandbox_var(cutlet::sandbox::pointer sb);
    ~sandbox_var() noexcept;

    virtual cutlet::variable::pointer
    operator()(cutlet::variable::pointer self,
               cutlet::interpreter &interp,
               const cutlet::list &parameters);

  private:
    cutlet::sandbox::pointer _sandbox;
  };
}

#endif /* _CUTLET_BUILTIN_H */
