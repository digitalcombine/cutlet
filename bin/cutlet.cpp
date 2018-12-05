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

#include <cutlet.h>
#include <iostream>
#include <fstream>

int main(int argc, char *argv[]) {
  cutlet::interpreter interpreter;

  try {
    if (argc > 1) {
      // Iterate over command lines for script files.
      for (int i = 1; i < argc; ++i) {
        std::ifstream input_file(argv[i]);
        interpreter.eval(input_file);
        input_file.close();
      }

    } else {
      // Nothing on the command line so read from stdin.
      interpreter.eval(std::cin);
    }

    /* If the return function was called by the script then attempt to return
     * it's value.
     */
    return cutlet::convert<int>(interpreter.frame_pop());

  } catch (parser::syntax_error &err) {
    // Caught a parsing error.
    std::cerr << "SYNTAX ERROR: " << err.what() << ", \""
              << (const std::string &)err.get_token() << "\"" << std::endl;
    return 1;

  } catch (std::exception &err) {
    // Caught some kind of exception.
    std::cerr << "ERROR: " << err.what() << std::endl;
    return 1;
  }
}
