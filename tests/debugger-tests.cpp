/*                                                                 -*- c++ -*-
 * Copyright © 2020 Ron R Wills <ron@digitalcombine.ca>
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
#include <iostream>
#include <fstream>
#include <cstring>

static int results = EXIT_SUCCESS;

static std::ifstream script;

/*************
 * break_int *
 *************/

static void break_int(cutlet::interpreter &interp,
                      const cutlet::ast::node &node) {

  // Initialize memory to read the script parts into.
  char *script_token = new char[node.body().size() + 3];
  std::memset(script_token, 0, node.body().size() + 3);

  // With the node position read text from the script.
  switch (node.id()) {
  case cutlet::A_STRING:
  case cutlet::A_COMMENT:
    script.seekg(node.position() + (std::streamoff)1);
    script.read(script_token, node.body().size());
    break;
  case cutlet::A_VARIABLE: {
    script.seekg(node.position());
    script.read(script_token, node.body().size());
    if (std::strncmp(script_token, "${", 2) == 0) {
      script.seekg(node.position() + (std::streamoff)2);
      script.read(script_token, node.body().size());
    } else if (std::strncmp(script_token, "$", 1) == 0) {
      script.seekg(node.position() + (std::streamoff)1);
      script.read(script_token, node.body().size());
    }
    break;
  }
  case cutlet::A_COMMAND:
    script.seekg(node.position());
    script.read(script_token, node.body().size());
    if (std::strncmp(script_token, "$", 1) == 0) {
      script.seekg(node.position() + (std::streamoff)1);
      script.read(script_token, node.body().size());
    }
    break;
  case cutlet::A_VALUE:
    script.seekg(node.position());
    script.read(script_token, node.body().size());
    if (std::strncmp(script_token, "{", 1) == 0) {
      script.seekg(node.position() + (std::streamoff)1);
      script.read(script_token, node.body().size());
    }
    break;
  default:
    script.seekg(node.position());
    script.read(script_token, node.body().size());
    break;
  }

  // Is the node matching with what was read from the script.
  if (node.body() != script_token) {

    std::cout << "Node @ " << node.position() << ": "
              << node.body() << std::endl;

    switch (node.id()) {
    case cutlet::A_VALUE:
      std::cout << "  Type: value" << std::endl;
      break;
    case cutlet::A_VARIABLE:
      std::cout << "  Type: variable" << std::endl;
      break;
    case cutlet::A_STRING:
      std::cout << "  Type: string" << std::endl;
      break;
    case cutlet::A_COMMAND:
      std::cout << "  Type: command" << std::endl;
      break;
    case cutlet::A_COMMENT:
      std::cout << "  Type: comment" << std::endl;
      break;
    default:
      std::cerr << "Unknown node type" << std::endl;
      break;
    }

    std::cerr << "  " << script_token << " != " << node.body()
              << std::endl;
    results = EXIT_FAILURE;
  }

  // Clean up after ourself.
  if (script_token) delete[] script_token;
}

/********
 * main *
 ********/

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  // Setup the interpreter and load the script.
  cutlet::interpreter interpreter;
  script.open("hello.cutlet");

  // Setup the debugger interface.
  cutlet::ast::node::debugger(break_int);
  cutlet::ast::node::break_all = true;

  // Lets get to work.
  interpreter.compile_file("hello.cutlet");

  return results;
}
