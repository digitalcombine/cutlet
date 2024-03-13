/*                                                                  -*- c++ -*-
 * Copyright © 2023 Ron R Wills <ron@digitalcombine.ca>
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

/* NOTE
 *
 * I've made the assumption that this would be loaded and used by a single
 * interpreter. So only a single global reference is kept for the interpreter.
 * Also readline functions are single global references that only expect a
 * single interactive prompt. If any software uses more than one interpreter
 * and expects to have each one interactive, it could get very messy.
 *
 * I'm sure this could all be made to work but it's not an issue for anyone
 * right now so I'm moving on until it is.
 *                                                                  - Ron Wills
 */

#include <cutlet>
#include <cstring>
#include <streambuf>
#include <readline/readline.h>

//#define DEBUG_INTERACTIVE 1

#if DEBUG_INTERACTIVE
#pragma message ("Interactive library debugging enabled")
#include <iostream>
#endif

// We need to declare init_cutlet as a C function.
extern "C" {
  DECLSPEC void init_cutlet(cutlet::interpreter *interp);

  DECLSPEC std::istream &interactive_stream();
}

namespace {

  /***********
   * _interp *
   ***********/


  cutlet::interpreter *_interp(cutlet::interpreter *i_ptr = nullptr) {
    // Keep a reference to the interpreter that loaded us.
    static cutlet::interpreter *i = nullptr;
    if (i_ptr) i = i_ptr;
    return i;
  }

  /****************************
   * component_name_generator *
   ****************************/

  char *component_name_generator(const char *text, int state) {
    // Component name generator if readline auto completion.
    static cutlet::sandbox::const_citerator iter, end;
    static int len;

    if (!state) {
#if DEBUG_INTERACTIVE
      std::cerr << "INTERACTIVE: Readline completion state reset" << std::endl;
#endif
      iter = _interp()->environment()->components().begin();
      end = _interp()->environment()->components().end();
      len = strlen(text);
    }

    while ((++iter != end)) {
#if DEBUG_INTERACTIVE
      std::cerr << "INTERACTIVE: " << iter->first << " == " << text << std::endl;
#endif
      if (iter->first.compare(0, len, text) == 0) {
#if DEBUG_INTERACTIVE
        std::cerr << "INTERACTIVE: match found" << std::endl;
#endif
        return strdup(iter->first.c_str());
      }
    }

#if DEBUG_INTERACTIVE
    std::cerr << "INTERACTIVE: no more matches" << std::endl;
#endif
    return nullptr;
  }

  /*****************************
   * component_name_completion *
   *****************************/

  char **component_name_completion(const char *text, int start, int end) {
    // Component name completion if readline auto completion.
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, component_name_generator);
  }

  /*********************
   * def cutlet_prompt *
   *********************/

  cutlet::variable::pointer
  _cutlet_prompt(cutlet::interpreter &interp, const cutlet::list &arguments) {
    (void)interp;
    (void)arguments;
    return cutlet::var<cutlet::string>("$ ");
  }

  /*********************
   * def exit ¿result? *
   *********************/

  cutlet::variable::pointer
  _iexit(cutlet::interpreter &interp, const cutlet::list &arguments) {
    cutlet::frame::pointer frame = interp.frame(1);

    switch (arguments.size()) {
    case 0:
      frame->state(cutlet::frame::FS_DONE);
      break;
    case 1:
      frame->done(arguments[0]);
      break;
    default:
      frame->done(cutlet::var<cutlet::list>(arguments));
      break;
    }
    interactive_stream().setstate(std::ios_base::eofbit);

    return nullptr;
  }

  /****************************************************************************
   * class readlinebuf
   */

  class readlinebuf : public std::streambuf {
    /* A stream buffer that uses the readline prompt/library as its input.
     */
  public:
    readlinebuf();
    virtual ~readlinebuf() noexcept;

  protected:
    virtual int_type underflow();

  private:
    std::string _linebuf;

    bool oflush();
  };

  /****************************
   * readlinebuf::readlinebuf *
   ****************************/

  readlinebuf::readlinebuf() {
    // Setup the stream buffers.
    setg(_linebuf.data(), _linebuf.data(), _linebuf.data());
  }

  /*****************************
   * readlinebuf::~readlinebuf *
   *****************************/

  readlinebuf::~readlinebuf() noexcept {}

  /**************************
   * readlinebuf::underflow *
   **************************/

  readlinebuf::int_type readlinebuf::underflow() {
    if (gptr() >= egptr()) {
      // Get the prompt. A custom prompt can be created with the cutlet
      // function cutlet_prompt.
      std::string prompt = "$ ";
      try {
        prompt = *(_interp()->call("cutlet_prompt", cutlet::list()));
      } catch (std::exception &err) {
#if DEBUG_INTERACTIVE
        std::cerr << "DEBUG: Error calling cutlet_prompt "
                  << err.what() << std::endl;
#endif
      }

      // The buffer has been exhausted, read more in from readline.
      char *line = readline(prompt.c_str());

      if (line == nullptr) {
        // End of file
#ifdef DEBUG_NSTREAM
        std::clog << "sockbuf::underflow eof" << std::endl;
#endif
        return traits_type::eof();
      }

      // Update the the buffer and free allocated strings from readline.
      _linebuf = line;
      _linebuf += '\n';
      free(line);

      // Update the buffer.
      setg(_linebuf.data(), _linebuf.data(),
           _linebuf.data() + _linebuf.size());
    }

    // Return the next character.
    return traits_type::to_int_type(*gptr());
  }
}

/***************
 * init_cutlet *
 ***************/

void init_cutlet(cutlet::interpreter *interp) {
  // Add the library to the interpreter. We also keep our own reference to the
  // interpreter for the command line stream buffer if needed.
  _interp(interp);
  interp->add("cutlet_prompt", _cutlet_prompt);
  interp->add("exit", _iexit);
}

/**********************
 * interactive_stream *
 **********************/

std::istream &interactive_stream() {
  // Create a singleton readline stream object for interactive use.
  static std::unique_ptr<std::istream> input = nullptr;
  static std::unique_ptr<std::streambuf> inputbuf = nullptr;

  if (not input) {
    rl_attempted_completion_function = component_name_completion;

    inputbuf = std::make_unique<readlinebuf>();
    input = std::make_unique<std::istream>(inputbuf.get());
  }

  return *input;
}
