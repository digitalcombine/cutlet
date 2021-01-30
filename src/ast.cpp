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
 *
 * The Abstract Syntax Tree.
 */

#include "ast.h"
#include <sstream>

//#define DEBUG_AST 1

#if DEBUG_AST
#pragma message ("AST debugging enabled")
#include <iostream>
#endif

template <typename Ty>
static bool is_type(cutlet::ast::node::pointer ast_object) {
  return ((dynamic_cast<Ty *>(&(*ast_object))) != nullptr);
}

/* This class extends cutlet::string to add a pointer to the node
 */
class value : public cutlet::string {
public:
  value(cutlet::ast::node &node) : cutlet::string(node.body()), _node(&node) {}
  virtual ~value() noexcept {}

protected:

private:
  cutlet::ast::node *_node;

  virtual operator cutlet::ast::node *() const {
    return _node;
  }
};

/******************************************************************************
 * class cutlet::ast::node
 */

bool cutlet::ast::node::break_all = false;

cutlet::debug_function_t cutlet::ast::node::_debug_function = nullptr;

void cutlet::ast::node::debugger(debug_function_t dfunc) {
  _debug_function = dfunc;
}

/***************************
 * cutlet::ast::node::node *
 ***************************/

cutlet::ast::node::node() : _break(false) {}

/****************************
 * cutlet::ast::node::~node *
 ****************************/

cutlet::ast::node::~node() noexcept {}

/********************************
 * cutlet::ast::node::set_break *
 ********************************/

void cutlet::ast::node::set_break(bool value) {
  _break = value;
}

/**********************************
 * cutlet::ast::node::break_point *
 **********************************/

void cutlet::ast::node::break_point(cutlet::interpreter &interp) {
  if (_debug_function and (_break or break_all))
    _debug_function(interp, *this);
}

/*****************************************************************************
 * class cutlet::ast::block
 */

/*****************************
 * cutlet::ast::block::block *
 *****************************/

cutlet::ast::block::block() : node() {}

/******************************
 * cutlet::ast::block::~block *
 ******************************/

cutlet::ast::block::~block() noexcept {}

/***************************
 * cutlet::ast::block::add *
 ***************************/

void cutlet::ast::block::add(cutlet::ast::node::pointer n) {
#if DEBUG_AST
  if (n.is_null()) {
    std::clog << "ast::block warning adding null ast node to block"
              << std::endl;
  }
#endif
  _nodes.push_back(n);
}

/***********************************
 * cutlet::ast::block::operator () *
 ***********************************/

cutlet::variable::pointer
cutlet::ast::block::operator()(cutlet::interpreter &interp) {
  for (auto node: _nodes) {
#if DEBUG_AST
    if (node.is_null()) {
      std::clog << "ast::block attempting to execute null ast node"
                << std::endl;
      continue;
    }
#endif
    (*node)(interp);
    if (interp.state() != frame::FS_RUNNING) break;
  }
  return nullptr;
}

/**************************
 * cutlet::ast::block::id *
 **************************/

unsigned int cutlet::ast::block::id() const {
  return cutlet::A_BLOCK;
}

/********************************
 * cutlet::ast::block::position *
 ********************************/

std::streampos cutlet::ast::block::position() const {
  if (not _nodes.empty()) return _nodes.front()->position();
  return 0;
}

/****************************
 * cutlet::ast::block::body *
 ****************************/

const std::string &cutlet::ast::block::body() const {
  return (const std::string &)token();
}

/*****************************
 * cutlet::ast::block::token *
 *****************************/

const parser::token &cutlet::ast::block::token() const {
  if (not _nodes.empty()) return _nodes.front()->token();
  throw cutlet::exception("empty ast::block");
}

/******************************************************************************
 * class cutlet::ast::value
 */

/*****************************
 * cutlet::ast::value::value *
 *****************************/

cutlet::ast::value::value(const parser::token &token) : node(), _token(token) {}

/******************************
 * cutlet::ast::value::~value *
 ******************************/

//#include <iostream>

cutlet::ast::value::~value() noexcept {}

/***********************************
 * cutlet::ast::value::operator () *
 ***********************************/

cutlet::variable::pointer
cutlet::ast::value::operator()(cutlet::interpreter &interp) {
  break_point(interp);

#if DEBUG_AST
  std::clog << "AST:" << position() << ": word " << (std::string)_token
            << std::endl;
#endif

  return new ::value(*this);
}

/**************************
 * cutlet::ast::value::id *
 **************************/

unsigned int cutlet::ast::value::id() const {
  return cutlet::A_VALUE;
}

/********************************
 * cutlet::ast::value::position *
 ********************************/

std::streampos cutlet::ast::value::position() const {
  return _token.position();
}

/*****************************
 * cutlet::ast::value::value *
 *****************************/

const std::string &cutlet::ast::value::body() const {
  return (const std::string &)token();
}

/*****************************
 * cutlet::ast::value::token *
 *****************************/

const parser::token &cutlet::ast::value::token() const {
  return _token;
}

/******************************************************************************
 * class cutlet::ast::variable
 */

/***********************************
 * cutlet::ast::variable::variable *
 ***********************************/

cutlet::ast::variable::variable(const parser::token &token)
  : node(), _token(token) {}

/************************************
 * cutlet::ast::variable::~variable *
 ************************************/

cutlet::ast::variable::~variable() noexcept {}

/*************************************
 * cutlet::ast::variable::operator() *
 *************************************/

cutlet::variable::pointer
cutlet::ast::variable::operator()(cutlet::interpreter &interp) {

  break_point(interp);

  try {
#if DEBUG_AST
    std::clog << "AST:" << position() << ": resolving variable $"
              << (std::string)_token << std::endl;
#endif
    return interp.var((const std::string &)_token);
  } catch (const cutlet::exception &err) {
    if (err.node() == nullptr) throw cutlet::exception(err.what(), *this);
    else throw;
  } catch (const std::exception& err) {
    throw cutlet::exception(err.what(), *this);
  }
}

/*****************************
 * cutlet::ast::variable::id *
 *****************************/

unsigned int cutlet::ast::variable::id() const {
  return cutlet::A_VARIABLE;
}

/***********************************
 * cutlet::ast::variable::position *
 ***********************************/

std::streampos cutlet::ast::variable::position() const {
  return _token.position();
}

/*******************************
 * cutlet::ast::variable::body *
 *******************************/

const std::string &cutlet::ast::variable::body() const {
  return (const std::string &)token();
}

/********************************
 * cutlet::ast::variable::token *
 ********************************/

const parser::token &cutlet::ast::variable::token() const {
  return _token;
}

/******************************************************************************
 * class cutlet::ast::command
 */

/*********************************
 * cutlet::ast::command::command *
 *********************************/

cutlet::ast::command::command(node::pointer n) : node(), _function(n) {}

/**********************************
 * cutlet::ast::command::~command *
 **********************************/

cutlet::ast::command::~command() noexcept {}

/***********************************
 * cutlet::ast::command::parameter *
 ***********************************/

void cutlet::ast::command::parameter(node::pointer n) {
  _parameters.push_back(n);
}

/************************************
 * cutlet::ast::command::operator() *
 ************************************/

cutlet::variable::pointer
cutlet::ast::command::operator()(cutlet::interpreter &interp) {
  cutlet::variable::pointer cmd = (*_function)(interp);

  cutlet::list c_params;
  for (auto &parameter: _parameters)
    c_params.push_back((*parameter)(interp));

  break_point(interp);

  try {
    if (is_type<ast::variable>(_function)) {
      // Execute the variable.
#if DEBUG_AST
      std::clog << "AST: operator $" << body() << " -> "
                << (std::string)*cmd << std::endl;
#endif
      return (*cmd)(cmd, interp, c_params);

    } else if (is_type<ast::command>(_function)) {
#if DEBUG_AST
      std::clog << "AST: command [" << (std::string)*cmd << "]"
                << std::endl;
#endif
      return (*cmd)(cmd, interp, c_params);

    } else if (is_type<ast::string>(_function)) {
#if DEBUG_AST
      std::clog << "AST: string " << (std::string)*cmd << std::endl;
#endif
      return (*cmd)(cmd, interp, c_params);

    } else {
      // Execute the function.
#if DEBUG_AST
      std::clog << "AST:" << position() << ": command " << (std::string)*cmd
                << std::endl;
#endif
      return interp.call((std::string)(*cmd), c_params);

    }
  } catch (const cutlet::exception &err) {
    throw;
  } catch (const std::exception &err) {
    /** @todo Need to add location function to nodes so we can let the users
     *        know where the problem was.
     */
    throw cutlet::exception(err.what(), *this);
  }
}

/****************************
 * cutlet::ast::command::id *
 ****************************/

unsigned int cutlet::ast::command::id() const {
  return cutlet::A_COMMAND;
}

/********************************
 * cutlet::ast::command::position *
 ********************************/

std::streampos cutlet::ast::command::position() const {
  return _function->position();
}

/******************************
 * cutlet::ast::command::body *
 ******************************/

const std::string &cutlet::ast::command::body() const {
  return (const std::string &)token();
}

/*****************************
 * cutlet::ast::command::token *
 *****************************/

const parser::token &cutlet::ast::command::token() const {
  return _function->token();
}

/******************************************************************************
 * class cutlet::ast::string
 */

/*******************************
 * cutlet::ast::string::string *
 *******************************/

cutlet::ast::string::string(const parser::token &token) : node(), _token(token) {}

/********************************
 * cutlet::ast::string::~string *
 ********************************/

cutlet::ast::string::~string() noexcept {}

/****************************
 * cutlet::ast::string::add *
 ****************************/

void cutlet::ast::string::add(const std::string &value) {
  if (not value.empty())
    _stringy.push_back({value, nullptr});
}

void cutlet::ast::string::add(node::pointer n) {
  _stringy.push_back({"", n});
}

/************************************
 * cutlet::ast::string::operator () *
 ************************************/

cutlet::variable::pointer
cutlet::ast::string::operator()(cutlet::interpreter &interp) {
  cutlet::string result;

  break_point(interp);

  // Run through the string parts and put it all together.
  for (auto &part: _stringy) {
    if (part.n.is_null()) {
      // Literal string part.
      result += part.s;
    } else {
      // Variable or command substitution.
      cutlet::variable::pointer v = (*(part.n))(interp);
      if (not v.is_null())
        result += (std::string)(*v);
    }
  }

#if DEBUG_AST
  std::clog << "AST:" << position() << ": \"" << result << "\"" << std::endl;
#endif

  return new cutlet::string(result);
}

/***************************
 * cutlet::ast::string::id *
 ***************************/

unsigned int cutlet::ast::string::id() const {
  return cutlet::A_STRING;
}

/*********************************
 * cutlet::ast::string::position *
 *********************************/

std::streampos cutlet::ast::string::position() const {
  return _token.position();
}

/*****************************
 * cutlet::ast::string::body *
 *****************************/

const std::string &cutlet::ast::string::body() const {
  return (const std::string &)token();
}

/******************************
 * cutlet::ast::string::token *
 ******************************/

const parser::token &cutlet::ast::string::token() const {
  return _token;
}

/******************************************************************************
 * class cutlet::ast::comment
 */

/*********************************
 * cutlet::ast::comment::comment *
 *********************************/

cutlet::ast::comment::comment(const parser::token &token)
  : node(), _token(token) {}

/**********************************
 * cutlet::ast::comment::~comment *
 **********************************/

cutlet::ast::comment::~comment() noexcept {}

/************************************
 * cutlet::ast::comment::operator () *
 ************************************/

cutlet::variable::pointer
cutlet::ast::comment::operator()(cutlet::interpreter &interp) {
  break_point(interp);

#if DEBUG_AST
  std::clog << "AST:" << position() << ": # " << (std::string)_token
            << std::endl;
#endif

  return nullptr;
}

/****************************
 * cutlet::ast::comment::id *
 ****************************/

unsigned int cutlet::ast::comment::id() const {
  return cutlet::A_COMMENT;
}

/**********************************
 * cutlet::ast::comment::position *
 **********************************/

std::streampos cutlet::ast::comment::position() const {
  return _token.position();
}

/******************************
 * cutlet::ast::comment::body *
 ******************************/

const std::string &cutlet::ast::comment::body() const {
  return (const std::string &)token();
}

/*******************************
 * cutlet::ast::comment::token *
 *******************************/

const parser::token &cutlet::ast::comment::token() const {
  return _token;
}
