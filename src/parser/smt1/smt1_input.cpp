/*********************                                                        */
/*! \file smt1_input.cpp
 ** \verbatim
 ** Original author: Morgan Deters
 ** Major contributors: Christopher L. Conway
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2014  New York University and The University of Iowa
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief [[ Add file-specific comments here ]].
 **
 ** [[ Add file-specific comments here ]]
 **/

#include <antlr3.hpp>

#include "parser/smt1/smt1_input.h"
#include "expr/expr_manager.h"
#include "parser/input.h"
#include "parser/parser.h"
#include "parser/parser_exception.h"
#include "parser/smt1/generated/Smt1Lexer.hpp"
#include "parser/smt1/generated/Smt1Parser.hpp"

namespace CVC4 {
namespace parser {

/* Use lookahead=2 */
Smt1Input::Smt1Input(AntlrInputStream<Smt1LexerTraits>& inputStream) :
  AntlrInput(inputStream, 2) {
  Smt1LexerTraits::InputStreamType* input = inputStream.getAntlr3InputStream();
  assert(input != NULL);

  d_pSmt1Lexer = new Smt1Lexer(input);
  if( d_pSmt1Lexer == NULL ) {
    throw ParserException("Failed to create SMT1 lexer.");
  }

  setAntlr3Lexer(d_pSmt1Lexer);

  Smt1LexerTraits::TokenStreamType* tokenStream = getTokenStream();
  assert(tokenStream != NULL);

  d_pSmt1Parser = new Smt1Parser(tokenStream);
  if(d_pSmt1Parser == NULL) {
    throw ParserException("Failed to create SMT1 parser.");
  }

  setAntlr3Parser(d_pSmt1Parser);
}


Smt1Input::~Smt1Input() {
  delete d_pSmt1Lexer;
  delete d_pSmt1Parser;
}

Command* Smt1Input::parseCommand() {
  return d_pSmt1Parser->parseCommand();
}

Expr Smt1Input::parseExpr() {
  return d_pSmt1Parser->parseExpr();
}

}/* CVC4::parser namespace */
}/* CVC4 namespace */
