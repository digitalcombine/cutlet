#                                                           -*- Makefile.am -*-

lib_LTLIBRARIES = libcutlet.la

libcutlet_la_SOURCES = cutlet.cpp memory.cpp parser.cpp builtin.cpp \
  list.cpp string.cpp sandbox.cpp utilities.cpp ast.cpp builtin.h \
  utilities.h ast.h
libcutlet_la_CPPFLAGS = -I@top_srcdir@/include
libcutlet_la_LDFLAGS = -release $(VERSION)
