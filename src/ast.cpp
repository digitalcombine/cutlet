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

/*****************************************************************************
 * class cutlet::ast::node
 */

/****************************
 * cutlet::ast::node::~node *
 ****************************/

cutlet::ast::node::~node() noexcept {}

/*******************************
 * cutlet::ast::node::location *
 *******************************/

std::string cutlet::ast::node::location() const {
  return "";
}

/*****************************************************************************
 * class cutlet::ast::block
 */

/*****************************
 * cutlet::ast::block::block *
 *****************************/

cutlet::ast::block::block() {}

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
    } else
#endif
    (*node)(interp);
    if (interp.frame_state() != frame::FS_RUNNING) break;
  }
  return nullptr;
}

/*****************************************************************************
 * class cutlet::ast::value
 */

/*****************************
 * cutlet::ast::value::value *
 *****************************/

cutlet::ast::value::value(const parser::token &token) : _token(token) {}

/******************************
 * cutlet::ast::value::~value *
 ******************************/

cutlet::ast::value::~value() noexcept {}

/***********************************
 * cutlet::ast::value::operator () *
 ***********************************/

cutlet::variable::pointer
cutlet::ast::value::operator()(cutlet::interpreter &interp) {
  (void)interp;

#if DEBUG_AST
    std::clog << "AST: word " << (std::string)_token << std::endl;
#endif

  return new cutlet::string((const std::string &)_token);
}

/********************************
 * cutlet::ast::value::location *
 ********************************/

std::string cutlet::ast::value::location() const {
  std::stringstream result;
  result << _token.position() << ": ";
  return result.str();
}

/*****************************************************************************
 * class cutlet::ast::variable
 */

/***********************************
 * cutlet::ast::variable::variable *
 ***********************************/

cutlet::ast::variable::variable(const parser::token &token)
  : _token(token) {}

/************************************
 * cutlet::ast::variable::~variable *
 ************************************/

cutlet::ast::variable::~variable() noexcept {}

/*************************************
 * cutlet::ast::variable::operator() *
 *************************************/

cutlet::variable::pointer
cutlet::ast::variable::operator()(cutlet::interpreter &interp) {
  try {
#if DEBUG_AST
    std::clog << "AST: resolving variable $" << (std::string)_token
              << std::endl;
#endif
    return interp.var((const std::string &)_token);
  } catch (const cutlet::exception &err) {
    if (err.node() == nullptr) throw cutlet::exception(err.what(), *this);
    else throw;
  } catch (const std::exception& err) {
    throw cutlet::exception(err.what(), *this);
  }
}

/***********************************
 * cutlet::ast::variable::location *
 ***********************************/

std::string cutlet::ast::variable::location() const {
  std::stringstream result;
  result << _token.position() << ": ";
  return result.str();
}

/*****************************************************************************
 * class cutlet::ast::command
 */

/*********************************
 * cutlet::ast::command::command *
 *********************************/

cutlet::ast::command::command(node::pointer n) : _function(n) {}

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

  try {
    if (is_type<ast::variable>(_function)) {
      // Execute the variable.
#if DEBUG_AST
      std::clog << "AST: operator $" << std::endl;
#endif
      return (*cmd)(cmd, interp, c_params);

    } else if (is_type<ast::command>(_function)) {
#if DEBUG_AST
      std::clog << "AST: command [" << (std::string)*cmd << "]"
                << std::endl;
#endif
      return (*cmd)(cmd, interp, c_params);

    } else {
      // Execute the function.
#if DEBUG_AST
      std::clog << "AST: command " << (std::string)*cmd
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

/**********************************
 * cutlet::ast::command::location *
 **********************************/

std::string cutlet::ast::command::location() const {
  return _function->location();
}

/*****************************************************************************
 * class cutlet::ast::string
 */

/*******************************
 * cutlet::ast::string::string *
 *******************************/

cutlet::ast::string::string(const parser::token &token) : _token(token) {}

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
  std::clog << "AST: \"" << result << "\"" << std::endl;
#endif

  return new cutlet::string(result);
}

/*********************************
 * cutlet::ast::string::location *
 *********************************/

std::string cutlet::ast::string::location() const {
  std::stringstream result;
  result << _token.position() << ": ";
  return result.str();
}

/*****************************************************************************
 * class cutlet::ast::comment
 */

/*********************************
 * cutlet::ast::comment::comment *
 *********************************/

cutlet::ast::comment::comment(const parser::token &token) : _token(token) {}

/**********************************
 * cutlet::ast::comment::~comment *
 **********************************/

cutlet::ast::comment::~comment() noexcept {}

/************************************
 * cutlet::ast::string::operator () *
 ************************************/

cutlet::variable::pointer
cutlet::ast::comment::operator()(cutlet::interpreter &interp) {
  (void)interp;
#if DEBUG_AST
  std::clog << "AST: # " << (std::string)_token << std::endl;
#endif

  return nullptr;
}
