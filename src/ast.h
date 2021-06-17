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

#include <cutlet>

#ifndef _CUTLET_AST_H
#define _CUTLET_AST_H

namespace cutlet {
  namespace ast {

    /**
     */
    class block : public node {
    public:
      block();
      virtual ~block() noexcept;

      void add(node::pointer n);

      virtual cutlet::variable::pointer
      operator()(cutlet::interpreter &interp);

      virtual unsigned int id() const;

      virtual std::string file() const;

      virtual std::streampos position() const;

      virtual const std::string &body() const;

      virtual const parser::token &token() const;

    private:
      std::list<node::pointer> _nodes;
    };

    /**
     */
    class value : public node {
    public:
      value(const parser::token &token);
      virtual ~value() noexcept;

      virtual cutlet::variable::pointer
      operator()(cutlet::interpreter &interp);

      virtual unsigned int id() const;

      virtual std::string file() const;

      virtual std::streampos position() const;

      virtual const std::string &body() const;

      virtual const parser::token &token() const;

    private:
      parser::token _token;
    };

    /**
     */
    class variable : public node {
    public:
      variable(const parser::token &token);
      virtual ~variable() noexcept;

      virtual cutlet::variable::pointer
      operator()(cutlet::interpreter &interp);

      virtual unsigned int id() const;

      virtual std::string file() const;

      virtual std::streampos position() const;

      virtual const std::string &body() const;

      virtual const parser::token &token() const;

    private:
      parser::token _token;
    };

    /**
     */
    class command : public node {
    public:
      command(node::pointer n);
      virtual ~command() noexcept;

      void parameter(node::pointer n);

      virtual cutlet::variable::pointer
      operator()(cutlet::interpreter &interp);

      virtual unsigned int id() const;

      virtual std::string file() const;

      virtual std::streampos position() const;

      virtual const std::string &body() const;

      virtual const parser::token &token() const;

    private:
      node::pointer _function;
      std::list<node::pointer> _parameters;
    };

    /**
     */
    class string : public node {
    public:
      string(const parser::token &token);
      virtual ~string() noexcept;

      void add(const std::string &value);
      void add(node::pointer n);

      virtual cutlet::variable::pointer
      operator()(cutlet::interpreter &interp);

      virtual unsigned int id() const;

      virtual std::string file() const;

      virtual std::streampos position() const;

      virtual const std::string &body() const;

      virtual const parser::token &token() const;

    private:
      struct _parts_s {
        std::string s;
        node::pointer n;
      };

      parser::token _token;
      std::list<_parts_s> _stringy;
    };

    /**
     */
    class comment : public node {
    public:
      comment(const parser::token &token);
      virtual ~comment() noexcept;

      virtual cutlet::variable::pointer
      operator()(cutlet::interpreter &interp);

      virtual unsigned int id() const;

      virtual std::string file() const;

      virtual std::streampos position() const;

      virtual const std::string &body() const;

      virtual const parser::token &token() const;

    private:
      parser::token _token;
    };
  }
}

#endif /* _CUTLET_AST_H */
