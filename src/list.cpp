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
#include <algorithm>

/***********
 * _d_less *
 ***********/

static bool _d_less(cutlet::variable::pointer v1,
                    cutlet::variable::pointer v2) {
  // Compares if string v1 is less than string v2

  return ((std::string)(*v1) < (std::string)(*v2));
}

/***********
 * _c_less *
 ***********/

class _c_less {
  /* Use a cutlet function for the less comparison for algorithms.
   */

public:
  _c_less(cutlet::interpreter &interp, cutlet::variable::pointer function)
    : _interp(interp), _function(function) {}

  bool operator ()(cutlet::variable::pointer v1,
                   cutlet::variable::pointer v2) {
    // Call the cutlet function.

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
  // Compares if string v1 is equal than string v2

  return ((std::string)(*v1) == (std::string)(*v2));
}

/******************************************************************************
 * class cutlet::list
 */

/**********************
 * cutlet::list::list *
 **********************/

cutlet::list::list()
  : std::deque<variable::pointer>() {}

cutlet::list::list(const_iterator first, const_iterator last)
  : std::deque<variable::pointer>(first, last) {}

cutlet::list::list(const std::initializer_list<variable::pointer> &items)
  : std::deque<variable::pointer>(items.begin(), items.end()) {}

cutlet::list::list(const list &other)
  : std::deque<variable::pointer>(other) {}

cutlet::list::~list() noexcept {}

/**********************
 * cutlet::list::join *
 **********************/

std::string cutlet::list::join(const std::string &delim) const {
  std::string result;
  bool first = true;

  for (auto &val: *this) {
    if (not first) result += delim;
    else first = false;

    if (not val.is_null())
      result += (std::string)(*val);
  }

  return result;
}

/*****************************
 * cutlet::list::operator () *
 *****************************/

cutlet::variable::pointer cutlet::list::operator()(variable::pointer self,
                                                   interpreter &interp,
                                                   const list &arguments) {
  (void)self;

  std::string op = *(arguments[0]);

  switch (op[0]) {
  case '=':
    if (op == "==" or op == "=") {
      // $list == $other or $list = $other
      return _equal(interp, arguments);
    }
    break;
  case '<':
  case '!':
    if (op == "<>" or op == "!=") {
      // $list <> $other or $list != $other
      return _nequal(interp, arguments);
    }
    break;
  case 'a':
    if (op == "append") {
      // $list append *args
      return _append(interp, arguments);
    }
    break;
  case 'c':
    if (op == "clear") {
      // $list clear

      if (arguments.size() == 1) {
        clear();
        return nullptr;
      } else {
        throw std::runtime_error(std::string("Invalid number of arguments to "
                                             "$list operator clear"));
      }
    }
    break;
  case 'e':
    if (op == "extend") {
      // $list extend *args
      return _extend(interp, arguments);
    }
    break;
  case 'f':
    if (op == "foreach") {
      // $list foreach item body
      return _foreach(interp, arguments);
    }
    break;
  case 'i':
    if (op == "index") {
      // $list index index ¿¿=? value?
      return _index(interp, arguments);
    }
    break;
  case 'j':
    if (op == "join") {
      // $list join ¿delim?
      return _join(interp, arguments);
    }
    break;
  case 'p':
    if (op == "prepend") {
      // $list prepend *args
      return _prepend(interp, arguments);
    }
    break;
  case 'r':
    if (op == "remove") {
      // $list remove index ¿end?
      return _remove(interp, arguments);

    } else if (op == "reverse") {
      // $list remove index ¿end?
      return _reverse(interp, arguments);
    }
    break;
  case 's':
    if (op == "shuffle") {
      // $list shuffle
      return _shuffle(interp, arguments);

    } else if (op == "size") {
      // $list size
      if (arguments.size() == 1) {
        return new cutlet::string(size());
      } else {
        throw std::runtime_error(std::string("Invalid number of arguments to "
                                             "$list size"));
      }
    } else if (op == "sort") {
      // $list sort ¿less?
      return _sort(interp, arguments);
    }
    break;
  case 't':
    if (op == "type") {
      // $list type
      if (arguments.size() == 1) {
        return new cutlet::string("list");
      } else {
        throw std::runtime_error(std::string("Invalid number of arguments to "
                                             "$list type"));
      }
    }
    break;
  case 'u':
    if (op == "unique") {
      // $list unique
      return _unique(interp, arguments);
    }
    break;
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
                                              const list &arguments) {
  (void)interp;

  // Create our result variable and default deliminator.
  cutlet::string *rvalue = new cutlet::string;
  std::string delim = " ";

  // Set the deliminator if it was specified.
  if (arguments.size() == 2)
    delim = (std::string)*(arguments[1]);

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
                                                const list &arguments) {
  (void)interp;

  auto it = arguments.begin(); ++it;
  for (; it != arguments.end(); ++it) push_back(*it);
  return nullptr;
}

/**************************
 * cutlet::list::_prepend *
 **************************/

cutlet::variable::pointer cutlet::list::_prepend(interpreter &interp,
                                                 const list &arguments) {
  (void)interp;

  auto it = arguments.begin(); ++it;
  for (; it != arguments.end(); ++it) push_front(*it);
  return nullptr;
}

/*************************
 * cutlet::list::_extend *
 *************************/

cutlet::variable::pointer cutlet::list::_extend(interpreter &interp,
                                                const list &arguments) {
  (void)interp;

  auto it = arguments.begin(); ++it;
  for (; it != arguments.end(); ++it) {
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
                                               const list &arguments) {
  (void)interp;

  if (arguments.size() < 2 or arguments.size() > 4) {
    throw std::runtime_error("Invalid number of arguments to "
                             "$list index index ¿¿=? value?");
  }
  int index = convert<int>(arguments[1]);

  // Negative indexes start from the back and index towards the front.
  if (index < 0) {
    index = size() + index;
  } else {
    index--;
  }

  // Make sure the index is in range.
  if (index < 0 or index >= (long long)size()) {
    // XXX Should specify the size of the list in error message.
    throw std::runtime_error("List index out of range " +
                             (std::string)*(arguments[1]));
  }

  if (arguments.size() == 3) {
    // $list index value
    at(index) = arguments[2];

  } else if (arguments.size() == 4) {
    // $list index = value
    if (*(arguments[2]) != "=")
      throw std::runtime_error("Unexpected character " +
                               (std::string)*(arguments[2]) +
                               ", expected =");
    at(index) = arguments[3];
  }

  return at(index);
}

/*************************
 * cutlet::list::_remove *
 *************************/

cutlet::variable::pointer cutlet::list::_remove(interpreter &interp,
                                                const list &arguments) {
  (void)interp;

  int index = convert<int>(arguments[1]);

  if (arguments.size() == 3) {
    int end = convert<int>(arguments[2]);
    erase(begin() + index, begin() + end);
  } else {
    erase(begin() + index);
  }

  return nullptr;
}

/**************************
 * cutlet::list::_reverse *
 **************************/

cutlet::variable::pointer cutlet::list::_reverse(interpreter &interp,
                                                 const list &arguments) {
  (void)interp;

  if (arguments.size() == 1) {
    std::reverse(begin(), end());
    return nullptr;
  } else {
    throw std::runtime_error(std::string("Invalid number of arguments to "
                                         "$list reverse"));
  }
}

/**************************
 * cutlet::list::_foreach *
 **************************/

cutlet::variable::pointer cutlet::list::_foreach(interpreter &interp,
                                                 const list &arguments) {
  if (arguments.size() != 3) {
    throw std::runtime_error("Invalid number of arguments to "
                             "$list foreach item body");
  }

  // $list foreach item block
  std::string item_name = convert<std::string>(arguments[1]);
  cutlet::ast::node::pointer ast;

  for (auto &item: *this) {
    // Setup the frame.
    interp.push(new cutlet::block_frame(interp.frame(0)));
    interp.local(item_name, item);

    // On the fly compiling and execution.
    if (ast.is_null())
      ast = interp(arguments[2]);
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
                                               const list &arguments) {
  (void)interp;

  if (arguments.size() == 2) {
    cutlet::list &other = cast<cutlet::list>(arguments[1]);

    return (std::equal(begin(), end(), other.begin(), _d_equal)
            ? new cutlet::boolean(true) : new cutlet::boolean(false));
  }

  throw std::runtime_error(std::string("Invalid number of arguments to "
                                       "$list == other"));
}

/*************************
 * cutlet::list::_nequal *
 *************************/

cutlet::variable::pointer cutlet::list::_nequal(interpreter &interp,
                                                const list &arguments) {
  (void)interp;

  if (arguments.size() == 2) {
    cutlet::list &other = cast<cutlet::list>(arguments[1]);

    return (std::equal(begin(), end(), other.begin(), _d_equal)
            ? new cutlet::boolean(false) : new cutlet::boolean(true));
  }

  throw std::runtime_error(std::string("Invalid number of arguments to "
                                       "$list <> other"));
}

/**************************
 * cutlet::list::_shuffle *
 **************************/

cutlet::variable::pointer cutlet::list::_shuffle(interpreter &interp,
                                                 const list &arguments) {
  if (arguments.size() == 1) {
    std::random_shuffle(begin(), end());
    return nullptr;

  } else if (arguments.size() == 2) {
    throw std::runtime_error(std::string("Invalid number of arguments to "
                                         "$list shuffle"));
  }
}

/***********************
 * cutlet::list::_sort *
 ***********************/

cutlet::variable::pointer cutlet::list::_sort(interpreter &interp,
                                              const list &arguments) {
  if (arguments.size() == 1) {
    std::sort(begin(), end(), _d_less);
    return nullptr;

  } else if (arguments.size() == 2) {
    _c_less comp_less(interp, arguments[1]);
    std::sort(begin(), end(), comp_less);
    return nullptr;
  }

  throw std::runtime_error(std::string("Invalid number of arguments to "
                                       "$list sort ¿less?"));
}

/*************************
 * cutlet::list::_unique *
 *************************/

cutlet::variable::pointer cutlet::list::_unique(interpreter &interp,
                                                const list &arguments) {
  if (arguments.size() == 1) {
    std::sort(begin(), end(), _d_less);
    auto it = std::unique(begin(), end(), _d_equal);
    resize(std::distance(begin(), it));
    return nullptr;
  } else {
    throw std::runtime_error(std::string("Invalid number of arguments to "
                                         "$list unique"));
  }
}
