# Shared libraries
pkglib_LTLIBRARIES = stdlib.la threading.la oo.la interactive.la shell.la \
	debugger.la
scriptlibdir = $(pkglibdir)
scriptlib_DATA = testsuite.cutlet

EXTRA_DIST = testsuite.cutlet

stdlib_la_SOURCES = stdlib.cpp
stdlib_la_CPPFLAGS = -I @top_srcdir@/include
stdlib_la_LIBADD = ../src/libcutlet.la
stdlib_la_LDFLAGS = -module -avoid-version -shared

threading_la_SOURCES = threading.cpp
threading_la_CPPFLAGS = -I @top_srcdir@/include
threading_la_LIBADD = ../src/libcutlet.la -lpthread
threading_la_LDFLAGS = -module -avoid-version -shared

oo_la_SOURCES = oo.cpp
oo_la_CPPFLAGS = -I @top_srcdir@/include
oo_la_LIBADD = ../src/libcutlet.la
oo_la_LDFLAGS = -module -avoid-version -shared

interactive_la_SOURCES = interactive.cpp
interactive_la_CPPFLAGS = -I @top_srcdir@/include $(READLINE_CFLAGS)
interactive_la_LIBADD = ../src/libcutlet.la $(READLINE_LIBS)
interactive_la_LDFLAGS = -module -avoid-version -shared

shell_la_SOURCES = shell.cpp
shell_la_CPPFLAGS = -I @top_srcdir@/include
shell_la_LIBADD = ../src/libcutlet.la
shell_la_LDFLAGS = -module -avoid-version -shared

debugger_la_SOURCES = debugger.cpp
debugger_la_CPPFLAGS = -I @top_srcdir@/include
debugger_la_LIBADD = ../src/libcutlet.la
debugger_la_LDFLAGS = -module -avoid-version -shared

# Remove all the unnecessary .la files.
install-exec-hook:
	@(cd "$(DESTDIR)$(pkglibdir)" && rm -f $(pkglib_LTLIBRARIES))

uninstall-hook:
	@(cd "$(DESTDIR)$(pkglibdir)" && rm -f stdlib.so threading.so shell.so \
		oo.so debugger.so)
