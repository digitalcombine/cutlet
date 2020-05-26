/*                                                                  -*- c++ -*-
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
  class frame;
}

/**
 */
std::ostream &operator <<(std::ostream &os, const cutlet::frame &frame);

/** @ns */
namespace cutlet {

  // Token IDs
  const unsigned int T_WORD     = 1;
  const unsigned int T_VARIABLE = 2;
  const unsigned int T_STRING   = 3;
  const unsigned int T_SUBCMD   = 4;
  const unsigned int T_BLOCK    = 5;
  const unsigned int T_COMMENT  = 6;
  const unsigned int T_EOL      = 7;

  // AST node IDs
  const unsigned int A_BLOCK    = 1;
  const unsigned int A_VALUE    = 2;
  const unsigned int A_VARIABLE = 3;
  const unsigned int A_COMMAND  = 4;
  const unsigned int A_STRING   = 5;
  const unsigned int A_COMMENT  = 6;

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

  /****************************************************************************
   */

  class list;
  class interpreter;
  namespace ast {
    class node;
  }

  /** Variable type.
   */
  class DECLSPEC variable : public memory::recyclable {
  public:
    /** Automatic managed memory pointer.
     */
    typedef memory::reference<variable> pointer;

    /** Cleans up the variable.
     */
    virtual ~variable() noexcept;

    /** When a variable is called this is used to implement operators for the
     * variable type.
     *
     * @param self Reference to the variable called.
     * @param interp Reference to the Cutlet interpreter.
     */
    virtual variable::pointer operator()(pointer self,
                                         interpreter &interp,
                                         const list &parameters);

    /** Cast the variable to a std::string type.
     */
    virtual operator std::string() const;

    friend class interpreter;

  protected:
    virtual operator ast::node *() const;
  };

  template <class Ty>
  Ty &cast(variable::pointer object) {
    try {
      if (object.is_null())
        throw std::runtime_error("Unable to cast a null reference");
      return dynamic_cast<Ty &>(*(object));
    } catch (std::bad_cast &) {
      throw std::runtime_error("Unable to cast variable to Ty");
    }
  }

  /** Utility template for creating value converters for variable values.
   */
  template <class Ty>
  Ty DECLSPEC convert(variable::pointer object) { return (Ty)(*(object)); }

  template <> int DECLSPEC convert<int>(variable::pointer object);
  template <> bool DECLSPEC convert<bool>(variable::pointer object);

  typedef void (*debug_function_t)(cutlet::interpreter &interp,
                                   const ast::node &);

  /** @ns */
  namespace ast {

    /** Base abstract syntax tree node.
     * Code is compiled into an abstract syntax tree and all execution is
     * done from it.
     */
    class DECLSPEC node : public memory::recyclable {
    public:
      typedef memory::reference<node> pointer;

      static bool break_all;
      static void debugger(debug_function_t dfunc);

      virtual ~node() noexcept;

      /** Execute the node.
       * @param interp The cutlet interpreter for the execution context.
       * @return Reference pointer to the result of the execution of the node.
       */
      virtual cutlet::variable::pointer
      operator()(cutlet::interpreter &interp) = 0;

      virtual unsigned int id() const = 0;

      /* Set or clear a break point on the node.
       * @param value The value for the break point.
       */
      void set_break(bool value = true);

      virtual std::streampos position() const = 0;

      virtual const std::string &body() const = 0;

      virtual const parser::token &token() const = 0;

    protected:
      node();

      /** To be used by the operator() method to implement break points.
       */
      void break_point(cutlet::interpreter &interp);

    private:
      static debug_function_t _debug_function;

      bool _break;
    };
  }

  /**
   */
  class DECLSPEC exception : public std::exception {
  public:
    exception() noexcept;
    exception(const std::string &message) noexcept;
    exception(const std::string &message, ast::node &node) noexcept;
    exception(const std::exception &other) noexcept;
    exception(const cutlet::exception &other) noexcept;
    virtual ~exception() noexcept;

    exception& operator=(const exception &other) noexcept;

    virtual const char *what() const noexcept;
    const ast::node *node() const noexcept { return _node; }

  protected:
    std::string _message;
    ast::node *_node;
  };

  /****************************************************************************
   */

  class DECLSPEC string : public variable, public std::string {
  public:
    string();
    string(const std::string &value);
    string(int value);
    virtual ~string() noexcept;

    virtual variable::pointer operator()(variable::pointer self,
                                         interpreter &interp,
                                         const list &parameters);

    virtual operator std::string() const;

    friend class interpreter;
  };

  /****************************************************************************
   */

  class DECLSPEC list : public variable,
                        public std::deque<variable::pointer> {
  public:
    list();
    list(const_iterator first, const_iterator last);
    list(const std::initializer_list<variable::pointer> &items);

    /** Copy constructor.
     */
    list(const list &other);

    std::string join(const std::string &delim = " ") const;

    virtual variable::pointer operator()(variable::pointer self,
                                         interpreter &interp,
                                         const list &parameters);

    virtual operator std::string() const;

  private:
    variable::pointer _join(interpreter &interp, const list &parameters);
    variable::pointer _append(interpreter &interp, const list &parameters);
    variable::pointer _prepend(interpreter &interp, const list &parameters);
    variable::pointer _extend(interpreter &interp, const list &parameters);
    variable::pointer _index(interpreter &interp, const list &parameters);
    variable::pointer _remove(interpreter &interp, const list &parameters);
    variable::pointer _foreach(interpreter &interp, const list &parameters);
  };

  /****************************************************************************
   */

  /** Executable component for the Cutlet interpreter.
   */
  class DECLSPEC component {
  public:
    /** Smart component reference pointer. */
    typedef memory::reference<component> pointer;

    /** Component clean up. */
    virtual ~component() noexcept;

    /** Call operator for the component.
     * @param interp The Cutlet interpreter.
     * @param parameters List of the parameters passed in the component call.
     */
    virtual variable::pointer
      operator ()(interpreter &interp, const list &parameters) = 0;

    virtual std::string documentation() const;
  };

  /****************************************************************************
   */

  /** Simplified function pointer type used by sandboxes. The sandbox will
   * automatically wrap the function pointer with a component, so a component
   * object doesn't have to developed for every function component.
   * @see cutlet::sandbox
   */
  typedef variable::pointer (*function_t)(interpreter &, const list &);

  extern "C" {
    typedef void (*libinit_t)(interpreter *);
  }

  /** A sandbox contains the global environment for a Cutlet interpreter.
   */
  class DECLSPEC sandbox {
  public:
    /** Smart sandbox reference pointer. */
    typedef memory::reference<sandbox> pointer;

    sandbox();
    sandbox(const sandbox &other) = delete;
    virtual ~sandbox() noexcept;

    void add(const std::string &name, function_t func,
             const std::string &doc = "");
    void add(const std::string &name, component::pointer comp);
    void remove(const std::string &name);

    /** Removes all components from the sandbox.
     * @note All variables are left alone. Not sure if that is the right thing
     *       to do.
     */
    void clear();

    component::pointer get(const std::string &name) const;

    cutlet::variable::pointer variable(interpreter &interp,
                                       const std::string &name);
    void variable(const std::string &name, variable::pointer value);
    bool has_variable(const std::string &name);

    cutlet::variable::pointer call(interpreter &interp,
                                   const std::string &procedure,
                                   const list &parameters);

    void *symbol(const std::string &name) const;

    friend class interpreter;

  private:
    std::map<std::string, variable::pointer> _variables;
    std::map<std::string, component::pointer> _components;
    std::list<void *> _native_libs;

    void load(interpreter &interp, const std::string &library_name);
  };

  /** Execution frame.
   */
  class DECLSPEC frame : public memory::recyclable {
  public:
    /** The possible executions states for the execution frame.
     */
    typedef enum {FS_DONE = 0, FS_RUNNING = 1, FS_BREAK = 2, FS_CONTINUE = 3}
    state_t;

    typedef memory::reference<frame> pointer;

    frame();
    frame(memory::reference<frame> uplevel, bool isblock = false);
    frame(const frame &other) = delete;
    virtual ~frame() noexcept;

    /** Retrieve the value of a local variable. If the variable doesn't exist
     * then a null pointer is returned.
     * @param name The name of the local variable.
     */
    virtual cutlet::variable::pointer variable(const std::string &name) const;
    virtual void variable(const std::string &name, variable::pointer value);

    /** Set the frame state to FS_DONE and
     */
    virtual void done(variable::pointer result);
    virtual bool done() const;

    /** Return the current execution state of the frame.
     * @return The currently set execution state of the frame.
     */
    state_t state() const;

    /** Set the execution state for the execution frame.
     * @see state_t
     */
    virtual void state(state_t new_state);

    void label(const std::string &value);

    friend class component;
    friend class interpreter;
    friend std::ostream &::operator <<(std::ostream &os,
                                       const cutlet::frame &frame);

  protected:
    state_t _state;
    bool _isblock;

  private:
    memory::reference<frame> _uplevel;
    //bool _isblock;
    std::string _label;
    ast::node::pointer _compiled;

    // Used to restore the global environment if it was changed.
    memory::reference<sandbox> _sandbox_orig;
    std::map<std::string, variable::pointer> _variables;

    variable::pointer _return;

    memory::reference<frame> uplevel(unsigned int levels) const;
  };

  class DECLSPEC block_frame : public cutlet::frame {
  public:
    block_frame(frame::pointer parent);
    block_frame(const std::string &label, frame::pointer parent);
    virtual ~block_frame() noexcept;

    virtual variable::pointer variable(const std::string &name) const;

    virtual void done(variable::pointer result);
    virtual bool done() const;

    virtual void state(state_t new_state);

  private:
    cutlet::frame::pointer _parent;
  };

  /**
   */
  class DECLSPEC interpreter : protected parser::grammer {
  public:
    /** Initialize a new interpreter.
     */
    interpreter();

    /** Disable the default copy constructor.
     */
    interpreter(const interpreter &other) = delete;

    /** Disable the default assignment operator.
     */
    interpreter &operator =(const interpreter &) = delete;

    /** Cleans up the interpreter.
     */
    virtual ~interpreter() noexcept;

    /** Get the value of a variable.
     * First the current frame is checked for the variable. If it doesn't
     * on the frame then the current sandbox is checked for a global
     * variable.
     *
     * @return A reference to the variable's value.
     */
    variable::pointer var(const std::string &name);

    /** Set the value for a global variable in the currently active sandbox.
     * @param name The name of the variable.
     * @param value The value set for variable. If the value is a null pointer
     *              then the variable is removed from the sandbox.
     */
    void global(const std::string &name, variable::pointer value);

    /** Set the value for a local variable in the current frame.
     * @param name The name of the variable.
     * @param value The value set for variable. If the value is a null pointer
     *              then the variable is removed from the frame.
     */
    void local(const std::string &name, variable::pointer value);

    /** Use the interpreter to create a new list variable from the given code.
     */
    variable::pointer list(const std::string code);

    variable::pointer list(const variable::pointer code);

    void add(const std::string &name, function_t func);
    void add(const std::string &name, component::pointer comp);

    /** Get the current global enviroment for the interpreter.
     * @see cutlet::sandbox
     */
    sandbox &environment();

    /**
     * @todo Rename to comp.
     */
    component::pointer get(const std::string &name) const;

    /** Compile and execute the code found in code. The abstract syntax tree
     * is returned which can be executed again later or used for debugging.
     * @param code A reference pointer to a cutlet variable containing the
     *             code to be compiled.
     * @return A reference pointer to the compiled abstract syntax tree.
     * @see cutlet::ast::node
     */
    ast::node::pointer compile(const variable::pointer code);
    ast::node::pointer compile(const std::string &code);
    ast::node::pointer compile(std::istream &in);

    ast::node::pointer compile_file(const std::string &filename);

    variable::pointer expr(const std::string &cmd);

    variable::pointer call(const std::string &procedure,
                           const cutlet::list &parameters);

    /** Get a reference to an excution stack frame.
     * @param levels The number of callers up the stack to go to retrieve the
     *               the frame. Level 0 is the current execution frame on the
     *               top of the stack.
     * @see cutlet::frame
     */
    cutlet::frame::pointer frame(unsigned int levels = 0) const;

    /** Create a new execution frame and push it on to the top of the stack.
     * @see cutlet::frame
     * @see frame
     * @see frame_pop
     */
    void frame_push(const std::string &label = "-");
    void frame_push(frame::pointer frm);
    void frame_push(frame::pointer frm, sandbox::pointer sb);

    /** Creates and new frame on the stack and replaces the global environment
     * with the given sandbox. When the frame is popped off the stack the
     * global frame will be restored as well.
     * @see frame_pop
     */
    void frame_push(sandbox::pointer sb);

    /** Pops the current execution frame off the top of the stack.
     * @return If a return value was set in the frame, then that value is
     *         returned, otherwise a null reference is returned.
     * @see frame_done
     */
    variable::pointer frame_pop();

    variable::pointer frame_pop(variable::pointer result);

    /** End the execution of the current frame with an optional return value.
     * The return value is returned when the frame is popped off the stack.
     * @param result The return value from the completed frame.
     * @see frame_pop
     */
    void frame_done(variable::pointer result = nullptr);

    frame::state_t frame_state() const;

    bool done() const;

    /** Load a dynamic shared library into the current global sandbox. Once
     * the library is loaded it expects to find the init_cutlet function to
     * initialize the library into the interpreter.
     * @param library_name The full path to the dynamic library to be loaded.
     */
    void load(const std::string &library_name);

    friend class component;

  protected:
    /** The entry point to the recursive descent parser. */
    virtual void entry();

    ast::node::pointer comment_();
    ast::node::pointer command_();
    ast::node::pointer variable_();
    ast::node::pointer string_();
    ast::node::pointer subcommand_();

  private:
    memory::reference<sandbox> _global;
    memory::reference<cutlet::frame> _frame;

    ast::node::pointer _compiled;

    /** Internal reference count of all the cutlet interpreters. */
    static unsigned int _interpreters;
  };
}

#endif /* _CUTLET_H */
