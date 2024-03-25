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

#include <cutlet>

#ifndef _CUTLET_AST_H
#define _CUTLET_AST_H

namespace cutlet {
  namespace ast {

    class block : public node {
    public:
      block();
      virtual ~block() noexcept override;

      void add(node::pointer n);

      virtual cutlet::variable::pointer
      operator()(cutlet::interpreter &interp) override;

      virtual unsigned int id() const override;

      virtual std::string file() const override;

      virtual std::streampos position() const override;

      virtual const std::string &body() const override;

      virtual const parser::token &token() const override;

    private:
      std::list<node::pointer> _nodes;
    };

    class value : public node {
    public:
      value(const parser::token &token);
      virtual ~value() noexcept override;

      virtual cutlet::variable::pointer
      operator()(cutlet::interpreter &interp) override;

      virtual unsigned int id() const override;

      virtual std::string file() const override;

      virtual std::streampos position() const override;

      virtual const std::string &body() const override;

      virtual const parser::token &token() const override;

    private:
      const parser::token _token;
    };

    class variable : public node {
    public:
      variable(const parser::token &token);
      virtual ~variable() noexcept override;

      virtual cutlet::variable::pointer
      operator()(cutlet::interpreter &interp) override;

      virtual unsigned int id() const override;

      virtual std::string file() const override;

      virtual std::streampos position() const override;

      virtual const std::string &body() const override;

      virtual const parser::token &token() const override;

    private:
      const parser::token _token;
    };

    class command : public node {
    public:
      command(node::pointer n);
      virtual ~command() noexcept override;

      void parameter(node::pointer n);

      virtual cutlet::variable::pointer
      operator()(cutlet::interpreter &interp) override;

      virtual unsigned int id() const override;

      virtual std::string file() const override;

      virtual std::streampos position() const override;

      virtual const std::string &body() const override;

      virtual const parser::token &token() const override;

    private:
      node::pointer _function;
      std::list<node::pointer> _parameters;
    };

    class expression : public node {
    public:
      using pointer = std::shared_ptr<expression>;

      expression(node::pointer n);
      virtual ~expression() noexcept override;

      void parameter(node::pointer n);

      virtual cutlet::variable::pointer
      operator()(cutlet::interpreter &interp) override;

      virtual unsigned int id() const override;

      virtual std::string file() const override;

      virtual std::streampos position() const override;

      virtual const std::string &body() const override;

      virtual const parser::token &token() const override;

    private:
      node::pointer _function;
      std::list<node::pointer> _parameters;
    };

    class string : public node {
    public:
      string(const parser::token &token);
      virtual ~string() noexcept override;

      void add(const std::string &value);
      void add(node::pointer n);

      virtual cutlet::variable::pointer
      operator()(cutlet::interpreter &interp) override;

      virtual unsigned int id() const override;

      virtual std::string file() const override;

      virtual std::streampos position() const override;

      virtual const std::string &body() const override;

      virtual const parser::token &token() const override;

    private:
      struct _parts_s {
        std::string s;
        node::pointer n;
      };

      parser::token _token;
      std::list<_parts_s> _stringy;
    };

    class comment : public node {
    public:
      comment(const parser::token &token);
      virtual ~comment() noexcept override;

      virtual cutlet::variable::pointer
      operator()(cutlet::interpreter &interp) override;

      virtual unsigned int id() const override;

      virtual std::string file() const override;

      virtual std::streampos position() const override;

      virtual const std::string &body() const override;

      virtual const parser::token &token() const override;

    private:
      const parser::token _token;
    };
  }
}

#endif /* _CUTLET_AST_H */
