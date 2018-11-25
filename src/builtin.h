/*                                                                 -*- c++ -*-
 * Copyright © Ron R Wills
 * All rights reserved
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
