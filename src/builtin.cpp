/*                                                                  -*- c++ -*-
 * Copyright © 2018 Ron R Wills <ron@digitalcombine.ca>
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
      _compiled = interp(_body);
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
    } else {
      throw std::runtime_error("include file " +
                               (std::string)*fname +
                               " not found.");
    }
  }

  return nullptr;
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

/**********************************
 * def uplevel ¿levels? body      *
 * def uplevel expr ¿levels? body *
 **********************************/

cutlet::variable::pointer
builtin::uplevel(cutlet::interpreter &interp,
                 const cutlet::list &arguments) {
  unsigned int levels = 2;
  size_t p_count = arguments.size();
  bool expr = false;

  if (p_count < 1 or p_count > 3) {
    std::stringstream mesg;
    mesg << "Invalid number of arguments for uplevel "
         << " (1 < " << p_count
         << " > 3)\n uplevel ¿expr? ¿levels? body";
    throw std::runtime_error(mesg.str());
  }

  // Sort out the arguments.
  cutlet::variable::pointer body(arguments[p_count - 1]);
  if (p_count >= 2) {
    int arg = 0;
    if (*arguments[0] == "expr") {
      expr = true;

      if (p_count == 3)
        arg = cutlet::convert<int>(arguments[1]);

    } else {
      arg = cutlet::convert<int>(arguments[0]);
    }

    if (arg < 0) {
      levels = interp.frames() + arg;
    } else {
      levels = arg + 2;
    }
  }

  // Create the new block frame and execute the body within it.
  cutlet::variable::pointer result;

  interp.push(levels, "uplevel body");

  /*std::clog << "TRACE: uplevel " << levels << std::endl;
  for (int i = 0; i < interp.frames(); i++) {
    std::clog << interp.frame(i) << std::endl;
    }*/

  if (expr)
    result = interp.expr(body);
  else
    interp(body);
  interp.pop();

  return result;
}

/********************
 * def return *args *
 ********************/

cutlet::variable::pointer
builtin::ret(cutlet::interpreter &interp,
             const cutlet::list &arguments) {
  cutlet::frame::pointer frame = interp.frame(1);
  switch (arguments.size()) {
  case 0:
    frame->state(cutlet::frame::FS_DONE);
    break;
  case 1:
    frame->done(arguments[0]);
    break;
  default:
    frame->done(new cutlet::list(arguments));
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
