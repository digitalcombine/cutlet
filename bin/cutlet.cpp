/*                                                                  -*- c++ -*-
 * Copyright © 2018-2020 Ron R Wills <ron@digitalcombine.ca>
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
            << "License GPLv3+: GNU GPL version 3 or later "
            << "<http://gnu.org/licenses/gpl.html>\n"
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
    std::cerr << interpreter.frame() << std::flush;
    return 1;

  } catch (std::exception &err) {
    // Caught some kind of exception.
    std::cerr << "ERROR: " << err.what() << std::endl;
    std::cerr << interpreter.frame() << std::flush;
    return 1;
  }
}
