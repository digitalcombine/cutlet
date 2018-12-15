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

/*****************************************************************************
 * class cutlet::ast::node
 */

/****************************
 * cutlet::ast::node::~node *
 ****************************/

cutlet::ast::node::~node() noexcept {}

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

void cutlet::ast::block::add(cutlet::ast::node_ptr n) {
  _nodes.push_back(n);
}

/***********************************
 * cutlet::ast::block::operator () *
 ***********************************/

cutlet::variable_ptr
cutlet::ast::block::operator()(cutlet::interpreter &interp) {
  for (auto &n: _nodes)
    (*n)(interp);
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

cutlet::variable_ptr
cutlet::ast::value::operator()(cutlet::interpreter &interp) {
  return new cutlet::string((const std::string &)_token);
}

/*****************************************************************************
 * class cutlet::ast::variable
 */

/***********************************
 * cutlet::ast::variable::variable *
 ***********************************/

cutlet::ast::variable::variable(const parser::token &token) : _token(token) {}

/************************************
 * cutlet::ast::variable::~variable *
 ************************************/

cutlet::ast::variable::~variable() noexcept {}

/*************************************
 * cutlet::ast::variable::operator() *
 *************************************/

cutlet::variable_ptr
cutlet::ast::variable::operator()(cutlet::interpreter &interp) {
  try {
    return interp.var((const std::string &)_token);
  } catch (std::runtime_error &err) {
    std::stringstream mesg;
    mesg << _token.line() << ":" << _token.offset() << ": "
         << err.what();
    throw std::runtime_error(mesg.str());
  }
}

/*****************************************************************************
 * class cutlet::ast::command
 */

/*********************************
 * cutlet::ast::command::command *
 *********************************/

cutlet::ast::command::command(node_ptr n) : _function(n) {}

/**********************************
 * cutlet::ast::command::~command *
 **********************************/

cutlet::ast::command::~command() noexcept {}

/***********************************
 * cutlet::ast::command::parameter *
 ***********************************/

void cutlet::ast::command::parameter(node_ptr n) {
  _parameters.push_back(n);
}

/************************************
 * cutlet::ast::command::operator() *
 ************************************/

cutlet::variable_ptr
cutlet::ast::command::operator()(cutlet::interpreter &interp) {
  cutlet::list c_params;

  for (auto &parameter: _parameters)
    c_params.push_back((*parameter)(interp));

  /* First attempt to cast the func node to a variable node.
   */
  cutlet::ast::variable *var =
    dynamic_cast<cutlet::ast::variable *>(&(*_function));
  cutlet::variable_ptr cmd = (*_function)(interp);

  try {
    if (var) {
      return (*cmd)(interp, c_params);
    } else {
      return interp.execute((std::string)(*cmd), c_params);
    }
  } catch (std::runtime_error &err) {
    /** @todo Need to add location function to nodes so we can let the users
     *        know where the problem was.
     */
    std::stringstream mesg;
    mesg << err.what();
    throw std::runtime_error(mesg.str());
  }
}

/*****************************************************************************
 * class cutlet::ast::string
 */

/*******************************
 * cutlet::ast::string::string *
 *******************************/

cutlet::ast::string::string() {}

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

void cutlet::ast::string::add(node_ptr n) {
  _stringy.push_back({"", n});
}

/************************************
 * cutlet::ast::string::operator () *
 ************************************/

cutlet::variable_ptr
cutlet::ast::string::operator()(cutlet::interpreter &interp) {
  cutlet::string *result = new cutlet::string();

  // Run through the string parts and put it all together.
  for (auto &part: _stringy) {
    if (part.n.is_null()) {
      // Literal string part.
      *result += part.s;
    } else {
      // Variable or command substitution.
      variable_ptr v = (*part.n)(interp);
      *result += (std::string)(*v);
    }
  }

  return result;
}
