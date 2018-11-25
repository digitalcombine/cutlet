/*                                                                  -*- c++ -*-
 * Copyright © 2018 Ron R Wills <ron@digitalcombine.ca>
 *
 * This file is part of Cutlet.
 *
 * Meat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Meat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Meat.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cutlet.h>

/*****************************************************************************
 * class cutlet::list
 */

/**********************
 * cutlet::list::list *
 **********************/

cutlet::list::list() : std::deque<variable_ptr>() {}

cutlet::list::list(const_iterator first, const_iterator last)
  : std::deque<variable_ptr>(first, last) {}

cutlet::list::list(const std::initializer_list<variable_ptr> &items)
  : std::deque<variable_ptr>(items.begin(), items.end()) {}

cutlet::list::list(const list &other) : std::deque<variable_ptr>(other) {}

/*************************
 * cutlet::list::execute *
 *************************/

cutlet::variable_ptr cutlet::list::execute(interpreter &interp,
                                           const list &parameters) {
  variable_ptr result;
  if (parameters.size() > 0) {

    // $list join ¿delim?
    if ((std::string)*(parameters[0]) == "join") {
      return _join(interp, parameters);

    // $list size
    } else if (convert<std::string>(parameters[0]) == "size") {
      return new cutlet::string(size());

    // $list clear
    } else if (convert<std::string>(parameters[0]) == "clear") {
      clear();
      return nullptr;

    // $list append item ¿…?
    } else if (convert<std::string>(parameters[0]) == "append") {
      return _append(interp, parameters);

    // $list prepend item ¿…?
    } else if (convert<std::string>(parameters[0]) == "prepend") {
      return _prepend(interp, parameters);

    // $list extend list ¿…?
    } else if (convert<std::string>(parameters[0]) == "extend") {
      return _extend(interp, parameters);

    // $list index index ¿=? ¿value?
    } else if (convert<std::string>(parameters[0]) == "index") {
      return _index(interp, parameters);

    // $list foreach item body
    } else if (convert<std::string>(parameters[0]) == "foreach") {
      return _foreach(interp, parameters);

    }
  }
  return result;
}

/**************************************
 * cutlet::list::operator std::string *
 **************************************/

cutlet::list::operator std::string() const {
  std::string result = "{";
  bool first = true;

  for (auto &val: *this) {
    if (not first) result += " ";
    else first = false;
    result += (std::string)(*val);
  }

  result += "}";
  return result;
}

/***********************
 * cutlet::list::_join *
 ***********************/

cutlet::variable_ptr cutlet::list::_join(interpreter &interp,
                                         const list &parameters) {
  // Create our result variable and default deliminator.
  cutlet::string *rvalue = new cutlet::string;
  std::string delim = " ";

  // Set the deliminator if it was specified.
  if (parameters.size() == 2)
    delim = (std::string)*(parameters[1]);

  // Put all our entries together.
  bool first = true;
  for (auto &val: *this) {
    if (not first) *rvalue += delim;
    else first = false;
    *rvalue += (std::string)*val;
  }

  return rvalue;
}

/*************************
 * cutlet::list::_append *
 *************************/

cutlet::variable_ptr cutlet::list::_append(interpreter &interp,
                                           const list &parameters) {
  auto it = parameters.begin(); ++it;
  for (; it != parameters.end(); ++it) push_back(*it);
  return nullptr;
}

/**************************
 * cutlet::list::_prepend *
 **************************/

cutlet::variable_ptr cutlet::list::_prepend(interpreter &interp,
                                            const list &parameters) {
  auto it = parameters.begin(); ++it;
  for (; it != parameters.end(); ++it) push_front(*it);
  return nullptr;
}

/*************************
 * cutlet::list::_extend *
 *************************/

cutlet::variable_ptr cutlet::list::_extend(interpreter &interp,
                                           const list &parameters) {
  auto it = parameters.begin(); ++it;
  for (; it != parameters.end(); ++it) {
    for (auto &item: cast<cutlet::list>(*it)) {
      push_back(item);
    }
  }
  return nullptr;
}

/************************
 * cutlet::list::_index *
 ************************/

cutlet::variable_ptr cutlet::list::_index(interpreter &interp,
                                          const list &parameters) {
  int index = convert<int>(parameters[1]);

  if (parameters.size() == 3) {
    at(index) = parameters[2];
  } else if (parameters.size() == 4) {
    if (convert<std::string>(parameters[2]) == "=")
      throw std::runtime_error(std::string("Unexpected character ") +
                               convert<std::string>(parameters[2]) +
                               ", expected =");
    at(index) = parameters[3];
  }

  return at(index);
}

/**************************
 * cutlet::list::_foreach *
 **************************/

cutlet::variable_ptr cutlet::list::_foreach(interpreter &interp,
                                            const list &parameters) {
  std::string item_name = convert<std::string>(parameters[1]);
  for (auto &item: *this) {
    interp.frame_push();
    interp.local(item_name, item);
    interp.eval(convert<std::string>(parameters[2]));
    interp.frame_pop();
  }
  return nullptr;
}
