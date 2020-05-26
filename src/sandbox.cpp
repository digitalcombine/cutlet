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

#include "builtin.h"

/*****************************************************************************
 * class cutlet::sandbox_var
 */

/*************************************
 * builtin::sandbox_var::sandbox_var *
 *************************************/

builtin::sandbox_var::sandbox_var(cutlet::sandbox::pointer sb)
  : _sandbox(sb) {}

/**************************************
 * builtin::sandbox_var::~sandbox_var *
 **************************************/

builtin::sandbox_var::~sandbox_var() noexcept {}

/*************************************
 * builtin::sandbox_var::operator () *
 *************************************/

cutlet::variable::pointer
builtin::sandbox_var::operator()(cutlet::variable::pointer self,
                                 cutlet::interpreter &interp,
                                 const cutlet::list &parameters) {
  (void)self;

  if (cutlet::convert<std::string>(parameters[0]) == "eval") {
    std::string command;

    bool first = true;
    auto it = parameters.begin(); ++it;
    for (; it != parameters.end(); ++it) {
      if (not first) command += " ";
      else first = false;

      command += cutlet::convert<std::string>(*it);
    }

    interp.frame_push(_sandbox);
    try {
      interp.compile(command);
    } catch (...) {
      /* If an error was thrown within the sandbox, we pop the context off the
       * stack to restore the previous environment then rethrow the
       * exception.
       */
      interp.frame_pop();
      throw;
    }
    interp.frame_pop();
  } else if (cutlet::convert<std::string>(parameters[0]) == "link") {
    // $sandbox link function
    _sandbox->add(cutlet::convert<std::string>(parameters[1]),
                  interp.get(cutlet::convert<std::string>(parameters[1])));

  } else if (cutlet::convert<std::string>(parameters[0]) == "global") {
    // $sandbox global name ¿=? value
    if (parameters.size() == 3) {
      _sandbox->variable(cutlet::convert<std::string>(parameters[1]),
                         parameters[2]);
    } else {
      _sandbox->variable(cutlet::convert<std::string>(parameters[1]),
                         parameters[3]);
    }

  } else {
    throw std::runtime_error(std::string("Unknown function \"") +
                             cutlet::convert<std::string>(parameters[0]) +
                             "\" for sandbox variable.");
  }
  return nullptr;
}
