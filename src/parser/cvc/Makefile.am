AM_CPPFLAGS = \
	-D__BUILDING_CVC4PARSERLIB \
	-I@builddir@/../.. -I@srcdir@/../../include -I@srcdir@/../.. $(ANTLR_INCLUDES)
AM_CXXFLAGS = -Wall -Wno-unknown-pragmas $(FLAG_VISIBILITY_HIDDEN) $(WNO_PARENTHESES) $(WNO_TAUTOLOGICAL_COMPARE) -Wno-unused-function -Wno-unused-variable -Wno-sign-compare $(WNO_UNINITIALIZED) $(WNO_CONVERSION_NULL)

ANTLR_OPTS =

noinst_LTLIBRARIES = libparsercvc.la

ANTLR_TOKEN_STUFF = \
	generated/Cvc.tokens
ANTLR_LEXER_STUFF = \
	generated/CvcLexer.hpp \
	generated/CvcLexer.cpp \
	$(ANTLR_TOKEN_STUFF)
ANTLR_PARSER_STUFF = \
	generated/CvcParser.hpp \
	generated/CvcParser.cpp
ANTLR_STUFF = \
	$(ANTLR_LEXER_STUFF) \
	$(ANTLR_PARSER_STUFF)

libparsercvc_la_SOURCES = \
	Cvc.g \
	cvc_input.h \
	cvc_input.cpp \
	$(ANTLR_STUFF)

BUILT_SOURCES = \
	generated/Cvc.tokens \
	generated/CvcLexer.hpp \
	generated/CvcLexer.cpp \
	generated/CvcParser.hpp \
	generated/CvcParser.cpp \
	stamp-generated

EXTRA_DIST = \
	README \
	stamp-generated

DISTCLEANFILES = $(ANTLR_STUFF)
distclean-local:
	-$(AM_V_at)rmdir generated
	-$(AM_V_at)rm -f stamp-generated

stamp-generated:
	$(AM_V_at)mkdir -p generated
	$(AM_V_at)touch stamp-generated

# antlr doesn't overwrite output files, it just leaves them.  So we have to delete them first.
generated/CvcLexer.hpp: Cvc.g stamp-generated
	-$(AM_V_at)rm -f $(ANTLR_STUFF)
	@if test -z "$(ANTLR)"; then echo "ERROR: antlr parser generator cannot be found, cannot generate the parser" >&2; exit 1; fi
	$(AM_V_GEN)$(ANTLR) $(ANTLR_OPTS) -fo "generated" "@srcdir@/Cvc.g"

# These don't actually depend on CvcLexer.hpp, but if we're doing parallel
# make and the lexer needs to be rebuilt, we have to keep the rules
# from running in parallel (since the token files will be deleted &
# recreated)
generated/CvcLexer.cpp generated/CvcParser.hpp generated/CvcParser.cpp $(ANTLR_TOKEN_STUFF): generated/CvcLexer.hpp
