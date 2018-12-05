#                                                           -*- Makefile.am -*-

lib_LTLIBRARIES = libcutlet.la

libcutlet_la_SOURCES = cutlet.cpp memory.cpp parser.cpp builtin.cpp \
  list.cpp string.cpp utilities.cpp builtin.h utilities.h
libcutlet_la_CPPFLAGS = -I@top_srcdir@/include
libcutlet_la_LDFLAGS = -release $(VERSION)
