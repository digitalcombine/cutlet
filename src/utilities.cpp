/*                                                                  -*- c++ -*-
 * Copyright © Ron R Wills
 * All rights reserved
 */

#include "utilities.h"
#include <unistd.h>

bool fexists(const std::string &filename) {
  return (access(filename.c_str(), F_OK) == 0);
}
