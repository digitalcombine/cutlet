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
#include <sstream>
#include "ast.h"

/*****************************************************************************
 * class cutlet::string
 */

/**************************
 * cutlet::string::string *
 **************************/

cutlet::string::string() : std::string() {}

cutlet::string::string(const std::string &value) : std::string(value) {}

cutlet::string::string(int value) {
  std::stringstream ss;
  ss << value;
  *this = ss.str();
}

/***************************
 * cutlet::string::~string *
 ***************************/

cutlet::string::~string() noexcept {}

/*******************************
 * cutlet::string::operator () *
 *******************************/

cutlet::variable::pointer cutlet::string::operator()(variable::pointer self,
                                                interpreter &interp,
                                                const list &parameters) {
  /* TODO
   *  - substr
   *  - index
   *  - find/rfind
   *  - startswith
   *  - endswith
   *  - insert
   */
  if (parameters.size() == 1) {
    std::string op = *(parameters[0]);

    if (op == "type") {
      return new cutlet::string("string");

    } else if (op == "length") {
      /* Note this doesn't return the byte size of the string but rather the
       * number of UTF-8 encoded characters in it.
       */
      unsigned long long count = 0;
      cutlet::utf8::iterator it(*this);
      while (it != it.end()) {
        count ++;
        it++;
      }
      return new cutlet::string(count);
    }

  } else if (parameters.size() == 2) {
    std::string op = *(parameters[0]);

    if (op == "==" or op == "=") {
      return (*this == cutlet::convert<std::string>(parameters[1])
              ? new cutlet::boolean(true) : new cutlet::boolean(false));
    } else if (op == "<>" or op == "!=") {
      return (*this != cutlet::convert<std::string>(parameters[1])
              ? new cutlet::boolean(true) : new cutlet::boolean(false));
    } else if (op == "<") {
      return (*this < cutlet::convert<std::string>(parameters[1])
              ? new cutlet::boolean(true) : new cutlet::boolean(false));
    } else if (op == "<=") {
      return (*this <= cutlet::convert<std::string>(parameters[1])
              ? new cutlet::boolean(true) : new cutlet::boolean(false));
    } else if (op == ">") {
      return (*this > cutlet::convert<std::string>(parameters[1])
              ? new cutlet::boolean(true) : new cutlet::boolean(false));
    } else if (op == ">=") {
      return (*this >= cutlet::convert<std::string>(parameters[1])
              ? new cutlet::boolean(true) : new cutlet::boolean(false));
    } else if (op == ".") {
      return new cutlet::string(*this +
                                cutlet::convert<std::string>(parameters[1]));
    }
  }
  return cutlet::variable::operator()(self, interp, parameters);
}

/****************************************
 * cutlet::string::operator std::string *
 ****************************************/

cutlet::string::operator std::string() const { return *this; }

template <> int cutlet::convert<int>(variable::pointer object) {
  if (object.is_null()) return 0;

  std::stringstream ss((std::string)(*(object)));
  int result;
  ss >> result;
  return result;
}

template <> bool cutlet::convert<bool>(variable::pointer object) {
  if (object.is_null()) return false;

  std::string value = *object;
  if (value == "false" or value == "0" or value.empty()) return false;
  return true;
}
