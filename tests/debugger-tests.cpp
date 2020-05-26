/*
 */

#include <cutlet.h>
#include <iostream>
#include <fstream>
#include <cstring>

static int results = EXIT_SUCCESS;

std::ifstream script;

void break_int(cutlet::interpreter &interp, const cutlet::ast::node &node) {
  //const parser::token &token = node.token();

  char *script_token = new char[node.body().size() + 3];
  std::memset(script_token, 0, node.body().size() + 3);

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
    if (script_token) {
      delete[] script_token;
      script_token = nullptr;
    }

    results = EXIT_FAILURE;
  }

  if (script_token) delete[] script_token;

  return;
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  cutlet::interpreter interpreter;
  script.open("hello.cutlet");

  cutlet::ast::node::debugger(break_int);
  cutlet::ast::node::break_all = true;

  interpreter.compile_file("hello.cutlet");

  return results;
}
