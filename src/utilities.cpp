/*                                                                  -*- c++ -*-
 * Copyright © Ron R Wills
 * All rights reserved
 */

#include "utils.h"
#include <unistd.h>

bool fexists(const std::string &filename) {
  return (access(filename.c_str(), F_OK) == 0);
}
