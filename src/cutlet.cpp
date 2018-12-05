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
#include <fstream>
#include "builtin.h"
#include "utilities.h"

class native_lib {
public:

  native_lib(const std::string &filename) {
#if defined (__linux__) || defined (__FreeBSD__)
    _handle = dlopen(filename.c_str(), RTLD_LAZY);
    if (_handle == NULL)
      throw std::runtime_error(dlerror());
#else
    throw std::runtime_error("Dynamic libraries are not supported.");
#endif
  }

  virtual ~native_lib() noexcept {
#if defined (__linux__) || defined (__FreeBSD__)
    if (_handle and dlclose(_handle) != 0)
      std::cerr << "WARNING: dlclose " << dlerror() << std::endl;
#endif
  }

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

/*****************************************************************************
 * class cutlet_tokenizer
 */

class cutlet_tokenizer : public parser::tokenizer {
public:
  // All of our token types
  static const unsigned int T_WORD     = 1;
  static const unsigned int T_VARIABLE = 2;
  static const unsigned int T_STRING   = 3;
  static const unsigned int T_SUBCMD   = 4;
  static const unsigned int T_BLOCK    = 5;
  static const unsigned int T_EOL      = 6;

  cutlet_tokenizer() {}

  friend std::ostream &operator <<(std::ostream &os,
                                   const cutlet_tokenizer &tks);

protected:
  virtual void parse_tokens();

protected:
  void parse_next_token();
};

#ifdef DEBUG
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

/************
 * is_space *
 ************/

static bool is_space(const std::string &value) {
  if (value == "\u0020") return true;  // SPACE
  if (value == "\u0009") return true;  // CHARACTER TABULATION (tab)

  if (value == "\u00a0") return true; // NO-BREAK SPACE
  if (value == "\u2000") return true; // EN QUAD
  if (value == "\u2001") return true; // EM QUAD
  if (value == "\u2002") return true; // EN SPACE (nut)
  if (value == "\u2003") return true; // EM SPACE (mutton)
  if (value == "\u2004") return true; // THREE-PER-EM SPACE (thick space)
  if (value == "\u2005") return true; // FOUR-PER-EM SPACE (mid space)
  if (value == "\u2006") return true; // SIX-PER-EM SPACE
  if (value == "\u2007") return true; // FIGURE SPACE
  if (value == "\u2008") return true; // PUNCTUATION SPACE
  if (value == "\u2009") return true; // THIN SPACE
  if (value == "\u200a") return true; // HAIR SPACE

  if (value == "\u202f") return true; // NARROW NO-BREAK SPACE
  if (value == "\u205f") return true; // MEDIUM MATHEMATICAL SPACE

  if (value == "\u3000") return true; // IDEOGRAPHIC SPACE
  return false;
}

/**********
 * is_eol *
 **********/

static bool is_eol(const std::string &value) {
  // Check for all of the unicode end of line characters.
  if (value == "\u000a") return true; // LF \n
  if (value == "\u000b") return true; // VT \v
  if (value == "\u000c") return true; // FF \f
  if (value == "\u000d\u000a") return true; // CR LF \r\n
  if (value == "\u000d") return true; // CR \r
  if (value == "\u0085") return true; // NEL
  if (value == "\u2028") return true; // LS
  if (value == "\u2029") return true; // PS
  return false;
}

/******************************************************************************
 * class cutlet::utf8::iterator
 */

/************************************
 * cutlet::utf8::iterator::iterator *
 ************************************/

cutlet::utf8::iterator::iterator(const std::string &value)
  : _value(&value), _index(0) {
  _length = 1;

  // Find the length of the character and make a copy for dereferencing.
  for (; ((*_value)[_index + _length] & 0xc0) == 0x80; ++_length);
  _current = _value->substr(_index, _length);
}

cutlet::utf8::iterator::iterator(const std::string &value, size_t offset)
  : _value(&value), _index(offset) {
  _length = 1;

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
 * cutlet::utf8::Iterator::operator -- *
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
    size_t tmp = _index;

    // Find the first character.
    for (--_index; ((*_value)[_index] & 0xc0) == 0x80 and _index > 0;
         --_index);

    // Get the length of the character and copy it.
    _length = tmp - _index;
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

cutlet::utf8::iterator cutlet::utf8::iterator::operator +(int value) const {
  auto result = *this;
  if (value > 0) for (; value > 0; ++result, --value);
  else for (; value < 0; --result, ++value);
  return result;
}

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
  if (start != start.end() and end._index >= start._index) {
    return value.substr(start._index, end._index - start._index);
  }
  return "";
  //throw std::range_error("sub string iterators out of range");
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


/*****************************************************************************
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
      if (not getline(*stream, code, '\0')) {
        tokens.push_back(parser::token(T_EOF, "", line, offset));
        stream = NULL;
        return;
      }

      // Position tracking.
      ++line;
      offset = 0;
    } else
      return;
  }

  // If we have some code then tokenize it.
  if (not code.empty()) {
    while (not code.empty())
      parse_next_token();
    if (not stream)
      tokens.push_back(parser::token(T_EOF, "", line, offset));
  }
}

/**************************************
 * cutlet_tokenizer::parse_next_token *
 **************************************/

void cutlet_tokenizer::parse_next_token() {
  if (not code.empty()) {
    cutlet::utf8::iterator it(code);

    // Skip any white space.
    while (is_space(*it) and it != it.end()) ++it;
    if (it == it.end()) {
      code.clear();
      return;
    }

    switch ((*it)[0]) {

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
      tokens.push_back(parser::token(T_VARIABLE,
                                     cutlet::utf8::substr(start, it),
                                     line, offset));
      break;
    }

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
                                     parser::token(T_STRING,
                                                   cutlet::utf8::substr(start,
                                                                        it),
                                                   line, offset));
      }

      // Add the token.
      tokens.push_back(parser::token(T_STRING,
                                     cutlet::utf8::substr(start, it),
                                     line, offset));
      ++it; // Remove trailing "
      break;
    }

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
                                     parser::token(T_STRING,
                                                   cutlet::utf8::substr(start,
                                                                        it),
                                                   line, offset));
      }

      // Add the token.
      tokens.push_back(parser::token(T_STRING,
                                     cutlet::utf8::substr(start, it),
                                     line, offset));
      ++it; // Remove trailing '
      break;
    }

    case '[': {
      ++it; // Remove the [ character.

      cutlet::utf8::iterator start(it), previous(it);
      unsigned int count = 1;

      // Find the matching ] character.
      while (count) {

        // Matching ] character not found, throw and error.
        if (is_eol(*it) or it == it.end())
          throw parser::syntax_error("Unmatched [",
                                     parser::token(T_SUBCMD,
                                                   cutlet::utf8::substr(start,
                                                                        it),
                                                   line, offset));

        // Keep track of subcommands within this subcommand.
        if (*it == "]") {
          count--;
          previous = it;
        } else if (*it == "[") count++;

        ++it; // Next character please.
      }

      // Add the token.
      tokens.push_back(parser::token(T_SUBCMD,
                                     cutlet::utf8::substr(start, previous),
                                     line, offset));
      break;
    }

    case '{': {
      ++it; // Remove the { character.

      cutlet::utf8::iterator start(it), previous(it);
      unsigned int count = 1;

      // Find the matching } character.
      while (count) {

        // Matching } character not found, throw and error.
        if (it == it.end())
          throw parser::syntax_error("Unmatched {",
                                     parser::token(T_BLOCK,
                                                   cutlet::utf8::substr(start,
                                                                        it),
                                                   line, offset));

        // Keep track of blocks within this block.
        if (*it == "}") {
          count--;
          previous = it;
        } else if (*it == "{") count++;

        ++it; // Next character please.
      }

      // Add the token.
      tokens.push_back(parser::token(T_BLOCK,
                                     cutlet::utf8::substr(start, previous),
                                     line, offset));
      break;
    }

    default: {
      cutlet::utf8::iterator start(it);

      if (is_eol(*it) or it == it.end()) {
        // End of line token.
        tokens.push_back(parser::token(T_EOL, *it, line, offset));
        ++it;

      } else if ((tokens.empty() or (unsigned int)tokens.back() == T_EOL) and
                 (*it)[0] == '#') {
        // Strip out the comment.
        while (not is_eol(*it)) ++it;

      } else {
        // Add the word token.
        while (not is_space(*it) and not is_eol(*it) and it != it.end())
          ++it;
        tokens.push_back(parser::token(T_WORD, cutlet::utf8::substr(start, it),
                                       line, offset));
      }
      break;
    }
    }

    // Update the remaining code.
    code = cutlet::utf8::substr(it, it.end());

    //std::cout << *this << std::flush;
  }
}

/*****************************************************************************
 * class cutlet::component
 */

/*********************************
 * cutlet::component::~component *
 *********************************/

cutlet::component::~component() noexcept {}

/*****************************************************************************
 * class cutlet::variable
 */

cutlet::variable::~variable() noexcept {}

/*****************************
 * cutlet::variable::execute *
 *****************************/

cutlet::variable_ptr
cutlet::variable::operator()(interpreter &interp, const list &parameters) {
  // By default variables are not executable.
  throw std::runtime_error("Variable is not executable");
}

/******************************************
 * cutlet::variable::operator std::string *
 ******************************************/

cutlet::variable::operator std::string() const { return ""; }

/*****************************************************************************
 * class cutlet::string
 */

/**************************
 * cutlet::string::string *
 **************************/

cutlet::string::string() : std::string() {}

cutlet::string::string(const std::string &value) : std::string(value) {}

cutlet::string::string(int value) {
  std::stringstream ss;
  ss << value;
  *this = ss.str();
}

/***************************
 * cutlet::string::~string *
 ***************************/

cutlet::string::~string() noexcept {}

/****************************************
 * cutlet::string::operator std::string *
 ****************************************/

cutlet::string::operator std::string() const { return *this; }

template <> int cutlet::convert<int>(variable_ptr object) {
  std::stringstream ss((std::string)(*(object)));
  int result;
  ss >> result;
  return result;
}

/*****************************************************************************
 * class _function
 *
 *  A component wrapper around the cutlet::function_t.
 */

class _function : public cutlet::component {
public:
  _function(cutlet::function_t func) : _function_ptr(func) {}
  virtual ~_function() noexcept {}

  virtual cutlet::variable_ptr
  operator ()(cutlet::interpreter &interp,
              const cutlet::list &parameters) {
    return _function_ptr(interp, parameters);
  }

private:
  cutlet::function_t _function_ptr;
};

/*****************************************************************************
 * class cutlet::sandbox
 */

/****************************
 * cutlet::sandbox::sandbox *
 ****************************/

cutlet::sandbox::sandbox() {
}

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

void cutlet::sandbox::add(const std::string &name, function_t func) {
  _components[name] = new _function(func);
}

void cutlet::sandbox::add(const std::string &name, component_ptr comp) {
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
  _components.clear();
}

/************************
 * cutlet::sandbox::get *
 ************************/

cutlet::component_ptr cutlet::sandbox::get(const std::string &name) const {
  return _components.at(name);
}

/*****************************
 * cutlet::sandbox::variable *
 *****************************/

cutlet::variable_ptr cutlet::sandbox::variable(interpreter &interp,
                                               const std::string &name) {
  if (_variables.find(name) != _variables.end())
    return _variables[name];
  else {
    try {
      list params({new cutlet::string(name)});
      return execute(interp, "¿variable?", params);
    } catch (std::runtime_error &err) {}
  }
  return nullptr;
}

void cutlet::sandbox::variable(const std::string &name, variable_ptr value) {
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

/****************************
 * cutlet::sandbox::execute *
 ****************************/

cutlet::variable_ptr
cutlet::sandbox::execute(interpreter &interp,
                         const std::string &name,
                         const list &parameters) {
  auto it = _components.find(name);
  if (it != _components.end()) {
    return (*it->second)(interp, parameters);
  } else {
    it = _components.find("¿component?");
    if (it != _components.end()) {
      cutlet::list params(parameters);
      params.push_front(new cutlet::string(name));
      return (*it->second)(interp, params);
    } else {
      throw std::runtime_error(std::string("Unresolved symbol ") + name);
    }
  }
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
      init(&interp);
    } else {
      delete lib;
      throw std::runtime_error(std::string("init_cultet missing in library ") +
                               library_name);
    }

    _native_libs.push_back(lib);
  }
}

/*****************************************************************************
 * class cutlet::interpreter
 */

/************************
 * cutlet::frame::frame *
 ************************/

cutlet::frame::frame() : _done(false) {
}

cutlet::frame::frame(memory::reference<frame> uplevel)
  : _uplevel(uplevel), _done(false) {
}

/*************************
 * cutlet::frame::~frame *
 *************************/

cutlet::frame::~frame() noexcept {
}

/***************************
 * cutlet::frame::variable *
 ***************************/

cutlet::variable_ptr cutlet::frame::variable(const std::string &name) const {
  auto item = _variables.find(name);
  if (item != _variables.end())
    return item->second;
  return nullptr;
}

void cutlet::frame::variable(const std::string &name, variable_ptr value) {
  if (value.is_null())
    _variables.erase(name);
  else
    _variables[name] = value;
}

/***********************
 * cutlet::frame::done *
 ***********************/

void cutlet::frame::done(variable_ptr result) {
  _return = result;
  _done = true;
}

bool cutlet::frame::done() const {
  return _done;
}

/*****************************************************************************
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
  _global->add("def", ::builtin::def);
  _global->add("return", ::builtin::ret);
  _global->add("list", ::builtin::list);
  _global->add("import", ::builtin::import);


  global("library.path", new cutlet::list());

  _frame = new frame();
}

/*************************************
 * cutlet::interpreter::~interpreter *
 *************************************/

cutlet::interpreter::~interpreter() {
  delete tokens;
}

/****************************
 * cutlet::interpreter::var *
 ****************************/

cutlet::variable_ptr cutlet::interpreter::var(const std::string &name) {
  variable_ptr result = _frame->variable(name);
  if (result.is_null())
    result = _global->variable(*this, name);

  if (result.is_null())
    throw std::runtime_error(std::string("Unable to resolve variable ") + name);

  return result;
}

/*******************************
 * cutlet::interpreter::global *
 *******************************/

void cutlet::interpreter::global(const std::string &name,
                                 variable_ptr value) {
  _global->variable(name, value);
}

/******************************
 * cutlet::interpreter::local *
 ******************************/

void cutlet::interpreter::local(const std::string &name,
                                variable_ptr value) {
  _frame->variable(name, value);
}

/*****************************
 * cutlet::interpreter::list *
 *****************************/

cutlet::variable_ptr cutlet::interpreter::list(const std::string value) {
  tokens->push(parser::token(cutlet_tokenizer::T_BLOCK, value));

  cutlet::list *result = new cutlet::list();
  while (*tokens and not tokens->expect(cutlet_tokenizer::T_EOF)) {
    if (tokens->expect(cutlet_tokenizer::T_BLOCK)) {
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

void cutlet::interpreter::add(const std::string &name, function_t func) {
  _global->add(name, func);
}

void cutlet::interpreter::add(const std::string &name, component_ptr comp) {
  _global->add(name, comp);
}

/*******************************
 * cutlet::interpreter::remove *
 *******************************/

void cutlet::interpreter::remove(const std::string &name) {
  _global->remove(name);
}

/******************************
 * cutlet::interpreter::clear *
 ******************************/

void cutlet::interpreter::clear() {
  _global->clear();
}

/****************************
 * cutlet::interpreter::get *
 ****************************/

cutlet::component_ptr cutlet::interpreter::get(const std::string &name) const {
  return _global->get(name);
}

/***********************************
 * cutlet::interpreter::frame_push *
 ***********************************/

void cutlet::interpreter::frame_push() {
  memory::reference<frame> new_frame = new frame(_frame);
  _frame = new_frame;
}

void cutlet::interpreter::frame_push(frame_ptr new_frame) {
  new_frame->_uplevel = _frame;
  _frame = new_frame;
}

void cutlet::interpreter::frame_push(sandbox_ptr sb) {
  memory::reference<frame> new_frame = new frame(_frame);
  _frame = new_frame;
  new_frame->_sandbox_orig = _global;
  _global = sb;
}

/**********************************
 * cutlet::interpreter::frame_pop *
 **********************************/

cutlet::variable_ptr cutlet::interpreter::frame_pop() {
  // Get the previous environment.
  variable_ptr result = _frame->_return;
  memory::reference<sandbox> sb_saved = _frame->_sandbox_orig;

  // Retore the frame.
  memory::reference<frame> uplevel = _frame->_uplevel;
  _frame = uplevel;

  // Restore the global environment if necessary.
  if (not sb_saved.is_null())
    _global = sb_saved;

  return result;
}

/***********************************
 * cutlet::interpreter::frame_done *
 ***********************************/

void cutlet::interpreter::frame_done(variable_ptr result) {
  _frame->done(result);
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
  while (tokens or not tokens->expect(cutlet_tokenizer::T_EOF)) {
    while (tokens->expect(cutlet_tokenizer::T_EOL)) tokens->next();
    if (tokens->expect(cutlet_tokenizer::T_EOF)) break;
    command();
    if (_frame->done()) return;
  }
}

/********************************
 * cutlet::interpreter::command *
 ********************************/

cutlet::variable_ptr cutlet::interpreter::command() {
  std::string cmd;
  cutlet::list args;
  variable_ptr evar;

  // Get the command name.
  if (tokens->expect(cutlet_tokenizer::T_WORD) or
      tokens->expect(cutlet_tokenizer::T_BLOCK)) {
    cmd = (const std::string &)tokens->get_token();

  } else if (tokens->expect(cutlet_tokenizer::T_STRING)) {
    auto result = string();
    if (not result.is_null())
      cmd = (const std::string &)(*result);

  } else if (tokens->expect(cutlet_tokenizer::T_VARIABLE)) {
    evar = variable();

  } else if (tokens->expect(cutlet_tokenizer::T_SUBCMD)) {
    auto result = subcommand();
    if (not result.is_null())
      cmd = (const std::string &)(*result);

  } else {
    throw parser::syntax_error("Invalid token", tokens->get_token());
  }

  // Collect the command arguments.
  while (not tokens->expect(cutlet_tokenizer::T_EOL) and
         not tokens->expect(cutlet_tokenizer::T_EOF)) {

    //std::cout << "  " << (const std::string &)token << std::endl;
    if (tokens->expect(cutlet_tokenizer::T_VARIABLE)) {
      args.push_back(variable());
    } else if (tokens->expect(cutlet_tokenizer::T_STRING)) {
      args.push_back(string());
    } else if (tokens->expect(cutlet_tokenizer::T_SUBCMD)) {
      args.push_back(subcommand());
    } else if (tokens->expect(cutlet_tokenizer::T_WORD) or
               tokens->expect(cutlet_tokenizer::T_BLOCK)) {
      auto token = tokens->get_token();
      args.push_back(new cutlet::string((const std::string &)token));
    } else {
      throw std::runtime_error("Unknown token");
    }
  }

  if (tokens->expect(cutlet_tokenizer::T_EOL)) tokens->next(); // Remove EOL

  // Now actually execute the command.
  if (not evar.is_null()) {
    return (*evar)(*this, args);
  } else {
    return _global->execute(*this, cmd, args);
  }
}

/*********************************
 * cutlet::interpreter::variable *
 *********************************/

cutlet::variable_ptr cutlet::interpreter::variable() {
  auto token = tokens->get_token();
  variable_ptr the_var = var((const std::string &)token);
  if (the_var.is_null())
    throw std::runtime_error(std::string("No such variable ") +
                             (const std::string &)token);
  return the_var;
}

/*******************************
 * cutlet::interpreter::string *
 *******************************/

cutlet::variable_ptr cutlet::interpreter::string() {
  auto token = tokens->get_token();
  std::string result = (const std::string &)token;

  // Scan the string for substitutions.
  auto index = utf8::iterator(result);
  for (; index != index.end(); ++index) {

    if (*index == "$") {
      // Variable substitution.
      auto start = index;
      ++index;

      if (*index == "{") {
        // Quoted variable name.
        ++index;
        while (*index != "}" and index != index.end()) ++index;
        if (index == index.end())
          throw parser::syntax_error("Unmatched ${ in string", token);

        std::string var_name = utf8::substr(start + 2, index);
        std::string value = (const std::string)(*var(var_name));

        result = utf8::replace(start, index, value);
        index = utf8::iterator(result, index.position() -
                               (((index.position() + index.length()) -
                                 start.position()) - value.size()));
      } else {
        // Unquoted variable name.
        ++index;
        while ((*index != "$" and not is_space(*index)) and
               index != index.end())
          ++index;

        std::string var_name = utf8::substr(start + 1, index);
        std::string value = (const std::string)(*var(var_name));

        result = utf8::replace(start, index - 1, value);
        index = utf8::iterator(result, index.position() -
                               (((index.position() + index.length()) -
                                 start.position()) - value.size()));
      }

    } else if (*index == "[") {
      // Subcommand substitution.
      auto start = index;
      ++index;
      while (*index != "]" and index != index.end()) ++index;
      if (index == index.end())
        throw parser::syntax_error("Unmatched [ in string", token);

      std::string cmd = utf8::substr(start + 1, index);
      tokens->push(cmd);
      std::string value = (const std::string)(*command());
      tokens->pop();

      result = utf8::replace(start, index, value);
      index = utf8::iterator(result, index.position() -
                             (((index.position() + index.length()) -
                               start.position()) - value.size()));

    } else if (*index == "\\") {
      // Escaped characters.
      auto start = index;
      ++index;
      if (*index == "$")
        result = utf8::replace(start, index, "$");
      else if (*index == "\"")
        result = utf8::replace(start, index, "\"");
      else if (*index == "'")
        result = utf8::replace(start, index, "'");
      else if (*index == "\\")
        result = utf8::replace(start, index, "\\");

      else if (*index == "a") // Bell/Alarm
        result = utf8::replace(start, index, "\x07");
      else if (*index == "b") // Backspace
        result = utf8::replace(start, index, "\x08");
      else if (*index == "e") // Backspace
        result = utf8::replace(start, index, "\x1b");
      else if (*index == "f") // Form feed
        result = utf8::replace(start, index, "\x0c");
      else if (*index == "n") // New line (line feed)
        result = utf8::replace(start, index, "\x0a");
      else if (*index == "r") // Carrage return
        result = utf8::replace(start, index, "\x0d");
      else if (*index == "t") // Horizontal tab
        result = utf8::replace(start, index, "\x09");
      else if (*index == "v") // Vertical tab
        result = utf8::replace(start, index, "\x0b");

      else if (*index == "x") {
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
          ch_val = (unsigned int)(value[0] - '0') << 4;
        else if (value[0] >= 'a' and value[0] <= 'f')
          ch_val = ((unsigned int)(value[0] - 'a') + 10) << 4;
        else if (value[0] >= 'A' and value[0] <= 'F')
          ch_val = ((unsigned int)(value[0] - 'A') + 10) << 4;

        if (value[1] >= '0' and value[1] <= '9')
          ch_val |= (unsigned int)(value[1] - '0');
        else if (value[1] >= 'a' and value[1] <= 'f')
          ch_val |= (unsigned int)(value[1] - 'a') + 10;
        else if (value[1] >= 'A' and value[1] <= 'F')
          ch_val |= (unsigned int)(value[1] - 'A') + 10;

        std::string x;
        x += ch_val;
        result = utf8::replace(start, index - 1, x);
        index = start + 1;
      }

      --index;
    }
  }

  return new cutlet::string(result);
}

/***********************************
 * cutlet::interpreter::subcommand *
 ***********************************/

cutlet::variable_ptr cutlet::interpreter::subcommand() {
  tokens->push();
  variable_ptr result = command();
  tokens->pop();
  return result;
}
