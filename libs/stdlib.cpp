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
#include <iostream>
#include <libcutlet/utilities>

namespace {

  /****************************************************************************
   * Helper utitilies
   */

  /*************
   * eval_body *
   *************/

  cutlet::frame::state_t eval_body(cutlet::interpreter &interp,
                                   cutlet::variable::pointer body,
                                   const std::string &label) {
    interp.push(1, label);

    interp(body);
    auto result = interp.state();
    interp.pop();

    return result;
  }

  /*************
   * loop_body *
   *************/

  static
  cutlet::ast::node::pointer loop_body(cutlet::interpreter &interp,
                                       cutlet::variable::pointer body,
                                       const std::string &label,
                                       cutlet::ast::node::pointer compiled) {
    interp.push(std::make_shared<cutlet::loop_frame>(label, interp.frame(1)));

    if (not compiled)
      compiled = interp(body);
    else
      (*compiled)(interp);
    interp.pop();

    return compiled;
  }

  /******************
   * expr_condition *
   ******************/

  bool expr_condition(cutlet::interpreter &interp,
                      cutlet::variable::pointer cond) {
    interp.push(1, *cond);
    bool result = cutlet::primative<bool>(interp.expr(cond));
    interp.pop();

    return result;
  }

  /****************************************************************************
   * Cutlet API
   */

  /*************
   * def false *
   *************/

  cutlet::variable::pointer
  _false(cutlet::interpreter &interp, const cutlet::list &arguments) {
    (void)interp;
    (void)arguments;
    return cutlet::var<cutlet::boolean>(false);
  }

  /************
   * def true *
   ************/

  cutlet::variable::pointer
  _true(cutlet::interpreter &interp, const cutlet::list &arguments) {
    (void)interp;
    (void)arguments;
    return cutlet::var<cutlet::boolean>(true);
  }

  /******************
   * def eval *args *
   ******************/

  cutlet::variable::pointer
  _eval(cutlet::interpreter &interp, const cutlet::list &arguments) {
    interp.push(1, "eval");
    interp(arguments.join());
    interp.pop();

    return nullptr;
  }

  /******************
   * def expr *args *
   ******************/

  cutlet::variable::pointer
  _expr(cutlet::interpreter &interp, const cutlet::list &arguments) {
    interp.push(1, arguments.join());
    auto result = interp.expr(arguments.join());
    interp.pop();
    return result;
  }

  /*************************************
   * def if condition ¿then? body      *
   *  ¿elif condition ¿then? body ...? *
   *  ¿else body?                      *
   *************************************/

  cutlet::variable::pointer
  _if(cutlet::interpreter &interp, const cutlet::list &arguments) {
    cutlet::variable::pointer cond;
    cutlet::variable::pointer body;

    cutlet::arg_tokens args(arguments);

    // Get the initial condition ¿then? body part of the if statement.
    cond = args.get();
    args.next();
    if (args.expect("then")) {
      body = args.next();
    } else {
      body = args.get();
    }

    // If the condition is true then eval the body and return.
    if (expr_condition(interp, cond)) {
      eval_body(interp, body, "then");
      return nullptr;
    }

    ++args;
    while (args.is_more()) {
      if (args.expect("elif")) {
        cond = args.next();

        args.next();
        if (args.expect("then")) {
          body = args.next();
        } else {
          body = args.get();
        }

        if (expr_condition(interp, cond)) {
          eval_body(interp, body, "elif");
          return nullptr;
        }

      } else {
        args.permit("else");
        body = args.next();
        eval_body(interp, body, "else");
        return nullptr;
      }
    }

    return nullptr;
  }

  /*********************************
   * def while condition ¿do? body *
   *********************************/

  cutlet::variable::pointer
  _while(cutlet::interpreter &interp, const cutlet::list &arguments) {
    cutlet::arg_tokens args(arguments, "while condition ¿do? body");
    cutlet::variable::pointer cond, body;

    // Get the initial condition ¿then? body part of the if statement.
    cond = args.get();
    args.next();
    if (args.expect("do")) {
      body = args.next();
    } else {
      body = args.get();
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
      case cutlet::frame::FS_RUNNING:
        break;
      }
    }

    return nullptr;
  }

  /*************
   * def break *
   *************/

  cutlet::variable::pointer
  _break(cutlet::interpreter &interp, const cutlet::list &arguments) {
    (void)arguments;

    interp.frame(1)->state(cutlet::frame::FS_BREAK);
    return nullptr;
  }

  /****************
   * def continue *
   ****************/

  cutlet::variable::pointer
  _continue(cutlet::interpreter &interp, const cutlet::list &arguments) {
    (void)arguments;

    interp.frame(1)->state(cutlet::frame::FS_CONTINUE);
    return nullptr;
  }

  /*******************
   * def raise *args *
   *******************/

  cutlet::variable::pointer
  _raise(cutlet::interpreter &interp, const cutlet::list &arguments) {
    // We remove our frame (for _raise) from the stack.
    interp.pop();

    // Throw the error.
    throw std::runtime_error(arguments.join());
  }

  /*****************************************
   * def try body ¿catch varname err_body? *
   *****************************************/

  cutlet::variable::pointer
  _try(cutlet::interpreter &interp, const cutlet::list &arguments) {
    cutlet::arg_tokens args(arguments, "try body ¿catch varname err_body?");

    auto cur_frame = interp.frame();

    interp.push(1, "try body");
    try {
      // Eval the body.
      interp(args.get());

    } catch (std::exception &err) {
      // An exception was caught, so eval the err_body.
      ++args;
      if (args.is_more()) {
        args.permit("catch");

        // Set the local variable with the error in it.
        auto local = args.next();

        // Clean up the stack
        interp.pop(cur_frame);

        interp.push(1, "catch body");
        interp.local(*local, cutlet::var<cutlet::string>(err.what()));

        // Eval the catch block
        interp(args.next());

        interp.pop();
      }
    }

    // Clean up the stack
    interp.pop(cur_frame);

    return nullptr;
  }

  /*********************
   * def sleep seconds *
   *********************/

  cutlet::variable::pointer
  _sleep(cutlet::interpreter &interp, const cutlet::list &arguments) {
    (void)interp;

    cutlet::variable::pointer secs = arguments[0];
    sleep(atoi(cutlet::cast<std::string>(secs).c_str()));

    return nullptr;
  }
} // namespace

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
