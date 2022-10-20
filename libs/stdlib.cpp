/*                                                                  -*- c++ -*-
 * Copyright © 2018 Ron R Wills <ron@digitalcombine.ca>
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
 *
 * This library includes:
 *
 *   def false
 *   def true
 *   def eval *args
 *   def expr *args
 *   def if ¿then? body
 *     ¿elif condition ¿then? body ...?
 *     ¿else body?
 *   def while condition ¿do? body
 *   def raise *args
 *   def try body ¿catch varname body?
 *   def sleep seconds
 */

#include <cutlet>
#include <unistd.h>
#include <cstdlib>

/******************************************************************************
 *  To make our lives easier we uses recursive decent parser like methods to
 * help us parse the function arguments.
 */

static bool is_more(cutlet::list::const_iterator &it,
                    const cutlet::list &list) {
  return (it != list.end());
}

static void next(cutlet::list::const_iterator &it,
                 const cutlet::list &list) {
  ++it;
  if (it == list.end())
    throw std::runtime_error("Expected more arguments.");
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
                                        cutlet::variable::pointer body,
                                        const std::string &label) {
  cutlet::frame::state_t result;
  cutlet::ast::node::pointer compiled;

  interp.push(1, label);
  try {
    compiled = interp(body);
  } catch(...) {
    result = interp.state();
    interp.pop();
    throw;
  }
  result = interp.state();
  interp.pop();

  return result;
}

static
cutlet::ast::node::pointer loop_body(cutlet::interpreter &interp,
                                     cutlet::variable::pointer body,
                                     const std::string &label,
                                     cutlet::ast::node::pointer compiled) {

  interp.push(new cutlet::loop_frame(label, interp.frame(1)));
  try {
    if (not compiled)
      compiled = interp(body);
    else
      (*compiled)(interp);
  } catch(...) {
    interp.pop();
    throw;
  }
  interp.pop();

  return compiled;
}

static bool expr_condition(cutlet::interpreter &interp,
                           cutlet::variable::pointer cond) {
  interp.push(1, *cond);
  bool result = cutlet::convert<bool>(interp.expr(cond));
  interp.pop();

  return result;
}

/******************************************************************************
 * Cutlet API
 */

/*************
 * def false *
 *************/

static cutlet::variable::pointer
_false(cutlet::interpreter &interp, const cutlet::list &arguments) {
  (void)interp;
  (void)arguments;
  return new cutlet::boolean(false);
}

/************
 * def true *
 ************/

static cutlet::variable::pointer
_true(cutlet::interpreter &interp, const cutlet::list &arguments) {
  (void)interp;
  (void)arguments;
  return new cutlet::boolean(true);
}

/******************
 * def eval *args *
 ******************/

static cutlet::variable::pointer
_eval(cutlet::interpreter &interp, const cutlet::list &arguments) {
  interp.push(1, "eval");
  interp(arguments.join());
  interp.pop();

  return nullptr;
}

/******************
 * def expr *args *
 ******************/

static cutlet::variable::pointer
_expr(cutlet::interpreter &interp, const cutlet::list &arguments) {
  interp.push(1, arguments.join());
  return interp.pop(interp.expr(arguments.join()));
}

/*************************************
 * def if condition ¿then? body      *
 *  ¿elif condition ¿then? body ...? *
 *  ¿else body?                      *
 *************************************/

static cutlet::variable::pointer
_if(cutlet::interpreter &interp, const cutlet::list &arguments) {
  cutlet::list::const_iterator it = arguments.begin();
  cutlet::variable::pointer cond;
  cutlet::variable::pointer body;

  // Get the initial condition ¿then? body part of the if statement.
  cond = *it;
  next(it, arguments);
  if (expect(it, "then")) {
    next(it, arguments);
    body = *it;
  } else {
    body = *it;
  }

  // If the condition is true then eval the body and return.
  if (expr_condition(interp, cond)) {
    eval_body(interp, body, "then");
    return nullptr;
  }

  ++it;
  while (is_more(it, arguments)) {
    if (expect(it, "elif")) {
      next(it, arguments);
      cond = *it;

      next(it, arguments);
      if (expect(it, "then")) {
        next(it, arguments);
        body = *it;
      } else {
        body = *it;
      }

      if (expr_condition(interp, cond)) {
        eval_body(interp, body, "elif");
        return nullptr;
      }

    } else {
      permit(it, "else");
      next(it, arguments);
      eval_body(interp, *it, "else");
      return nullptr;
    }
  }

  return nullptr;
}

/*********************************
 * def while condition ¿do? body *
 *********************************/

static cutlet::variable::pointer
_while(cutlet::interpreter &interp, const cutlet::list &arguments) {
  cutlet::list::const_iterator it = arguments.begin();
  cutlet::variable::pointer cond, body;

  // Get the initial condition ¿then? body part of the if statement.
  cond = *it;
  next(it, arguments);
  if (expect(it, "do")) {
    next(it, arguments);
    body = *it;
  } else {
    body = *it;
  }

  // If the condition is true then eval the body and return.
  cutlet::ast::node::pointer compiled;

  while (expr_condition(interp, cond)) {
    compiled = loop_body(interp, body, "while", compiled);

    switch (interp.state()) {
    case cutlet::frame::FS_BREAK:
    case cutlet::frame::FS_DONE:
      return nullptr;
    case cutlet::frame::FS_CONTINUE:
      interp.state(cutlet::frame::FS_RUNNING);
      break;
    default:
      break;
    }
  }

  return nullptr;
}

/*************
 * def break *
 *************/

static cutlet::variable::pointer
_break(cutlet::interpreter &interp, const cutlet::list &arguments) {
  (void)arguments;

  interp.frame(1)->state(cutlet::frame::FS_BREAK);
  return nullptr;
}

/****************
 * def continue *
 ****************/

static cutlet::variable::pointer
_continue(cutlet::interpreter &interp, const cutlet::list &arguments) {
  (void)arguments;

  interp.frame(1)->state(cutlet::frame::FS_CONTINUE);
  return nullptr;
}

/*******************
 * def raise *args *
 *******************/

static cutlet::variable::pointer
_raise(cutlet::interpreter &interp, const cutlet::list &arguments) {
  // We remove our frame from the stack.
  interp.pop();

  // Throw the error.
  throw std::runtime_error(arguments.join());
}

/*****************************************
 * def try body ¿catch varname err_body? *
 *****************************************/

static cutlet::variable::pointer
_try(cutlet::interpreter &interp, const cutlet::list &arguments) {
  auto it = arguments.begin();

  interp.push(1, "try body");
  try {
    // Eval the body.
    interp(*it);

  } catch (std::exception &err) {
    // An exception was caught, so eval the err_body.
    ++it;
    if (is_more(it, arguments)) {
      permit(it, "catch");

      // Set the local variable with the error in it.
      next(it, arguments);
      interp.push(1, "catch body");
      interp.local(*(*it), new cutlet::string(err.what()));

      // Eval the catch block
      next(it, arguments);
      interp(*it);

      interp.pop();
    }
  }
  interp.pop(); // Remove try body frame.

  return nullptr;
}

/*********************
 * def sleep seconds *
 *********************/

static cutlet::variable::pointer
_sleep(cutlet::interpreter &interp, const cutlet::list &arguments) {
  (void)interp;

  cutlet::variable::pointer secs = arguments[0];
  sleep(atoi(((std::string)*secs).c_str()));

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
  interp->add("eval", _eval);
  interp->add("expr", _expr);
  interp->add("if", _if);
  interp->add("while", _while);
  interp->add("break", _break);
  interp->add("continue", _continue);
  interp->add("raise", _raise);
  interp->add("try", _try);
  interp->add("sleep", _sleep);
}
