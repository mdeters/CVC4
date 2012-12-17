AM_CPPFLAGS = \
	-D__BUILDING_CVC4PARSERLIB \
	-I@srcdir@/../../include -I@srcdir@/../.. -I@builddir@/../.. $(ANTLR_INCLUDES)
AM_CXXFLAGS = -Wall -Wno-unknown-pragmas $(FLAG_VISIBILITY_HIDDEN) -Wno-unused-function -Wno-unused-variable $(WNO_CONVERSION_NULL)

# Compile generated C files using C++ compiler
CC=$(CXX)
AM_CFLAGS = $(AM_CXXFLAGS)

ANTLR_OPTS =

# hide this included makefile from automake
@mk_include@ @srcdir@/../Makefile.antlr_tracing

noinst_LTLIBRARIES = libparsermjollnir.la

ANTLR_TOKEN_STUFF = \
	generated/Mjollnir.tokens
ANTLR_LEXER_STUFF = \
	generated/MjollnirLexer.h \
	generated/MjollnirLexer.c \
	$(ANTLR_TOKEN_STUFF)
ANTLR_PARSER_STUFF = \
	generated/MjollnirParser.h \
	generated/MjollnirParser.c
ANTLR_STUFF = \
	$(ANTLR_LEXER_STUFF) \
	$(ANTLR_PARSER_STUFF)

libparsermjollnir_la_SOURCES = \
	Mjollnir.g \
	mjollnir_input.h \
	mjollnir_input.cpp \
	$(ANTLR_STUFF)

BUILT_SOURCES = \
	generated/Mjollnir.tokens \
	generated/MjollnirLexer.h \
	generated/MjollnirLexer.c \
	generated/MjollnirParser.h \
	generated/MjollnirParser.c \
	stamp-generated

EXTRA_DIST = \
	stamp-generated

DISTCLEANFILES = $(ANTLR_STUFF)
distclean-local:
	-$(AM_V_at)rmdir generated
	-$(AM_V_at)rm -f stamp-generated

stamp-generated:
	$(AM_V_at)mkdir -p generated
	$(AM_V_at)touch stamp-generated

# antlr doesn't overwrite output files, it just leaves them.  So we have to delete them first.
generated/MjollnirLexer.h: Mjollnir.g stamp-generated
	-$(AM_V_at)rm -f $(ANTLR_STUFF)
	@if test -z "$(ANTLR)"; then echo "ERROR: antlr parser generator cannot be found, cannot generate the parser" >&2; exit 1; fi
	$(AM_V_GEN)$(ANTLR) $(ANTLR_OPTS) -fo "generated" "@srcdir@/Mjollnir.g"

# These don't actually depend on MjollnirLexer.h, but if we're doing parallel
# make and the lexer needs to be rebuilt, we have to keep the rules
# from running in parallel (since the token files will be deleted &
# recreated)
generated/MjollnirLexer.c generated/MjollnirParser.h generated/MjollnirParser.c $(ANTLR_TOKEN_STUFF): generated/MjollnirLexer.h
