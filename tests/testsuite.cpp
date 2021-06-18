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

 #include "testsuite.h"

/****************************************************************************
  * class test::Test
  */

/*********************
 * test::Test::~Test *
 *********************/

test::Test::~Test() noexcept {}

/********************
 * test::Test::Test *
 ********************/

test::Test::Test(const std::string &title) : _title(title), _passed(true) {
}

/*******************
 * test::Test::str *
 *******************/

std::string test::Test::str() const {
  /*  We override the str() method to return a formated string with the title,
   * pass/fail result and any messages from the stream.
   */
  std::string result = "TEST: ";
  result += _title + "... ";

  if (_passed) {
    // Passed test.
    result += "passed\n";
  } else {
    // Failed test.
    result += "failed\n  ";

    // Include the message from the stream.
    std::string mesg(std::ostringstream::str());
    if (mesg.empty())
      result += "Unspecified reason.\n";
    else
      result += std::ostringstream::str() + "\n";
  }

  return result;
}

/**************
 * test::fail *
 **************/

std::ostream &test::fail(std::ostream &out) {
  // Set the stream to failed.
  Test *ostream = dynamic_cast<Test *>(&out);
  if (ostream != NULL) {
    ostream->str("");
    ostream->_passed = false;
  }
  return out;
}

/**************
 * test::pass *
 **************/

std::ostream &test::pass(std::ostream &os) {
  /*  This is superficial. If a test has passed then we can set a passed, but
   * we can set a test as passed if it been flagged as failed. Even through
   * this doesn't really doesn't do anything we are going to go through the
   * motions anyway ;)
   */
  Test *ostream = dynamic_cast<Test *>(&os);
  if (ostream != NULL and ostream->_passed) ostream->_passed = true;
  return os;
}

/***************
 * test::clear *
 ***************/

std::ostream &test::clear(std::ostream &os) {
  Test *ostream = dynamic_cast<Test *>(&os);
  if (ostream != NULL and ostream->_passed) ostream->str("");
  return os;
}

/****************************************************************************
  * class test::assert
  */

/************************
 * test::assert::assert *
 ************************/

test::assert::assert(bool result) : osmanip<bool>(result) {}

/*************************
 * test::assert::~assert *
 *************************/

test::assert::~assert() noexcept {}

/****************************
 * test::assert::operator() *
 ****************************/

std::ostream &test::assert::operator()(std::ostream &os) const {
  Test *ostream = dynamic_cast<Test *>(&os);
  if (ostream != NULL and ostream->_passed) {
    if (not arg) ostream->str("");
    ostream->_passed = arg;
  }
  return os;
}

/****************************************************************************
  * class test::TestSuite
  */

/******************************
 * test::TestSuite::TestSuite *
 ******************************/

test::TestSuite::TestSuite(const std::string &title) : _title(title) {}

/*******************************
 * test::TestSuite::~TestSuite *
 *******************************/

test::TestSuite::~TestSuite() noexcept {
  // Clean up all the tests.
  std::list<Test *>::iterator it = tests.begin();
  for (; it != tests.end(); ++it)
    delete *it;
}

/*************************
 * test::TestSuite::test *
 *************************/

test::Test &test::TestSuite::test(const std::string &title) {
  // Allocate a new test stream and return it.
  tests.push_back(new Test(title));
  return *tests.back();
}

bool test::TestSuite::passed() const {
  std::list<Test *>::const_iterator it = tests.begin();
  for (; it != tests.end(); ++it)
    if (not (*it)->passed()) return false;
  return true;
}

/***************
 * operator << *
 ***************/

std::ostream &operator <<(std::ostream &out, const test::TestSuite &suite) {
  // Write the suite title.
  out << std::string(79, '=') << "\n";
  out << "TEST SUITE: " << suite._title << "\n";

  unsigned int passed = 0;
  unsigned int failed = 0;

  // Display all the test results and keep count.
  std::list<test::Test *>::const_iterator it = suite.tests.begin();
  for (; it != suite.tests.end(); ++it) {
    if ((*it)->passed()) passed++;
    else failed++;

    out << (*it)->str();
  }

  // Display the counted summaries.
  if (not failed) {
    out << std::string(79, '=') << "\n";
    out << "All " << suite.tests.size() << " tests passed.\n";
  } else {
    out << std::string(79, '=') << "\n";
    out << suite.tests.size() << " tests completed.\n";
    out << passed << " tests passed.\n";
    out << failed << " tests failed.\n";
  }

  return out;
}
