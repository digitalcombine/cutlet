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

#include <libcutlet/memory>
#include <iostream>
#include "testsuite.h"

/** The test object that keeps track of how many instances it has.
 */
class TestObject {
public:
  static unsigned int count;

  TestObject() { ++count; }
  virtual ~TestObject() throw() { --count; }
};

unsigned int TestObject::count = 0;

typedef memory::reference<TestObject> ref;

/***************
 * ref_examine *
 ***************/

static bool ref_examine(test::Test &test, ref &object, bool is_null, bool weak,
                        unsigned int refs, unsigned int wrefs,
                        unsigned int objs) {
  test << std::boolalpha;

  test << test::assert(object.is_null() == is_null)
       << "is_null() -> " << object.is_null() << " != " << is_null;
  if (not test.passed()) return false;

  test << test::assert(object.is_weak() == weak)
       << "is_weak() -> " << object.is_weak() << " != " << weak;
  if (not test.passed()) return false;

  test << test::assert(object.references() == refs)
       << "references() -> " << object.references() << " != " << refs;
  if (not test.passed()) return false;

  test << test::assert(object.weak_references() == wrefs)
       << "weak_references() -> " << object.weak_references()
       << " != " << wrefs;
  if (not test.passed()) return false;

  test << test::assert(TestObject::count == objs)
       << (unsigned int)TestObject::count << " objects != " << objs;
  if (not test.passed()) return false;

  return true;
}

/*****************
 * ref_parameter *
 *****************/

void ref_parameter(test::Test &test, ref object) {
  if (test)
    test << test::assert(not object.is_null())
         << "Parameter reference was null";
  if (test)
    test << test::assert(object.references() == 3)
         << "Parameter reference had "
         << (unsigned int)object.references() << " references. (!= 3)";
  if (test)
    test << test::assert(TestObject::count == 1)
         << "Test Object count wrong, got "
         << (unsigned int)TestObject::count << " objects. (!= 1)";
}

/**************
 * ref_result *
 **************/

static ref ref_result() {
  TestObject *obj = new TestObject;
  ref ptr(obj);
  return ptr;
}

/**********************
 * basic_ref_counting *
 **********************/

static bool basic_ref_counting() {
  test::TestSuite tests("Basic Reference Counting");
  ref ptr1 = new TestObject;
  ref ptr2;

  test::Test &test_nul = tests.test("Initial Null Reference");
  ref_examine(test_nul, ptr2, true, false, 0, 0, 1);

  test::Test &test_int = tests.test("Initial Reference Count");
  ref_examine(test_int, ptr1, false, false, 1, 0, 1);

  test::Test &test_sec = tests.test("Second Reference");
  ptr2 = ptr1;
  test_sec << test::assert(not ptr1.is_null())
           << "Initial pointer was null";
  if (test_sec)
    test_sec << test::assert(ptr1.references() == 2)
             << "Initial reference had "
             << (unsigned int)ptr1.references() << " references. (!= 2)";
  if (test_sec)
    test_sec << test::assert(not ptr2.is_null())
             << "Second pointer was null";
  if (test_sec)
    test_sec << test::assert(ptr2.references() == 2)
             << "Second reference had "
             << (unsigned int)ptr1.references() << " references. (!= 2)";
  if (test_sec)
    test_sec << test::assert(TestObject::count == 1)
             << "Test Object count wrong, got "
             << (unsigned int)TestObject::count << " objects. (!= 1)";

  test::Test &test_par = tests.test("Passing Reference as a Parameter");
  ref_parameter(test_par, ptr1);
  if (test_par)
    test_par << test::assert(not ptr1.is_null())
             << "Initial pointer was null";
  if (test_par)
    test_par << test::assert(ptr1.references() == 2)
             << "Initial reference had "
             << (unsigned int)ptr1.references() << " references. (!= 2)";
  if (test_par)
    test_par << test::assert(not ptr2.is_null())
             << "Second pointer was null";
  if (test_par)
    test_par << test::assert(ptr2.references() == 2)
             << "Second reference had "
             << (unsigned int)ptr1.references() << " references. (!= 2)";
  if (test_par)
    test_par << test::assert(TestObject::count == 1)
             << "Test Object count wrong, got "
             << (unsigned int)TestObject::count << " objects. (!= 1)";

  test::Test &test_oth = tests.test("Passing Other Reference as a Parameter");
  ref_parameter(test_oth, ptr2);
  if (test_oth)
    test_oth << test::assert(not ptr1.is_null())
             << "Initial pointer was null";
  if (test_oth)
    test_oth << test::assert(ptr1.references() == 2)
             << "Initial reference had "
             << (unsigned int)ptr1.references() << " references. (!= 2)";
  if (test_oth)
    test_oth << test::assert(not ptr2.is_null())
             << "Second pointer was null";
  if (test_oth)
    test_oth << test::assert(ptr2.references() == 2)
             << "Second reference had "
             << (unsigned int)ptr1.references() << " references. (!= 2)";
  if (test_oth)
    test_oth << test::assert(TestObject::count == 1)
             << "Test Object count wrong, got "
             << (unsigned int)TestObject::count << " objects. (!= 1)";

  test::Test &test_rel = tests.test("Releasing Reference");
  ptr1 = NULL;
  test_rel << test::assert(ptr1.is_null())
           << "Initial pointer wasn't null";
  if (test_rel)
    test_rel << test::assert(ptr1.references() == 0)
             << "Initial reference had "
             << (unsigned int)ptr1.references() << " references. (!= 0)";
  if (test_rel)
    test_rel << test::assert(not ptr2.is_null())
             << "Second pointer was null";
  if (test_rel)
    test_rel << test::assert(ptr2.references() == 1)
             << "Second reference had "
             << (unsigned int)ptr1.references() << " references. (!= 1)";
  if (test_rel)
    test_rel << test::assert(TestObject::count == 1)
             << "Test Object count wrong, got "
             << (unsigned int)TestObject::count << " objects. (!= 1)";

  test::Test &test_lst = tests.test("Releasing Last Reference");
  ptr2 = NULL;
  test_lst << test::assert(ptr1.is_null())
           << "Initial pointer wasn't null";
  if (test_lst)
    test_lst << test::assert(ptr1.references() == 0)
             << "Initial reference had "
             << (unsigned int)ptr1.references() << " references. (!= 0)";
  if (test_lst)
    test_lst << test::assert(ptr2.is_null())
             << "Second pointer wasn't null";
  if (test_lst)
    test_lst << test::assert(ptr2.references() == 0)
             << "Second reference had "
             << (unsigned int)ptr1.references() << " references. (!= 0)";
  if (test_lst)
    test_lst << test::assert(TestObject::count == 0)
             << "Test Object count wrong, got "
             << (unsigned int)TestObject::count << " objects. (!= 0)";

  std::cout << tests << std::endl;
  return tests.passed();
}

/***************
 * ref_replace *
 ***************/

static bool ref_replace() {
  test::TestSuite tests("Reference Replacing");
  ref ptr1 = new TestObject;
  ref ptr2;

  test::Test &test_null = tests.test("Initial Null Reference");
  test_null << test::assert(ptr2.is_null())
            << "Initial pointer wasn't null";
  if (test_null)
    test_null << test::assert(ptr2.references() == 0)
              << (unsigned int)ptr2.references() << " references.";

  test::Test &test_count = tests.test("Initial Reference Count");
  test_count << test::assert(not ptr1.is_null())
             << "Initial reference was null";
  if (test_count)
    test_count << test::assert(ptr1.references() == 1)
               << "Initial reference had "
               << (unsigned int)ptr1.references() << " references.";
  if (test_count)
    test_count << test::assert(TestObject::count == 1)
               << "Test Object count wrong, got "
               << (unsigned int)TestObject::count << " objects.";

  test::Test &test_sec = tests.test("Second Reference");
  ptr2 = ptr1;
  test_sec << test::assert(not ptr2.is_null() and not ptr1.is_null())
           << "A reference was null";
  if (test_sec)
    test_sec << test::assert(ptr2.references() == 2)
             << "Second reference had "
             << (unsigned int)ptr2.references() << " references (!= 2).";
  if (test_sec)
    test_sec << test::assert(ptr1.references() == 2)
             << "Initial reference had "
             << (unsigned int)ptr2.references() << " references (!= 2).";

  test::Test &test_rep = tests.test("Replacing Reference");
  ptr1 = new TestObject;
  test_rep << test::assert(not ptr1.is_null())
           << "Initial reference was null.";
  if (test_rep)
    test_rep << test::assert(not ptr2.is_null())
             << "Second reference was null.";
  if (test_rep)
    test_rep << test::assert(not ptr2.is_null())
             << "Second reference was null.";
  if (test_rep)
    test_rep << test::assert(ptr1.references() == 1)
             << "Initial reference had "
             << (unsigned int)ptr1.references() << " references.";
  if (test_rep)
    test_rep << test::assert(TestObject::count == 2)
             << "Test Object count wrong, got "
             << (unsigned int)TestObject::count << " objects.";

  test::Test &test_rec = tests.test("Receiving Reference as Result");
  ptr1 = ref_result();
  test_rec << test::assert(not ptr1.is_null())
           << "Initial reference was null";
  if (test_rec)
    test_rec << test::assert(ptr1.references() == 1)
             << "Initial reference had "
             << (unsigned int)ptr1.references() << " references.";
  if (test_rec)
    test_rec << test::assert(not ptr2.is_null())
             << "Second reference was null";
  if (test_rec)
    test_rec << test::assert(ptr2.references() == 1)
             << "Second reference had "
             << (unsigned int)ptr2.references() << " references.";
  if (test_rec)
    test_rec << test::assert(TestObject::count == 2)
             << "Test Object count wrong, got "
             << (unsigned int)TestObject::count << " objects.";

  test::Test &test_ign = tests.test("Ignored Reference Result");
  ref_result();
  test_ign << test::assert(not ptr1.is_null())
           << "Initial reference was null";
  if (test_ign)
    test_ign << test::assert(ptr1.references() == 1)
             << "Initial reference had "
             << (unsigned int)ptr1.references() << " references. (!= 1)";
  if (test_ign)
    test_ign << test::assert(not ptr2.is_null())
             << "Second reference was null";
  if (test_ign)
    test_ign << test::assert(ptr2.references() == 1)
             << "Second reference had "
             << (unsigned int)ptr1.references() << " references. (!= 1)";
  if (test_ign)
    test_ign << test::assert(TestObject::count == 2)
             << "Test Object count wrong, got "
             << (unsigned int)TestObject::count << " objects.";

  test::Test &test_rel = tests.test("Releasing References");
  ptr1 = NULL;
  ptr2 = NULL;
  test_rel << test::assert(ptr1.is_null())
           << "Initial reference wasn't null";
  if (test_rel)
    test_rel << test::assert(ptr1.references() == 0)
             << "Initial reference had "
             << (unsigned int)ptr1.references() << " references. (!= 0)";
  if (test_rel)
    test_rel << test::assert(ptr2.is_null())
             << "Second reference wasn't null";
  if (test_rel)
    test_rel << test::assert(ptr2.references() == 0)
             << "Second reference had "
             << (unsigned int)ptr1.references() << " references. (!= 0)";
  if (test_rel)
    test_rel << test::assert(TestObject::count == 0)
             << "Test Object count wrong, got "
             << (unsigned int)TestObject::count << " objects. (!= 0)";

  std::cout << tests << std::endl;
  return tests.passed();
}

static ref global;

static ref &get_reference() {
  return global;
}

static bool weak_ref() {
  test::TestSuite tests("Weak References");
  ref ptr1 = new TestObject;
  ref ptr2 = ptr1.weak();

  test::Test &test_weak = tests.test("Weak Reference");
  ref_examine(test_weak, ptr1, false, false, 1, 1, 1) and
    ref_examine(test_weak, ptr2, false, true, 1, 1, 1);

  ptr1 = NULL;
  test::Test &test_null = tests.test("Weak Reference to Null Object");
  ref_examine(test_null, ptr1, true, false, 0, 0, 0) and
    ref_examine(test_null, ptr2, true, true, 0, 1, 0);

  ptr1 = new TestObject;
  test::Test &test_res = tests.test("Weak Reference to Result Reference");
  get_reference() = ptr1.weak();
  test_res << test::assert(get_reference().is_weak())
  << "The result reference isn't weak";

  std::cout << tests << std::endl;
  return tests.passed();
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  int result = 0;

  if (not basic_ref_counting()) result = 1;
  if (not ref_replace()) result = 1;
  if (not weak_ref()) result = 1;

  /* We have to make sure all our references are cleaned up here. At the moment
   * the gc destructor segfaults if it has anything to clean up :P
   */
  get_reference() = nullptr;
  memory::gc::collect_all();

  return result;
}
