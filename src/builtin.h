/*                                                                  -*- c++ -*-
 * Copyright © 2018 Ron R Wills <ron@digitalcombine.ca>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
