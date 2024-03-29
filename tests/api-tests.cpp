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

#include "testsuite.h"
#include <cutlet>
#include <iostream>

namespace {

  /*************
   * test_utf8 *
   *************/

  void test_utf8(test::TestSuite &suite) {
    auto &test = suite.test("UTF-8 Iterator");

    std::string tstring("Hello World"), result;

    cutlet::utf8::iterator it(tstring);
    for (; it != it.end(); it++) {
      result += *it;
    }
    test << test::assert(result == "Hello World");

    result.clear();
    for (; it != it.begin(); it--) {
      result += *(it - 1);
    }
    test << test::assert(result == "dlroW olleH");
  }
}

/******************************************************************************
 * Let's get started.
 */

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  test::TestSuite suite("Cutlet API Tests");

  test_utf8(suite);

  std::cout << suite << std::flush;
  return (suite.passed() ? 0 : 1);
}
