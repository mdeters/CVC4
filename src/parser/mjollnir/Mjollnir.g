/* *******************                                                        */
/*! \file Mjollnir.g
 ** \verbatim
 ** Original author: mdeters
 ** Major contributors: none
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009-2012  New York University and The University of Iowa
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Parser for Mjollnir's language
 **
 ** Parser for Mjollnir's language.  See David Monniaux, "Quantifier
 ** Elimination by Lazy Model Enumeration," in CAV 2010.  This parser
 ** is used to parse his benchmarks (and has some minor extensions),
 ** available here:
 ** http://www-verimag.imag.fr/~monniaux/download/Mjollnir_examples_CAV2010.zip
 **/

grammar Mjollnir;

options {
  // C output for antlr
  language = 'C';

  // Skip the default error handling, just break with exceptions
  // defaultErrorHandler = false;

  // Only lookahead of <= k requested (disable for LL* parsing)
  // Note that CVC4's BoundedTokenBuffer requires a fixed k !
  // If you change this k, change it also in mjollnir_input.cpp !
  k = 2;
}/* options */

tokens {
  T_FORALL = 'ForAll';
  T_EXISTS = 'Exists';
  T_AND = 'And';
  T_OR = 'Or';
  T_NOT = 'Not';
}/* tokens */

@header {
/**
 ** This file is part of CVC4.
 ** Copyright (c) 2009-2012  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.
 **/
}/* @header */

@lexer::includes {

/** This suppresses warnings about the redefinition of token symbols between different
  * parsers. The redefinitions should be harmless as long as no client: (a) #include's
  * the lexer headers for two grammars AND (b) uses the token symbol definitions. */
#pragma GCC system_header

#if defined(CVC4_COMPETITION_MODE) && !defined(CVC4_SMTCOMP_APPLICATION_TRACK)
/* This improves performance by ~10 percent on big inputs.
 * This option is only valid if we know the input is ASCII (or some 8-bit encoding).
 * If we know the input is UTF-16, we can use ANTLR3_INLINE_INPUT_UTF16.
 * Otherwise, we have to let the lexer detect the encoding at runtime.
 */
#  define ANTLR3_INLINE_INPUT_ASCII
#  define ANTLR3_INLINE_INPUT_8BIT
#endif /* CVC4_COMPETITION_MODE && !CVC4_SMTCOMP_APPLICATION_TRACK */

#include "parser/antlr_tracing.h"
#include "util/integer.h"
#include "parser/antlr_input.h"
#include "parser/parser.h"

}/* @lexer::includes */

@parser::includes {

#include <stdint.h>
#include <cassert>
#include "expr/command.h"
#include "parser/parser.h"
#include "util/subrange_bound.h"
#include "parser/antlr_tracing.h"

}/* @parser::includes */

@parser::postinclude {

#include "expr/expr.h"
#include "expr/kind.h"
#include "expr/type.h"
#include "parser/antlr_input.h"
#include "parser/parser.h"
#include "util/output.h"

#include <vector>
#include <string>
#include <sstream>

using namespace CVC4;
using namespace CVC4::parser;

/* These need to be macros so they can refer to the PARSER macro, which will be defined
 * by ANTLR *after* this section. (If they were functions, PARSER would be undefined.) */
#undef PARSER_STATE
#define PARSER_STATE ((Parser*)PARSER->super)
#undef EXPR_MANAGER
#define EXPR_MANAGER PARSER_STATE->getExprManager()
#undef MK_EXPR
#define MK_EXPR EXPR_MANAGER->mkExpr
#undef MK_CONST
#define MK_CONST EXPR_MANAGER->mkConst
#define UNSUPPORTED PARSER_STATE->unimplementedFeature

}/* @parser::postinclude */

parseCommand returns [CVC4::Command* cmd = NULL]
@init {
  Expr e;
}
  : formula[e] { cmd = new CheckSatCommand(e); }
  | EOF
  ;

parseExpr returns [CVC4::Expr expr = CVC4::Expr()]
  : formula[expr]
  | EOF
  ;

formula[CVC4::Expr& f]
@init {
  Expr e;
}
  : T_FORALL '[' bindvars[e] ',' formula[f] ']'
    { f = EXPR_MANAGER->mkExpr(kind::FORALL, e, f); }
  | T_EXISTS '[' bindvars[e] ',' formula[f] ']'
    { f = EXPR_MANAGER->mkExpr(kind::EXISTS, e, f); }
  | T_AND '[' formula[e] ',' formula[f] ']'
    { f = EXPR_MANAGER->mkExpr(kind::AND, e, f); }
  | T_OR '[' formula[e] ',' formula[f] ']'
    { f = EXPR_MANAGER->mkExpr(kind::OR, e, f); }
  | T_NOT '[' formula[f] ']'
    { f = EXPR_MANAGER->mkExpr(kind::NOT, f); }
  | sum[e]
    ( '<=' sum[f]
      { f = EXPR_MANAGER->mkExpr(kind::LEQ, e, f); }
    | '>=' sum[f]
      { f = EXPR_MANAGER->mkExpr(kind::GEQ, e, f); }
    | '<' sum[f]
      { f = EXPR_MANAGER->mkExpr(kind::LT, e, f); }
    | '>' sum[f]
      { f = EXPR_MANAGER->mkExpr(kind::GT, e, f); }
    | '=' sum[f]
      { f = EXPR_MANAGER->mkExpr(kind::EQUAL, e, f); }
    | '!=' sum[f]
      { f = EXPR_MANAGER->mkExpr(kind::EQUAL, e, f).notExpr(); }
    )
  ;

sum[CVC4::Expr& e]
@init {
  Expr e2;
}
  : product[e]
    ( '+' sum[e2] { e = EXPR_MANAGER->mkExpr(kind::PLUS, e, e2); }
    | '-' sum[e2] { e = EXPR_MANAGER->mkExpr(kind::MINUS, e, e2); } )?
  ;

product[CVC4::Expr& e]
@init {
  Expr e2;
}
  : constant[e]
    ( '*' var[e2] { e = EXPR_MANAGER->mkExpr(kind::MULT, e, e2); } )?
  | var[e]
    ( '*' constant[e2] { e = EXPR_MANAGER->mkExpr(kind::MULT, e, e2); } )?
  | '(' sum[e] ')'
  ;

bindvars[CVC4::Expr& f]
@init {
  std::vector<Expr> vars;
}
  : IDENTIFIER
    { f = EXPR_MANAGER->mkExpr(kind::BOUND_VAR_LIST, PARSER_STATE->mkBoundVar(AntlrInput::tokenText($IDENTIFIER), EXPR_MANAGER->integerType())); }
  | '{' v1=IDENTIFIER { vars.push_back(PARSER_STATE->mkBoundVar(AntlrInput::tokenText($v1), EXPR_MANAGER->integerType())); }
    ( ',' v2=IDENTIFIER { vars.push_back(PARSER_STATE->mkBoundVar(AntlrInput::tokenText($v2), EXPR_MANAGER->integerType())); } )* '}'
    { f = EXPR_MANAGER->mkExpr(kind::BOUND_VAR_LIST, vars); }
  ;

var[CVC4::Expr& f]
  : IDENTIFIER
    { f = PARSER_STATE->getVariable(AntlrInput::tokenText($IDENTIFIER)); }
  | '-' IDENTIFIER
    { f = EXPR_MANAGER->mkExpr(kind::UMINUS, PARSER_STATE->getVariable(AntlrInput::tokenText($IDENTIFIER))); }
  ;

constant[CVC4::Expr& c]
  : INTEGER_LITERAL
    { c = EXPR_MANAGER->mkConst(Rational(AntlrInput::tokenText($INTEGER_LITERAL))); }
  | '-' INTEGER_LITERAL
    { c = EXPR_MANAGER->mkConst(-Rational(AntlrInput::tokenText($INTEGER_LITERAL))); }
  ;

/**
 * An integer.
 */
INTEGER_LITERAL : DIGIT+;

/**
 * Matches an identifier from the input.
 */
IDENTIFIER : (ALPHA | '_') (ALPHA | DIGIT | '_' | '\'')*;

/**
 * Matches any letter ('a'-'z' and 'A'-'Z').
 */
fragment ALPHA : 'a'..'z' | 'A'..'Z';

/**
 * Matches the decimal digits (0-9)
 */
fragment DIGIT : '0'..'9';

/**
 * Matches and skips whitespace in the input and ignores it.
 */
WHITESPACE : (' ' | '\t' | '\f' | '\r' | '\n')+ { SKIP(); };

/**
 * Matches the comments and ignores them
 */
COMMENT : '#' (~('\n' | '\r'))* { SKIP(); };
