/*                                                                  -*- c++ -*-
 * Copyright ©
 * All rights reserved
 */

#include <cutlet.h>
#include <iostream>
#include <fstream>

int main(int argc, char *argv[]) {
  cutlet::interpreter interpreter;

  try {
    if (argc > 1) {
      for (int i = 1; i < argc; ++i) {
        std::ifstream input_file(argv[i]);
        interpreter.eval(input_file);
        input_file.close();
      }
    } else {
      interpreter.eval(std::cin);
    }
  } catch (parser::syntax_error &err) {
    std::cerr << "SYNTAX ERROR: " << err.what() << ", \""
              << (const std::string &)err.get_token() << "\"" << std::endl;
    return 1;
  } catch (std::exception &err) {
    std::cerr << "ERROR: " << err.what() << std::endl;
    return 1;
  }

  return 0;
}
