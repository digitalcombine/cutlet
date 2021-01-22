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
                                                      const list &parameters) {
  (void)self;
  (void)interp;

  std::string op = *(parameters[0]);

  if (parameters.size() == 1) {
    if (op == "not") {
      return (not _value
              ? new cutlet::boolean(true) : new cutlet::boolean(false));
    } else if (op == "type") {
      return new cutlet::string("boolean");
    }
  } else if (parameters.size() == 2) {
    if (op == "==" or op == "=") {
      return (_value == cutlet::convert<bool>(parameters[1])
              ? new cutlet::boolean(true) : new cutlet::boolean(false));
    } else if (op == "<>" or op == "!=") {
      return (_value != cutlet::convert<bool>(parameters[1])
              ? new cutlet::boolean(true) : new cutlet::boolean(false));
    } else if (op == "and") {
      return (_value and cutlet::convert<bool>(parameters[1])
              ? new cutlet::boolean(true) : new cutlet::boolean(false));
    } else if (op == "or") {
      return (_value or cutlet::convert<bool>(parameters[1])
              ? new cutlet::boolean(true) : new cutlet::boolean(false));
    } else if (op == "xor") {
      return (_value xor cutlet::convert<bool>(parameters[1])
              ? new cutlet::boolean(true) : new cutlet::boolean(false));
    }
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
