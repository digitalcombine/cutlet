/*                                                                  -*- c++ -*-
 * Copyright © 2021 Ron R Wills <ron@digitalcombine.ca>
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
 * thread body
 * $thread join
 * mutex
 * $mutex ¿try? body
 */

#include <cutlet>
#include <thread>
#include <mutex>
#include <iostream>

namespace {
  using thread_args_t = struct {
    cutlet::interpreter *interp;
    cutlet::variable::pointer body;
  };

  /*****************
   * _thread_entry *
   *****************/

  void _thread_entry(thread_args_t *args) {
    /* XXX Need to catch exceptions here and add them to the _thread_var class.
     * The exception can then be rethrown by the join operator.
     */

    try {
      /* Create a new Cutlet interpreter to execute the the new thread
       * in. This gives the thread its own Cutlet frame stack.
       */
      cutlet::interpreter tinterp;

      // Copy the global environment into the new interpreter.
      tinterp.push(args->interp->environment());

      // Execute the body.
      tinterp(args->body);
    } catch (std::exception &err) {
    }
  }

  void (*_entry_ptr)(thread_args_t *) = nullptr;

  /******************************************************************************
   * class prototypes
   */

  /*********************
   * class _thread_var *
   *********************/

  class _thread_var : public cutlet::variable {
  public:
    _thread_var(cutlet::interpreter &interp,
                cutlet::variable::pointer body);
    virtual ~_thread_var() noexcept;

    virtual cutlet::variable::pointer
    operator ()(cutlet::variable::pointer self,
                cutlet::interpreter &interp,
                const cutlet::list &arguments);

  private:
    thread_args_t _args;
    std::thread _thread;
  };

  /********************
   * class _mutex_var *
   ********************/

  class _mutex_var : public cutlet::variable {
  public:
    _mutex_var();
    virtual ~_mutex_var() noexcept;

    virtual cutlet::variable::pointer
    operator ()(cutlet::variable::pointer self,
                cutlet::interpreter &interp,
                const cutlet::list &arguments);

  private:
    std::recursive_mutex _mutex;
    cutlet::ast::node::pointer _compiled;
  };
}

/******************************************************************************
 * class _thread_var
 */

/****************************
 * _thread_var::_thread_var *
 ****************************/

_thread_var::_thread_var(cutlet::interpreter &interp,
                           cutlet::variable::pointer body)
  : _args({&interp, body}), _thread(_thread_entry, &_args) {
}

/*****************************
 * _thread_var::~_thread_var *
 *****************************/

_thread_var::~_thread_var() noexcept {
  if (_thread.joinable()) {
    _thread.join();
    std::cerr << "WARNING: Thread joined during garbage collection. "
              << "This is incredibly unreliable, use the join operator "
              << "instead!" << std::endl;
  }
}

/****************************
 * _thread_var::operator () *
 ****************************/

cutlet::variable::pointer
_thread_var::operator ()(cutlet::variable::pointer self,
                         cutlet::interpreter &interp,
                         const cutlet::list &arguments) {
  (void)self;
  (void)interp;

  std::string op = *(arguments[0]);

  if (op == "join") {
    if (arguments.size() == 1) {
      if (_thread.joinable()) _thread.join();
      return nullptr;
    } else {
      throw std::runtime_error(std::string("Invalid number of arguments to "
                                           "thread operator join"));
    }
  }

  throw std::runtime_error(std::string("Unknown operator ") +
                           op + " for thread variable.");
}

/******************************************************************************
 * class _mutex_var
 */

/**************************
 * _mutex_var::_mutex_var *
 **************************/

_mutex_var::_mutex_var() : _mutex() {}

/*****************************
 * _mutex_var::~_mutex_var *
 *****************************/

_mutex_var::~_mutex_var() noexcept {}

/****************************
 * _mutex_var::operator () *
 ****************************/

cutlet::variable::pointer
_mutex_var::operator ()(cutlet::variable::pointer self,
                         cutlet::interpreter &interp,
                         const cutlet::list &arguments) {
  (void)self;

  size_t args = arguments.size();

  if (args == 1) {
    _mutex.lock();
    interp(arguments[0]);
    _mutex.unlock();
  } else if (args == 2 and *(arguments[0]) == "try") {
    _mutex.try_lock();
    interp(arguments[1]);
    _mutex.unlock();
  }

  throw std::runtime_error("Unknown operator for mutex variable.");
}

/******************************************************************************
 * Public API
 */

#include <type_traits>

namespace {

  /*******************
   * def thread body *
   *******************/

  cutlet::variable::pointer
  _thread(cutlet::interpreter &interp, const cutlet::list &arguments) {
    size_t argc = arguments.size();
    if (argc == 1) {
      return std::make_shared<_thread_var>(interp, arguments[0]);
    }
    return nullptr;
  }

  /*************
   * def mutex *
   *************/

  cutlet::variable::pointer
  _mutex(cutlet::interpreter &interp, const cutlet::list &arguments) {
    (void)interp;
    (void)arguments;

    return std::make_shared<_mutex_var>();
  }
}

/******************************************************************************
 * We need to declare init_cutlet as a C function.
 */
extern "C" {
  DECLSPEC void init_cutlet(cutlet::interpreter *interp);
}

/***************
 * init_cutlet *
 ***************/

void init_cutlet(cutlet::interpreter *interp) {
  // Add the API to the interpreter.
  interp->add("thread", _thread);
  interp->add("mutex", _mutex);
}
