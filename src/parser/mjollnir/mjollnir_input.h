/*********************                                                        */
/*! \file smt2_input.h
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

#include "cvc4parser_private.h"

#ifndef __CVC4__PARSER__MJOLLNIR_INPUT_H
#define __CVC4__PARSER__MJOLLNIR_INPUT_H

#include "parser/antlr_input.h"
#include "parser/mjollnir/generated/MjollnirLexer.h"
#include "parser/mjollnir/generated/MjollnirParser.h"

namespace CVC4 {

class Command;
class Expr;
class ExprManager;

namespace parser {

class MjollnirInput : public AntlrInput {
  typedef AntlrInput super;

  /** The ANTLR3 Mjollnir lexer for the input. */
  pMjollnirLexer d_pMjollnirLexer;

  /** The ANTLR3 Mjollnir parser for the input. */
  pMjollnirParser d_pMjollnirParser;

  /**
   * Initialize the class. Called from the constructors once the input
   * stream is initialized.
   */
  void init();

public:

  /**
   * Create an input.
   *
   * @param inputStream the input stream to use
   */
  MjollnirInput(AntlrInputStream& inputStream);

  /** Destructor. Frees the lexer and the parser. */
  virtual ~MjollnirInput();

  /** Get the language that this Input is reading. */
  InputLanguage getLanguage() const throw() {
    return language::input::LANG_MJOLLNIR;
  }

protected:

  /**
   * Parse a command from the input. Returns <code>NULL</code> if
   * there is no command there to parse.
   *
   * @throws ParserException if an error is encountered during parsing.
   */
  Command* parseCommand()
    throw(ParserException, TypeCheckingException);

  /**
   * Parse an expression from the input. Returns a null
   * <code>Expr</code> if there is no expression there to parse.
   *
   * @throws ParserException if an error is encountered during parsing.
   */
  Expr parseExpr()
    throw(ParserException, TypeCheckingException);

};/* class MjollnirInput */

}/* CVC4::parser namespace */
}/* CVC4 namespace */

#endif /* __CVC4__PARSER__MJOLLNIR_INPUT_H */
