AM_CPPFLAGS = \
	-D__BUILDING_CVC4PARSERLIB \
	-I@builddir@/../.. -I@srcdir@/../../include -I@srcdir@/../.. $(ANTLR_INCLUDES)
AM_CXXFLAGS = -Wall -Wno-unknown-pragmas $(FLAG_VISIBILITY_HIDDEN) $(WNO_PARENTHESES) $(WNO_TAUTOLOGICAL_COMPARE) -Wno-unused-function -Wno-unused-variable -Wno-sign-compare

ANTLR_OPTS =

noinst_LTLIBRARIES = libparsersmt2.la

ANTLR_TOKEN_STUFF = \
	generated/Smt2.tokens
ANTLR_LEXER_STUFF = \
	generated/Smt2Lexer.hpp \
	generated/Smt2Lexer.cpp \
	$(ANTLR_TOKEN_STUFF)
ANTLR_PARSER_STUFF = \
	generated/Smt2Parser.hpp \
	generated/Smt2Parser.cpp
ANTLR_STUFF = \
	$(ANTLR_LEXER_STUFF) \
	$(ANTLR_PARSER_STUFF)

libparsersmt2_la_SOURCES = \
	Smt2.g \
	smt2.h \
	smt2.cpp \
	smt2_input.h \
	smt2_input.cpp \
	$(ANTLR_STUFF)

BUILT_SOURCES = \
	generated/Smt2.tokens \
	generated/Smt2Lexer.hpp \
	generated/Smt2Lexer.cpp \
	generated/Smt2Parser.hpp \
	generated/Smt2Parser.cpp \
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
generated/Smt2Lexer.hpp: Smt2.g stamp-generated
	-$(AM_V_at)rm -f $(ANTLR_STUFF)
	@if test -z "$(ANTLR)"; then echo "ERROR: antlr parser generator cannot be found, cannot generate the parser" >&2; exit 1; fi
	$(AM_V_GEN)$(ANTLR) $(ANTLR_OPTS) -fo "generated" "@srcdir@/Smt2.g"

# These don't actually depend on SmtLexer.hpp, but if we're doing parallel
# make and the lexer needs to be rebuilt, we have to keep the rules
# from running in parallel (since the token files will be deleted &
# recreated)
generated/Smt2Lexer.cpp generated/Smt2Parser.hpp generated/Smt2Parser.cpp $(ANTLR_TOKEN_STUFF): generated/Smt2Lexer.hpp
