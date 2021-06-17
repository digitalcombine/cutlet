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

#include "builtin.h"
#include <sstream>

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
                                 const cutlet::list &arguments) {
  (void)self;
  std::string op = *(arguments[0]);
  size_t args = arguments.size();

  if (op == "eval") {
    // $sandbox eval body ...
    bool first = true;

    interp.push(_sandbox);
    for (auto &command: arguments) {
      if (first) {
        first = false;
      } else {
        try {
          interp.compile(command);
        } catch (...) {
          /* If an error was thrown within the sandbox, we pop the
           * context off the stack to restore the previous environment
           * then rethrow the exception.
           */
          interp.pop();
          throw;
        }
      }
    }
    interp.pop();

  } else if (op == "expr") {
    // $sandbox expr body ...
    if (args == 2) {
      variable::pointer result;

      interp.push(_sandbox);
      try {
        result = interp.expr(arguments[1]);
      } catch (...) {
        /* If an error was thrown within the sandbox, we pop the context
         * off the stack to restore the previous environment then
         * rethrow the exception.
         */
        interp.pop();
        throw;
      }
      interp.pop();

      return result;
    } else {
      std::stringstream mesg;
      mesg << "Invalid number of arguments for $sandbox expr "
         << " (2 = " << args << ").\n $sandbox expr body";
      throw std::runtime_error(mesg.str());
    }

  } else if (op == "link") {
    // $sandbox link component ¿as name?
    // $sandbox link *args
    if (args == 4 and *(arguments[2]) == "as") {
      _sandbox->add(*(arguments[3]),
                    interp.get(*(arguments[1])));
    } else {
      bool first = true;
      for (auto &parm: arguments) {
        if (first) first = false;
        else _sandbox->add(*parm, interp.get(*parm));
      }
    }

  } else if (op == "unlink") {
    // $sandbox unlink *args

    bool first = true;
    for (auto &parm: arguments) {
      if (first) first = false;
      else _sandbox->remove(*parm);
    }

  } else if (op == "clear") {
    // $sandbox clear
    if (args != 2)
      throw std::runtime_error("To many arguments to sandbox operator clear");

    _sandbox->clear();

  } else if (op == "type") {
    // $sandbox type
    if (args != 2)
      throw std::runtime_error("To many arguments to sandbox operator type");

    return new cutlet::string("sandbox");

  } else if (op == "global") {
    // $sandbox global name ¿=? ¿value?
    switch (args) {
    case 2:
      _sandbox->variable(*(arguments[1]), nullptr);
      break;
    case 3:
      if (*(arguments[1]) == "=")
        _sandbox->variable(*(arguments[1]), nullptr);
      else
        _sandbox->variable(*(arguments[1]), arguments[2]);
      break;
    case 4:
      if (*(arguments[2]) != "=") {
        throw std::runtime_error("global name ¿=? value\n"
                                 " Expected = got " +
                                 (std::string)*(arguments[2]));
      }
      _sandbox->variable(*(arguments[1]), arguments[3]);
      break;
    default:
      throw std::runtime_error("Invalid arguments to sandbox operator clear");
    }

  } else {
    throw std::runtime_error("Unknown operator \"" +
                             (std::string)*(arguments[0]) +
                             "\" for sandbox type.");
  }

  return nullptr;
}
