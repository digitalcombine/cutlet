/*                                                                 -*- c++ -*-
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
#include "utilities.h"
#include <iostream>
#include <sstream>
#include <fstream>

/*****************************************************************************
 * class _block
 *
 *  A _block is a specialized type of frame that includes variables from a
 * parent frame to be included in local variable substituions.
 */

class _block : public cutlet::frame {
public:
  _block(cutlet::frame_ptr parent) : _parent(parent) {}
  virtual ~_block() noexcept {}

  virtual cutlet::variable_ptr variable(const std::string &name) const {
    cutlet::variable_ptr result = cutlet::frame::variable(name);
    if (result.is_null()) result = _parent->variable(name);
    return result;
  }

private:
  cutlet::frame_ptr _parent;
};

/*****************************************************************************
 */

class _def_function : public cutlet::component {
public:
  _def_function(cutlet::variable_ptr parameters,
                cutlet::variable_ptr body)
    : _parameters(parameters), _body(body) {}
  virtual ~_def_function() noexcept {}

  /** Execute the function.
   */
  virtual cutlet::variable_ptr
  operator ()(cutlet::interpreter &interp, const cutlet::list &args) {
    interp.frame_push(); // New frame for the function.

    // Populate the parameters of the function.
    auto p_it = cutlet::cast<cutlet::list>(_parameters).begin();
    auto a_it = args.begin();

    for (; p_it != cutlet::cast<cutlet::list>(_parameters).end() and
           a_it != args.end(); ++p_it, ++a_it) {
      cutlet::list *l = dynamic_cast<cutlet::list *>(&(*(*p_it)));
      if (l) {
        // Set the value for the parameter that has a default value.
        interp.local((std::string)*(l->front()), *a_it);
      } else {
        std::string name((std::string)*(*p_it));
        if (name == "*args") {
          // If *args found populate it with a list of the remaining values.
          interp.local("args", new cutlet::list(a_it, args.end()));
        } else {
          // Set the value for the parameter.
          interp.local(name, *a_it);
        }
      }
    }

    /* If there are any remaining parameters that we didn't get a value for
     * the attempt to populate them with default values if any were given.
     */
    if (cutlet::cast<cutlet::list>(_parameters).size() > args.size()) {
      // Set default parameter values if needed.
      while (p_it != cutlet::cast<cutlet::list>(_parameters).end()) {
        cutlet::list *l = dynamic_cast<cutlet::list *>(&(*(*p_it)));
        if (l) {
          interp.local((std::string)*(l->front()), l->back());
        } else {
          // Silly programmer.
          throw std::runtime_error(
            std::string("Missing value for parameter ") +
            (std::string)*(*p_it));
        }
      }
    }

    // We made it, now run the function.
    interp.eval((std::string)*_body);

    // Clean up the stack and return a value if there was one.
    return interp.frame_pop();
  }

private:
  cutlet::variable_ptr _parameters;
  cutlet::variable_ptr _body;
};

/*****************************************************************************
 */

// def def name ¿parameters? body
cutlet::variable_ptr
builtin::def(cutlet::interpreter &interp,
             const cutlet::list &parameters) {
  // Make sure we have to right number of parameters.
  size_t p_count = parameters.size();
  if (p_count < 2 or p_count > 3) {
   std::stringstream mesg;
   mesg << "Invalid number of parameters for def "
        << (p_count >= 1 ? cutlet::convert<std::string>(parameters[0]) : "")
        << " (2 <= " << p_count
        << " <= 3).\n def name ¿parameters? body";
   throw std::runtime_error(mesg.str());
  }

  // Get the function name.
  std::string name = *(parameters[0]);

  // Get the method parameters and body.
  cutlet::variable_ptr body;
  cutlet::variable_ptr def_parameters;
  if (p_count == 2) {
    def_parameters = new cutlet::list();
    body = parameters[1];
  } else {
    def_parameters = interp.list(*(parameters[1]));
    body = parameters[2];
  }

  // Add the function to the interpreter.
  interp.add(name, new _def_function(def_parameters, body));

  return nullptr;
}

// def import library
cutlet::variable_ptr
builtin::import(cutlet::interpreter &interp,
                const cutlet::list &parameters) {
  cutlet::variable_ptr paths = interp.var("library.path");
  std::string libname = cutlet::convert<std::string>(parameters[0]);

  for (auto &path: cutlet::cast<cutlet::list>(paths)) {
    std::string dir = cutlet::convert<std::string>(path);
    if (fexists(dir + "/" + libname + ".ctl")) {
      std::ifstream libfile(dir + "/" + libname + ".ctl");
      interp.eval(libfile);
      libfile.close();
      return nullptr;
    } else if (fexists(dir + "/" + libname + SOEXT)) {
      interp.load(dir + "/" + libname + SOEXT);
      return nullptr;
    }
  }

  throw std::runtime_error(std::string("Library ") + libname + " not found.");
}

// global name ¿=? value
cutlet::variable_ptr
builtin::global(cutlet::interpreter &interp,
                const cutlet::list &parameters) {
  if (parameters.size() == 2) {
    interp.global((std::string)*(parameters[0]), parameters[1]);
  } else if (parameters.size() == 3) {
    if ((std::string)*(parameters[1]) != "=") {
      throw std::runtime_error(std::string("global name ¿=? value\n"
                                           " Expected = got ") +
                                           (std::string)*(parameters[1]));
    }

    interp.global((std::string)*(parameters[0]), parameters[2]);
  }

  return nullptr;
}

// local name ¿=? value
cutlet::variable_ptr
builtin::local(cutlet::interpreter &interp,
               const cutlet::list &parameters) {

  if (parameters.size() == 2) {
    interp.local((std::string)*(parameters[0]), parameters[1]);
  } else if (parameters.size() == 3) {
    // XXX Trigger an error here.
    if ((std::string)*(parameters[1]) != "=") return nullptr;

    interp.local((std::string)*(parameters[0]), parameters[2]);
  }

  return nullptr;
}

// block ¿levels? body
cutlet::variable_ptr
builtin::block(cutlet::interpreter &interp,
               const cutlet::list &parameters) {
  unsigned int levels = 0, p_count = parameters.size();
  cutlet::variable_ptr body;
  if (p_count == 2) {
    levels = cutlet::convert<int>(parameters[0]);
    body = parameters[1];
  } else if (p_count == 1) {
    body = parameters[0];
  } else {
    std::stringstream mesg;
    mesg << "Invalid number of parameters for block "
         << " (1 <= " << p_count
         << " <= 2).\n block ¿levels? body";
    throw std::runtime_error(mesg.str());
  }

  cutlet::frame_ptr parent = interp.frame(levels);
  cutlet::frame_ptr block_frame = new _block(parent);

  interp.frame_push(block_frame);
  interp.eval(*body);
  interp.frame_pop();
}

// return ¿value?
cutlet::variable_ptr
builtin::ret(cutlet::interpreter &interp,
             const cutlet::list &parameters) {
  if (parameters.size() > 0)
    interp.frame_done(parameters[0]);
  else
    interp.frame_done();
  return nullptr;
}

// print *args
cutlet::variable_ptr
builtin::print(cutlet::interpreter &interp,
               const cutlet::list &parameters) {
  bool first = true;
  for (auto &value: parameters) {
    if (not first) std::cout << " ";
    if (not value.is_null()) std::cout << (std::string)(*value);
    first = false;
  }
  std::cout << std::endl;
  return nullptr;
}

// list *args
cutlet::variable_ptr
builtin::list(cutlet::interpreter &interp,
              const cutlet::list &parameters) {
  cutlet::variable_ptr result;
  if (parameters.size() == 1)
    result = interp.list(cutlet::convert<std::string>(parameters[0]));
  else
    result = new cutlet::list(parameters);
  return result;
}
