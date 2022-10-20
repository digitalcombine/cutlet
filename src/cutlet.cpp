/*                                                                  -*- c++ -*-
 * Copyright © 2018-2022 Ron R Wills <ron@digitalcombine.ca>
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
#include <iostream>
#include <fstream>
#include <sstream>

#include "builtin.h"
#include "utilities.h"
#include "ast.h"

/** Wrapper for native dynamic library loading.
 */
class native_lib {
public:

  /** Load a native library.
   * @param filename The path to the library to be loaded.
   * @throws runtime_error
   */
  native_lib(const std::string &filename) {
#if defined (__linux__) || defined (__FreeBSD__)
    _handle = dlopen(filename.c_str(), RTLD_LAZY);
    if (_handle == NULL)
      throw std::runtime_error(dlerror());
#else
    throw std::runtime_error("Dynamic libraries are not supported.");
#endif
  }

  /** Close and cleanup the native library.
   */
  virtual ~native_lib() noexcept;

  /** Lookup a symbol within the library.
   * @return A pointer to the symbol. If the symbol doesn't exist then a null
   *         pointer is returned.
   */
  void *symbol(const std::string &name) {
#if defined (__linux__) || defined (__FreeBSD__)
    if (_handle) return dlsym(_handle, name.c_str());
    return nullptr;
#else
    return nullptr;
#endif
  }

private:
  nativelib_t _handle;
};

native_lib::~native_lib() noexcept {
#if defined (__linux__) || defined (__FreeBSD__)
  if (_handle and dlclose(_handle) != 0)
    std::cerr << "WARNING: dlclose " << dlerror() << std::endl;
#endif
}

/***************
 * operator << *
 ***************/

std::ostream &operator <<(std::ostream &os,
                          const cutlet::frame &frame) {
  cutlet::frame::pointer ptr = frame._uplevel;

  // Count the number of frames we have.
  unsigned int level = 1;
  while (ptr and (not ptr->_sandbox_orig)) {
    ptr = ptr->_uplevel;
    level++;
  }
  if (ptr and ptr->_sandbox_orig) level++;

  // Display information about the frame.
  os << level << ": " << frame.label();
  switch (frame.state()) {
  case cutlet::frame::FS_DONE:
    os << " (done)";
    break;
  case cutlet::frame::FS_BREAK:
    os << " (break)";
    break;
  case cutlet::frame::FS_CONTINUE:
    os << " (continue)";
    break;
  case cutlet::frame::FS_RUNNING: // Shut up compiler warnings.
    break;
  }

  // Display the variables in the frame.
  for (auto &it: frame._variables) {
    os << "\n  $" << it.first;
  }

  return os;
}

std::ostream &operator <<(std::ostream &os,
                          const cutlet::frame::pointer &frame) {
  os << *frame;
  return os;
}

/******************************************************************************
 * class cutlet_tokenizer
 */

class cutlet_tokenizer : public parser::tokenizer {
public:
  cutlet_tokenizer() {}

  friend std::ostream &operator <<(std::ostream &os,
                                   const cutlet_tokenizer &tks);

protected:
  virtual void parse_tokens();

protected:
  void parse_next_token();
};

#ifdef DEBUG
/** Debug stream operator for displaying parsed tokens.
 */
std::ostream &operator <<(std::ostream &os, const cutlet_tokenizer &tks) {
  unsigned int size = tks.tokens.size();
  for (auto &token: tks.tokens) {
    os << "token(" << size << "): " << (unsigned int)token << " ";
    if ((unsigned int)token != 6)
      os << (std::string)token << "\n";
    else
      os << "EOL\n";
  }
  return os;
}
#endif

/******************************************************************************
 * class cutlet::utf8::iterator
 */

/************************************
 * cutlet::utf8::iterator::iterator *
 ************************************/

cutlet::utf8::iterator::iterator(const std::string &value)
  : _value(&value), _index(0), _length(1) {
  // Find the length of the character and make a copy for dereferencing.
  for (; ((*_value)[_index + _length] & 0xc0) == 0x80; ++_length);
  _current = _value->substr(_index, _length);
}

cutlet::utf8::iterator::iterator(const std::string &value, size_t offset)
  : _value(&value), _index(offset), _length(1) {
  // Find the length of the character and make a copy for dereferencing.
  for (; ((*_value)[_index + _length] & 0xc0) == 0x80; ++_length);
  _current = _value->substr(_index, _length);
}

cutlet::utf8::iterator::iterator(const iterator &other)
  : _value(other._value), _index(other._index), _length(other._length),
    _current(other._current) {
}

/**************************************
 * cutlet::utf8::iterator::operator = *
 **************************************/

cutlet::utf8::iterator &
cutlet::utf8::iterator::operator =(const iterator &other) {
  if (this != &other) {
    _value = other._value;
    _index = other._index;
    _length = other._length;
    _current = other._current;
  }
  return *this;
}

/***************************************
 * cutlet::utf8::iterator::operator ++ *
 ***************************************/

cutlet::utf8::iterator &cutlet::utf8::iterator::operator ++() {
  // Increment the index.
  if (_index < _value->length()) _index += _length;

  // Get the length of the character and copy the character.
  if (_index < _value->length()) {
    for (_length = 1; ((*_value)[_index + _length] & 0xc0) == 0x80; ++_length);
    _current = _value->substr(_index, _length);
  } else {
    _current.clear();
  }

  return *this;
}

cutlet::utf8::iterator cutlet::utf8::iterator::operator ++(int) {
  iterator tmp(*this);

  // Increment the index.
  if (_index < _value->length()) _index += _length;

  // Get the length of the character and copy it.
  if (_index < _value->length()) {
    for (_length = 1; ((*_value)[_index + _length] & 0xc0) == 0x80; ++_length);
    _current = _value->substr(_index, _length);
  } else {
    _current.clear();
  }

  return tmp;
}

/***************************************
 * cutlet::utf8::iterator::operator -- *
 ***************************************/

cutlet::utf8::iterator &cutlet::utf8::iterator::operator --() {
  if (_index > 0) {
    size_t tmp = _index;

    // Find the first character.
    for (--_index; ((*_value)[_index] & 0xc0) == 0x80 and _index > 0; --_index);

    // Get the length of the character and copy it.
    _length = tmp - _index;
    _current = _value->substr(_index, _length);
  } else {
    _current.clear();
  }

  return *this;
}

cutlet::utf8::iterator cutlet::utf8::iterator::operator --(int) {
  iterator tmp(*this);

  if (_index > 0) {
    size_t tmps = _index;

    // Find the first character.
    for (--_index; ((*_value)[_index] & 0xc0) == 0x80 and _index > 0;
         --_index);

    // Get the length of the character and copy it.
    _length = tmps - _index;
    _current = _value->substr(_index, _length);
  } else {
    _current.clear();
  }

  return tmp;
}

/***************************************
 * cutlet::utf8::iterator::operator == *
 ***************************************/

bool cutlet::utf8::iterator::operator ==(const iterator &other) const {
  return _index == other._index;
}

/***************************************
 * cutlet::utf8::iterator::operator != *
 ***************************************/

bool cutlet::utf8::iterator::operator !=(const iterator &other) const {
  return _index != other._index;
}

/**************************************
 * cutlet::utf8::iterator::operator + *
 **************************************/

cutlet::utf8::iterator cutlet::utf8::iterator::operator +(int value) const {
  auto result = *this;
  if (value > 0) for (; value > 0; ++result, --value);
  else for (; value < 0; --result, ++value);
  return result;
}

/**************************************
 * cutlet::utf8::iterator::operator - *
 **************************************/

cutlet::utf8::iterator cutlet::utf8::iterator::operator -(int value) const {
  auto result = *this;
  if (value > 0) for (; value > 0; --result, --value);
  else for (; value < 0; ++result, ++value);
  return result;
}

/**************************************
 * cutlet::utf8::iterator::operator * *
 **************************************/

const std::string &cutlet::utf8::iterator::operator *() const {
  return _current;
}

/*******************************
 * cutlet::utf8::iterator::end *
 *******************************/

cutlet::utf8::iterator cutlet::utf8::iterator::end() const {
  iterator tmp(*this);
  tmp._index = _value->length();
  return tmp;
}

/************************************
 * cutlet::utf8::iterator::position *
 ************************************/

size_t cutlet::utf8::iterator::position() const {
  return _index;
}

/**********************************
 * cutlet::utf8::iterator::length *
 **********************************/

size_t cutlet::utf8::iterator::length() const {
  return _length;
}

/************************
 * cutlet::utf8::substr *
 ************************/

std::string cutlet::utf8::substr(const iterator &start, const iterator &end) {
  const std::string &value = *start._value;

#ifdef TESTING
  if (value.substr(start._index, start._length) != *start)
    throw std::range_error("iter values out of sync");
#endif

  if (start != start.end() and end._index >= start._index) {
    return value.substr(start._index, end._index - start._index);
  }
  //return "";
  throw std::range_error("utf-8 sub-string iterators out of range");
}

/*************************
 * cutlet::utf8::replace *
 *************************/

std::string cutlet::utf8::replace(const iterator &start, const iterator &end,
                                  const std::string &value) {
  std::string new_value = *start._value;
  if (start != start.end() and end._index >= start._index) {
    return new_value.replace(start._index,
                             (end._index + end._length) - start._index,
                             value);
  }
  throw std::range_error("sub string iterators out of range");
}


/******************************************************************************
 * class cutlet_tokenizer
 */

/**********************************
 * cutlet_tokenizer::parse_tokens *
 **********************************/

void cutlet_tokenizer::parse_tokens() {
  // If we don't have any code then see if we can get more from the stream.
  if (code.empty()) {
    if (stream) {

      /*  Attempt to read a line and set the EOF token if the stream is
       * exhausted.
       */
      position = stream->tellg();
      if (not getline(*stream, code, '\0')) {
        tokens.push_back(parser::token(T_EOF, "", position));
        stream = NULL;
        return;
      }
    } else
      return;
  }

  // If we have some code then tokenize it.
  if (not code.empty()) {
    while (not code.empty()) {
      parse_next_token();
    }
    if (not stream)
      tokens.push_back(parser::token(T_EOF, "", position));
  }

  //std::clog << "TOKENS: " << (parser::tokenizer)*this << std::endl;

#ifdef TESTING
  /* I'd like to have this in the test suites, but unfortunately
   * testing protected and private entities needs protected/private
   * code.
   *
   * XXX Currently this messes up the stream.
   */
  /*if (stream) {
    stream->clear(); // Clear any error states.

    for (auto &token: tokens) {
      const std::string &value = (const std::string &)token;
      char *codev = new char[value.length() + 1];
      std::streampos fpos = token.position();

      switch ((unsigned int)token) {
      case cutlet::T_COMMENT:
      case cutlet::T_VARIABLE:
      case cutlet::T_SUBCMD:
      case cutlet::T_BLOCK:
      case cutlet::T_STRING:
        fpos += 1;
        break;
      }

      stream->seekg(fpos);
      stream->read(codev, value.length());
      codev[value.length()] = 0;

      if (value != codev) {
        throw std::runtime_error(std::string("Token position tracting failed.\n  \"") +
                                 value + "\" != \"" + codev + "\"\n  ");
      }

      delete[] codev;
    }
  } else {

    for (auto &token: tokens) {
      }
  }*/
#endif /* TESTING */
}

/**************************************
 * cutlet_tokenizer::parse_next_token *
 **************************************/

void cutlet_tokenizer::parse_next_token() {
  if (not code.empty()) {
    cutlet::utf8::iterator it(code);
    std::streampos start_pos = position;

    //std::clog << "PARSING: " << code << std::endl;

    // Skip any white space.
    while (is_space(*it) and it != it.end()) {
      ++it;
      position += 1;
    }
    if (it == it.end()) {
      code.clear();
      return;
    }

    switch ((*it)[0]) {

    // End of File
    case '\0':
      it = it.end();
      break;

    // Varible token.
    case '$': {
      ++it; // Remove the $ character.

      cutlet::utf8::iterator start(it);
      std::string result;

      // Find the end of the variable name.
      while ((not is_space(*it) and not is_eol(*it))
              and it != it.end())
        ++it;

      // Add the token.
      add_token(cutlet::T_VARIABLE,
                cutlet::utf8::substr(start, it),
                position, 1);
      break;
    }

    // String token.
    case '"': {
      ++it; // Remove the " character.

      cutlet::utf8::iterator start(it);
      bool ignore = false;

      // Find the matching " character.
      for (; *it != "\"" or ignore; ++it) {

        // Make sure the " isn't escaped (\").
        if (*it == "\\" and not ignore) ignore = true;
        else ignore = false;

        // Matching " not found, throw and error.
        if (is_eol(*it) or it == it.end())
          throw parser::syntax_error("Unmatched \"",
                                     parser::token(cutlet::T_STRING,
                                                   cutlet::utf8::substr(start,
                                                                        it),
                                                   position));
      }

      // Add the token.
      add_token(cutlet::T_STRING,
                cutlet::utf8::substr(start, it),
                position, 1);
      ++it; // Remove trailing "
      break;
    }

    // String token.
    case '\'': {
      ++it; // Remove the ' character

      cutlet::utf8::iterator start(it);
      bool ignore = false;

      // Find the matching ' character.
      for (; *it != "'" or ignore; ++it) {

        // Make sure the ' isn't escaped (\').
        if (*it == "\\" and not ignore) ignore = true;
        else ignore = false;

        // Matching ' not found, throw and error.
        if (is_eol(*it) or it == it.end())
          throw parser::syntax_error("Unmatched '",
                                     parser::token(cutlet::T_STRING,
                                                   cutlet::utf8::substr(start,
                                                                        it),
                                                   position));
      }

      // Add the token.
      add_token(cutlet::T_STRING,
                cutlet::utf8::substr(start, it),
                position, 1);
      ++it; // Remove trailing '
      break;
    }

    // Subcommand token.
    case '[': {
      ++it; // Remove the [ character.

      cutlet::utf8::iterator start(it), previous(it);
      unsigned int count = 1;
      unsigned int blocks = 0;

      // Find the matching ] character.
      while (count) {

        // Matching ] character not found, throw and error.
        if ((is_eol(*it) and not blocks) or it == it.end())
          throw parser::syntax_error("Unmatched [",
                                     parser::token(cutlet::T_SUBCMD,
                                                   cutlet::utf8::substr(start,
                                                                        it),
                                                   position));

        // Keep track of subcommands within this subcommand.
        if (*it == "]") {
          count--;
          previous = it;
        } else if (*it == "[") count++;

        // Keep track of blocks within this subcommand.
        if (*it == "}") blocks--;
        else if (*it == "{") blocks++;

        ++it; // Next character please.
      }

      // Add the token.
      add_token(cutlet::T_SUBCMD,
                cutlet::utf8::substr(start, previous),
                position, 1);
      break;
    }

    // Block token.
    case '{': {
      ++it; // Remove the { character.

      cutlet::utf8::iterator start(it), previous(it);
      unsigned int count = 1;

      // Find the matching } character.
      while (count) {

        // Matching } character not found, throw and error.
        if (it == it.end()) {
          std::stringstream msg;
          msg << file << ":" << position << ": Unmatched {";
          throw parser::syntax_error(msg.str(),
                                     parser::token(cutlet::T_BLOCK,
                                                   cutlet::utf8::substr(start,
                                                                        it),
                                                   position));
        }

        // Keep track of blocks within this block.
        if (*it == "}") {
          count--;
          previous = it;
        } else if (*it == "{") count++;

        ++it; // Next character please.
      }

      // Add the token.
      add_token(cutlet::T_BLOCK,
                cutlet::utf8::substr(start, previous),
                position, 1);
      break;
    }

    default: {
      cutlet::utf8::iterator start(it);

      if (is_eol(*it) or it == it.end()) {
        // Add an end of line token.
        add_token(cutlet::T_EOL, *it, position);
        ++it;

      } else if ((tokens.empty() or
                  (unsigned int)tokens.back() == cutlet::T_EOL) and
                 (*it)[0] == '#') {
        // Add a comment token.
        cutlet::utf8::iterator startx(it);
        while (not is_eol(*it)) ++it;
        add_token(cutlet::T_COMMENT,
                  cutlet::utf8::substr(startx + 1, it),
                  position, 1);

      } else {
        // Add a word token.
        while (not is_space(*it) and not is_eol(*it) and it != it.end())
          ++it;
        add_token(cutlet::T_WORD,
                  cutlet::utf8::substr(start, it),
                  position);
      }
      break;
    }
    }

    position = start_pos + (std::streampos)it.position();

    // Update the remaining code.
    if (it != it.end())
      code = cutlet::utf8::substr(it, it.end());
    else
      code.clear();
  }
}

/******************************************************************************
 * class cutlet::component
 */

/*********************************
 * cutlet::component::~component *
 *********************************/

cutlet::component::~component() noexcept {}

/************************************
 * cutlet::component::documentation *
 ************************************/

std::string cutlet::component::documentation() const {
  return "";
}

/******************************************************************************
 * class cutlet::variable
 */

/*******************************
 * cutlet::variable::~variable *
 *******************************/

cutlet::variable::~variable() noexcept {}

/*********************************
 * cutlet::variable::operator == *
 *********************************/

bool cutlet::variable::operator ==(const std::string &value) const {
  return ((std::string)*this == value);
}

/*********************************
 * cutlet::variable::operator != *
 *********************************/

bool cutlet::variable::operator !=(const std::string &value) const {
  return ((std::string)*this != value);
}

/*********************************
 * cutlet::variable::operator () *
 *********************************/

cutlet::variable::pointer
cutlet::variable::operator()(variable::pointer self, interpreter &interp,
                             const list &arguments) {
  (void)self;
  return interp.call((std::string)*this, arguments);
}

/******************************************
 * cutlet::variable::operator std::string *
 ******************************************/

cutlet::variable::operator std::string() const { return ""; }

/**************************
 * cutlet::variable::node *
 **************************/

cutlet::ast::node *cutlet::variable::node() const { return nullptr; }

/******************************************************************************
 * class _function
 *
 *  A component wrapper around the cutlet::function_t.
 */

class _function : public cutlet::component {
public:
  _function(const std::string &label, cutlet::function_t func,
            const std::string &doc)
    : _label(label), _doc(doc), _function_ptr(func) {}

  virtual ~_function() noexcept;

  virtual cutlet::variable::pointer
  operator ()(cutlet::interpreter &interp, const cutlet::list &arguments) {
    interp.push(_label);
    interp.finish(_function_ptr(interp, arguments));
    return interp.pop();
  }

  virtual std::string documentation() const { return _doc; }

private:
  std::string _label;
  std::string _doc;
  cutlet::function_t _function_ptr;
};

_function::~_function() noexcept {}

/******************************************************************************
 * class cutlet::exception
 */

/********************************
 * cutlet::exception::exception *
 ********************************/

cutlet::exception::exception() noexcept : _node(nullptr) {}

cutlet::exception::exception(const std::string &message) noexcept
  : _message(message), _node(nullptr) { }

cutlet::exception::exception(const std::string &message,
                             ast::node &node) noexcept
  : _message(message), _node(&node) { }

cutlet::exception::exception(const std::exception &other) noexcept
  : _message(other.what()), _node(nullptr) { }

cutlet::exception::exception(const cutlet::exception &other) noexcept
  : _message(other._message), _node(other._node) { }

/*********************************
 * cutlet::exception::~exception *
 *********************************/

cutlet::exception::~exception() noexcept {}

/*********************************
 * cutlet::exception::operator = *
 *********************************/

cutlet::exception &
cutlet::exception::operator=(const exception &other) noexcept {
  if (&other != this) {
    _message = other._message;
    _node = other._node;
  }
  return *this;
}

/***************************
 * cutlet::exception::what *
 ***************************/

const char *cutlet::exception::what() const noexcept {
  return _message.c_str();
}

/******************************************************************************
 * class cutlet::sandbox
 */

/****************************
 * cutlet::sandbox::sandbox *
 ****************************/

cutlet::sandbox::sandbox() {}

/*****************************
 * cutlet::sandbox::~sandbox *
 *****************************/

cutlet::sandbox::~sandbox() noexcept {
  _variables.clear();
  _components.clear();
  for (auto &item: _native_libs) {
    delete (native_lib *)item;
  }
}

/************************
 * cutlet::sandbox::add *
 ************************/

void cutlet::sandbox::add(const std::string &name, function_t func,
                          const std::string &doc) {
  _components[name] = new _function(name, func, doc);
}

void cutlet::sandbox::add(const std::string &name, component::pointer comp) {
  _components[name] = comp;
}

/***************************
 * cutlet::sandbox::remove *
 ***************************/

void cutlet::sandbox::remove(const std::string &name) {
  _components.erase(name);
}

/**************************
 * cutlet::sandbox::clear *
 **************************/

void cutlet::sandbox::clear() {
  _variables.clear();
  _components.clear();
}

/************************
 * cutlet::sandbox::get *
 ************************/

cutlet::component::pointer
cutlet::sandbox::get(const std::string &name) const {
  return _components.at(name);
}

/*****************************
 * cutlet::sandbox::variable *
 *****************************/

cutlet::variable::pointer cutlet::sandbox::variable(interpreter &interp,
                                                    const std::string &name) {
  if (_variables.find(name) != _variables.end())
    return _variables[name];
  else {
    try {
      list arguments({new cutlet::string(name)});
      return call(interp, "¿variable?", arguments);
    } catch (std::runtime_error &err) {}
  }
  return nullptr;
}

void cutlet::sandbox::variable(const std::string &name,
                               variable::pointer value) {
  if (value.is_null())
    _variables.erase(name);
  else
    _variables[name] = value;
}

/*********************************
 * cutlet::sandbox::has_variable *
 *********************************/

bool cutlet::sandbox::has_variable(const std::string &name) {
  return (_variables.find(name) != _variables.end());
}

/*************************
 * cutlet::sandbox::call *
 *************************/

cutlet::variable::pointer
cutlet::sandbox::call(interpreter &interp,
                      const std::string &name,
                      const list &arguments) {
  auto it = _components.find(name);

  if (it != _components.end()) {
    // Execute the found component.
    return (*it->second)(interp, arguments);

  } else {

    /* We couldn't find the component, so now look for the component
     * ¿component? and execute it.
     */
    it = _components.find("¿component?");
    if (it != _components.end()) {
      cutlet::list args(arguments);
      args.push_front(new cutlet::string(name));
      return (*it->second)(interp, args);
    } else {
      throw std::runtime_error("Unresolved component \"" + name + "\"");
    }
  }
}

/***************************
 * cutlet::sandbox::symbol *
 ***************************/

void *cutlet::sandbox::symbol(const std::string &name) const {
  void *result = nullptr;

  // Search all the loaded libraries for the symbol.
  for (auto &lib: _native_libs) {
    result = reinterpret_cast<native_lib *>(lib)->symbol(name);
    if (result) return result;
  }

  throw std::runtime_error("Internal symbol " + name +
                           " not found in loaded native libraries.");
}

/*************************
 * cutlet::sandbox::load *
 *************************/

void cutlet::sandbox::load(interpreter &interp,
                           const std::string &library_name) {
  if (fexists(library_name)) {
    native_lib *lib = new native_lib(library_name);
    libinit_t init = (libinit_t)lib->symbol("init_cutlet");

    if (init != nullptr) {
      _native_libs.push_back(lib);
      init(&interp);
    } else {
      delete lib;
      throw std::runtime_error(std::string("init_cutlet missing in library ")
                               + library_name);
    }
  }
}

/******************************************************************************
 * class cutlet::interpreter
 */

 /************************************
  * cutlet::interpreter::interpreter *
  ************************************/

cutlet::interpreter::interpreter() {
  tokens = new cutlet_tokenizer;
  _global = new sandbox();
  _global->add("print", ::builtin::print);
  _global->add("global", ::builtin::global);
  _global->add("local", ::builtin::local);
  _global->add("uplevel", ::builtin::uplevel);
  _global->add("def", ::builtin::def,
               "def name ¿arguments? body\n");
  _global->add("return", ::builtin::ret);
  _global->add("list", ::builtin::list);
  _global->add("include", ::builtin::incl);
  _global->add("import", ::builtin::import);
  _global->add("sandbox", ::builtin::sandbox);

  // Create the global library.path list variable.
  cutlet::list *path = new cutlet::list();
  std::string env_path = env("CUTLETPATH");

  // Parse CUTLETPATH
  auto start = 0U;
  auto end = env_path.find(":");
  while (end != std::string::npos) {
    path->push_back(new cutlet::string(env_path.substr(start, end - start)));
    start = end + 1;
    end = env_path.find(":", start);
  }

  cutlet::variable::pointer pkglibdir = new cutlet::string(PKGLIBDIR);
  path->push_back(pkglibdir);
  global("library.path", path);
  global("library.dir", pkglibdir);

  // Create the toplevel frame with the default program return value.
  _frame = new cutlet::frame();
  _frame->label("_main_");
  _frame->_return = new cutlet::string(0);

  _interpreters++;
}

/*************************************
 * cutlet::interpreter::~interpreter *
 *************************************/

cutlet::interpreter::~interpreter() noexcept {
  /*  Clean up anything that is managed by reference points and the garbage
   * collector. This way when we run the collector nothing gets missed.
   */
  delete tokens;
  _frame = nullptr;
  _global = nullptr;
  _compiled = nullptr;

  _interpreters--;

  /*  When all the interpreters are destroyed then run a full memory
   * collection otherwise just run a normal one.
   */
  if (_interpreters == 0) { memory::gc::collect_all(); }
  else { memory::gc::collect(); }
}

/****************************
 * cutlet::interpreter::var *
 ****************************/

cutlet::variable::pointer cutlet::interpreter::var(const std::string &name) {
  // First attempt to resolve a local variable.
  variable::pointer result = _frame->variable(name);

  // If we don't have a local variable attempt to resolved to a global
  // variable.
  if (result.is_null())
    result = _global->variable(*this, name);

  // We couldn't resolve the variable.
  if (result.is_null())
    throw std::runtime_error(std::string("Unable to resolve variable $") +
                             name);

  return result;
}

/*******************************
 * cutlet::interpreter::global *
 *******************************/

void cutlet::interpreter::global(const std::string &name,
                                 variable::pointer value) {
  _global->variable(name, value);
}

/******************************
 * cutlet::interpreter::local *
 ******************************/

void cutlet::interpreter::local(const std::string &name,
                                variable::pointer value) {
  _frame->variable(name, value);
}

/*****************************
 * cutlet::interpreter::list *
 *****************************/

cutlet::variable::pointer cutlet::interpreter::list(const std::string value) {
  tokens->push(parser::token(cutlet::T_BLOCK, value));

  cutlet::list *result = new cutlet::list();
  while (*tokens and not tokens->expect(cutlet_tokenizer::T_EOF)) {
    if (tokens->expect(cutlet::T_BLOCK)) {
      auto token = tokens->get_token();
      result->push_back(list((const std::string &)token));
    } else {
      auto token = tokens->get_token();
      result->push_back(new cutlet::string((const std::string &)token));
    }
  }

  tokens->pop();
  return result;
}

cutlet::variable::pointer
cutlet::interpreter::list(const variable::pointer value) {
  ast::node *n = value->node();
  if (n) {
    tokens->push(n->token());
  } else {
    return list((std::string)*value);
  }

  cutlet::list *result = new cutlet::list();
  while (*tokens and not tokens->expect(cutlet_tokenizer::T_EOF)) {
    if (tokens->expect(cutlet::T_BLOCK)) {
      auto token = tokens->get_token();
      result->push_back(list((const std::string &)token));
    } else {
      auto token = tokens->get_token();
      result->push_back(new cutlet::string((const std::string &)token));
    }
  }

  tokens->pop();
  return result;
}

/****************************
 * cutlet::interpreter::add *
 ****************************/

void cutlet::interpreter::add(const std::string &name, function_t func,
                              const std::string &docs) {
  _global->add(name, func, docs);
}

void cutlet::interpreter::add(const std::string &name,
                              component::pointer comp) {
  _global->add(name, comp);
}

/************************************
 * cutlet::interpreter::environment *
 ************************************/

cutlet::sandbox::pointer cutlet::interpreter::environment() {
  return _global;
}

/****************************
 * cutlet::interpreter::get *
 ****************************/

cutlet::component::pointer
cutlet::interpreter::get(const std::string &name) const {
  return _global->get(name);
}

/************************************
 * cutlet::interpreter::operator () *
 ************************************/

cutlet::ast::node::pointer
cutlet::interpreter::operator()(variable::pointer code) {
  ast::node *n = code->node();
  if (n) {
    //parser::grammer::eval((std::string)*code);
    parser::grammer::eval(n->token());
  } else {
    parser::grammer::eval((std::string)*code);
  }
  _frame->_compiled = _compiled;
  return _compiled;
}

cutlet::ast::node::pointer
cutlet::interpreter::operator()(const std::string &code) {
  parser::grammer::eval(code);
  _frame->_compiled = _compiled;
  return _compiled;
}

cutlet::ast::node::pointer
cutlet::interpreter::operator()(std::istream &in) {
  parser::grammer::eval(in);
  _frame->_compiled = _compiled;
  return _compiled;
}

/*************************************
 * cutlet::interpreter::compile_file *
 *************************************/

cutlet::ast::node::pointer
cutlet::interpreter::compile_file(const std::string &filename) {
  std::ifstream input_file(filename);
  parser::grammer::eval(input_file, filename);
  input_file.close();
  _frame->_compiled = _compiled;
  return _compiled;
}

/*****************************
 * cutlet::interpreter::expr *
 *****************************/

cutlet::variable::pointer cutlet::interpreter::expr(variable::pointer cmd) {
  ast::node *n = cmd->node();
  if (n) {
    tokens->push(n->token());
  } else {
    tokens->push((std::string)*cmd);
  }
  ast::node::pointer result = command_();
  tokens->pop();

  if (result.is_null()) {
    std::clog << "DEBUG: " << (std::string)*cmd << std::endl;
  }

  return (*result)(*this);
}

cutlet::variable::pointer cutlet::interpreter::expr(const std::string &cmd) {
  tokens->push(cmd);
  ast::node::pointer result = command_();
  tokens->pop();
  return (*result)(*this);
}

/*****************************
 * cutlet::interpreter::call *
 *****************************/

cutlet::variable::pointer
cutlet::interpreter::call(const std::string &procedure,
                          const cutlet::list &arguments) {
  return _global->call(*this, procedure, arguments);
}

/******************************
 * cutlet::interpreter::frame *
 ******************************/

cutlet::frame::pointer cutlet::interpreter::frame(unsigned int levels) const {
  if (levels == 0) return _frame;
  return _frame->uplevel(levels);
}

int cutlet::interpreter::frames() const {
  frame::pointer ptr = _frame;
  int count = 0;
  while (ptr and not ptr->_sandbox_orig) {
    count++;
    ptr = ptr->_uplevel;
  }

  if (ptr and ptr->_sandbox_orig)
    count++;

  return count;
}

/*****************************
 * cutlet::interpreter::push *
 *****************************/

void cutlet::interpreter::push(const std::string &label) {
  frame::pointer new_frame = new cutlet::frame(label);
  push(new_frame);
}

void cutlet::interpreter::push(int level, const std::string &label) {
  push(new cutlet::block_frame(label, frame(level)));
}

void cutlet::interpreter::push(sandbox::pointer sb, const std::string &label) {
  frame::pointer new_frame = new cutlet::frame(label);
  push(new_frame, sb);
}

void cutlet::interpreter::push(frame::pointer new_frame) {
  new_frame->parent(_frame);
  _frame = new_frame;
}

void cutlet::interpreter::push(frame::pointer new_frame, sandbox::pointer sb) {
  new_frame->_sandbox_orig = _global;
  _global = sb;
  push(new_frame);
}

/****************************
 * cutlet::interpreter::pop *
 ****************************/

cutlet::variable::pointer cutlet::interpreter::pop() {
  // Get the previous environment.
  variable::pointer result = _frame->_return;
  memory::reference<sandbox> sb_saved = _frame->_sandbox_orig;

  // Retore the frame.
  memory::reference<cutlet::frame> uplevel = _frame->parent();
  _frame = uplevel;

  // Restore the global environment if necessary.
  if (not sb_saved.is_null()) _global = sb_saved;

  return result;
}

cutlet::variable::pointer
cutlet::interpreter::pop(variable::pointer result) {
  finish(result);
  return pop();
}

/*******************************
 * cutlet::interpreter::finish *
 *******************************/

void cutlet::interpreter::finish(variable::pointer result) {
  _frame->done(result);
}

/******************************
 * cutlet::interpreter::state *
 ******************************/

cutlet::frame::state_t cutlet::interpreter::state() const {
  return _frame->state();
}

void cutlet::interpreter::state(frame::state_t new_state) {
  _frame->state(new_state);
}

/*****************************
 * cutlet::interpreter::done *
 *****************************/

bool cutlet::interpreter::done() const {
  return _frame->done();
}

/*****************************
 * cutlet::interpreter::load *
 *****************************/

void cutlet::interpreter::load(const std::string &library_name) {
  _global->load(*this, library_name);
}

/******************************
 * cutlet::interpreter::entry *
 ******************************/

void cutlet::interpreter::entry() {
  ast::block *ast_tree = new ast::block();

  while (tokens or not tokens->expect(parser::tokenizer::T_EOF)) {

    // Remove empty lines.
    while (tokens->expect(cutlet::T_EOL)) tokens->next();

    // End of the file.
    if (tokens->expect(parser::tokenizer::T_EOF)) break;

    // Now we only expect commands and comments.
    if (tokens->expect(cutlet::T_COMMENT))
      ast_tree->add(comment_());
    else
      ast_tree->add(command_());
  }

  (*ast_tree)(*this);
  _compiled = ast_tree;
}

/*********************************
 * cutlet::interpreter::comment_ *
 *********************************/

cutlet::ast::node::pointer cutlet::interpreter::comment_() {
  return new ast::comment(tokens->get_token());
}

/*********************************
 * cutlet::interpreter::command_ *
 *********************************/

cutlet::ast::node::pointer cutlet::interpreter::command_() {
  ast::command *cmd_ast;

  // Get the command name.
  if (tokens->expect(cutlet::T_WORD) or
      tokens->expect(cutlet::T_BLOCK)) {
    cmd_ast = new ast::command(new ast::value(tokens->get_token()));

  } else if (tokens->expect(cutlet::T_STRING)) {
    cmd_ast = new ast::command(string_());

  } else if (tokens->expect(cutlet::T_VARIABLE)) {
    cmd_ast = new ast::command(variable_());

  } else if (tokens->expect(cutlet::T_SUBCMD)) {
    cmd_ast = new ast::command(subcommand_());

  } else {
    throw parser::syntax_error("Invalid token", tokens->get_token());
  }

  // Collect the command arguments.
  while (not tokens->expect(cutlet::T_EOL) and
         not tokens->expect(parser::tokenizer::T_EOF)) {

    if (tokens->expect(cutlet::T_VARIABLE)) {
      cmd_ast->parameter(variable_());
    } else if (tokens->expect(cutlet::T_STRING)) {
      cmd_ast->parameter(string_());
    } else if (tokens->expect(cutlet::T_SUBCMD)) {
      cmd_ast->parameter(subcommand_());
    } else if (tokens->expect(cutlet::T_WORD) or
               tokens->expect(cutlet::T_BLOCK)) {
      cmd_ast->parameter(new ast::value(tokens->get_token()));
    } else {
      throw parser::syntax_error("Invalid token", tokens->get_token());
    }
  }

  return cmd_ast;
}

/**********************************
 * cutlet::interpreter::variable_ *
 **********************************/

cutlet::ast::node::pointer cutlet::interpreter::variable_() {
  return new ast::variable(tokens->get_token());
}

/********************************
 * cutlet::interpreter::string_ *
 ********************************/

cutlet::ast::node::pointer cutlet::interpreter::string_() {
  auto token = tokens->get_token();
  ast::string *ast_str = new ast::string(token);
  std::string result((const std::string &)token);
  std::string part;

  // Scan the string for substitutions.
  utf8::iterator index(result);
  auto s_index = index;
  for (; index != index.end(); ++index) {

    if (*index == "$") {
      // Variable substitution.

      if (not part.empty()) {
        // Add the current string parts collected before the variable
        // substitution.
        ast_str->add(part);
        part.clear();
      }

      auto start = index;
      ++index;

      if (*index == "{") {
        // Quoted variable name.
        ++index;
        while (*index != "}" and index != index.end()) ++index;
        if (index == index.end())
          throw parser::syntax_error("Unmatched ${ in string", token);

        std::string var_name = utf8::substr(start + 2, index);

        ast_str->add(new ast::variable(parser::token(cutlet::T_VARIABLE,
                                                     var_name,
                                                     (size_t)token.position() +
                                                     start.position() + 1, 2)));
      } else {
        // Unquoted variable name.
        ++index;
        while ((*index != "$" and not is_space(*index)) and
               index != index.end())
          ++index;

        std::string var_name = utf8::substr(start + 1, index);

        ast_str->add(new ast::variable(parser::token(cutlet::T_VARIABLE,
                                                     var_name,
                                                     (size_t)token.position() +
                                                     start.position() + 1, 1)));
        --index;
      }

    } else if (*index == "[") {
      // Subcommand substitution.

      if (not part.empty()) {
        // Add the current string parts collected before the subcommand.
        ast_str->add(part);
        part.clear();
      }

      auto start = index;
      ++index;
      while (*index != "]" and index != index.end()) ++index;
      if (index == index.end())
        throw parser::syntax_error("Unmatched [ in string", token);

      parser::token cmd(cutlet::T_SUBCMD,
                        utf8::substr(start + 1, index),
                        (size_t)token.position() + start.position() + 1, 1);
      tokens->push(cmd);
      ast_str->add(command_());
      tokens->pop();

    } else if (*index == "\\") {
      // Escaped characters.

      auto start = index;
      ++index;
      if (*index == "$")
        part += "$";
      else if (*index == "\"")
        part += "\"";
      else if (*index == "'")
        part += "'";
      else if (*index == "[")
        part += "[";
      else if (*index == "]")
        part += "]";
      else if (*index == "\\")
        part += "\\";


      else if (*index == "a") // Bell/Alarm
        part += "\x07";
      else if (*index == "b") // Backspace
        part += "\x08";
      else if (*index == "e") // Escape
        part += "\x1b";
      else if (*index == "f") // Form feed
        part += "\x0c";
      else if (*index == "n") // New line (line feed)
        part += "\x0a";
      else if (*index == "r") // Carrage return
        part += "\x0d";
      else if (*index == "t") // Horizontal tab
        part += "\x09";
      else if (*index == "v") // Vertical tab
        part += "\x0b";

      else if (*index == "x") { // Hex byte values.
        ++index;
        std::string value;
        for (int count = 0; count < 2; ++count, ++index) {
          value += *index;
          if ((*index >= "0" and *index <= "9") or
              (*index >= "A" and *index <= "F") or
              (*index >= "a" and *index <= "f")) {
          } else {
            throw std::runtime_error(
              std::string("Invalid escaped hex value \\x") + value);
          }
        }

        unsigned char ch_val = 0;
        if (value[0] >= '0' and value[0] <= '9')
          ch_val = (unsigned char)(value[0] - '0') << 4;
        else if (value[0] >= 'a' and value[0] <= 'f')
          ch_val = ((unsigned char)(value[0] - 'a') + 10) << 4;
        else if (value[0] >= 'A' and value[0] <= 'F')
          ch_val = ((unsigned char)(value[0] - 'A') + 10) << 4;

        if (value[1] >= '0' and value[1] <= '9')
          ch_val |= (unsigned char)(value[1] - '0');
        else if (value[1] >= 'a' and value[1] <= 'f')
          ch_val |= (unsigned char)(value[1] - 'a') + 10;
        else if (value[1] >= 'A' and value[1] <= 'F')
          ch_val |= (unsigned char)(value[1] - 'A') + 10;

        std::string x;
        x += ch_val;
        part += x;
        --index;
      }

    } else {
      // Collect the actual literal parts of the string.
      part += *index;
    }
  }

  if (not part.empty()) {
    ast_str->add(part);
  }

  return ast_str;
}

/************************************
 * cutlet::interpreter::subcommand_ *
 ************************************/

cutlet::ast::node::pointer cutlet::interpreter::subcommand_() {
  tokens->push();
  ast::node::pointer result = command_();
  tokens->pop();
  return result;
}

unsigned int cutlet::interpreter::_interpreters = 0;
