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
 * This library includes:
 * def false
 * def true
 * def eval *args
 * def expr *args
 * def if ¿then? body
 *  ¿elif condition ¿then? body ...?
 *  ¿else body?
 * def while condition ¿do? body
 * def raise *args
 * def try body ¿catch varname body?
 */

#include <cutlet.h>

/******************************************************************************
 *  To make our lives easier we recursive decent parser like methods to help
 * us parse the function parameters.
 */

static bool is_more(cutlet::list::const_iterator &it,
                    const cutlet::list &list) {
  return (it != list.end());
}

static void next(cutlet::list::const_iterator &it,
                 const cutlet::list &list) {
  ++it;
  if (it == list.end())
    throw std::runtime_error("Expected more parameters.");
}

static bool expect(cutlet::list::const_iterator &it,
                   const std::string &value) {
  if (cutlet::convert<std::string>(*it) == value) return true;
  return false;
}

static void permit(cutlet::list::const_iterator &it,
                   const std::string &value) {
  if (cutlet::convert<std::string>(*it) != value)
    throw std::runtime_error(std::string("Expected ") + value +
                             " but got " +  cutlet::convert<std::string>(*it)
                             + " instead.");
}

static cutlet::frame::state_t eval_body(cutlet::interpreter &interp,
                                        const std::string &body) {
  cutlet::frame::state_t result;
  interp.frame_push(new cutlet::block_frame(interp.frame()));
  try {
    interp.eval(body);
  } catch(...) {
    result = interp.frame_state();
    interp.frame_pop();
    throw;
  }
  result = interp.frame_state();
  interp.frame_pop();

  return result;
}

/******************************************************************************
 */

// def false
static cutlet::variable_ptr
_false(cutlet::interpreter &interp, const cutlet::list &parameters) {
  return new cutlet::string("false");
}

// def true
static cutlet::variable_ptr
_true(cutlet::interpreter &interp, const cutlet::list &parameters) {
  return new cutlet::string("true");
}

// def eval *args
static cutlet::variable_ptr
_eval(cutlet::interpreter &interp, const cutlet::list &parameters) {
  interp.eval(parameters.join());

  return nullptr;
}

// def expr *args
static cutlet::variable_ptr
_expr(cutlet::interpreter &interp, const cutlet::list &parameters) {
  return interp.expr(parameters.join());
}

/* def if condition ¿then? body
 *  ¿elif condition ¿then? body ...?
 *  ¿else body?
 */
static cutlet::variable_ptr
_if(cutlet::interpreter &interp, const cutlet::list &parameters) {
  cutlet::list::const_iterator it = parameters.begin();
  std::string cond, body;

  // Get the initial condition ¿then? body part of the if statement.
  cond = *(*it);
  next(it, parameters);
  if (expect(it, "then")) {
    next(it, parameters);
    body = *(*it);
  } else {
    body = *(*it);
  }

  // If the condition is true then eval the body and return.
  if (cutlet::convert<bool>(interp.expr(cond))) {
    eval_body(interp, body);
    return nullptr;
  }

  ++it;
  while (is_more(it, parameters)) {
    if (expect(it, "elif")) {
      next(it, parameters);
      cond = *(*it);

      next(it, parameters);
      if (expect(it, "then")) {
        next(it, parameters);
        body = *(*it);
      } else {
        body = *(*it);
      }

      if (cutlet::convert<bool>(interp.expr(cond))) {
        eval_body(interp, body);
        return nullptr;
      }

    } else {
      permit(it, "else");
      next(it, parameters);
      eval_body(interp, *(*it));
      return nullptr;
    }
  }

  return nullptr;
}

// def while condition ¿do? body
static cutlet::variable_ptr
_while(cutlet::interpreter &interp, const cutlet::list &parameters) {
  cutlet::list::const_iterator it = parameters.begin();
  std::string cond, body;

  // Get the initial condition ¿then? body part of the if statement.
  cond = *(*it);
  next(it, parameters);
  if (expect(it, "do")) {
    next(it, parameters);
    body = *(*it);
  } else {
    body = *(*it);
  }

  // If the condition is true then eval the body and return.
  while (cutlet::convert<bool>(interp.expr(cond))) {
    cutlet::frame::state_t st = eval_body(interp, body);
    if (st == cutlet::frame::FS_BREAK or
        st == cutlet::frame::FS_DONE) break;
  }

  return nullptr;
}

// def break
static cutlet::variable_ptr
_break(cutlet::interpreter &interp, const cutlet::list &parameters) {
  interp.frame()->state(cutlet::frame::FS_BREAK);
}

// def continue
static cutlet::variable_ptr
_continue(cutlet::interpreter &interp, const cutlet::list &parameters) {
  interp.frame()->state(cutlet::frame::FS_CONTINUE);
}

// def raise *args
static cutlet::variable_ptr
_raise(cutlet::interpreter &interp, const cutlet::list &parameters) {
  // Throw the error.
  throw std::runtime_error(parameters.join());
}

// def try body ¿catch varname err_body?
static cutlet::variable_ptr
_try(cutlet::interpreter &interp, const cutlet::list &parameters) {
  auto it = parameters.begin();

  try {
    // Eval the body.
    eval_body(interp, *(*it));
  } catch (std::exception &err) {
    // An exception was caught, so eval the err_body.
    ++it;
    if (is_more(it, parameters)) {
      interp.frame_push(new cutlet::block_frame(interp.frame()));
      permit(it, "catch");
      next(it, parameters);
      interp.local(*(*it), new cutlet::string(err.what()));
      next(it, parameters);
      try {
        interp.eval(*(*it));
      } catch (...) {
        interp.frame_pop();
        throw;
      }
      interp.frame_pop();
    }
  }

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
  interp->add("false", _false);
  interp->add("true", _true);
  interp->add("eval", _expr);
  interp->add("expr", _eval);
  interp->add("if", _if);
  interp->add("while", _while);
  interp->add("break", _break);
  interp->add("continue", _continue);
  interp->add("raise", _raise);
  interp->add("try", _try);
}
