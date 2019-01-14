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

#include <cutlet.h>

#ifndef _CUTLET_AST_H
#define _CUTLET_AST_H

namespace cutlet {
  namespace ast {

    class block : public node {
    public:
      block();
      virtual ~block() noexcept;

      void add(node_ptr n);

      virtual cutlet::variable_ptr operator()(cutlet::interpreter &interp);

    private:
      std::list<node_ptr> _nodes;
    };

    class value : public node {
    public:
      value(const parser::token &token);
      virtual ~value() noexcept;

      virtual cutlet::variable_ptr operator()(cutlet::interpreter &interp);

    protected:
      virtual std::string location() const;

    private:
      parser::token _token;
    };

    class variable : public node {
    public:
      variable(const parser::token &token);
      virtual ~variable() noexcept;

      virtual cutlet::variable_ptr operator()(cutlet::interpreter &interp);

    protected:
      virtual std::string location() const;

    private:
      parser::token _token;
    };

    class command : public node {
    public:
      command(node_ptr n);
      virtual ~command() noexcept;

      void parameter(node_ptr n);

      virtual cutlet::variable_ptr operator()(cutlet::interpreter &interp);

    protected:
      virtual std::string location() const;

    private:
      node_ptr _function;
      std::list<node_ptr> _parameters;
    };

    class string : public node {
    public:
      string(const parser::token &token);
      virtual ~string() noexcept;

      void add(const std::string &value);
      void add(node_ptr n);

      virtual cutlet::variable_ptr operator()(cutlet::interpreter &interp);

    protected:
      virtual std::string location() const;

    private:
      struct _parts_s {
        std::string s;
        node_ptr n;
      };

      parser::token _token;
      std::list<_parts_s> _stringy;
    };
  }
}

#endif /* _CUTLET_AST_H */
