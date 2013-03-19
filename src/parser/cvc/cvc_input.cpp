/*********************                                                        */
/*! \file cvc_input.cpp
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

#include "expr/expr_manager.h"
#include "parser/antlr_input.h"
#include "parser/parser_exception.h"
#include "parser/cvc/cvc_input.h"
#include "parser/cvc/generated/CvcLexer.hpp"
#include "parser/cvc/generated/CvcParser.hpp"

namespace CVC4 {
namespace parser {

/* Use lookahead=3 */
CvcInput::CvcInput(AntlrInputStream<CvcLexerTraits>& inputStream) :
  AntlrInput(inputStream, 6) {
  CvcLexerTraits::InputStreamType* input = inputStream.getAntlr3InputStream();
  assert(input != NULL);

  d_pCvcLexer = new CvcLexer(input);
  if(d_pCvcLexer == NULL) {
    throw ParserException("Failed to create CVC lexer.");
  }

  setAntlr3Lexer(d_pCvcLexer);

  CvcLexerTraits::TokenStreamType* tokenStream = getTokenStream();
  assert(tokenStream != NULL);

  d_pCvcParser = new CvcParser(tokenStream);
  if(d_pCvcParser == NULL) {
    throw ParserException("Failed to create CVC parser.");
  }

  setAntlr3Parser(d_pCvcParser);
}


CvcInput::~CvcInput() {
  delete d_pCvcLexer;
  delete d_pCvcParser;
}

Command* CvcInput::parseCommand() {
  return d_pCvcParser->parseCommand();
}

Expr CvcInput::parseExpr() {
  return d_pCvcParser->parseExpr();
}

}/* CVC4::parser namespace */
}/* CVC4 namespace */
