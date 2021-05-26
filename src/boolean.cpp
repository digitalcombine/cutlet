/*                                                                  -*- c++ -*-
 * Copyright © 2018-2021 Ron R Wills <ron@digitalcombine.ca>
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
#include "ast.h"

/*****************************************************************************
 * class cutlet::boolean
 */

/****************************
 * cutlet::boolean::boolean *
 ****************************/

cutlet::boolean::boolean() : _value(true) {}

cutlet::boolean::boolean(const std::string &value) : _value(false) {
  std::string cooked(value);
  std::transform(cooked.begin(), cooked.end(), cooked.begin(), ::tolower);
  if ((cooked == "true") or (cooked == "yes") or (cooked == "on"))
    _value = true;
}

cutlet::boolean::boolean(bool value) : _value(value) {}

/*****************************
 * cutlet::boolean::~boolean *
 *****************************/

cutlet::boolean::~boolean() noexcept {}

/********************************
 * cutlet::boolean::operator () *
 ********************************/

cutlet::variable::pointer cutlet::boolean::operator()(variable::pointer self,
                                                      interpreter &interp,
                                                      const list &arguments) {
  (void)self;
  (void)interp;

  unsigned int args = arguments.size();
  //if (args == 0) return new boolean(_value);

  std::string op = *(arguments[0]);

  if (op == "not") {
    // $boolean not
    if (args != 1)
      throw std::runtime_error("Invalid number of arguments to "
                               "boolean operator not");

    return (not _value ? new boolean(true) : new boolean(false));

  } else if (op == "type") {
    // $boolean type
    if (args != 1)
      throw std::runtime_error("Invalid number of arguments to "
                               "boolean operator type");

    return new string("boolean");

  } else if (op == "==" or op == "=") {
    // $boolean == other
    if (args != 2)
      throw std::runtime_error("Invalid number of arguments to "
                               "boolean operator ==");

    return (_value == convert<bool>(arguments[1])
            ? new boolean(true) : new boolean(false));

  } else if (op == "<>" or op == "!=") {
    // $boolean <> other
    if (args != 2)
      throw std::runtime_error("Invalid number of arguments to "
                               "boolean operator <>");

    return (_value != convert<bool>(arguments[1])
            ? new boolean(true) : new boolean(false));

  } else if (op == "and") {
    // $boolean and other
    if (args != 2)
      throw std::runtime_error("Invalid number of arguments to "
                               "boolean operator and");

    return (_value and convert<bool>(arguments[1])
            ? new boolean(true) : new boolean(false));

  } else if (op == "nand") {
    // $boolean and other
    if (args != 2)
      throw std::runtime_error("Invalid number of arguments to "
                               "boolean operator nand");

    return (not (_value and convert<bool>(arguments[1]))
            ? new boolean(true) : new boolean(false));

  } else if (op == "or") {
    // $boolean or other
    if (args != 2)
      throw std::runtime_error("Invalid number of arguments to "
                               "boolean operator or");

    return (_value or convert<bool>(arguments[1])
            ? new boolean(true) : new boolean(false));

  } else if (op == "nor") {
    // $boolean or other
    if (args != 2)
      throw std::runtime_error("Invalid number of arguments to "
                               "boolean operator nor");

    return (not (_value or convert<bool>(arguments[1]))
            ? new boolean(true) : new boolean(false));

  }  else if (op == "xor") {
    // $boolean xor other
    if (args != 2)
      throw std::runtime_error("Invalid number of arguments to "
                               "boolean operator xor");

    return (_value xor convert<bool>(arguments[1])
            ? new boolean(true) : new boolean(false));
  }

  throw std::runtime_error(std::string("Unknown operator ") +
                           op + " for boolean variable.");
}

/*****************************************
 * cutlet::boolean::operator std::string *
 *****************************************/

cutlet::boolean::operator std::string() const {
  return (_value ? "true" : "false");
}
