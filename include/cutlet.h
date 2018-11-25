/*                                                                 -*- c++ -*-
 * Copyright © 2018 Ron R Wills <ron@digitalcombine.ca>
 *
 * This file is part of Cutlet.
 *
 * Meat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Meat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Meat.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cutlet/memory.h>
#include <cutlet/parser.h>

#if defined (_WIN32) || defined (_WIN64)
#  ifdef DLL_EXPORT
#    define DECLSPEC __declspec(dllexport)
#  else
#    define DECLSPEC __declspec(dllimport)
#  endif
#elif defined(__linux__) || defined(__FreeBSD__)
#   define DECLSPEC __attribute__((visibility("default")))
#else
#   error("Don't know how to export shared object libraries")
#endif

#ifndef _CUTLET_H
#define _CUTLET_H

namespace cutlet {

  namespace utf8 {

    /** String iterator for UTF-8 encoded strings.
     */
    class iterator : public std::iterator<std::bidirectional_iterator_tag,
                                          std::string> {
    public:
      iterator(const std::string &value);
      iterator(const std::string &value, size_t offset);
      iterator(const iterator &other);

      iterator &operator =(const iterator &other);

      iterator &operator ++();
      iterator operator ++(int);
      iterator &operator --();
      iterator operator --(int);

      bool operator ==(const iterator &other) const;
      bool operator !=(const iterator &other) const;

      const std::string &operator *() const;

      iterator end() const;
      size_t position() const;
      size_t length() const;

      friend std::string substr(const iterator &start,
                                const iterator &end);

      friend std::string replace(const iterator &start,
                                 const iterator &end,
                                 const std::string &value);
    private:
      const std::string *_value;

      size_t _index, _length;
      std::string _current;
    };

    std::string substr(const iterator &start, const iterator &end);

    std::string replace(const iterator &start, const iterator &end,
                        const std::string &value);
  }

  /***************************************************************************
   */

  class list;
  class variable;
  typedef memory::reference<variable> variable_ptr;
  class interpreter;

  /** Variable type.
   */
  class variable {
  public:
    virtual ~variable() noexcept;

    virtual variable_ptr execute(interpreter &interp, const list &parameters);

    virtual operator std::string() const;
  };

  template <class Ty>
  Ty &cast(variable_ptr object) { return dynamic_cast<Ty &>(*(object)); }

  template <class Ty>
  Ty convert(variable_ptr object) { return (Ty)(*(object)); }

  template <> int convert<int>(variable_ptr object);

  /***************************************************************************
   */

  class string : public variable, public std::string {
  public:
    string();
    string(const std::string &value);
    string(int value);
    virtual ~string() noexcept;

    virtual operator std::string() const;
  };

  /***************************************************************************
   */

  class list : public variable, public std::deque<variable_ptr> {
  public:
    list();
    list(const_iterator first, const_iterator last);
    list(const std::initializer_list<variable_ptr> &items);
    list(const list &other);

    virtual variable_ptr execute(interpreter &interp, const list &parameters);

    virtual operator std::string() const;

  private:
    variable_ptr _join(interpreter &interp, const list &parameters);
    variable_ptr _append(interpreter &interp, const list &parameters);
    variable_ptr _prepend(interpreter &interp, const list &parameters);
    variable_ptr _extend(interpreter &interp, const list &parameters);
    variable_ptr _index(interpreter &interp, const list &parameters);
    variable_ptr _delete(interpreter &interp, const list &parameters);
    variable_ptr _foreach(interpreter &interp, const list &parameters);
  };

  /***************************************************************************
   */

  class component {
  public:
    virtual ~component() noexcept;

    virtual variable_ptr
    operator ()(interpreter &interp, const list &parameters) = 0;
  };

  typedef memory::reference<component> component_ptr;

  /***************************************************************************
   */

  typedef variable_ptr (*function_t)(interpreter &,
                                     const list &);

  extern "C" {
    typedef void (*libinit_t)(interpreter *);
  }

  class sandbox {
  public:
    sandbox();
    sandbox(const sandbox &other);
    virtual ~sandbox() noexcept;

    void add(const std::string &name, function_t func);
    void add(const std::string &name, component_ptr comp);
    void remove(const std::string &name);
    void clear();
    component_ptr get(const std::string &name) const;

    variable_ptr variable(interpreter &interp, const std::string &name);
    void variable(const std::string &name, variable_ptr value);
    bool has_variable(const std::string &name);

    variable_ptr execute(interpreter &interp,
                         const std::string &procedure,
                         const list &parameters);

    friend class interpreter;

  private:
    std::map<std::string, variable_ptr> _variables;
    std::map<std::string, component_ptr> _components;
    std::list<void *> _native_libs;

    void load(interpreter &interp, const std::string &library_name);
  };

  typedef memory::reference<sandbox> sandbox_ptr;

  /***************************************************************************
   */

  class frame {
  public:
    frame();
    frame(memory::reference<frame> uplevel);
    frame(const frame &other) = delete;
    virtual ~frame() noexcept;

    virtual variable_ptr variable(const std::string &name) const;
    virtual void variable(const std::string &name, variable_ptr value);

    void done(variable_ptr result);
    bool done() const;

    friend class component;
    friend class interpreter;

  private:
    memory::reference<frame> _uplevel;

    // Used to restore the global environment if it was changed.
    memory::reference<sandbox> _sandbox_orig;
    std::map<std::string, variable_ptr> _variables;

    bool _done;
    variable_ptr _return;
  };

  typedef memory::reference<frame> frame_ptr;

  /***************************************************************************
   */

  class interpreter : public parser::grammer {
  public:
    interpreter();
    interpreter(const interpreter &other) = delete;
    virtual ~interpreter() noexcept;

    variable_ptr var(const std::string &name);
    void global(const std::string &name, variable_ptr value);
    void local(const std::string &name, variable_ptr value);

    /** Uses the parser to create a list variable.
     */
    variable_ptr list(const std::string code);

    void add(const std::string &name, function_t func);
    void add(const std::string &name, component_ptr comp);
    void remove(const std::string &name);
    void clear();
    /**
     * @todo Rename to comp.
     */
    component_ptr get(const std::string &name) const;

    void frame_push();
    void frame_push(frame_ptr frm);
    void frame_push(sandbox_ptr sb);
    void frame_push(frame_ptr frm, sandbox_ptr sb);
    variable_ptr frame_pop();
    /** End the execution of the current frame with an optional return value.
     * This is typically used to create a return procedure.
     */
    void frame_done(variable_ptr result = nullptr);

    /** Load a shared library into the interpreter.
     */
    void load(const std::string &library_name);

    /*variable_ptr eval(const std::string &procedure,
                        const list &parameters);*/

    friend class component;

  protected:
    virtual void entry();

    variable_ptr command();
    variable_ptr variable();
    variable_ptr string();
    variable_ptr subcommand();

  private:
    memory::reference<sandbox> _global;
    memory::reference<frame> _frame;
  };

}

#endif /* _CUTLET_H */
