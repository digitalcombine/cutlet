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

  namespace ast {
    class node;
    typedef memory::reference<node> node_ptr;
  }

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

      iterator operator +(int value) const;
      iterator operator -(int value) const;

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

    virtual variable_ptr operator()(interpreter &interp,
                                    const list &parameters);

    /** Cast the variable to a std::string type.
     */
    virtual operator std::string() const;
  };

  template <class Ty>
  Ty &cast(variable_ptr object) { return dynamic_cast<Ty &>(*(object)); }

  /** Utility template for creating value converters for variable values.
   */
  template <class Ty>
  Ty convert(variable_ptr object) { return (Ty)(*(object)); }

  template <> int convert<int>(variable_ptr object);
  template <> bool convert<bool>(variable_ptr object);

  /***************************************************************************
   */

  class string : public variable, public std::string {
  public:
    string();
    string(const std::string &value);
    string(int value);
    virtual ~string() noexcept;

    virtual variable_ptr operator()(interpreter &interp,
                                    const list &parameters);

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

    virtual variable_ptr operator()(interpreter &interp,
                                    const list &parameters);

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

  /** A sandbox contains the global environment for a Cutlet interpreter.
   */
  class sandbox {
  public:
    sandbox();
    sandbox(const sandbox &other) = delete;
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

  /** Execution frame.
   */
  class frame {
  public:
    frame();
    frame(memory::reference<frame> uplevel);
    frame(const frame &other) = delete;
    virtual ~frame() noexcept;

    /** Retrieve the value of a local variable. If the variable doesn't exist
     * then the return value will be null.
     * @param name The name of the local variable.
     */
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

    memory::reference<frame> uplevel(unsigned int levels) const;
  };

  typedef memory::reference<frame> frame_ptr;

  /***************************************************************************
   */

  class interpreter : public parser::grammer {
  public:
    /** Initialize a interpreter.
     */
    interpreter();

    /** Disable the default copy constructor.
     */
    interpreter(const interpreter &other) = delete;
    virtual ~interpreter() noexcept;

    /** Get the value of a variable.
     */
    variable_ptr var(const std::string &name);

    /** Set the value for a variable in the currently active sandbox.
     * @param name The name of the variable.
     * @param value The value set for variable. If the value is null then the
     *              variable is removed from the sandbox.
     */
    void global(const std::string &name, variable_ptr value);
    void local(const std::string &name, variable_ptr value);

    /** Use the parser to create a list variable.
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

    variable_ptr expr(const std::string &cmd);

    frame_ptr frame(unsigned int levels = 0) const;
    void frame_push();
    void frame_push(frame_ptr frm);
    void frame_push(sandbox_ptr sb);
    void frame_push(frame_ptr frm, sandbox_ptr sb);

    /** Pop the current execution frame off the stack.
     * @return If a return value was set, then return that value. If a return
     *         wasn't set then null is returned.
     * @see frame_done
     */
    variable_ptr frame_pop();

    /** End the execution of the current frame with an optional return value.
     * The return value is returned when the frame is popped off the stack.
     * @param result The return value from the completed frame.
     * @see frame_pop
     */
    void frame_done(variable_ptr result = nullptr);

    /** Load a dynamic shared library into the current global sandbox. Once
     * the library is loaded it expects to find the init_cutlet function to
     * initialize the library into the interpreter.
     * @param library_name The full path to the dynamic library to be loaded.
     */
    void load(const std::string &library_name);

    variable_ptr execute(const std::string &procedure,
                         const cutlet::list &parameters);

    friend class component;

  protected:
    /** Entry point to the recursive descent parser.
     */
    virtual void entry();

    ast::node_ptr command();
    ast::node_ptr variable();
    ast::node_ptr string();
    ast::node_ptr subcommand();

  private:
    memory::reference<sandbox> _global;
    memory::reference<cutlet::frame> _frame;
  };

}

#endif /* _CUTLET_H */
