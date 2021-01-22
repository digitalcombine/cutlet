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
 *
 * This is the absolute bare bones API used in all newly created Cutlet
 * interpreters.
 */

#include "builtin.h"
#include "utilities.h"
#include "ast.h"
#include <iostream>
#include <sstream>
#include <fstream>

/*****************************************************************************
 * The Cutlet component for functions created by def.
 */

class _def_function : public cutlet::component {
public:
  _def_function(const std::string &label,
                cutlet::variable::pointer parameters,
                cutlet::variable::pointer body)
    : _label(label), _parameters(parameters), _body(body) {}
  virtual ~_def_function() noexcept {}

  /** Execute the function.
   */
  virtual cutlet::variable::pointer
  operator ()(cutlet::interpreter &interp, const cutlet::list &args) {
    interp.push(_label); // New frame for the function.

    // Populate the parameters of the function.
    auto p_it = cutlet::cast<cutlet::list>(_parameters).begin();
    auto a_it = args.begin();

    for (; p_it != cutlet::cast<cutlet::list>(_parameters).end() and
           a_it != args.end(); ++p_it, ++a_it) {
      cutlet::list *l = dynamic_cast<cutlet::list *>(&(*(*p_it)));
      if (l) {
        // Set the value for the parameter that has a default value.
        interp.local(*(l->front()), *a_it);
      } else {
        std::string name(*(*p_it));
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
          interp.local(*(l->front()), l->back());
        } else {
          // Silly programmer.
          throw std::runtime_error("Missing value for parameter " +
                                   (std::string)*(*p_it));
        }
      }
    }

    // We made it, now run the function.
    if (_compiled.is_null())
      _compiled = interp.compile(_body);
    else
      (*_compiled)(interp);

    // Clean up the stack and return a value if there was one.
    return interp.pop();
  }

private:
  std::string _label;
  cutlet::variable::pointer _parameters;
  cutlet::variable::pointer _body;

  cutlet::ast::node::pointer _compiled;
};

/******************************************************************************
 * Cutlets Builtin Public API
 */

/**********************************
 * def def name ¿parameters? body *
 **********************************/

cutlet::variable::pointer
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
  cutlet::variable::pointer body;
  cutlet::variable::pointer def_parameters;
  if (p_count == 2) {
    def_parameters = new cutlet::list();
    body = parameters[1];
  } else {
    def_parameters = interp.list(parameters[1]);
    body = parameters[2];
  }

  // Add the function to the interpreter.
  interp.add(name, new _def_function(name, def_parameters, body));

  return nullptr;
}

/*********************
 * def include *args *
 *********************/

cutlet::variable::pointer
builtin::incl(cutlet::interpreter &interp,
              const cutlet::list &parameters) {
  // Make sure we have at least one argument
  if (parameters.size() == 0) {
    throw std::runtime_error("include called without arguments");
  }

  for (auto &fname: parameters) {
    if (fexists(*fname)) {
      interp.compile_file(*fname);
      return nullptr;
    }

    throw std::runtime_error("import file " +
                             (std::string)*fname +
                             " not found.");
  }
}

/********************
 * def import *args *
 ********************/

cutlet::variable::pointer
builtin::import(cutlet::interpreter &interp,
                const cutlet::list &parameters) {
  // Make sure we have at least one argument
  if (parameters.size() == 0) {
    throw std::runtime_error("import called without arguments");
  }

  cutlet::variable::pointer paths = interp.var("library.path");

  for (auto &libname: parameters) {
    for (auto &path: cutlet::cast<cutlet::list>(paths)) {
      std::string dir(*path);

      if (fexists(dir + "/" + (std::string)(*libname) + ".cutlet")) {
        interp.compile_file(dir + "/" + (std::string)(*libname) + ".cutlet");
        return nullptr;
      } else if (fexists(dir + "/" + (std::string)(*libname) + SOEXT)) {
        interp.load(dir + "/" + (std::string)(*libname) + SOEXT);
        return nullptr;
      }
    }

    throw std::runtime_error("Library " + (std::string)(*libname) +
                             " not found.");
  }


}

/*******************************
 * def global name ¿=? ¿value? *
 *******************************/

cutlet::variable::pointer
builtin::global(cutlet::interpreter &interp,
                const cutlet::list &parameters) {
  switch (parameters.size()) {
  case 0:
    throw std::runtime_error("global called without arguments");
  case 1:
    // global name
    interp.global(*(parameters[0]), nullptr);
    break;
  case 2:
    if (*(parameters[1]) == "=") {
      // global name =
      interp.global(*(parameters[0]), nullptr);
    } else {
      // global name value
      interp.global(*(parameters[0]), parameters[1]);
    }
    break;
  case 3:
    // global name = value
    if (*(parameters[1]) != "=") {
      throw std::runtime_error("global name = value\n"
                               " expected = got " +
                               (std::string)*(parameters[1]));
    }

    interp.global(*(parameters[0]), parameters[2]);
    break;
  default:
    throw std::runtime_error("global called with too many arguments");
  }

  return nullptr;
}

/******************************
 * def local name ¿=? ¿value? *
 ******************************/

cutlet::variable::pointer
builtin::local(cutlet::interpreter &interp,
               const cutlet::list &parameters) {
  switch (parameters.size()) {
  case 0:
    throw std::runtime_error("local called without arguments");
  case 1:
    // local name
    interp.frame(1)->variable(*(parameters[0]), nullptr);
    break;
  case 2:
    if (*(parameters[1]) == "=") {
      // local name =
      interp.frame(1)->variable(*(parameters[0]), nullptr);
    } else {
      // local name value
      interp.frame(1)->variable(*(parameters[0]), parameters[1]);
    }
    break;
  case 3:
    // local name = value
    if (*(parameters[1]) != "=") {
      throw std::runtime_error("local name = value\n"
                               " expected = got " +
                               (std::string)*(parameters[1]));
    }

    interp.frame(1)->variable(*(parameters[0]), parameters[2]);
    break;
  default:
    throw std::runtime_error("local called with too many arguments");
  }

  return nullptr;
}

/*****************************
 * def uplevel ¿levels? body *
 *****************************/

cutlet::variable::pointer
builtin::uplevel(cutlet::interpreter &interp,
                 const cutlet::list &parameters) {
  unsigned int levels = 0, p_count = parameters.size();

  // Sort out the parameters.
  cutlet::variable::pointer body;
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

  // Create the new block frame and execute the body within it.
  try {
    interp.push(new cutlet::block_frame(interp.frame(levels + 1)));
    interp.compile(body);
    interp.pop();
  } catch (...) {
    interp.pop();
    throw;
  }

  return nullptr;
}

/********************
 * def return *args *
 ********************/

cutlet::variable::pointer
builtin::ret(cutlet::interpreter &interp,
             const cutlet::list &parameters) {
  cutlet::frame::pointer uplevel = interp.frame(1);
  switch (parameters.size()) {
  case 0:
    uplevel->done();
    break;
  case 1:
    uplevel->done(parameters[0]);
    break;
  default:
    uplevel->done(new cutlet::list(parameters));
    break;
  }
  return nullptr;
}

/*******************
 * def print *args *
 *******************/

cutlet::variable::pointer
builtin::print(cutlet::interpreter &interp,
               const cutlet::list &parameters) {
  (void)interp;

  std::cout << parameters.join() << std::endl;
  return nullptr;
}

/******************
 * def list *args *
 ******************/

cutlet::variable::pointer
builtin::list(cutlet::interpreter &interp,
              const cutlet::list &parameters) {
  cutlet::variable::pointer result;
  if (parameters.size() == 1)
    result = interp.list(*(parameters[0]));
  else
    result = new cutlet::list(parameters);
  return result;
}

/***************
 * def sandbox *
 ***************/

cutlet::variable::pointer
builtin::sandbox(cutlet::interpreter &interp,
                 const cutlet::list &parameters) {
  (void)interp;
  (void)parameters;

  if (parameters.size() > 0)
    throw std::runtime_error("Invalid number of parameters for sandbox");

  return new builtin::sandbox_var(new cutlet::sandbox);
}
