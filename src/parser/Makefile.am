# LIBCVC4PARSER_VERSION (-version-info) is in the form current:revision:age
#
# current -
#   increment if interfaces have been added, removed or changed
# revision -
#   increment if source code has changed
#   set to zero if current is incremented
# age -
#   increment if interfaces have been added
#   set to zero if interfaces have been removed
#   or changed
#
LIBCVC4PARSER_VERSION = @CVC4_PARSER_LIBRARY_VERSION@

AM_CPPFLAGS = \
	-D__BUILDING_CVC4PARSERLIB \
	-I@builddir@/.. $(ANTLR_INCLUDES) -I@srcdir@/../include -I@srcdir@/..
AM_CXXFLAGS = -Wall -Wno-unknown-pragmas $(FLAG_VISIBILITY_HIDDEN)

SUBDIRS = smt1 smt2 cvc tptp

lib_LTLIBRARIES = libcvc4parser.la

libcvc4parser_la_LDFLAGS = $(ANTLR_LDFLAGS) \
	-version-info $(LIBCVC4PARSER_VERSION)

libcvc4parser_la_LIBADD = \
	@builddir@/smt1/libparsersmt1.la \
	@builddir@/smt2/libparsersmt2.la \
	@builddir@/tptp/libparsertptp.la \
	@builddir@/cvc/libparsercvc.la \
	@builddir@/../libcvc4.la

if CVC4_NEEDS_REPLACEMENT_FUNCTIONS
libcvc4parser_la_LIBADD += \
	@builddir@/../lib/libreplacements.la
endif

libcvc4parser_la_SOURCES = \
	antlr_input.h \
	input.h \
	input.cpp \
	parser.h \
	parser.cpp \
	parser_builder.h \
	parser_builder.cpp \
	parser_exception.h

libcvc4parser_noinst_la_SOURCES = \
	antlr_input.h \
	input.h \
	input.cpp \
	parser.h \
	parser.cpp \
	parser_builder.h \
	parser_builder.cpp \
	parser_exception.h \
	antlr_tracing.h

EXTRA_DIST = \
	cvc4parser.i \
	input.i \
	parser_builder.i \
	parser_exception.i \
	parser.i
