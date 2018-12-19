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
#include <iostream>
#include <sstream>
#include <set>

class _def_class;
class _var_object;

/** Method definition as a cutlet component.
 */
class _def_method : public cutlet::component {
public:
  _def_method(cutlet::variable_ptr parameters,
              cutlet::variable_ptr body);
  _def_method(const _def_method &other) = delete;

  virtual ~_def_method() noexcept;

  virtual cutlet::variable_ptr
  operator ()(cutlet::interpreter &interp, const cutlet::list &args);

private:
  cutlet::variable_ptr _parameters;
  cutlet::variable_ptr _body;
};

/** An object as a cutlet variable.
 */
class _var_object : public cutlet::variable {
public:
  _var_object(cutlet::component_ptr cls);
  _var_object(const _var_object &other) = delete;

  virtual ~_var_object() noexcept;

  virtual cutlet::variable_ptr
  operator()(cutlet::variable_ptr self, cutlet::interpreter &interp,
             const cutlet::list &parameters);

  bool has_property(const std::string &name) const;
  cutlet::variable_ptr property(const std::string &name) const;
  void property(const std::string &name, cutlet::variable_ptr value);

private:
  cutlet::component_ptr _class;
  std::map<std::string, cutlet::variable_ptr> _properties;
};

/** Class definition as a cutlet component.
 */
class _def_class : public cutlet::component {
public:
  _def_class(cutlet::interpreter &interp,
             const std::string &name,
             cutlet::variable_ptr parents);
  _def_class(const _def_class &other) = delete;

  virtual ~_def_class() noexcept;

  /** Class methods are called from here.
   */
  virtual cutlet::variable_ptr
  operator ()(cutlet::interpreter &interp, const cutlet::list &parameters);

  /** Method calls from the object.
   */
  virtual cutlet::variable_ptr
  operator ()(const std::string &method,
              cutlet::interpreter &interp, const cutlet::list &parameters);

  void add(const std::string &name, cutlet::component_ptr comp);

  void add_class(const std::string &name, cutlet::component_ptr comp);

  void property(const std::string &name);

  bool has_property(const std::string &name) const;

private:
  std::string _name;
  std::list<cutlet::component_ptr> _parents;
  std::map<std::string, cutlet::variable_ptr> class_properties;
  std::set<std::string> _properties;

  std::map<std::string, cutlet::component_ptr> _class_methods;
  std::map<std::string, cutlet::component_ptr> _methods;
};

/** Object execution frames to implement object and class properties.
 */
class _obj_frame : public cutlet::frame {
public:
  _obj_frame(cutlet::variable_ptr self);
  virtual ~_obj_frame() noexcept;

  /** We override these methods to implement object properties.
   */
  virtual cutlet::variable_ptr variable(const std::string &name) const;
  virtual void variable(const std::string &name, cutlet::variable_ptr value);

private:
  cutlet::variable_ptr _self;
};

/** Return the global reference to the class sandbox. We need to wrap it in a
 * function for the created dynamic library, otherwise the global variable
 * will be duplicated having unexpected results.
 */
static cutlet::sandbox_ptr &_oo_sandbox() {
  static cutlet::sandbox_ptr _sb_class;
  return _sb_class;
}

/*****************************************************************************
 * class _def_method
 */

/****************************
 * _def_method::_def_method *
 ****************************/

_def_method::_def_method(cutlet::variable_ptr parameters,
                         cutlet::variable_ptr body)
  : _parameters(parameters), _body(body) {
}

/*****************************
 * _def_method::~_def_method *
 *****************************/

_def_method::~_def_method() noexcept {
}

/****************************
 * _def_method::operator () *
 ****************************/

cutlet::variable_ptr _def_method::operator ()(cutlet::interpreter &interp,
                                              const cutlet::list &args) {
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
        throw
          std::runtime_error(std::string("Missing value for parameter ") +
                             (std::string)*(*p_it));
    }
  }

  interp.eval((std::string)*_body);
  return nullptr;
}

/*****************************************************************************
 * class _var_object
 */

_var_object::_var_object(cutlet::component_ptr cls) : _class(cls) {
}

_var_object::~_var_object() noexcept {
}

/************************
 * _var_object::execute *
 ************************/

cutlet::variable_ptr
_var_object::operator()(cutlet::variable_ptr self, cutlet::interpreter &interp,
                        const cutlet::list &parameters) {
  cutlet::list params(parameters.begin() + 1, parameters.end());

  //cutlet::variable_ptr self = this; ++self;
  interp.frame_push(new _obj_frame(self));
  try {
    dynamic_cast<_def_class &>(*(_class))(*(parameters[0]),
                                          interp, params);
  } catch (std::exception &err) {
    interp.frame_pop();
    throw err;
  }
  return interp.frame_pop();
}

/*****************************
 * _var_object::has_property *
 *****************************/

bool _var_object::has_property(const std::string &name) const {
  return dynamic_cast<const _def_class &>(*(_class)).has_property(name);
}

/*************************
 * _var_object::property *
 *************************/

cutlet::variable_ptr _var_object::property(const std::string &name) const {
  return _properties.at(name);
}

void _var_object::property(const std::string &name,
                           cutlet::variable_ptr value) {
  _properties[name] = value;
}

/*****************************************************************************
 * class _def_class
 */

/**************************
 * _def_class::_def_class *
 **************************/

_def_class::_def_class(cutlet::interpreter &interp,
                       const std::string &name,
                       cutlet::variable_ptr parents) : _name(name) {
  for (auto &parent_name: cutlet::cast<cutlet::list>(parents)) {
    std::string name = cutlet::convert<std::string>(parent_name);
    _parents.push_back(interp.get(name));
  }
}

/***************************
 * _def_class::~_def_class *
 ***************************/

_def_class::~_def_class() noexcept {
}

/***************************
 * _def_class::operator () *
 ***************************/

cutlet::variable_ptr _def_class::operator ()(cutlet::interpreter &interp,
                                             const cutlet::list &parameters) {
  cutlet::variable_ptr object;

  if (cutlet::convert<std::string>(parameters[0]) == "new") {
    // Create our new object
    _var_object *obj = new _var_object(interp.get(_name));
    for (auto &property: _properties) { // Add the properties
      obj->property(property, nullptr);
    }

    object = obj;
    (*object)(object, interp, parameters);
    return object;
  } else {

  }
  return nullptr;
}

cutlet::variable_ptr _def_class::operator ()(const std::string &method,
                                             cutlet::interpreter &interp,
                                             const cutlet::list &parameters) {
  auto m = _methods.find(method);
  if (m != _methods.end()) {
    (*(m->second))(interp, parameters);
  }
  return nullptr;
}

/*******************
 * _def_class::add *
 *******************/

void _def_class::add(const std::string &name, cutlet::component_ptr comp) {
  _methods[name] = comp;
}

/*************************
 * _def_class::add_class *
 *************************/

void _def_class::add_class(const std::string &name,
                           cutlet::component_ptr comp) {
  _class_methods[name] = comp;
}

/************************
 * _def_class::property *
 ************************/

void _def_class::property(const std::string &name) {
  _properties.insert(name);
}

/****************************
 * _def_class::has_property *
 ****************************/

bool _def_class::has_property(const std::string &name) const {
  return (_properties.find(name) != _properties.end());
}

/*****************************************************************************
 * class _obj_frame
 */

/**************************
 * _obj_frame::_obj_frame *
 **************************/

_obj_frame::_obj_frame(cutlet::variable_ptr self) : _self(self) {
}

/***************************
 * _obj_frame::~_obj_frame *
 ***************************/

_obj_frame::~_obj_frame() noexcept {
}

/************************
 * _obj_frame::variable *
 ************************/

cutlet::variable_ptr _obj_frame::variable(const std::string &name) const {
  _var_object &self = cutlet::cast<_var_object>(_self);
  if (name == "self") {
    return _self;
  } else if (self.has_property(name)) {
    return self.property(name);
  } else
    return cutlet::frame::variable(name);
}

void _obj_frame::variable(const std::string &name,
                          cutlet::variable_ptr value) {
  _var_object &self = cutlet::cast<_var_object>(_self);

  if (self.has_property(name)) {
    self.property(name, value);
  } else {
    cutlet::frame::variable(name, value);
  }
}

/******************************************************************************
 * Simple procedures.
 */

// def property name ¿name ...?
static cutlet::variable_ptr _property(cutlet::interpreter &interp,
                                      const cutlet::list &parameters) {
  cutlet::component_ptr self = interp.get("self");

  for (auto &item: parameters) {
    dynamic_cast<_def_class &>(*(self)).property(*(item));
  }

  return nullptr;
}

// def method name ¿parameters? body
static cutlet::variable_ptr _method(cutlet::interpreter &interp,
                                    const cutlet::list &parameters) {
  // Make sure we have to right number of parameters.
  size_t p_count = parameters.size();
  if (p_count < 2 or p_count > 3) {
    std::stringstream mesg;
    mesg << "Invalid number of parameters for method "
         << (p_count >= 2 ? cutlet::convert<std::string>(parameters[0]) : "")
         << " (2 <= " << p_count
         << " <= 3).\n method name ¿parameters? body";
    throw std::runtime_error(mesg.str());
  }

  // Get the class and method name.
  cutlet::component_ptr self = interp.get("self");
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

  // Create and add our method to the class.
  dynamic_cast<_def_class &>(*(self)).add(name,
                                          new _def_method(def_parameters,
                                                          body));

  // No droids here.
  return nullptr;
}

// def c_method name ¿parameters? body
static cutlet::variable_ptr _c_method(cutlet::interpreter &interp,
                                      const cutlet::list &parameters) {
  // Make sure we have to right number of parameters.
  size_t p_count = parameters.size();
  if (p_count < 2 or p_count > 3) {
    std::stringstream mesg;
    mesg << "Invalid number of parameters for method (2 <= " << p_count
         << " <= 3).\n method name ¿parameters? body";
    throw std::runtime_error(mesg.str());
  }

  // Get the class and method name.
  cutlet::component_ptr self = interp.get("self");
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

  // Create and add our method to the class.
  dynamic_cast<_def_class &>(*(self)).add_class(name,
                                                new _def_method(def_parameters,
                                                                body));

  // No droids here.
  return nullptr;
}

// def class name ¿supers? body
static cutlet::variable_ptr _class(cutlet::interpreter &interp,
                                   const cutlet::list &parameters) {
  // Make sure we have to right number of parameters.
  size_t p_count = parameters.size();
  if (p_count < 2 or p_count > 3) {
   std::stringstream mesg;
   mesg << "Invalid number of parameters for class (2 <= " << p_count
        << " <= 3).\n class name ¿supers? body";
   throw std::runtime_error(mesg.str());
  }

  // Get the class name.
  std::string name = cutlet::convert<std::string>(parameters[0]);

  // Get the super class names and class body.
  cutlet::variable_ptr parents;
  std::string body;
  if (parameters.size() == 3) {
    parents = interp.list(*(parameters[1]));
    body = *(parameters[2]);
  } else {
    parents = new cutlet::list();
    body = *(parameters[1]);
  }

  // Create the class component.
  cutlet::component_ptr new_class = new _def_class(interp, name, parents);

  // Evaluation the class body.
  interp.frame_push(_oo_sandbox());
  interp.add("self", new_class);
  try {
    interp.eval(body);
  } catch (std::exception &err) {
    interp.frame_pop();
    throw;
  }
  interp.frame_pop();

  // If we get here, add the class component to the interpreter.
  interp.add(name, new_class);

  return nullptr;
}

/***************
 * init_cutlet *
 ***************/

// We need to declare init_cutlet as a C function.
extern "C" {
  DECLSPEC void init_cutlet(cutlet::interpreter *interp);
}

void init_cutlet(cutlet::interpreter *interp) {
  _oo_sandbox() = new cutlet::sandbox();
  _oo_sandbox()->add("method", _method);
  _oo_sandbox()->add("c_method", _c_method);
  _oo_sandbox()->add("property", _property);

  interp->add("class", _class);
}
