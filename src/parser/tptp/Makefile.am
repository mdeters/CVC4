AM_CPPFLAGS = \
	-D__BUILDING_CVC4PARSERLIB \
	-I@builddir@/../.. -I@srcdir@/../../include -I@srcdir@/../.. $(ANTLR_INCLUDES)
AM_CXXFLAGS = -Wall -Wno-unknown-pragmas $(FLAG_VISIBILITY_HIDDEN) $(WNO_PARENTHESES) $(WNO_TAUTOLOGICAL_COMPARE) -Wno-unused-function -Wno-unused-variable -Wno-sign-compare

ANTLR_OPTS =

noinst_LTLIBRARIES = libparsertptp.la

ANTLR_TOKEN_STUFF = \
	generated/Tptp.tokens
ANTLR_LEXER_STUFF = \
	generated/TptpLexer.hpp \
	generated/TptpLexer.cpp \
	$(ANTLR_TOKEN_STUFF)
ANTLR_PARSER_STUFF = \
	generated/TptpParser.hpp \
	generated/TptpParser.cpp
ANTLR_STUFF = \
	$(ANTLR_LEXER_STUFF) \
	$(ANTLR_PARSER_STUFF)

libparsertptp_la_SOURCES = \
	Tptp.g \
	tptp.h \
	tptp.cpp \
	tptp_input.h \
	tptp_input.cpp \
	$(ANTLR_STUFF)

BUILT_SOURCES = \
	generated/Tptp.tokens \
	generated/TptpLexer.hpp \
	generated/TptpLexer.cpp \
	generated/TptpParser.hpp \
	generated/TptpParser.cpp \
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
generated/TptpLexer.hpp: Tptp.g stamp-generated
	-$(AM_V_at)rm -f $(ANTLR_STUFF)
	@if test -z "$(ANTLR)"; then echo "ERROR: antlr parser generator cannot be found, cannot generate the parser" >&2; exit 1; fi
	$(AM_V_GEN)$(ANTLR) $(ANTLR_OPTS) -fo "generated" "@srcdir@/Tptp.g"

# These don't actually depend on TptpLexer.hpp, but if we're doing parallel
# make and the lexer needs to be rebuilt, we have to keep the rules
# from running in parallel (since the token files will be deleted &
# recreated)
generated/TptpLexer.cpp generated/TptpParser.hpp generated/TptpParser.cpp $(ANTLR_TOKEN_STUFF): generated/TptpLexer.hpp
