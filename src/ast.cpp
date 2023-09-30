/*                                                                 -*- c++ -*-
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
 */

#include "ast.h"
#include <sstream>

//#define DEBUG_AST 1

#if defined(DEBUG_AST)
#pragma message ("AST debugging enabled")
#include <iostream>
#endif

namespace {
  template <typename Ty>
  constexpr bool is_type(cutlet::ast::node::pointer &ast_object) {
    return ((dynamic_cast<Ty *>(&(*ast_object))) != nullptr);
  }

  /* This class extends cutlet::string to add a pointer to the node
   */
  class value : public cutlet::string {
  public:
    value(cutlet::ast::node &node)
      : cutlet::string(node.body()), _token(node.token()) { }
    virtual ~value() noexcept override;

  protected:
    virtual const parser::token *token() const override {
      return &_token;
    }

  private:
    parser::token _token;
  };
} // namespace

value::~value() noexcept {}

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

cutlet::ast::node::node() {}

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

void cutlet::ast::block::add(cutlet::ast::node::pointer node) {
#if defined(DEBUG_AST)
  if (not node) {
    std::clog << "ast::block attempting to add null ast node to block"
              << std::endl;
  }
#endif
  if (node) {
    _nodes.push_back(node);
  }
}

/***********************************
 * cutlet::ast::block::operator () *
 ***********************************/

cutlet::variable::pointer
cutlet::ast::block::operator()(cutlet::interpreter &interp) {
  bool empty = _nodes.empty();

  if (!empty) {
    cutlet::frame &frame = *interp.frame();

    for (auto &node: _nodes) {
#if defined(DEBUG_AST)
      if (not node) {
        std::clog << "ast::block attempting to execute null ast node"
                  << std::endl;
        continue;
      }
#endif

      if (node) (*node)(interp);
      if (frame.state() != frame::FS_RUNNING) break;
    }
  }

  return nullptr;
}

/**************************
 * cutlet::ast::block::id *
 **************************/

unsigned int cutlet::ast::block::id() const {
  return cutlet::A_BLOCK;
}

/****************************
 * cutlet::ast::block::file *
 ****************************/

std::string cutlet::ast::block::file() const {
  if (not _nodes.empty()) return _nodes.front()->file();
  return 0;
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
  return static_cast<const std::string &>(token());
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

cutlet::ast::value::value(const parser::token &token)
  : node(), _token(token) {}

/******************************
 * cutlet::ast::value::~value *
 ******************************/

cutlet::ast::value::~value() noexcept {}

/***********************************
 * cutlet::ast::value::operator () *
 ***********************************/

cutlet::variable::pointer
cutlet::ast::value::operator()(cutlet::interpreter &interp) {
  break_point(interp);

#if defined(DEBUG_AST)
  std::clog << "AST:" << position() << "<" << file() << ">"
            << ": word " << (std::string)_token
            << std::endl;
#endif

  return cutlet::var<::value>(*this);
}

/**************************
 * cutlet::ast::value::id *
 **************************/

unsigned int cutlet::ast::value::id() const {
  return cutlet::A_VALUE;
}

/****************************
 * cutlet::ast::value::file *
 ****************************/

std::string cutlet::ast::value::file() const {
  return _token.file();
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
  return static_cast<const std::string &>(token());
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
#if defined(DEBUG_AST)
    std::clog << "AST:" << position() << "<" << file() << ">"
              << ": resolving variable $"
              << (std::string)_token
              << " = " << (std::string)*interp.var((const std::string &)_token)
              << std::endl;
#endif
    return interp.var(static_cast<const std::string &>(_token));
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

/*******************************
 * cutlet::ast::variable::file *
 *******************************/

std::string cutlet::ast::variable::file() const {
  return _token.file();
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
  return static_cast<const std::string &>(token());
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
  for (auto &parameter: _parameters) {
    c_params.push_back((*parameter)(interp));
  }

  break_point(interp);

  try {
    if (is_type<ast::variable>(_function)) {
      // Execute the variable.
#if defined(DEBUG_AST)
      std::clog << "AST: operator $" << body() << " -> "
                << (std::string)*c_params[0] << std::endl;
#endif
      if (c_params.size())
        return (*cmd)(cmd, interp, c_params);
      else
        return cmd;

    } else if (is_type<ast::command>(_function)) {
#if defined(DEBUG_AST)
      std::clog << "AST:" << position() << "<" << file()
                << ">: command [" << (std::string)*cmd << "]"
                << std::endl;
#endif
      if (c_params.size())
        return (*cmd)(cmd, interp, c_params);
      else
        return cmd;

    } else if (is_type<ast::string>(_function)) {
#if defined(DEBUG_AST)
      std::clog << "AST: string " << (std::string)*cmd << std::endl;
#endif
      if (c_params.size())
        return (*cmd)(cmd, interp, c_params);
      else
        return cmd;

    } else {
      // Execute the function.
#if defined(DEBUG_AST)
      std::clog << "AST:" << position() << "<" << file()
                << ">: command " << (std::string)*cmd
                << std::endl;
#endif
      return interp.call(cutlet::cast<std::string>(cmd), c_params);

    }
  } catch (const cutlet::exception &err) {
    (void)err;
    throw;
  } catch (const std::exception &err) {
    /** @todo Need to add location function to nodes so we can let the users
     *        know where the problem was.
     */
    std::stringstream msg;
    msg << file() << ":" << position() << ": " << err.what();
    throw cutlet::exception(msg.str(), *this);
  }
}

/****************************
 * cutlet::ast::command::id *
 ****************************/

unsigned int cutlet::ast::command::id() const {
  return cutlet::A_COMMAND;
}

/******************************
 * cutlet::ast::command::file *
 ******************************/

std::string cutlet::ast::command::file() const {
  return _function->file();
}

/**********************************
 * cutlet::ast::command::position *
 **********************************/

std::streampos cutlet::ast::command::position() const {
  return _function->position();
}

/******************************
 * cutlet::ast::command::body *
 ******************************/

const std::string &cutlet::ast::command::body() const {
  return static_cast<const std::string &>(token());
}

/*******************************
 * cutlet::ast::command::token *
 *******************************/

const parser::token &cutlet::ast::command::token() const {
  return _function->token();
}

/******************************************************************************
 * class cutlet::ast::string
 */

/*******************************
 * cutlet::ast::string::string *
 *******************************/

cutlet::ast::string::string(const parser::token &token)
  : node(), _token(token) {}

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
    if (not part.n) {
      // Literal string part.
      result += part.s;
    } else {
      // Variable or command substitution.
      auto v = (*(part.n))(interp);
      if (v)
        result += static_cast<std::string>(*v);
    }
  }

#if defined(DEBUG_AST)
  std::clog << "AST:" << position() << "<" << file() << ">"
            << ": \"" << result << "\"" << std::endl;
#endif

  return cutlet::var<cutlet::string>(result);
}

/***************************
 * cutlet::ast::string::id *
 ***************************/

unsigned int cutlet::ast::string::id() const {
  return cutlet::A_STRING;
}

/*****************************
 * cutlet::ast::string::file *
 *****************************/

std::string cutlet::ast::string::file() const {
  return _token.file();
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
  return static_cast<const std::string &>(token());
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

/*************************************
 * cutlet::ast::comment::operator () *
 *************************************/

cutlet::variable::pointer
cutlet::ast::comment::operator()(cutlet::interpreter &interp) {
  break_point(interp);

#if defined(DEBUG_AST)
  std::clog << "AST:" << position() << "<" << file() << ">"
            << ": # " << (std::string)_token
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

/******************************
 * cutlet::ast::comment::file *
 ******************************/

std::string cutlet::ast::comment::file() const {
  return _token.file();
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
  return static_cast<const std::string &>(token());
}

/*******************************
 * cutlet::ast::comment::token *
 *******************************/

const parser::token &cutlet::ast::comment::token() const {
  return _token;
}
