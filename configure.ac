#                                                             -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([cutlet], [0.9.1], [ron@digitalcombine.ca])
AM_INIT_AUTOMAKE
AM_SILENT_RULES
AC_CONFIG_SRCDIR([src/cutlet.cpp])
AC_CONFIG_MACRO_DIR([m4])

# Platform detection.
AC_CANONICAL_HOST

# Some build options.
AC_ARG_ENABLE([testing],
  [AS_HELP_STRING([--enable-testing],
  [compile in testing code (default is no)])],
     [case "${enableval}" in
        yes) testing=true; AC_DEFINE(TESTING) ;;
        no)  testing=false ;;
        *)   AC_MSG_ERROR([bad value ${enableval} for --enable-testing]) ;;
      esac], [testing=false])
AM_CONDITIONAL([TESTING], [test x$testing = xtrue])

AC_ARG_ENABLE([debug],
  [AS_HELP_STRING([--enable-debug],
  [compile in debugging code (default is no)])],
     [case "${enableval}" in
        yes) debug=true; AC_DEFINE(DEBUG) ;;
        no)  debug=false ;;
        *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
      esac], [debug=false])
AM_CONDITIONAL([DEBUG], [test x$debug = xtrue])

# Checks for programs.
AC_LANG(C++)
AC_PROG_CXX
AX_CHECK_COMPILE_FLAG([-std=c++11],
  [AX_APPEND_FLAG([-std=c++11], CXXFLAGS)],
  AC_MSG_ERROR(C++11 not supported by compiler))
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for libraries.
AC_CHECK_LIB(m, pow)
case $host_os in
  linux*)
    AC_CHECK_LIB([dl], [dlopen], ,
         [AC_MSG_ERROR([Library dl missing for your platform])])
    ;;
  freebsd*)
    ;;
  mingw*)
	  AC_DEFINE([DLL_EXPORT])
    ;;
  *)
    # Default Case
    AC_MSG_ERROR([Your platform is not currently supported])
    ;;
esac

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 include/Makefile
                 bin/Makefile
                 libs/Makefile
                 man/Makefile
                 tests/Makefile])
AC_OUTPUT
