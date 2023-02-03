/*                                                                  -*- c++ -*-
 * Copyright © 2018-2021 Ron R Wills <ron@digitalcombine.ca>
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

    return (not _value
            ? std::make_shared<boolean>(true)
            : std::make_shared<boolean>(false));

  } else if (op == "type") {
    // $boolean type
    if (args != 1)
      throw std::runtime_error("Invalid number of arguments to "
                               "boolean operator type");

    return std::make_shared<string>("boolean");

  } else if (op == "==" or op == "=") {
    // $boolean == other
    if (args != 2)
      throw std::runtime_error("Invalid number of arguments to "
                               "boolean operator ==");

    return (_value == primative<bool>(arguments[1])
            ? std::make_shared<boolean>(true)
            : std::make_shared<boolean>(false));

  } else if (op == "<>" or op == "!=") {
    // $boolean <> other
    if (args != 2)
      throw std::runtime_error("Invalid number of arguments to "
                               "boolean operator <>");

    return (_value != primative<bool>(arguments[1])
            ? std::make_shared<boolean>(true)
            : std::make_shared<boolean>(false));

  } else if (op == "and") {
    // $boolean and other
    if (args != 2)
      throw std::runtime_error("Invalid number of arguments to "
                               "boolean operator and");

    return (_value and primative<bool>(arguments[1])
            ? std::make_shared<boolean>(true)
            : std::make_shared<boolean>(false));

  } else if (op == "nand") {
    // $boolean and other
    if (args != 2)
      throw std::runtime_error("Invalid number of arguments to "
                               "boolean operator nand");

    return (not (_value and primative<bool>(arguments[1]))
            ? std::make_shared<boolean>(true)
            : std::make_shared<boolean>(false));

  } else if (op == "or") {
    // $boolean or other
    if (args != 2)
      throw std::runtime_error("Invalid number of arguments to "
                               "boolean operator or");

    return (_value or primative<bool>(arguments[1])
            ? std::make_shared<boolean>(true)
            : std::make_shared<boolean>(false));

  } else if (op == "nor") {
    // $boolean or other
    if (args != 2)
      throw std::runtime_error("Invalid number of arguments to "
                               "boolean operator nor");

    return (not (_value or primative<bool>(arguments[1]))
            ? std::make_shared<boolean>(true)
            : std::make_shared<boolean>(false));

  }  else if (op == "xor") {
    // $boolean xor other
    if (args != 2)
      throw std::runtime_error("Invalid number of arguments to "
                               "boolean operator xor");

    return (_value xor primative<bool>(arguments[1])
            ? std::make_shared<boolean>(true)
            : std::make_shared<boolean>(false));
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
