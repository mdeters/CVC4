/*********************                                                        */
/*! \file mjollnir_input.cpp
 ** \verbatim
 ** Original author: mdeters
 ** Major contributors: none
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009-2012  New York University and The University of Iowa
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief [[ Add file-specific comments here ]].
 **
 ** [[ Add file-specific comments here ]]
 **/

#include <antlr3.h>

#include "parser/mjollnir/mjollnir_input.h"
#include "expr/expr_manager.h"
#include "parser/input.h"
#include "parser/parser.h"
#include "parser/parser_exception.h"
#include "parser/mjollnir/generated/MjollnirLexer.h"
#include "parser/mjollnir/generated/MjollnirParser.h"

namespace CVC4 {
namespace parser {

/* Use lookahead=2 */
MjollnirInput::MjollnirInput(AntlrInputStream& inputStream) :
  AntlrInput(inputStream, 2) {
  pANTLR3_INPUT_STREAM input = inputStream.getAntlr3InputStream();
  assert( input != NULL );

  d_pMjollnirLexer = MjollnirLexerNew(input);
  if( d_pMjollnirLexer == NULL ) {
    throw ParserException("Failed to create MJOLLNIR lexer.");
  }

  setAntlr3Lexer( d_pMjollnirLexer->pLexer );

  pANTLR3_COMMON_TOKEN_STREAM tokenStream = getTokenStream();
  assert( tokenStream != NULL );

  d_pMjollnirParser = MjollnirParserNew(tokenStream);
  if( d_pMjollnirParser == NULL ) {
    throw ParserException("Failed to create MJOLLNIR parser.");
  }

  setAntlr3Parser(d_pMjollnirParser->pParser);
}


MjollnirInput::~MjollnirInput() {
  d_pMjollnirLexer->free(d_pMjollnirLexer);
  d_pMjollnirParser->free(d_pMjollnirParser);
}

Command* MjollnirInput::parseCommand()
  throw (ParserException, TypeCheckingException) {
  return d_pMjollnirParser->parseCommand(d_pMjollnirParser);
}

Expr MjollnirInput::parseExpr()
  throw (ParserException, TypeCheckingException) {
  return d_pMjollnirParser->parseExpr(d_pMjollnirParser);
}

}/* CVC4::parser namespace */
}/* CVC4 namespace */
