AM_CPPFLAGS = \
	-D__BUILDING_CVC4LIB \
	-I@builddir@/.. -I@srcdir@/../include -I@srcdir@/..
AM_CXXFLAGS = -Wall -Wno-unknown-pragmas $(FLAG_VISIBILITY_HIDDEN)

# These are subdirs without build interdependences that can in principle
# all be built in parallel.  They are built at the time of ".".
PARALLEL_SUBDIRS = builtin booleans uf arith bv arrays datatypes quantifiers rewriterules idl
# Unfortunately, to ensure that automake does the correct thing in other
# recursive cases, e.g., make clean or make dist, we have to repeat the
# PARALLEL_SUBDIRS in SUBDIRS---otherwise PARALLEL_SUBDIRS are being hidden
# from automake and bad things happen.
SUBDIRS = $(PARALLEL_SUBDIRS)

# also distribute the "example" theory
DIST_SUBDIRS = $(SUBDIRS) example

noinst_LTLIBRARIES = libtheory.la

libtheory_la_SOURCES = \
	decision_attributes.h \
	logic_info.h \
	logic_info.cpp \
	output_channel.h \
	interrupted.h \
	type_enumerator.h \
	theory_engine.h \
	theory_engine.cpp \
	theory_test_utils.h \
	theory.h \
	theory.cpp \
	theoryof_mode.h \
	theory_registrar.h \
	rewriter.h \
	rewriter_attributes.h \
	rewriter.cpp \
	substitutions.h \
	substitutions.cpp \
	valuation.h \
	valuation.cpp \
	shared_terms_database.h \
	shared_terms_database.cpp \
	term_registration_visitor.h \
	term_registration_visitor.cpp \
	ite_simplifier.h \
	ite_simplifier.cpp \
	unconstrained_simplifier.h \
	unconstrained_simplifier.cpp \
	quantifiers_engine.h \
	quantifiers_engine.cpp \
	model.h \
	model.cpp \
	rep_set.h \
	rep_set.cpp \
	atom_requests.h \
	atom_requests.cpp

nodist_libtheory_la_SOURCES = \
	rewriter_tables.h \
	theory_traits.h \
	type_enumerator.cpp

libtheory_la_LIBADD = \
	@builddir@/builtin/libbuiltin.la \
	@builddir@/booleans/libbooleans.la \
	@builddir@/uf/libuf.la \
	@builddir@/arith/libarith.la \
	@builddir@/arrays/libarrays.la \
	@builddir@/bv/libbv.la \
	@builddir@/datatypes/libdatatypes.la \
	@builddir@/quantifiers/libquantifiers.la \
	@builddir@/rewriterules/librewriterules.la \
	@builddir@/idl/libidl.la

.PHONY: always-rebuild
$(PARALLEL_SUBDIRS:%=%/lib%.la): always-rebuild
	@dir="$(firstword $(subst /, ,$@))"; \
	echo "Making all in $$dir"; \
	$(MAKE) -C "$$dir" all

EXTRA_DIST = \
	logic_info.i \
	options_handlers.h \
	rewriter_tables_template.h \
	theory_traits_template.h \
	type_enumerator_template.cpp \
	mktheorytraits \
	mkrewriter \
	Makefile.subdirs

BUILT_SOURCES = \
	rewriter_tables.h \
	theory_traits.h \
	type_enumerator.cpp

CLEANFILES = \
	rewriter_tables.h \
	theory_traits.h \
	type_enumerator.cpp

include @top_srcdir@/src/theory/Makefile.subdirs

rewriter_tables.h: rewriter_tables_template.h mkrewriter @top_builddir@/src/theory/.subdirs @top_srcdir@/src/theory/*/kinds
	$(AM_V_at)chmod +x @srcdir@/mkrewriter
	$(AM_V_at)$(am__mv) $@ $@~ 2>/dev/null || true
	$(AM_V_GEN)(@srcdir@/mkrewriter \
		$< \
		`cat @top_builddir@/src/theory/.subdirs` \
	> $@) || (rm -f $@ && exit 1)

theory_traits.h: theory_traits_template.h mktheorytraits @top_builddir@/src/theory/.subdirs @top_srcdir@/src/theory/*/kinds
	$(AM_V_at)chmod +x @srcdir@/mktheorytraits
	$(AM_V_at)$(am__mv) $@ $@~ 2>/dev/null || true
	$(AM_V_GEN)(@srcdir@/mktheorytraits \
		$< \
		`cat @top_builddir@/src/theory/.subdirs` \
	> $@) || (rm -f $@ && exit 1)

type_enumerator.cpp: type_enumerator_template.cpp mktheorytraits @top_builddir@/src/theory/.subdirs @top_srcdir@/src/theory/*/kinds
	$(AM_V_at)chmod +x @srcdir@/mktheorytraits
	$(AM_V_at)$(am__mv) $@ $@~ 2>/dev/null || true
	$(AM_V_GEN)(@srcdir@/mktheorytraits \
		$< \
		`cat @top_builddir@/src/theory/.subdirs` \
	> $@) || (rm -f $@ && exit 1)
