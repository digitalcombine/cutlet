/*                                                                 -*- c++ -*-
 * Copyright © 2020 Ron R Wills <ron@digitalcombine.ca>
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

#include <cutlet>
#include <iostream>
#include <fstream>
#include <cstring>

static int results = EXIT_SUCCESS;

static std::ifstream script;

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
  /*if (node.body() != script_token) {

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
    }*/

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

  add_path(interpreter, "../libs/.libs");
  add_path(interpreter, "../libs");

  //script.open("hello.cutlet");
  script.open("oo.cutlet");
  if (not script) {
    std::cerr << "Failed to open script oo.cutlet" << std::endl;
    return EXIT_FAILURE;
  }

  // Setup the debugger interface.
  cutlet::ast::node::debugger(break_int);
  cutlet::ast::node::break_all = true;

  // Lets get to work.
  //interpreter.compile_file("hello.cutlet");
  interpreter.compile_file("oo.cutlet");

  return results;
}
