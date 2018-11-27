/*                                                                 -*- c++ -*-
 * Copyright © Ron R Wills
 * All rights reserved
 */

#include "builtin.h"
#include "utilities.h"
#include <iostream>
#include <fstream>

/*****************************************************************************
 */

class _def_function : public cutlet::component {
public:
  _def_function(cutlet::variable_ptr parameters,
                cutlet::variable_ptr body)
    : _parameters(parameters), _body(body) {}
  virtual ~_def_function() noexcept {}

  virtual cutlet::variable_ptr
  operator ()(cutlet::interpreter &interp, const cutlet::list &args) {
    interp.frame_push();

    auto p_it = cutlet::cast<cutlet::list>(_parameters).begin();
    auto a_it = args.begin();

    for (; p_it != cutlet::cast<cutlet::list>(_parameters).end() and
           a_it != args.end(); ++p_it, ++a_it) {
      cutlet::list *l = dynamic_cast<cutlet::list *>(&(*(*p_it)));
      if (l) {
        interp.local((std::string)*(l->front()), *a_it);
      } else {
        std::string name((std::string)*(*p_it));
        if (name == "*args") {
          interp.local("args", new cutlet::list(a_it, args.end()));
        } else {
          interp.local(name, *a_it);
        }
      }
    }

    if (cutlet::cast<cutlet::list>(_parameters).size() > args.size()) {
      // Set default parameter values.
      while (p_it != cutlet::cast<cutlet::list>(_parameters).end()) {
        cutlet::list *l = dynamic_cast<cutlet::list *>(&(*(*p_it)));
        if (l) {
          interp.local((std::string)*(l->front()), l->back());
        } else
          throw std::runtime_error(
            std::string("Missing value for parameter ") +
            (std::string)*(*p_it));
      }
    }

    interp.eval((std::string)*_body);

    return interp.frame_pop();
  }

private:
  cutlet::variable_ptr _parameters;
  cutlet::variable_ptr _body;
};

/*****************************************************************************
 */

// def name ¿parameters? body
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

  interp.add(name, new _def_function(def_parameters, body));

  return nullptr;
}

// import library
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
    // XXX Trigger an error here.
    if ((std::string)*(parameters[1]) != "=") return nullptr;

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
