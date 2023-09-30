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
#include <sstream>
#include "ast.h"

namespace {

  /***********
   * str_len *
   ***********/

  /** Returns the number of UTF-8 characters in the string.
   * @param value The UTF-8 string.
   * @return The number of UTF-8 characters.
   */
  size_t str_len(const std::string &value) {
    size_t count = 0;
    cutlet::utf8::iterator it(value);
    for (; it != it.end(); count++, it++);

    return count;
  }

  /*************
   * str_index *
   *************/

  /** Convert the index from Cutlet to c++. Cutlet indexes start at 1 and
   * negative values count from the back.
   * @param index The Cutlet index.
   * @param len The character length of the string.
   * @return Index usable with c++.
   */
  size_t str_index(int index, size_t len) {
    // Negative indexes start from the back and index towards the front.
    int idx = index;
    if (index < 0) {
      idx = static_cast<int>(len) + index;
    } else if (index > 0) {
      // The first character is at index 1.
      idx--;
    } else {
      std::stringstream mesg;
      mesg << "String index " << index << " out of range (1 - "
           << len << ")";
      throw std::runtime_error(mesg.str());
    }

    // Make sure the index is in range.
    if (idx < 0 or idx >= static_cast<long>(len)) {
      std::stringstream mesg;
      mesg << "String index " << index << " out of range (1 - "
           << len << ")";
      throw std::runtime_error(mesg.str());
    }

    return static_cast<size_t>(idx);
  }

  /************
   * str_utf8 *
   ************/

  /** Returns an UTF-8 character from the given index.
   */
  std::string str_utf8(size_t index, const std::string &value) {
    cutlet::utf8::iterator it(value);
    while (index and it != it.end()) {
      index--;
      it++;
    }

    return *it;
  }


  /*************
   * _endswith *
   *************/

  inline
  cutlet::variable::pointer _endswith(const std::string &self,
                                      cutlet::interpreter &interp,
                                      const cutlet::list &arguments) {
    (void)interp;

    if (arguments.size() != 2)
      throw std::runtime_error("Invalid number of arguments to "
                               "string operator startswith");

    std::string other(*(arguments[1]));

    // If the other is longer than us it can't be equal.
    if (other.length() > self.length())
      return cutlet::var<cutlet::boolean>(false);

    const char *end = &((self.c_str())[self.length() - other.length()]);

    return
      cutlet::var<cutlet::boolean>((std::strncmp(end,
                                                      other.c_str(),
                                                      other.length()) == 0));
  }

  /*********
   * _find *
   *********/

  inline
  cutlet::variable::pointer _find(const std::string &self,
                                  cutlet::interpreter &interp,
                                  const cutlet::list &arguments) {
    (void)interp;

    if (arguments.size() != 2)
      throw std::runtime_error("Invalid number of arguments to "
                               "string operator find");

    std::string other(*(arguments[1]));

    auto pos = self.find(other);
    if (pos == std::string::npos)
      return cutlet::var<cutlet::boolean>(false);

    return cutlet::var<cutlet::string>(pos);
  }

  /**********
   * _index *
   **********/

  inline
  cutlet::variable::pointer _index(const std::string &self,
                                   cutlet::interpreter &interp,
                                   const cutlet::list &arguments) {
    (void)interp;

    if (arguments.size() < 2 or arguments.size() > 4) {
      throw std::runtime_error("Invalid number of arguments to "
                               "string operator index");
    }

    int index = cutlet::primative<int>(arguments[1]);
    size_t len = str_len(self);
    size_t idx = str_index(index, len);

    if (arguments.size() == 2) {
      // Return the character.
      return cutlet::var<cutlet::string>(str_utf8(idx, self));

    } else {
      // Act like the insert operator.
      std::string value = *(arguments[2]);
      if (arguments.size() == 4) {
        if (*(arguments[2]) != "=") {
          throw std::runtime_error("Unexpected character " +
                                   static_cast<std::string>(*(arguments[2])) +
                                   ", expected =");
        }
        value = *(arguments[3]);
      }

      auto result(self);
      cutlet::utf8::iterator it(result);
      for (; idx > 0; it++, idx--);
      result.insert(it.position(), value);

      return cutlet::var<cutlet::string>(result);
    }
  }

  /***********
   * _insert *
   ***********/

  inline
  cutlet::variable::pointer _insert(const std::string &self,
                                    cutlet::interpreter &interp,
                                    const cutlet::list &arguments) {
    (void)interp;

    if (arguments.size() != 3) {
      throw std::runtime_error("Invalid number of arguments to "
                               "string operator insert");
    }
    int index = cutlet::primative<int>(arguments[1]);
    size_t len = str_len(self);
    size_t idx = str_index(index, len);

    auto result(self);
    cutlet::utf8::iterator it(result);

    for (; idx > 0; it++, idx--);

    result.insert(it.position(), *(arguments[2]));

    return cutlet::var<cutlet::string>(result);
  }

  /***************
   * _startswith *
   ***************/

  inline
  cutlet::variable::pointer _startswith(const std::string &self,
                                        cutlet::interpreter &interp,
                                        const cutlet::list &arguments) {
    (void)interp;

    if (arguments.size() != 2)
      throw std::runtime_error("Invalid number of arguments to "
                               "string operator startswith");

    std::string other(*(arguments[1]));

    return
      cutlet::var<cutlet::boolean>(std::strncmp(self.c_str(),
                                                     other.c_str(),
                                                     other.length()) == 0);
  }

  /***********
   * _substr *
   ***********/

  inline
  cutlet::variable::pointer _substr(const std::string &self,
                                    cutlet::interpreter &interp,
                                    const cutlet::list &arguments) {
    (void)interp;

    if (arguments.size() != 3)
      throw std::runtime_error("Invalid number of arguments to "
                               "string operator substr");

    size_t len = str_len(self);
    size_t start = str_index(cutlet::primative<int>(arguments[1]), len);
    size_t end = str_index(cutlet::primative<int>(arguments[2]), len);

    cutlet::utf8::iterator start_it(self);
    cutlet::utf8::iterator end_it(self);

    for (; start > 0; start_it++, start--);
    for (; end > 0; end_it++, end--);

    return cutlet::var<cutlet::string>(
      cutlet::utf8::substr(start_it, end_it));
  }
}

/*****************************************************************************
 * class cutlet::string
 */

/**************************
 * cutlet::string::string *
 **************************/

cutlet::string::string() : std::string() {}

cutlet::string::string(const string &value) : std::string(value) {}

cutlet::string::string(const std::string &value) : std::string(value) {}

cutlet::string::string(int value) : std::string(std::to_string(value)) {}

/***************************
 * cutlet::string::~string *
 ***************************/

cutlet::string::~string() noexcept {}

/*******************************
 * cutlet::string::operator () *
 *******************************/

cutlet::variable::pointer cutlet::string::operator()(variable::pointer self,
                                                     interpreter &interp,
                                                     const list &arguments) {
  std::string op = *(arguments[0]);
  size_t args = arguments.size();

  /* TODO
   *  - rfind
   */

  // We use a switch here to make the operator look up a bit faster.
  switch (op[0]) {
  case '=':
    if (op == "==" or op == "=") {
      // $string == other or $string = other
      if (args != 2)
        throw std::runtime_error(std::string("Invalid number of arguments to "
                                             "string operator =="));
      return (*this == static_cast<std::string>(*(arguments[1]))
              ? cutlet::var<cutlet::boolean>(true)
              : cutlet::var<cutlet::boolean>(false));
    }
    break;
  case '!':
    if (op == "!=") {
      // $string <> other or $string != other
      if (args != 2)
        throw std::runtime_error(std::string("Invalid number of arguments to "
                                             "string operator !="));
      return (*this != static_cast<std::string>(*(arguments[1]))
              ? cutlet::var<cutlet::boolean>(true)
              : cutlet::var<cutlet::boolean>(false));
    }
    break;
  case '<':
    if (op == "<>") {
      // $string <> other or $string != other
      if (args != 2)
        throw std::runtime_error(std::string("Invalid number of arguments to "
                                             "string operator <>"));
      return (*this != static_cast<std::string>(*(arguments[1]))
              ? cutlet::var<cutlet::boolean>(true)
              : cutlet::var<cutlet::boolean>(false));

    } else if (op == "<") {
      // $string < other
      if (args != 2)
        throw std::runtime_error(std::string("Invalid number of arguments to "
                                             "string operator <"));
      return (*this < static_cast<std::string>(*(arguments[1]))
              ? cutlet::var<cutlet::boolean>(true)
              : cutlet::var<cutlet::boolean>(false));

    } else if (op == "<=") {
      // $string <= other
      if (args != 2)
        throw std::runtime_error(std::string("Invalid number of arguments to "
                                             "string operator <="));
      return (*this <= static_cast<std::string>(*(arguments[1]))
              ? cutlet::var<cutlet::boolean>(true)
              : cutlet::var<cutlet::boolean>(false));
    }
    break;
  case '>':
    if (op == ">") {
      // $string > other
      if (args != 2)
        throw std::runtime_error(std::string("Invalid number of arguments to "
                                             "string operator >"));
      return (*this > static_cast<std::string>(*(arguments[1]))
              ? cutlet::var<cutlet::boolean>(true)
              : cutlet::var<cutlet::boolean>(false));

    } else if (op == ">=") {
      // $string >= other
      if (args != 2)
        throw std::runtime_error(std::string("Invalid number of arguments to "
                                             "string operator >="));
      return (*this >= static_cast<std::string>(*(arguments[1]))
              ? cutlet::var<cutlet::boolean>(true)
              : cutlet::var<cutlet::boolean>(false));
    }
    break;
  case '+':
    if (op == "+") {
      // $string + *args
      if (args < 2)
        throw std::runtime_error(std::string("Invalid number of arguments to "
                                             "string operator +"));
      cutlet::list sargs(arguments);
      sargs.pop_front(); // Remove the operator.

      return cutlet::var<cutlet::string>(*this + sargs.join());


    }
    break;
  case 'a':
    if (op == "append") {
      // $string append *args
      if (args < 2)
        throw std::runtime_error(std::string("Invalid number of arguments to "
                                             "string operator append"));
      cutlet::list sargs(arguments);
      sargs.pop_front(); // Remove the operator.

      return cutlet::var<cutlet::string>(*this + sargs.join());
    }
    break;
  case 'e':
    if (op == "endswith") {
      // $string endswith value
      return _endswith(*this, interp, arguments);
    }
    break;
  case 'f':
    if (op == "find") {
      // $string find value
      return _find(*this, interp, arguments);
    }
    break;
  case 'i':
    if (op == "index") {
      // $string index offset ¿¿=? value?
      return _index(*this, interp, arguments);

    } else if (op == "insert") {
      // $string insert offset value
      return _insert(*this, interp, arguments);
    }
    break;
  case 'l':
    if (op == "length") {
      // $string length
      if (args != 1)
        throw std::runtime_error(std::string("Invalid number of arguments to "
                                             "string operator length"));
      return cutlet::var<cutlet::string>(str_len(*this));
    }
    break;
  case 's':
    if (op == "startswith") {
      // $string startswith value
      return _startswith(*this, interp, arguments);

    } else if (op == "substr") {
      // $string substr start end
      return _substr(*this, interp, arguments);
    }
    break;
  case 't':
    if (op == "type") {
      // $string type
      if (args != 1)
        throw std::runtime_error(std::string("Invalid number of arguments to "
                                             "string operator type"));
      return cutlet::var<cutlet::string>("string");
    }
    break;
  }

  // Pass through
  return variable::operator()(self, interp, arguments);
}

/****************************************
 * cutlet::string::operator std::string *
 ****************************************/

cutlet::string::operator std::string() const { return *this; }

/******************************
 * template cutlet::primative *
 ******************************/

template <> int cutlet::primative<int>(variable::pointer object) {
  if (not object) return 0;

  return std::stoi(static_cast<std::string>(*object));
}

template <> bool cutlet::primative<bool>(variable::pointer object) {
  if (not object) return false;

  // First we see if the variable is a boolean type.
  auto *bptr = dynamic_cast<cutlet::boolean *>(&(*object));
  if (bptr) {
    return static_cast<bool>(*bptr);

  } else {
    // It not a boolean type so we treat it like a string.
    std::string value = *object;
    if (value == "false" or value == "0" or value.empty()) return false;
    return true;
  }
}
