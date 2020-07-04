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
 * An Object Oriented API for Cutlet.
 */

#include <cutlet.h>
#include <sstream>
#include <set>

class _def_class;
class _var_object;

/** Method definition as a cutlet component for cutlet code.
 */
class _def_method : public cutlet::component {
public:
  _def_method(cutlet::variable::pointer parameters,
              cutlet::variable::pointer body);
  _def_method(const _def_method &other) = delete;

  virtual ~_def_method() noexcept;

  virtual cutlet::variable::pointer
  operator ()(cutlet::interpreter &interp, const cutlet::list &args);

private:
  cutlet::variable::pointer _parameters;
  cutlet::variable::pointer _body;

  cutlet::ast::node::pointer _compiled;
};

/** Method definition as a cutlet component for native c++ functions.
 */
class _def_method_func : public cutlet::component {
public:
  _def_method_func(cutlet::function_t func) : _function_ptr(func) {}
  virtual ~_def_method_func() noexcept {}

  virtual cutlet::variable::pointer
  operator ()(cutlet::interpreter &interp,
              const cutlet::list &parameters) {
    return _function_ptr(interp, parameters);
  }

private:
  cutlet::function_t _function_ptr;
};


/** An object as a cutlet variable.
 */
class _var_object : public cutlet::variable {
public:
  _var_object(cutlet::component::pointer cls);
  _var_object(const _var_object &other) = delete;

  virtual ~_var_object() noexcept;

  _def_class &type();

  virtual cutlet::variable::pointer
  operator()(cutlet::variable::pointer self, cutlet::interpreter &interp,
             const cutlet::list &parameters);

  virtual cutlet::variable::pointer
  operator()(cutlet::component &cls, cutlet::variable::pointer self,
             cutlet::interpreter &interp,
             const cutlet::list &parameters);

  bool has_property(const std::string &name) const;
  cutlet::variable::pointer property(const std::string &name) const;
  void property(const std::string &name, cutlet::variable::pointer value);

  virtual operator std::string () const { return "_oo::object_"; }

private:
  cutlet::component::pointer _class;
  std::map<std::string, cutlet::variable::pointer> _properties;
};

/** Class definition as a cutlet component.
 */
class _def_class : public cutlet::component {
public:
  typedef void (*deletefn_t)(void *data);

  _def_class(cutlet::interpreter &interp,
             const std::string &name,
             cutlet::variable::pointer parents);
  _def_class(const _def_class &other) = delete;

  virtual ~_def_class() noexcept;

  /** Compile the body of the class. This should be used instead of using
   * cutlet::interpreter:compile directly as it keeps the resulting AST for the
   * life of the class.
   */
  void compile(cutlet::interpreter &interp, cutlet::variable::pointer body);

  /** Class methods are called from here.
   */
  virtual cutlet::variable::pointer
  operator ()(cutlet::interpreter &interp, const cutlet::list &parameters);

  /** Method calls from the object. This is called from
   * _var_object::operator ().
   */
  virtual cutlet::variable::pointer
  operator ()(const std::string &method,
              cutlet::interpreter &interp, const cutlet::list &parameters);

  /** Add a method component to the class.
   * @param name The name of the method.
   * @param comp Reference to the method component.
   */
  void add(const std::string &name, cutlet::component::pointer comp);

  void add_class(const std::string &name, cutlet::component::pointer comp);

  void add_property(const std::string &name);

  void add_class_property(const std::string &name);

  cutlet::variable::pointer class_property(const std::string &name) const;

  void class_property(const std::string &name, cutlet::variable::pointer value);

  //bool has_property(const std::string &name) const;

  bool has_class_property(const std::string &name) const;

  virtual operator std::string () const { return "_oo::class_"; }

private:
  std::string _name;
  std::list<cutlet::component::pointer> _parents;
  std::map<std::string, cutlet::variable::pointer> _class_properties;
  std::set<std::string> _properties;

  std::map<std::string, cutlet::component::pointer> _class_methods;
  std::map<std::string, cutlet::component::pointer> _methods;
  void *_data;
  deletefn_t _delete_fn;

  cutlet::ast::node::pointer _compiled;

  void add_properties(_var_object &obj) const;
};

/** Object execution frames to implement object and class properties.
 */
class _obj_frame : public cutlet::frame {
public:
  _obj_frame(const std::string &label, cutlet::variable::pointer self);
  virtual ~_obj_frame() noexcept;

  /** We override these methods to implement object properties.
   */
  virtual cutlet::variable::pointer variable(const std::string &name) const;
  virtual void variable(const std::string &name,
                        cutlet::variable::pointer value);

private:
  cutlet::variable::pointer _self;
};

/** Class execution frames to implement class properties.
 */
class _cls_frame : public cutlet::frame {
public:
  _cls_frame(const std::string &label, _def_class &cls);
  virtual ~_cls_frame() noexcept;

  /** We override these methods to implement object properties.
   */
  virtual cutlet::variable::pointer variable(const std::string &name) const;
  virtual void variable(const std::string &name, cutlet::variable::pointer value);

private:
  _def_class &_class;
};

/** Return the global reference to the class sandbox. We need to wrap it in a
 * function for the created dynamic library, otherwise the global variable
 * will be duplicated having unexpected results.
 */
static cutlet::sandbox::pointer &_oo_sandbox() {
  static cutlet::sandbox::pointer _sb_class;
  return _sb_class;
}

/*****************************************************************************
 * class _def_method
 */

/****************************
 * _def_method::_def_method *
 ****************************/

_def_method::_def_method(cutlet::variable::pointer parameters,
                         cutlet::variable::pointer body)
  : _parameters(parameters), _body(body) {
}

/*****************************
 * _def_method::~_def_method *
 *****************************/

_def_method::~_def_method() noexcept {}

/****************************
 * _def_method::operator () *
 ****************************/

cutlet::variable::pointer _def_method::operator ()(cutlet::interpreter &interp,
                                                   const cutlet::list &args) {
  auto p_it = cutlet::cast<cutlet::list>(_parameters).begin();
  auto a_it = args.begin();

  // Populate the local frame with the calls parameters.
  for (; p_it != cutlet::cast<cutlet::list>(_parameters).end() and
         a_it != args.end(); ++p_it, ++a_it) {

    // Set if the parameter was defined as a list -> {name default_value}.
    cutlet::list *l = dynamic_cast<cutlet::list *>(&(*(*p_it)));

    if (l) {
      // Was a list, so use the first entry as parameter name.
      interp.local((std::string)*(l->front()), *a_it);
    } else {

      std::string name((std::string)*(*p_it));
      if (name == "*args") {
        // Create a special local args with the remaining parameters as a list.
        // This is our form of varadic arguments.
        interp.local("args", new cutlet::list(a_it, args.end()));
      } else {
        // Plain old parameter.
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

  if (_compiled.is_null()) {
    // Source code hasn't been compiled into an AST tree yet, this will do it.
    _compiled = interp.compile(_body);
  } else {
    // We have a compiled AST tree, so run it.
    (*_compiled)(interp);
  }

  return nullptr;
}

/*****************************************************************************
 * class _var_object
 */

/****************************
 * _var_object::_var_object *
 ****************************/

_var_object::_var_object(cutlet::component::pointer cls) : _class(cls) {}

/*****************************
 * _var_object::~_var_object *
 *****************************/

_var_object::~_var_object() noexcept {}

/*********************
 * _var_object::type *
 *********************/

_def_class &_var_object::type() {
  return dynamic_cast<_def_class &>(*_class);
}

/****************************
 * _var_object::operator () *
 ****************************/

cutlet::variable::pointer
_var_object::operator()(cutlet::variable::pointer self,
                        cutlet::interpreter &interp,
                        const cutlet::list &parameters) {
  if (parameters.size() == 0) {
    throw std::runtime_error("no method given calling object");
  }

  cutlet::list params(parameters.begin() + 1, parameters.end());

  interp.push(new _obj_frame(*(parameters[0]), self));
  try {
    dynamic_cast<_def_class &>(*(_class))(*(parameters[0]), interp, params);
  } catch (...) {
    //interp.pop();
    throw;
  }
  return interp.pop();
}

cutlet::variable::pointer
_var_object::operator()(cutlet::component &cls,
                        cutlet::variable::pointer self,
                        cutlet::interpreter &interp,
                        const cutlet::list &parameters) {
  if (parameters.size() == 0) {
    throw std::runtime_error("no method given calling object");
  }

  cutlet::list params(parameters.begin() + 1, parameters.end());

  interp.push(new _obj_frame(*(parameters[0]), self));
  try {
    dynamic_cast<_def_class &>(cls)(*(parameters[0]), interp, params);
  } catch (...) {
    //interp.pop();
    throw;
  }
  return interp.pop();
}

/*****************************
 * _var_object::has_property *
 *****************************/

bool _var_object::has_property(const std::string &name) const {
  return (_properties.find(name) != _properties.end());
}

/*************************
 * _var_object::property *
 *************************/

cutlet::variable::pointer _var_object::property(const std::string &name) const {
  if (_properties.find(name) == _properties.end())
    throw std::runtime_error(std::string("Property ")  + name +
                             " not found in object.");
  return _properties.at(name);
}

void _var_object::property(const std::string &name,
                           cutlet::variable::pointer value) {
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
                       cutlet::variable::pointer parents)
  : _name(name), _data(nullptr), _delete_fn(nullptr) {

  for (auto &parent_name: cutlet::cast<cutlet::list>(parents)) {
    std::string name = cutlet::convert<std::string>(parent_name);
    _parents.push_back(interp.get(name));
  }
}

/***************************
 * _def_class::~_def_class *
 ***************************/

_def_class::~_def_class() noexcept {
  if (_delete_fn != nullptr) _delete_fn(_data);
  _compiled = nullptr;
}

/***********************
 * _def_class::compile *
 ***********************/

void _def_class::compile(cutlet::interpreter &interp,
                         cutlet::variable::pointer body) {
  /* We need to keep the AST even though it should never be called again. The
   * tokens will be needed later when compiling the methods and for the
   * debugging API.
   */
  _compiled = interp.compile(body);
}

/***************************
 * _def_class::operator () *
 ***************************/

cutlet::variable::pointer _def_class::operator ()(cutlet::interpreter &interp,
                                                  const cutlet::list &parameters) {
  cutlet::variable::pointer object;
  if (parameters.size() == 0) {
    throw std::runtime_error("no method called for class");
  }

  if (cutlet::convert<std::string>(parameters[0]) == "new") {
    // Create our new object
    _var_object *obj = new _var_object(interp.get(_name));
    add_properties(*obj);

    object = obj;
    interp.push(new _obj_frame(*(parameters[0]), object));
    (*object)(object, interp, parameters);
    interp.pop();
    return object;
  } else {
    std::string method = cutlet::convert<std::string>(parameters[0]);
    cutlet::list params(parameters.begin() + 1, parameters.end());

    // Find the class method.
    auto m = _class_methods.find(method);
    if (m == _class_methods.end()) {
      // Check the parent class for the method.
      for (auto _parent_class: _parents) {
        m = dynamic_cast<_def_class &>(*(_parent_class))._class_methods.find(method);
        if (m != _class_methods.end()) break;
      }
    }

    if (m != _class_methods.end()) {
      interp.push(new _cls_frame(*(parameters[0]), *this));
      try {
        (*(m->second))(interp, params);
      } catch (...) {
        interp.pop();
        throw;
      }
      return interp.pop();
    } else {
      throw std::runtime_error(std::string("class method ") + method +
                               " not found");
    }
  }

  return nullptr;
}

cutlet::variable::pointer
_def_class::operator ()(const std::string &method,
                        cutlet::interpreter &interp,
                        const cutlet::list &parameters) {
  /* This gets called from the object variable.
   */

  auto m = _methods.find(method);
  if (m != _methods.end()) {
    // We found the method, now call it.
    return (*(m->second))(interp, parameters);
  } else {
    // Now check the parent class for the method.
    for (auto _parent_class: _parents) {
      m = dynamic_cast<_def_class &>(*(_parent_class))._methods.find(method);
      if (m != _methods.end()) {
        return (*(m->second))(interp, parameters);
      }
    }
  }

  throw std::runtime_error(std::string("method ") + method +
                           " not found");
}

/*******************
 * _def_class::add *
 *******************/

void _def_class::add(const std::string &name, cutlet::component::pointer comp) {
  _methods[name] = comp;
}

/*************************
 * _def_class::add_class *
 *************************/

void _def_class::add_class(const std::string &name,
                           cutlet::component::pointer comp) {
  _class_methods[name] = comp;
}

/****************************
 * _def_class::add_property *
 ****************************/

void _def_class::add_property(const std::string &name) {
  _properties.insert(name);
}

/**********************************
 * _def_class::add_class_property *
 **********************************/

void _def_class::add_class_property(const std::string &name) {
  _class_properties[name] = new cutlet::string();
}

/******************************
 * _def_class::class_property *
 ******************************/

cutlet::variable::pointer
_def_class::class_property(const std::string &name) const {
  auto res = _class_properties.find(name);
  if (res != _class_properties.end()) {
    return res->second;
  } else {
    for (auto &parent: _parents) {
      if (dynamic_cast<const _def_class &>(*parent).has_class_property(name))
        return dynamic_cast<const _def_class &>(*parent).class_property(name);
    }
  }
  return nullptr;
}

void _def_class::class_property(const std::string &name,
                                cutlet::variable::pointer value) {
  if (_class_properties.find(name) != _class_properties.end()) {
    _class_properties[name] = value;
  } else {
    for (auto &parent: _parents) {
      if (dynamic_cast<_def_class &>(*parent).has_class_property(name))
        dynamic_cast<_def_class &>(*parent).class_property(name, value);
    }
  }
}

/**********************************
 * _def_class::has_class_property *
 **********************************/

bool _def_class::has_class_property(const std::string &name) const {
  // Check if we have the class property.
  if (_class_properties.find(name) != _class_properties.end()) return true;

  // We didn't have the class property, now check our parents.
  for (auto &parent: _parents) {
    if (dynamic_cast<const _def_class &>(*parent).has_class_property(name))
      return true;
  }
  return false;
}

/******************************
 * _def_class::add_properties *
 ******************************/

void _def_class::add_properties(_var_object &obj) const {
  for (auto &property: _properties) { // Add the properties
    /*std::clog << "DEBUG oo: Adding property " << property << " to object."
      << std::endl;*/
    obj.property(property, new cutlet::string(""));
  }

  for (auto &cls: _parents) {
    dynamic_cast<const _def_class &>(*cls).add_properties(obj);
  }
}

/******************************************************************************
 * class _obj_frame
 */

/**************************
 * _obj_frame::_obj_frame *
 **************************/

_obj_frame::_obj_frame(const std::string &label,
                       cutlet::variable::pointer self) : _self(self) {
  this->label(std::string("method ") + label);
}

/***************************
 * _obj_frame::~_obj_frame *
 ***************************/

_obj_frame::~_obj_frame() noexcept {}

/************************
 * _obj_frame::variable *
 ************************/

cutlet::variable::pointer _obj_frame::variable(const std::string &name) const {
  auto &self = cutlet::cast<_var_object>(_self);
  auto &cls = self.type();

  if (name == "self") {
    return _self;
  } else if (self.has_property(name)) {
    return self.property(name);
  } else if (cls.has_class_property(name)) {
    return cls.class_property(name);
  } else
    return cutlet::frame::variable(name);
}

void _obj_frame::variable(const std::string &name,
                          cutlet::variable::pointer value) {
  auto &self = cutlet::cast<_var_object>(_self);
  auto &cls = self.type();

  if (self.has_property(name)) {
    self.property(name, value);
  } else if (cls.has_class_property(name)) {
    cls.class_property(name, value);
  } else {
    cutlet::frame::variable(name, value);
  }
}

/******************************************************************************
 * class _cls_frame
 */

/**************************
 * _cls_frame::_cls_frame *
 **************************/

_cls_frame::_cls_frame(const std::string &label, _def_class &cls)
  : _class(cls) {
  this->label(std::string("class method ") + label);
}

/***************************
 * _cls_frame::~_cls_frame *
 ***************************/

_cls_frame::~_cls_frame() noexcept {}

/************************
 * _cls_frame::variable *
 ************************/

cutlet::variable::pointer _cls_frame::variable(const std::string &name) const {
  if (_class.has_class_property(name)) {
    return _class.class_property(name);
  } else
    return cutlet::frame::variable(name);
}

void _cls_frame::variable(const std::string &name,
                          cutlet::variable::pointer value) {
  if (_class.has_class_property(name)) {
    _class.class_property(name, value);
  } else {
    cutlet::frame::variable(name, value);
  }
}

/******************************************************************************
 * The exposed API.
 */

// def property name ¿name ...?
static cutlet::variable::pointer _property(cutlet::interpreter &interp,
                                           const cutlet::list &parameters) {
  cutlet::component::pointer self = interp.get("self");

  for (auto &item: parameters) {
    dynamic_cast<_def_class &>(*(self)).add_property(*(item));
  }

  return nullptr;
}

// def class.property name ¿=? value?
static cutlet::variable::pointer _c_property(cutlet::interpreter &interp,
                                             const cutlet::list &parameters) {
  cutlet::component::pointer self = interp.get("self");

  size_t p_count = parameters.size();
  if (p_count < 1 or p_count > 3) {
    std::stringstream mesg;
    mesg << "Invalid number of parameters for class.property "
         << (p_count >= 1 ? cutlet::convert<std::string>(parameters[0]) : "")
         << " (1 <= " << p_count
         << " <= 3).\n class.property name ¿¿=? value?";
    throw std::runtime_error(mesg.str());
  }

  dynamic_cast<_def_class &>(*(self)).add_class_property(*parameters[0]);
  if (p_count > 1) {
    if (p_count == 3 and (std::string)*parameters[1] != "=") {
      throw std::runtime_error("class.property expected \"=\".\n"
                               " class.property name ¿¿=? value?");
    }
    dynamic_cast<_def_class &>(*(self)).class_property(*parameters[0],
                                                       parameters[p_count - 1]);
  }

  return nullptr;
}

// def method name ¿parameters? body
static cutlet::variable::pointer _method(cutlet::interpreter &interp,
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
  cutlet::component::pointer self = interp.get("self");
  std::string name = *(parameters[0]);

  // Get the method parameters and body.
  cutlet::variable::pointer body;
  cutlet::variable::pointer def_parameters;
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

// def class.method name ¿parameters? body
static cutlet::variable::pointer _c_method(cutlet::interpreter &interp,
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
  cutlet::component::pointer self = interp.get("self");
  std::string name = *(parameters[0]);

  // Get the method parameters and body.
  cutlet::variable::pointer body;
  cutlet::variable::pointer def_parameters;
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
static cutlet::variable::pointer _class(cutlet::interpreter &interp,
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
  cutlet::variable::pointer parents;
  cutlet::variable::pointer body;
  if (parameters.size() == 3) {
    parents = interp.list(*(parameters[1]));
    body = parameters[2];
  } else {
    parents = new cutlet::list();
    body = parameters[1];
  }

  // Create the class component.
  _def_class *cls = new _def_class(interp, name, parents);
  cutlet::component::pointer new_class = cls;

  // Evaluation the class body.
  if (not (std::string(*body)).empty()) {
    interp.push(_oo_sandbox());

    /* This has to be a weak reference or we get segfaults if an error is
     * generated creating the class.
     */
    interp.add("self", new_class.weak());

    try {
      cls->compile(interp, body);
    } catch (std::exception &err) {
      interp.pop();
      throw;
    }
    interp.pop();
  }

  // If we get here, add the class component to the interpreter.
  interp.add(name, new_class);

  return nullptr;
}

// def super class method *args
static cutlet::variable::pointer _super(cutlet::interpreter &interp,
                                        const cutlet::list &parameters) {

  if (parameters.size() < 2) {
    throw std::runtime_error("super class self method *args");
  }

  cutlet::component &cls = *interp.get(*parameters[0]);

  cutlet::variable::pointer self = interp.frame(1)->variable("self");
  _var_object *obj = dynamic_cast<_var_object *>(&(*self));
  if (obj) {
    cutlet::list parms(parameters.begin() + 1, parameters.end());
    return (*obj)(cls, self, interp, parms);
  } else {
    throw std::runtime_error("self isn't an object");
  }

  return nullptr;
}

// We need to declare init_cutlet as a C function.
extern "C" {
  DECLSPEC void init_cutlet(cutlet::interpreter *interp);

  DECLSPEC void oo_add_method(cutlet::interpreter &interp,
                              const std::string &class_name,
                              const std::string &method_name,
                              cutlet::function_t method);

  DECLSPEC void oo_add_class_method(cutlet::interpreter &interp,
                                    const std::string &class_name,
                                    const std::string &method_name,
                                    cutlet::function_t method);

  DECLSPEC cutlet::variable::pointer
  oo_get_property(cutlet::interpreter &interp,
                  cutlet::variable::pointer &object,
                  const std::string &prop_name);
}

/***************
 * init_cutlet *
 ***************/

void init_cutlet(cutlet::interpreter *interp) {
  _oo_sandbox() = new cutlet::sandbox();
  _oo_sandbox()->add("method", _method);
  _oo_sandbox()->add("class.method", _c_method);
  _oo_sandbox()->add("property", _property);
  _oo_sandbox()->add("class.property", _c_property);

  interp->add("class", _class);
  interp->add("super", _super);
}

/*****************
 * oo_add_method *
 *****************/

void oo_add_method(cutlet::interpreter &interp,
                   const std::string &class_name,
                   const std::string &method_name,
                   cutlet::function_t method) {
  cutlet::component::pointer comp = interp.get(class_name);
  dynamic_cast<_def_class &>(*comp).add(method_name,
                                        new _def_method_func(method));
}

/***********************
 * oo_add_class_method *
 ***********************/

void oo_add_class_method(cutlet::interpreter &interp,
                         const std::string &class_name,
                         const std::string &method_name,
                         cutlet::function_t method) {
  cutlet::component::pointer comp = interp.get(class_name);
  dynamic_cast<_def_class &>(*comp).add_class(method_name,
                                              new _def_method_func(method));
}

/*******************
 * oo_add_property *
 *******************/

cutlet::variable::pointer oo_get_property(cutlet::interpreter &interp,
                                          cutlet::variable::pointer &object,
                                          const std::string &prop_name) {
  (void)interp;
  return (cutlet::cast<_var_object>(object)).property(prop_name);
}
