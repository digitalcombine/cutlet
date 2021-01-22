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
#include <algorithm>

/***********
 * _d_less *
 ***********/

static bool _d_less(cutlet::variable::pointer v1,
                    cutlet::variable::pointer v2) {
  return ((std::string)(*v1) < (std::string)(*v2));
}

class _c_less {
public:
  _c_less(cutlet::interpreter &interp, cutlet::variable::pointer function)
    : _interp(interp), _function(function) {}

  bool operator ()(cutlet::variable::pointer v1,
                   cutlet::variable::pointer v2) {
    cutlet::list parms({v1, v2});
    return cutlet::convert<bool>(_interp.call((std::string)(*_function),
                                              parms));
  }

private:
  cutlet::interpreter &_interp;
  cutlet::variable::pointer _function;
};

/************
 * _d_equal *
 ************/

static bool _d_equal(cutlet::variable::pointer v1,
                     cutlet::variable::pointer v2) {
  return ((std::string)(*v1) == (std::string)(*v2));
}

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

  std::string op = *(parameters[0]);

  // XXX swap
  // XXX reverse
  // XXX unique
  // XXX shuffle

  if (op == "type") {
    if (parameters.size() == 1) {
      return new cutlet::string("list");
    } else {
      throw std::runtime_error(std::string("Invalid number of parameters to "
                                           "list operator type"));
    }

    // $list join ¿delim?
  } else if (op == "join") {
    return _join(interp, parameters);

    // $list size
  } else if (op == "size") {
    if (parameters.size() == 1) {
      return new cutlet::string(size());
    } else {
      throw std::runtime_error(std::string("Invalid number of parameters to "
                                           "list operator size"));
    }

    // $list clear
  } else if (op == "clear") {
    if (parameters.size() == 1) {
      clear();
      return nullptr;
    } else {
      throw std::runtime_error(std::string("Invalid number of parameters to "
                                           "list operator clear"));
    }

    // $list append *args
  } else if (op == "append") {
    return _append(interp, parameters);

    // $list prepend *args
  } else if (op == "prepend") {
    return _prepend(interp, parameters);

    // $list extend *args
  } else if (op == "extend") {
    return _extend(interp, parameters);

    // $list index index ¿=? ¿value?
  } else if (op == "index") {
    return _index(interp, parameters);

    // $list remove index ¿end?
  } else if (op == "remove") {
    return _remove(interp, parameters);

    // $list foreach item body
  } else if (op == "foreach") {
    return _foreach(interp, parameters);

    // $list sort ¿less?
  } else if (op == "sort") {
    return _sort(interp, parameters);

    // $list unique
  } else if (op == "unique") {
    if (parameters.size() == 1) {
      std::sort(begin(), end(), _d_less);
      auto it = std::unique(begin(), end(), _d_equal);
      resize(std::distance(begin(), it));
      return nullptr;
    } else {
      throw std::runtime_error(std::string("Invalid number of parameters to "
                                           "list operator unique"));
    }

    // $list == $other
  } else if (op == "==" or op == "=") {
    return _equal(interp, parameters);

    // $list <> $other
  } else if (op == "<>" or op == "!=") {
    return _nequal(interp, parameters);

  }

  throw std::runtime_error(std::string("Unknown operator ") +
                           op + " for list variable.");
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

  if (parameters.size() < 2 or parameters.size() > 4) {
    throw std::runtime_error("Invalid number of parameters to "
                             "list operator index");
  }
  int index = convert<int>(parameters[1]);

  if (index == 0) {
    // Indexing starts at one so an index of zero is always invalid.
    throw std::runtime_error("List index out of range " +
                             convert<std::string>(parameters[1]));
  } else if (index < 0) {
    // Negative indexes start from the back and index towards the front.
    index = size() + index;
  } else {
    index--;
  }

  if (abs(index) >= size()) {
    // XXX Should specify the size of the list in error message.
    throw std::runtime_error("List index out of range " +
                             convert<std::string>(parameters[1]));
  }

  if (parameters.size() == 3) {
    // $list index value
    at(index) = parameters[2];

  } else if (parameters.size() == 4) {
    // $list index = value
    if (convert<std::string>(parameters[2]) != "=")
      throw std::runtime_error("Unexpected character " +
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
  if (parameters.size() != 3) {
    throw std::runtime_error("Invalid number of parameters to "
                             "list operator foreach");
  }

  // $list foreach item block
  std::string item_name = convert<std::string>(parameters[1]);
  cutlet::ast::node::pointer ast;

  for (auto &item: *this) {
    // Setup the frame.
    interp.push(new cutlet::block_frame(interp.frame(0)));
    interp.local(item_name, item);

    // On the fly compiling and execution.
    if (ast.is_null())
      ast = interp.compile(parameters[2]);
    else
      (*ast)(interp);

    // Clean up.
    interp.pop();
  }

  // We never return anything.
  return nullptr;
}

/************************
 * cutlet::list::_equal *
 ************************/

cutlet::variable::pointer cutlet::list::_equal(interpreter &interp,
                                               const list &parameters) {
  (void)interp;

  if (parameters.size() == 2) {
    cutlet::list &other = cast<cutlet::list>(parameters[1]);

    return (std::equal(begin(), end(), other.begin(), _d_equal)
            ? new cutlet::boolean(true) : new cutlet::boolean(false));
  }

  throw std::runtime_error(std::string("Invalid number of parameters to "
                                       "list operator =="));
}

/************************
 * cutlet::list::_nequal *
 ************************/

cutlet::variable::pointer cutlet::list::_nequal(interpreter &interp,
                                                const list &parameters) {
  (void)interp;

  if (parameters.size() == 2) {
    cutlet::list &other = cast<cutlet::list>(parameters[1]);

    return (std::equal(begin(), end(), other.begin(), _d_equal)
            ? new cutlet::boolean(false) : new cutlet::boolean(true));
  }

  throw std::runtime_error(std::string("Invalid number of parameters to "
                                       "list operator <>"));
}

/************************
 * cutlet::list::_sort *
 ************************/

cutlet::variable::pointer cutlet::list::_sort(interpreter &interp,
                                              const list &parameters) {
  if (parameters.size() == 1) {
    std::sort(begin(), end(), _d_less);
    return nullptr;

  } else if (parameters.size() == 2) {
    _c_less comp_less(interp, parameters[1]);
    std::sort(begin(), end(), comp_less);
    return nullptr;
  }

  throw std::runtime_error(std::string("Invalid number of parameters to "
                                       "list sort ¿less?"));
}
