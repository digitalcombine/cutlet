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
 * $thread joinable
 * $thread join
 * $thread detach
 * mutex
 * $mutex body
 * $mutex try ¿timeout? body
 * $mutex shared body
 * $mutex try shared ¿timeout? body
 */

#include <cutlet>
#include <chrono>
#include <thread>
#include <shared_mutex>

//#define DEBUG_THREADING 1

#if DEBUG_THREADING
#pragma message ("Threading library debugging enabled")
#include <iostream>
#endif

namespace {
  std::chrono::milliseconds interval(100);

  using thread_args_t = struct {
    cutlet::sandbox::pointer  env;
    cutlet::variable::pointer body;
    std::exception_ptr        err;
  };

  /*****************
   * _thread_entry *
   *****************/

  void _thread_entry(thread_args_t &args) {
    try {
      /* Create a new interpreter to execute the the new thread in. This gives
       * the thread its own frame stack.
       */
      cutlet::interpreter tinterp;

      // Pass the global environment into the new interpreter.
      tinterp.push(args.env);

      // Execute the body.
      tinterp(args.body);

    } catch (...) {
      // Capture any errors to be rethrown during join.
      args.err = std::current_exception();
    }
  }

  /****************************************************************************
   * class prototypes
   */

  /*********************
   * class _thread_var *
   *********************/

  class _thread_var : public cutlet::variable {
  public:
    _thread_var(cutlet::interpreter &interp,
                cutlet::variable::pointer body);
    virtual ~_thread_var() noexcept override;

    virtual cutlet::variable::pointer
    operator ()(cutlet::variable::pointer self,
                cutlet::interpreter &interp,
                const cutlet::list &arguments) override;

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
    virtual ~_mutex_var() noexcept override;

    virtual cutlet::variable::pointer
    operator ()(cutlet::variable::pointer self,
                cutlet::interpreter &interp,
                const cutlet::list &arguments) override;

  private:
    std::shared_timed_mutex _mutex;
    cutlet::ast::node::pointer _compiled;
  };
} // namespace

/******************************************************************************
 * class _thread_var
 */

/****************************
 * _thread_var::_thread_var *
 ****************************/

_thread_var::_thread_var(cutlet::interpreter &interp,
                           cutlet::variable::pointer body)
  : _args{interp.environment(), body, nullptr},
    _thread(_thread_entry, std::ref(_args)) {}

/*****************************
 * _thread_var::~_thread_var *
 *****************************/

_thread_var::~_thread_var() noexcept {
  if (_thread.joinable()) _thread.join();
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

  if (arguments.size() == 1) {
    if (op == "join") {
      // Clean up and join the thread.
      _thread.join();

      // If any exceptions where throw by the thread rethrow them here.
      if (_args.err) std::rethrow_exception(_args.err);
      return nullptr;

    } else if (op == "joinable") {
      return cutlet::var<cutlet::boolean>(_thread.joinable());

    } else if (op == "detach") {
      // The thread is now on its own.
      _thread.detach();
      return nullptr;
    }
  } else {
    throw std::runtime_error("Invalid number of arguments to "
                             "thread operator " + op);
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
  std::string op = *(arguments[0]);

  if (args == 1) {
    // $mutex body
    _mutex.lock();
    interp(arguments[0]);
    _mutex.unlock();
    return nullptr;

  } else if (args == 2) {

    if (op == "try") {
      // $mutex try body
      if (_mutex.try_lock()) {
        interp(arguments[1]);
        _mutex.unlock();
        return cutlet::var<cutlet::boolean>(true);
      }
      return cutlet::var<cutlet::boolean>(false);

    } else if (op == "shared") {
      // $mutex shared body
      _mutex.lock_shared();
      interp(arguments[1]);
      _mutex.unlock_shared();
      return nullptr;
    }

  } else if (args == 3) {
    auto to = std::chrono::milliseconds(std::stol(*(arguments[1])));

    if (op == "try" and *(arguments[1]) == "shared") {
      // $mutex try shared body
      if (_mutex.try_lock_shared()) {
        interp(arguments[2]);
        _mutex.unlock_shared();
        return cutlet::var<cutlet::boolean>(true);
      }
      return cutlet::var<cutlet::boolean>(false);

    } else if (op == "try") {
      // $mutex try timeout body
      if (_mutex.try_lock_for(to)) {
        interp(arguments[2]);
        _mutex.unlock();
        return cutlet::var<cutlet::boolean>(true);
      }
      return cutlet::var<cutlet::boolean>(false);
    }

  } else if (args == 4) {
    auto to = std::chrono::milliseconds(std::stol(*(arguments[2])));

    if (op == "try" and *(arguments[1]) == "shared") {
      // $mutex try shared timeout body
      if (_mutex.try_lock_shared_for(to)) {
        interp(arguments[3]);
        _mutex.unlock_shared();
        return cutlet::var<cutlet::boolean>(true);
      }
      return cutlet::var<cutlet::boolean>(false);
    }
  }

  throw std::runtime_error(std::string("Unknown operator ") +
                           op + " for mutex variable.");
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
    auto argc = arguments.size();
    if (argc == 1) {
      return cutlet::var<_thread_var>(interp, arguments[0]);
    }

    throw std::runtime_error(std::string("Invalid arguments for thread (1 <= ")
                             + std::to_string(argc) + " <= 1)\n thread body");
  }

  /*************
   * def mutex *
   *************/

  cutlet::variable::pointer
  _mutex(cutlet::interpreter &interp, const cutlet::list &arguments) {
    (void)interp;
    auto argc = arguments.size();

    if (argc == 0) {
      return cutlet::var<_mutex_var>();
    }

    throw std::runtime_error(std::string("Invalid arguments for mutex (0 <= ")
                             + std::to_string(argc) + " <= 0)\n mutex");
  }
} // namespace

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
