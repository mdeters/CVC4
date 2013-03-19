/*********************                                                        */
/*! \file tptp_input.cpp
 ** \verbatim
 ** Original author: Francois Bobot
 ** Major contributors: none
 ** Minor contributors (to current version): Morgan Deters
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

#include "parser/tptp/tptp_input.h"
#include "expr/expr_manager.h"
#include "parser/input.h"
#include "parser/parser.h"
#include "parser/parser_exception.h"
#include "parser/tptp/tptp.h"
#include "parser/tptp/generated/TptpLexer.hpp"
#include "parser/tptp/generated/TptpParser.hpp"

namespace CVC4 {
namespace parser {

/* Use lookahead=2 */
TptpInput::TptpInput(AntlrInputStream<TptpLexerTraits>& inputStream) :
  AntlrInput(inputStream, 2) {
  TptpLexerTraits::InputStreamType* input = inputStream.getAntlr3InputStream();
  assert(input != NULL);

  d_pTptpLexer = new TptpLexer(input);
  if(d_pTptpLexer == NULL) {
    throw ParserException("Failed to create TPTP lexer.");
  }

  setAntlr3Lexer(d_pTptpLexer);

  TptpLexerTraits::TokenStreamType* tokenStream = getTokenStream();
  assert(tokenStream != NULL);

  d_pTptpParser = new TptpParser(tokenStream);
  if(d_pTptpParser == NULL) {
    throw ParserException("Failed to create TPTP parser.");
  }

  setAntlr3Parser(d_pTptpParser);
}


TptpInput::~TptpInput() {
  delete d_pTptpLexer;
  delete d_pTptpParser;
}

Command* TptpInput::parseCommand() {
  return d_pTptpParser->parseCommand();
}

Expr TptpInput::parseExpr() {
  return d_pTptpParser->parseExpr();
}

}/* CVC4::parser namespace */
}/* CVC4 namespace */
