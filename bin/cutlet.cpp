/*                                                                  -*- c++ -*-
 * Copyright © 2018-2020 Ron R Wills <ron@digitalcombine.ca>
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

/* Automake automatically adds "-I." to the compiling so we have to include
 * cutlet this way or the generated cutlet from automake gets included instead.
 * This should be fine since this source shouldn't be distributed beyond the
 * build.
 */
#include "../include/cutlet"
#include <iostream>
#include <fstream>
#include <getopt.h>
#include <unistd.h>

/********
 * help *
 ********/

static void help() {
  std::cout << "Cutlet v" << VERSION << "\n\n"
            << "cutlet [-i path] filename ...\n"
            << "cutlet -h\n"
            << "  -i path      Include path to the library search\n"
            << "  -V           Display the version\n"
            << "  -h           Displays this help"
            << std::endl;
}

/***********
 * version *
 ***********/

static void version () {
  std::cout << "Cutlet Version " << VERSION << "\n\n"
            << "Copyright (c) 2019 Ron R Wills <ron@digitalcombine.ca>\n"
            << "License BSD: 3-Clause BSD License\n"
            << "  <https://opensource.org/licenses/BSD-3-Clause>\n"
            << "This is free software: you are free to change and "
            << "redistribute it.\n"
            << "There is NO WARRANTY, to the extent permitted by law."
            << std::endl;
}

/************
 * add_path *
 ************/

/** Adds a file path to the search paths list for libraries.
 * @param interp The cutlet interpreter.
 * @param path The path to add to the search list.
 */
static void add_path(cutlet::interpreter &interp, const std::string &path) {
  cutlet::variable::pointer lib_path = interp.var("library.path");
  cutlet::cast<cutlet::list>(lib_path).push_back(new cutlet::string(path));
}

/******************************************************************************
 * Program entry right here!
 */

int main(int argc, char *argv[]) {
  cutlet::interpreter interpreter;

  // Parse the command line options.
  opterr = 0;
  int opt;
  while ((opt = getopt(argc, argv, "i:hV")) != -1) {
    switch (opt) {
    case 'i': // Path option
      add_path(interpreter, optarg);
      break;
    case 'h': // Help option
      help();
      return 0;
    case 'V': // Version option
      version();
      return 0;
    default: // Unknown option.
      std::cerr << "FATAL: unknown option -" << (char)optopt << '\n';
      return 1;
    }
  }

  cutlet::ast::node::pointer compiled;
  try {
    if (optind < argc) {
      // Iterate over command lines for script files.
      for (int i = optind; i < argc; ++i)
        compiled = interpreter.compile_file(argv[i]);

    } else {
      // Nothing on the command line so read from stdin.

      if (isatty(STDIN_FILENO)) {
        /* This appears to be an interactive shell so read and execute line
         * by line.
         */
        for(std::string line; getline(std::cin, line); ) {
          compiled = interpreter.compile(line);
        }

      } else {
        // This appears to be piped in so do everything at once.
        compiled = interpreter.compile(std::cin);
      }
    }

    /* If the return function was called by the script then attempt to return
     * it's value.
     */
    return cutlet::convert<int>(interpreter.pop());

  } catch (parser::syntax_error &err) {
    // Caught a parsing error.
    std::cerr << "SYNTAX ERROR: " << err.what() << ", \""
              << (const std::string &)err.get_token() << "\"" << std::endl;
    return 1;

  } catch (cutlet::exception &err) {
    // Caught a exception through the AST.
    std::cerr << "ERROR: " << err.what() << std::endl;
    for (int i = 0; i < interpreter.frames(); i++) {
      std::cerr << interpreter.frame(i) << std::endl;
    }
    return 1;

  } catch (std::exception &err) {
    // Caught some kind of exception.
    std::cerr << "ERROR: " << err.what() << std::endl;
    for (int i = 0; i < interpreter.frames(); i++) {
      std::cerr << interpreter.frame(i) << std::endl;
    }
    return 1;
  }
}
