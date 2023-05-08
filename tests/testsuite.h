/*                                                                 -*- c++ -*-
 * Copyright © 2018 Ron R Wills <ron@digitalcombine.ca>
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

#include <sstream>
#include <list>
#include <string>

#if defined(__llvm__) && defined(assert)
inline void (assert)(bool e) { assert(e); }
#undef assert
#endif

#ifndef _TESTSUITE_H
#define _TESTSUITE_H

namespace test {
  class TestSuite;
}

/** Writes the summary of a test::TestSuite to a byte stream.
 * @code{.cpp}
 * std::clog << my_test_suite << std::flush;
 * @endcode
 * @see test::TestSuite
 */
std::ostream &operator <<(std::ostream &os, const test::TestSuite &suite);

namespace test {

  template <class Ty>
  class osmanip {
  public:
    explicit osmanip(Ty value) : arg(value) {}
    virtual ~osmanip() noexcept {}

    virtual std::ostream &operator()(std::ostream &os) const = 0;

  protected:
    Ty arg;
  };

  /** A test::Test object that contains the results of a test. It is a
   * std::ostringstream with the addition of recording the results of a test
   * and formatting the string with the test results and comments.
   * @see test::TestSuite
   * @see test::pass
   * @see test::fail
   * @see test::assert
   */
  class Test : public std::ostringstream {
  public:
    virtual ~Test() noexcept;

    /** Overrides the string streams str() method to return a formatted
     * string of the test results.
     * @returns The human readable result of the test.
     */
    std::string str() const;

    void str (const std::string& s) { std::ostringstream::str(s); }

    /** Returns a boolean of the test result.
     * @returns True if the test passed, otherwise false.
     */
    bool passed() const { return _passed; }

    operator bool () const { return _passed; }

    friend class TestSuite;
    friend std::ostream &fail(std::ostream &);
    friend std::ostream &pass(std::ostream &);
    friend std::ostream &clear(std::ostream &);
    friend class assert;

  private:
    std::string _title;
    bool _passed;

    /** Constructor for new test::Test objects.
     * @param[in] title Sets the title for the test being conducted.
     */
    Test(const std::string &title);
  };

  /** Stream modifier that flags a test as failed.
   * @code{.cpp}
   * my_test << test::fail << "The message why it failed.";
   * @endcode
   * @see test::pass
   */
  std::ostream &fail(std::ostream &os);

  /** Stream modifier that flags a test as passed. This cannot override a
   * tests that have already been flagged as failed.
   * @code{.cpp}
   * my_test << test::pass << "Messages for passed tests are ignored.";
   * @endcode
   * @see test::fail
   */
  std::ostream &pass(std::ostream &os);

  std::ostream &clear(std::ostream &os);

  /**
  * @code{.cpp}
  * my_test << test::assert(index < 10) << "Index overflow...";
  * @endcode
   */
  class assert : public osmanip<bool> {
  public:
    explicit assert(bool result);
    virtual ~assert() noexcept;
    virtual std::ostream &operator()(std::ostream &os) const;
  };

  /**
   */
  class TestSuite {
  public:
    TestSuite(const std::string &title);
    virtual ~TestSuite() noexcept;

    /** Creates a new test::Test object, adds it to the test::TestSuite and
     * returns the object.
     * @param[in] title The title of the new test.
     * @returns A reference to the new test::Test object.
     * @see test::Test
     */
    Test &test(const std::string &title);

    bool passed() const;

    operator bool () const { return this->passed(); }

    friend std::ostream &(::operator <<)(std::ostream &out,
                                         const TestSuite &suite);

  private:
    std::string _title;
    std::list<Test *> tests;
  };
}

template <class Ty>
std::ostream& operator <<(std::ostream &os, const test::osmanip<Ty> &manip) {
  return manip(os);
}


#endif /* _TESTSUITE_H */
