#                                                           -*- Makefile.am -*-

TESTS = memory-tests core.cutlet hello.cutlet strings.cutlet stdlib.cutlet \
  unknown.cutlet bad_method.cutlet sandbox.cutlet oo.cutlet debugger-tests
XFAIL_TESTS = bad_method.cutlet
TEST_EXTENSIONS = .cutlet
CUTLET_LOG_COMPILER = @top_srcdir@/bin/cutlet
AM_CUTLET_LOG_FLAGS = -i ../libs/.libs/

EXTRA_DIST = $(TESTS)

check_PROGRAMS = memory-tests debugger-tests

noinst_LIBRARIES = libtesting.a
libtesting_a_SOURCES = testsuite.cpp testsuite.h
libtesting_a_CPPFLAGS = -I@top_srcdir@/include

memory_tests_SOURCES = memory-tests.cpp
memory_tests_CPPFLAGS = -I@top_srcdir@/include
memory_tests_LDADD = @top_srcdir@/src/libcutlet.la libtesting.a
memory_tests_LDFLAGS = -rdynamic

debugger_tests_SOURCES = debugger-tests.cpp
debugger_tests_CPPFLAGS = -I@top_srcdir@/include
debugger_tests_LDADD = @top_srcdir@/src/libcutlet.la
debugger_tests_LDFLAGS = -rdynamic
