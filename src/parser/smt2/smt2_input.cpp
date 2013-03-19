/*********************                                                        */
/*! \file smt2_input.cpp
 ** \verbatim
 ** Original author: Christopher L. Conway
 ** Major contributors: Morgan Deters
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

#include "parser/smt2/smt2_input.h"
#include "expr/expr_manager.h"
#include "parser/input.h"
#include "parser/parser.h"
#include "parser/parser_exception.h"
#include "parser/smt2/smt2.h"
#include "parser/smt2/generated/Smt2Lexer.hpp"
#include "parser/smt2/generated/Smt2Parser.hpp"

namespace CVC4 {
namespace parser {

/* Use lookahead=2 */
Smt2Input::Smt2Input(AntlrInputStream<Smt2LexerTraits>& inputStream) :
  AntlrInput(inputStream, 2) {
  Smt2LexerTraits::InputStreamType* input = inputStream.getAntlr3InputStream();
  assert( input != NULL );

  d_pSmt2Lexer = new Smt2Lexer(input);
  if(d_pSmt2Lexer == NULL) {
    throw ParserException("Failed to create SMT2 lexer.");
  }

  setAntlr3Lexer(d_pSmt2Lexer);

  Smt2LexerTraits::TokenStreamType* tokenStream = getTokenStream();
  assert(tokenStream != NULL);

  d_pSmt2Parser = new Smt2Parser(tokenStream);
  if(d_pSmt2Parser == NULL) {
    throw ParserException("Failed to create SMT2 parser.");
  }

  setAntlr3Parser(d_pSmt2Parser);
}


Smt2Input::~Smt2Input() {
  delete d_pSmt2Lexer;
  delete d_pSmt2Parser;
}

Command* Smt2Input::parseCommand() {
  return d_pSmt2Parser->parseCommand();
}

Expr Smt2Input::parseExpr() {
  return d_pSmt2Parser->parseExpr();
}

}/* CVC4::parser namespace */
}/* CVC4 namespace */
