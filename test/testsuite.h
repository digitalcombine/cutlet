/*                                                                 -*- c++ -*-
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
/** @file */

#include <sstream>
#include <list>
#include <string>

#ifndef _TESTSUITE_H
#define _TESTSUITE_H

/** @ns */
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

/** @ns */
namespace test {

  template <class Ty>
	class osmanip {
  public:
		explicit osmanip(Ty arg) : arg(arg) {}

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

    operator bool () const { return _passed; };

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
    virtual std::ostream &operator()(std::ostream &os) const;
  };

  /**
   */
	class TestSuite {
	public:
    TestSuite(const std::string &title);
    virtual ~TestSuite() throw();

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
    std::string title;
    std::list<Test *> tests;
	};
}

template <class Ty>
std::ostream& operator <<(std::ostream &os, const test::osmanip<Ty> &manip) {
  return manip(os);
}


#endif /* _TESTSUITE_H */
