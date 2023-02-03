#                                                           -*- Makefile.am -*-

check_PROGRAMS = debugger-tests

TESTS = core.cutlet hello.cutlet booleans.cutlet strings.cutlet \
	lists.cutlet stdlib.cutlet unknown.cutlet bad_method.cutlet sandbox.cutlet \
	oo.cutlet threading.cutlet debugger-tests
XFAIL_TESTS = bad_method.cutlet
TEST_EXTENSIONS = .cutlet
CUTLET_LOG_COMPILER = ../bin/cutlet
AM_CUTLET_LOG_FLAGS = -I ../libs/.libs/ -I ../libs/ \
	-I @top_srcdir@/libs/.libs/ -I @top_srcdir@/libs/

EXTRA_DIST = $(TESTS) include.cutlet

noinst_LIBRARIES = libtesting.a
libtesting_a_SOURCES = testsuite.cpp testsuite.h include.cutlet
libtesting_a_CPPFLAGS = -I$(top_srcdir)/include

debugger_tests_SOURCES = debugger-tests.cpp
debugger_tests_CPPFLAGS = -I$(top_srcdir)/include
debugger_tests_LDADD = ../src/libcutlet.la
debugger_tests_LDFLAGS = -rdynamic
