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

#include <cutlet.h>

/******************************************************************************
 * class cutlet::list
 */

/**********************
 * cutlet::list::list *
 **********************/

cutlet::list::list() : std::deque<variable::pointer>() {}

cutlet::list::list(const_iterator first, const_iterator last)
  : std::deque<variable::pointer>(first, last) {}

cutlet::list::list(const std::initializer_list<variable::pointer> &items)
  : std::deque<variable::pointer>(items.begin(), items.end()) {}

cutlet::list::list(const list &other) : std::deque<variable::pointer>(other) {}

/**********************
 * cutlet::list::join *
 **********************/

std::string cutlet::list::join(const std::string &delim) const {
  std::string result;
  bool first = true;

  for (auto &val: *this) {
    if (not first) result += delim;
    else first = false;

    //if (not val.is_null())
    result += (std::string)(*val);
  }

  return result;
}

/*****************************
 * cutlet::list::operator () *
 *****************************/

cutlet::variable::pointer cutlet::list::operator()(variable::pointer self,
                                                   interpreter &interp,
                                                   const list &parameters) {
  (void)self;

  variable::pointer result;
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

      // $list remove index ¿end?
    } else if (convert<std::string>(parameters[0]) == "remove") {
      return _remove(interp, parameters);

      // $list foreach item body
    } else if (convert<std::string>(parameters[0]) == "foreach") {
      return _foreach(interp, parameters);

    } else {
      throw std::runtime_error(std::string("Unknown command ") +
                               convert<std::string>(parameters[0]) +
                               " for list variable.");
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

cutlet::variable::pointer cutlet::list::_join(interpreter &interp,
                                              const list &parameters) {
  (void)interp;

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

cutlet::variable::pointer cutlet::list::_append(interpreter &interp,
                                                const list &parameters) {
  (void)interp;

  auto it = parameters.begin(); ++it;
  for (; it != parameters.end(); ++it) push_back(*it);
  return nullptr;
}

/**************************
 * cutlet::list::_prepend *
 **************************/

cutlet::variable::pointer cutlet::list::_prepend(interpreter &interp,
                                                 const list &parameters) {
  (void)interp;

  auto it = parameters.begin(); ++it;
  for (; it != parameters.end(); ++it) push_front(*it);
  return nullptr;
}

/*************************
 * cutlet::list::_extend *
 *************************/

cutlet::variable::pointer cutlet::list::_extend(interpreter &interp,
                                                const list &parameters) {
  (void)interp;

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

cutlet::variable::pointer cutlet::list::_index(interpreter &interp,
                                               const list &parameters) {
  (void)interp;

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

/*************************
 * cutlet::list::_remove *
 *************************/

cutlet::variable::pointer cutlet::list::_remove(interpreter &interp,
                                                const list &parameters) {
  (void)interp;

  int index = convert<int>(parameters[1]);

  if (parameters.size() == 3) {
    int end = convert<int>(parameters[2]);
    erase(begin() + index, begin() + end);
  } else {
    erase(begin() + index);
  }

  return nullptr;
}

/**************************
 * cutlet::list::_foreach *
 **************************/

cutlet::variable::pointer cutlet::list::_foreach(interpreter &interp,
                                                 const list &parameters) {
  // $list foreach item block
  std::string item_name = convert<std::string>(parameters[1]);
  for (auto &item: *this) {
    interp.push(new cutlet::block_frame(interp.frame(0)));
    interp.local(item_name, item);
    interp.compile(parameters[2]);
    interp.pop();
  }
  return nullptr;
}
