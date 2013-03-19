AM_CPPFLAGS = \
	-D__BUILDING_CVC4PARSERLIB \
	-I@builddir@/../.. -I@srcdir@/../../include -I@srcdir@/../.. $(ANTLR_INCLUDES)
AM_CXXFLAGS = -Wall -Wno-unknown-pragmas $(FLAG_VISIBILITY_HIDDEN) $(WNO_PARENTHESES) $(WNO_TAUTOLOGICAL_COMPARE) -Wno-unused-function -Wno-unused-variable -Wno-sign-compare

ANTLR_OPTS =

noinst_LTLIBRARIES = libparsersmt1.la

ANTLR_TOKEN_STUFF = \
	generated/Smt1.tokens
ANTLR_LEXER_STUFF = \
	generated/Smt1Lexer.hpp \
	generated/Smt1Lexer.cpp \
	$(ANTLR_TOKEN_STUFF)
ANTLR_PARSER_STUFF = \
	generated/Smt1Parser.hpp \
	generated/Smt1Parser.cpp
ANTLR_STUFF = \
	$(ANTLR_LEXER_STUFF) \
	$(ANTLR_PARSER_STUFF)

libparsersmt1_la_SOURCES = \
	Smt1.g \
        smt1.h \
        smt1.cpp \
	smt1_input.h \
	smt1_input.cpp \
	$(ANTLR_STUFF)

BUILT_SOURCES = \
	generated/Smt1.tokens \
	generated/Smt1Lexer.hpp \
	generated/Smt1Lexer.cpp \
	generated/Smt1Parser.hpp \
	generated/Smt1Parser.cpp \
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
generated/Smt1Lexer.hpp: Smt1.g stamp-generated
	-$(AM_V_at)rm -f $(ANTLR_STUFF)
	@if test -z "$(ANTLR)"; then echo "ERROR: antlr parser generator cannot be found, cannot generate the parser" >&2; exit 1; fi
	$(AM_V_GEN)$(ANTLR) $(ANTLR_OPTS) -fo "generated" "@srcdir@/Smt1.g"

# These don't actually depend on Smt1Lexer.hpp, but if we're doing parallel
# make and the lexer needs to be rebuilt, we have to keep the rules
# from running in parallel (since the token files will be deleted &
# recreated)
generated/Smt1Lexer.cpp generated/Smt1Parser.hpp generated/Smt1Parser.cpp $(ANTLR_TOKEN_STUFF): generated/Smt1Lexer.hpp
