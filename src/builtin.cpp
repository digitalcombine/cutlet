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
                cutlet::variable::pointer arguments,
                cutlet::variable::pointer body)
    : _label(label), _arguments(arguments), _body(body) {}
  virtual ~_def_function() noexcept;

  /** Execute the function.
   */
  virtual cutlet::variable::pointer
  operator ()(cutlet::interpreter &interp, const cutlet::list &args) {
    interp.push(_label); // New frame for the function.

    // Populate the arguments of the function.
    auto p_it = cutlet::cast<cutlet::list>(_arguments).begin();
    auto a_it = args.begin();

    for (; p_it != cutlet::cast<cutlet::list>(_arguments).end() and
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

    /* If there are any remaining arguments that we didn't get a value for
     * the attempt to populate them with default values if any were given.
     */
    if (cutlet::cast<cutlet::list>(_arguments).size() > args.size()) {
      // Set default parameter values if needed.
      while (p_it != cutlet::cast<cutlet::list>(_arguments).end()) {
        cutlet::list *l = dynamic_cast<cutlet::list *>(&(*(*p_it)));
        if (l) {
          interp.local(*(l->front()), l->back());
        } else {
          // Silly programmer.
          throw std::runtime_error("Missing value for argument " +
                                   (std::string)*(*p_it) +
                                   " for function " + _label);
        }
      }
    }

    // We made it, now run the function.
    if (_compiled.is_null()) {
      (std::string)*_body; // XXX This shouldn't be necessary!!
      _compiled = interp.compile(_body);
    } else
      (*_compiled)(interp);

    // Clean up the stack and return a value if there was one.
    return interp.pop();
  }

private:
  std::string _label;
  cutlet::variable::pointer _arguments;
  cutlet::variable::pointer _body;

  cutlet::ast::node::pointer _compiled;
};

_def_function::~_def_function() noexcept {}

/******************************************************************************
 * Cutlets Builtin Public API
 */

/*********************************
 * def def name ¿arguments? body *
 *********************************/

cutlet::variable::pointer
builtin::def(cutlet::interpreter &interp,
             const cutlet::list &arguments) {

  // Make sure we have to right number of arguments.
  size_t p_count = arguments.size();
  if (p_count < 2 or p_count > 3) {
   std::stringstream mesg;
   mesg << "Invalid number of arguments for def "
        << (p_count >= 1 ? cutlet::convert<std::string>(arguments[0]) : "")
        << " (2 <= " << p_count
        << " <= 3).\n def name ¿arguments? body";
   throw std::runtime_error(mesg.str());
  }

  // Get the function name.
  std::string name = *(arguments[0]);

  // Get the method arguments and body.
  cutlet::variable::pointer body;
  cutlet::variable::pointer def_arguments;
  if (p_count == 2) {
    def_arguments = new cutlet::list();
    body = arguments[1];
  } else {
    def_arguments = interp.list(arguments[1]);
    body = arguments[2];
  }

  // Add the function to the interpreter.
  interp.add(name, new _def_function(name, def_arguments, body));

  return nullptr;
}

/*********************
 * def include *args *
 *********************/

cutlet::variable::pointer
builtin::incl(cutlet::interpreter &interp,
              const cutlet::list &arguments) {
  // Make sure we have at least one argument
  if (arguments.size() == 0) {
    throw std::runtime_error("include called without arguments");
  }

  for (auto &fname: arguments) {
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
                const cutlet::list &arguments) {
  // Make sure we have at least one argument.
  if (arguments.size() == 0) {
    throw std::runtime_error("import called without arguments");
  }

  // Get the library search paths.
  cutlet::variable::pointer paths = interp.var("library.path");

  // Iterate through our arguments.
  for (auto &libname: arguments) {
    bool lib_loaded = false;

    // Iterate through the library paths.
    for (auto &path: cutlet::cast<cutlet::list>(paths)) {
      std::string dir(*path);

      // If the library exists, load it.
      if (fexists(dir + "/" + (std::string)(*libname) + ".cutlet")) {
        interp.compile_file(dir + "/" + (std::string)(*libname) + ".cutlet");
        lib_loaded = true;
        break;
      } else if (fexists(dir + "/" + (std::string)(*libname) + SOEXT)) {
        interp.load(dir + "/" + (std::string)(*libname) + SOEXT);
        lib_loaded = true;
        break;
      }
    }

    // If the library wasn't found in any of the search paths raise an error.
    if (not lib_loaded) {
      throw std::runtime_error("Library " + (std::string)(*libname) +
                               " not found.");
    }
  }

  // All done.
  return nullptr;
}

/*******************************
 * def global name ¿=? ¿value? *
 *******************************/

cutlet::variable::pointer
builtin::global(cutlet::interpreter &interp,
                const cutlet::list &arguments) {
  switch (arguments.size()) {
  case 0:
    throw std::runtime_error("global called without arguments");
  case 1:
    // global name
    interp.global(*(arguments[0]), nullptr);
    break;
  case 2:
    if (*(arguments[1]) == "=") {
      // global name =
      interp.global(*(arguments[0]), nullptr);
    } else {
      // global name value
      interp.global(*(arguments[0]), arguments[1]);
    }
    break;
  case 3:
    // global name = value
    if (*(arguments[1]) != "=") {
      throw std::runtime_error("global name = value\n"
                               " expected = got " +
                               (std::string)*(arguments[1]));
    }

    interp.global(*(arguments[0]), arguments[2]);
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
               const cutlet::list &arguments) {
  switch (arguments.size()) {
  case 0:
    throw std::runtime_error("local called without arguments");
  case 1:
    // local name
    interp.frame(1)->variable(*(arguments[0]), nullptr);
    break;
  case 2:
    if (*(arguments[1]) == "=") {
      // local name =
      interp.frame(1)->variable(*(arguments[0]), nullptr);
    } else {
      // local name value
      interp.frame(1)->variable(*(arguments[0]), arguments[1]);
    }
    break;
  case 3:
    // local name = value
    if (*(arguments[1]) != "=") {
      throw std::runtime_error("local name = value\n"
                               " expected = got " +
                               (std::string)*(arguments[1]));
    }

    interp.frame(1)->variable(*(arguments[0]), arguments[2]);
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
                 const cutlet::list &arguments) {
  unsigned int levels = 0;
  size_t p_count = arguments.size();

  // Sort out the arguments.
  cutlet::variable::pointer body;
  if (p_count == 2) {
    int arg = cutlet::convert<int>(arguments[0]);
    if (arg < 0) {
      std::stringstream mesg;
      mesg << "Invalid argument level for uplevel "
           << " (" << arg
           << " >= 0).\n uplevel ¿levels? body";
      throw std::runtime_error(mesg.str());
    }
    levels = (unsigned int)arg;
    body = arguments[1];
  } else if (p_count == 1) {
    body = arguments[0];
  } else {
    std::stringstream mesg;
    mesg << "Invalid number of arguments for uplevel "
         << " (1 <= " << p_count
         << " <= 2).\n uplevel ¿levels? body";
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
             const cutlet::list &arguments) {
  cutlet::frame::pointer uplevel = interp.frame(1);
  switch (arguments.size()) {
  case 0:
    uplevel->done();
    break;
  case 1:
    uplevel->done(arguments[0]);
    break;
  default:
    uplevel->done(new cutlet::list(arguments));
    break;
  }
  return nullptr;
}

/*******************
 * def print *args *
 *******************/

cutlet::variable::pointer
builtin::print(cutlet::interpreter &interp,
               const cutlet::list &arguments) {
  (void)interp;

  std::cout << arguments.join() << std::endl;
  return nullptr;
}

/******************
 * def list *args *
 ******************/

cutlet::variable::pointer
builtin::list(cutlet::interpreter &interp,
              const cutlet::list &arguments) {
  cutlet::variable::pointer result;
  if (arguments.size() == 1)
    result = interp.list(*(arguments[0]));
  else
    result = new cutlet::list(arguments);
  return result;
}

/***************
 * def sandbox *
 ***************/

cutlet::variable::pointer
builtin::sandbox(cutlet::interpreter &interp,
                 const cutlet::list &arguments) {
  (void)interp;
  (void)arguments;

  if (arguments.size() > 0)
    throw std::runtime_error("Invalid number of arguments for sandbox");

  return new builtin::sandbox_var(new cutlet::sandbox);
}
