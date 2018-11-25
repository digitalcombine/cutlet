# Shared libraries
pkglib_LTLIBRARIES = oo.la shell.la
EXTRA_DIST = oo.cpp shell.cpp

oo_la_SOURCES = oo.cpp
oo_la_CPPFLAGS = -I$(top_srcdir)/include
oo_la_LIBADD = $(top_srcdir)/src/libcutlet.la
oo_la_LDFLAGS = -module -avoid-version -shared

shell_la_SOURCES = shell.cpp
shell_la_CPPFLAGS = -I$(top_srcdir)/include
shell_la_LIBADD = $(top_srcdir)/src/libcutlet.la
shell_la_LDFLAGS = -module -avoid-version -shared
