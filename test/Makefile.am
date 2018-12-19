#                                                           -*- Makefile.am -*-

TESTS = memory-tests hello.ctl stdlib_tests.ctl unknown.ctl bad_method.ctl \
  sandbox.ctl
XFAIL_TESTS = bad_method.ctl
TEST_EXTENSIONS = .ctl
CTL_LOG_COMPILER = @top_srcdir@/bin/cutlet
AM_CTL_LOG_FLAGS =

EXTRA_DIST = $(TESTS)

check_PROGRAMS = memory-tests

noinst_LIBRARIES = libtesting.a
libtesting_a_SOURCES = testsuite.cpp
libtesting_a_CPPFLAGS = -I@top_srcdir@/include

memory_tests_SOURCES = memory-tests.cpp
memory_tests_CPPFLAGS = -I@top_srcdir@/include
memory_tests_LDADD = @top_srcdir@/src/libcutlet.la libtesting.a
memory_tests_LDFLAGS = -rdynamic
