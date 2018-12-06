# Shared libraries
pkglib_LTLIBRARIES = stdlib.la oo.la shell.la
EXTRA_DIST = oo.cpp shell.cpp

stdlib_la_SOURCES = stdlib.cpp
stdlib_la_CPPFLAGS = -I$(top_srcdir)/include
stdlib_la_LIBADD = $(top_srcdir)/src/libcutlet.la
stdlib_la_LDFLAGS = -module -avoid-version -shared

oo_la_SOURCES = oo.cpp
oo_la_CPPFLAGS = -I$(top_srcdir)/include
oo_la_LIBADD = $(top_srcdir)/src/libcutlet.la
oo_la_LDFLAGS = -module -avoid-version -shared

shell_la_SOURCES = shell.cpp
shell_la_CPPFLAGS = -I$(top_srcdir)/include
shell_la_LIBADD = $(top_srcdir)/src/libcutlet.la
shell_la_LDFLAGS = -module -avoid-version -shared

# Remove all the unnecessary .la files.
install-exec-hook:
	@(cd $(DESTDIR)$(pkglibdir) && $(RM) $(pkglib_LTLIBRARIES))
