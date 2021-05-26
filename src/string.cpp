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
#include <sstream>
#include "ast.h"

/***********
 * str_len *
 ***********/

/* Returns the number of UTF-8 characters in the string.
 * @param value The UTF-8 string.
 * @return The number of UTF-8 characters.
 */
static size_t str_len(const std::string &value) {
  size_t count = 0;

  cutlet::utf8::iterator it(value);
  for (size_t count = 0; it != it.end(); count++, it++);

  return count;
}

/*************
 * str_index *
 *************/

/* Convert the index from Cutlet to c++. Cutlet indexes start at 1 and negative
 * values count from the back.
 * @param index The Cutlet index.
 * @param len The character length of the string.
 * @return Index usable with c++.
 */
static size_t str_index(int index, size_t len) {
  // Negative indexes start from the back and index towards the front.
  if (index < 0) {
    index = len + index;
  } else {
    index--;
  }

  // Make sure the index is in range.
  if (index < 0 or index >= (long long)len) {
    std::stringstream mesg;
    mesg << "String index " << index << "out of range";
    throw std::runtime_error(mesg.str());
  }

  return index;
}

/************
 * str_utf8 *
 ************/

/* Returns an UTF-8 character from the given index.
 */
static std::string str_utf8(size_t index, const std::string &value) {
  cutlet::utf8::iterator it(value);
  while (index and it != it.end()) {
    index--;
    it++;
  }

  return *it;
}

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
                                                const list &arguments) {
  std::string op = *(arguments[0]);
  unsigned int args = arguments.size();

  /* TODO
   *  - rfind
   *  - endswith
   */

  if (op == "type") {
    if (args != 1)
      throw std::runtime_error(std::string("Invalid number of arguments to "
                                           "string operator type"));

    return new string("string");

  } else if (op == "length") {
    if (args != 1)
      throw std::runtime_error(std::string("Invalid number of arguments to "
                                           "string operator length"));

    /* Note this doesn't return the byte size of the string but rather the
     * number of UTF-8 encoded characters in it.
     */
    unsigned long long count = 0;
    utf8::iterator it(*this);
    while (it != it.end()) {
      count ++;
      it++;
    }
    return new string(count);
  } else if (op == "==" or op == "=") {
    if (args != 2)
      throw std::runtime_error(std::string("Invalid number of arguments to "
                                           "string operator =="));

    return (*this == *(arguments[1])
            ? new boolean(true) : new boolean(false));
  } else if (op == "<>" or op == "!=") {
    if (args != 2)
      throw std::runtime_error(std::string("Invalid number of arguments to "
                                           "string operator <>"));

    return (*this != *(arguments[1])
            ? new boolean(true) : new boolean(false));
  } else if (op == "<") {
    if (args != 2)
      throw std::runtime_error(std::string("Invalid number of arguments to "
                                           "string operator <"));

    return (*this < (std::string)*(arguments[1])
            ? new boolean(true) : new boolean(false));
  } else if (op == "<=") {
    if (args != 2)
      throw std::runtime_error(std::string("Invalid number of arguments to "
                                           "string operator <="));

    return (*this <= (std::string)*(arguments[1])
            ? new boolean(true) : new boolean(false));
  } else if (op == ">") {
    if (args != 2)
      throw std::runtime_error(std::string("Invalid number of arguments to "
                                           "string operator >"));

    return (*this > (std::string)*(arguments[1])
            ? new boolean(true) : new boolean(false));
  } else if (op == ">=") {
    if (args != 2)
      throw std::runtime_error(std::string("Invalid number of arguments to "
                                           "string operator >="));

    return (*this >= (std::string)*(arguments[1])
            ? new boolean(true) : new boolean(false));
  } else if (op == "endswith") {
    return _endswith(interp, arguments);
  } else if (op == "find") {
    return _find(interp, arguments);
  } else if (op == "index") {
    return _index(interp, arguments);
  } else if (op == "startswith") {
    return _startswith(interp, arguments);
  } else if (op == "substr") {
    return _substr(interp, arguments);
  }

  // Pass through
  return variable::operator()(self, interp, arguments);
}

/*****************************
 * cutlet::string::_endswith *
 *****************************/

cutlet::variable::pointer cutlet::string::_endswith(interpreter &interp,
                                                    const list &arguments) {
  (void)interp;

  if (arguments.size() != 2)
    throw std::runtime_error("Invalid number of arguments to "
                             "string operator startswith");

  std::string other(*(arguments[1]));

  // If the other is longer than us it can't be equal.
  if (other.length() > length()) return new boolean(false);

  const char *end = &((this->c_str())[length() - other.length()]);

  return new boolean((std::strncmp(end, other.c_str(), other.length()) == 0));
}

/*************************
 * cutlet::string::_find *
 *************************/

cutlet::variable::pointer cutlet::string::_find(interpreter &interp,
                                                const list &arguments) {
  (void)interp;

  if (arguments.size() != 2)
    throw std::runtime_error("Invalid number of arguments to "
                             "string operator find");

  std::string other(*(arguments[1]));

  utf8::iterator it(*this);
  while (it != it.end()) {
    if (std::strncmp(c_str(), other.c_str(), other.length()) == 0) {
      return new boolean(true);
    }
  }

  return new boolean(false);
}

/**************************
 * cutlet::string::_index *
 **************************/

cutlet::variable::pointer cutlet::string::_index(interpreter &interp,
                                                 const list &arguments) {
  (void)interp;

  if (arguments.size() < 2 or arguments.size() > 4) {
    throw std::runtime_error("Invalid number of arguments to "
                             "string operator index");
  }
  int index = convert<int>(arguments[1]);
  size_t len = str_len(*this);
  size_t idx = str_index(index, len);

  return new string(str_utf8(idx, *this));
}

/***************************
 * cutlet::string::_insert *
 ***************************/

cutlet::variable::pointer cutlet::string::_insert(interpreter &interp,
                                                  const list &arguments) {
  (void)interp;

  if (arguments.size() != 3) {
    throw std::runtime_error("Invalid number of arguments to "
                             "string operator insert");
  }
  int index = convert<int>(arguments[1]);
  size_t len = str_len(*this);
  size_t idx = str_index(index, len);

  utf8::iterator it(*this);

  for (; idx > 0; it++, idx--);

  insert(it.position(), *(arguments[2]));

  return nullptr;
}

/*******************************
 * cutlet::string::_startswith *
 *******************************/

cutlet::variable::pointer cutlet::string::_startswith(interpreter &interp,
                                                      const list &arguments) {
  (void)interp;

  if (arguments.size() != 2)
    throw std::runtime_error("Invalid number of arguments to "
                             "string operator startswith");

  std::string other(*(arguments[1]));

  return new boolean(std::strncmp(c_str(), other.c_str(),
                                  other.length()) == 0);
}

/***************************
 * cutlet::string::_substr *
 ***************************/

cutlet::variable::pointer cutlet::string::_substr(interpreter &interp,
                                                  const list &arguments) {
  (void)interp;

  if (arguments.size() != 3)
    throw std::runtime_error("Invalid number of arguments to "
                             "string operator substr");

  size_t len = str_len(*this);
  size_t start = str_index(convert<int>(arguments[1]), len);
  size_t end = str_index(convert<int>(arguments[2]), len);

  utf8::iterator start_it(*this);
  utf8::iterator end_it(*this);

  for (; start > 0; start_it++, start--);
  for (; end > 0; end_it++, end--);

  return new string(cutlet::utf8::substr(start_it, end_it));
}

/****************************************
 * cutlet::string::operator std::string *
 ****************************************/

cutlet::string::operator std::string() const { return *this; }

template <> int cutlet::convert<int>(variable::pointer object) {
  if (object.is_null()) return 0;

  std::stringstream ss((std::string)(*object));
  int result;
  ss >> result;
  return result;
}

template <> bool cutlet::convert<bool>(variable::pointer object) {
  if (object.is_null()) return false;

  // First we see if the variable is a boolean type.
  auto *bptr = dynamic_cast<cutlet::boolean *>(&(*object));
  if (bptr) {
    return (bool)(*bptr);

  } else {
    // It not a boolean type so we treat it like a string.
    std::string value = *object;
    if (value == "false" or value == "0" or value.empty()) return false;
    return true;
  }
}
