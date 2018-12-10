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

cutlet::ast::node::~node() noexcept {}

/*****************************************************************************
 * class cutlet::ast::block
 */

cutlet::ast::block::block() {}

cutlet::ast::block::~block() noexcept {}

void cutlet::ast::block::add(cutlet::ast::node_ptr n) {
  nodes.push_back(n);
}

cutlet::variable_ptr
cutlet::ast::block::operator()(cutlet::interpreter &interp) {
  for (auto &n: nodes)
    (*n)(interp);
  return nullptr;
}

/*****************************************************************************
 * class cutlet::ast::value
 */

cutlet::ast::value::value(const parser::token &token) : token(token) {}

cutlet::ast::value::~value() noexcept {}

cutlet::variable_ptr
cutlet::ast::value::operator()(cutlet::interpreter &interp) {
  return new cutlet::string((std::string)token);
}

/*****************************************************************************
 * class cutlet::ast::variable
 */

cutlet::ast::variable::variable(const parser::token &token) : token(token) {}

cutlet::ast::variable::~variable() noexcept {}

#include <iostream>

cutlet::variable_ptr
cutlet::ast::variable::operator()(cutlet::interpreter &interp) {
  variable_ptr the_var = interp.var((const std::string &)token);
  if (the_var.is_null()) {
    throw std::runtime_error(std::string("No such variable ") +
                             (const std::string &)token);
  }
  return the_var;
}

/*****************************************************************************
 * class cutlet::ast::command
 */

cutlet::ast::command::command(node_ptr n) : func(n) {}

cutlet::ast::command::~command() noexcept {}

void cutlet::ast::command::parameter(node_ptr n) {
  params.push_back(n);
}

cutlet::variable_ptr
cutlet::ast::command::operator()(cutlet::interpreter &interp) {
  cutlet::list c_params;

  for (auto &p: params)
    c_params.push_back((*p)(interp));

  cutlet::ast::variable *var = dynamic_cast<cutlet::ast::variable *>(&(*func));
  cutlet::variable_ptr cmd = (*func)(interp);
  if (var) {
    return (*cmd)(interp, c_params);
  } else {
    return interp.execute((std::string)(*cmd), c_params);
  }
}

/*****************************************************************************
 * class cutlet::ast::string
 */

cutlet::ast::string::string() {}

cutlet::ast::string::~string() noexcept {}

void cutlet::ast::string::add(const std::string &value) {
  if (not value.empty())
    stringy.push_back({value, nullptr});
}

void cutlet::ast::string::add(node_ptr n) {
  stringy.push_back({"", n});
}

cutlet::variable_ptr
cutlet::ast::string::operator()(cutlet::interpreter &interp) {
  cutlet::string *result = new cutlet::string();

  for (auto &part: stringy) {
    if (part.n.is_null()) {
      *result += part.s;
    } else {
      variable_ptr v = (*part.n)(interp);
      *result += (std::string)(*v);
    }
  }

  return result;
}
