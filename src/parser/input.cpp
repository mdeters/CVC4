/*********************                                                        */
/*! \file input.cpp
 ** \verbatim
 ** Original author: Christopher L. Conway
 ** Major contributors: Morgan Deters
 ** Minor contributors (to current version): Dejan Jovanovic
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2014  New York University and The University of Iowa
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief A super-class for input language parsers.
 **
 ** A super-class for input language parsers
 **/

#include "parser/input.h"
#include "parser/parser_exception.h"
#include "parser/parser.h"

#include "expr/command.h"
#include "expr/type.h"
#include "parser/antlr_input.h"
#include "util/output.h"

#include "parser/cvc/generated/CvcLexer.hpp"
#include "parser/smt1/generated/Smt1Lexer.hpp"
#include "parser/smt2/generated/Smt2Lexer.hpp"
#include "parser/tptp/generated/TptpLexer.hpp"

using namespace std;
using namespace CVC4;
using namespace CVC4::parser;
using namespace CVC4::kind;

namespace CVC4 {
namespace parser {

InputStreamException::InputStreamException(const std::string& msg) :
  Exception(msg) {
}

const std::string InputStream::getName() const {
  return d_name;
}

Input::Input(InputStream& inputStream) :
    d_inputStream( &inputStream ) {
}

Input::~Input() {
  delete d_inputStream;
}

InputStream *Input::getInputStream() {
  return d_inputStream;
}

Input* Input::newFileInput(InputLanguage lang,
                           const std::string& filename,
                           bool useMmap)
  throw (InputStreamException) {
  switch(lang) {
  case language::input::LANG_CVC4: {
    AntlrInputStream<CvcLexerTraits>* inputStream =
      AntlrInputStream<CvcLexerTraits>::newFileInputStream(filename, useMmap);
    return new CvcTraits<void>::InputType(*inputStream);
  }
  case language::input::LANG_SMTLIB_V1: {
    AntlrInputStream<Smt1LexerTraits>* inputStream =
      AntlrInputStream<Smt1LexerTraits>::newFileInputStream(filename, useMmap);
    return new Smt1Traits<void>::InputType(*inputStream);
  }
  case language::input::LANG_SMTLIB_V2: {
    AntlrInputStream<Smt2LexerTraits>* inputStream =
      AntlrInputStream<Smt2LexerTraits>::newFileInputStream(filename, useMmap);
    return new Smt2Traits<void>::InputType(*inputStream);
  }
  case language::input::LANG_TPTP: {
    AntlrInputStream<TptpLexerTraits>* inputStream =
      AntlrInputStream<TptpLexerTraits>::newFileInputStream(filename, useMmap);
    return new TptpTraits<void>::InputType(*inputStream);
  }
  default:
    Warning() << "not supported: " << lang << std::endl;
    assert(false);
  }
}

Input* Input::newStreamInput(InputLanguage lang,
                             std::istream& input,
                             const std::string& name,
                             bool lineBuffered)
  throw (InputStreamException) {
  switch(lang) {
  case language::input::LANG_CVC4: {
    AntlrInputStream<CvcLexerTraits>* inputStream =
      AntlrInputStream<CvcLexerTraits>::newStreamInputStream(input, name, lineBuffered);
    return new CvcTraits<void>::InputType(*inputStream);
  }
  case language::input::LANG_SMTLIB_V1: {
    AntlrInputStream<Smt1LexerTraits>* inputStream =
      AntlrInputStream<Smt1LexerTraits>::newStreamInputStream(input, name, lineBuffered);
    return new Smt1Traits<void>::InputType(*inputStream);
  }
  case language::input::LANG_SMTLIB_V2: {
    AntlrInputStream<Smt2LexerTraits>* inputStream =
      AntlrInputStream<Smt2LexerTraits>::newStreamInputStream(input, name, lineBuffered);
    return new Smt2Traits<void>::InputType(*inputStream);
  }
  case language::input::LANG_TPTP: {
    AntlrInputStream<TptpLexerTraits>* inputStream =
      AntlrInputStream<TptpLexerTraits>::newStreamInputStream(input, name, lineBuffered);
    return new TptpTraits<void>::InputType(*inputStream);
  }
  default:
    Warning() << "not supported: " << lang << std::endl;
    assert(false);
  }
}

Input* Input::newStringInput(InputLanguage lang,
                             const std::string& str,
                             const std::string& name)
  throw (InputStreamException) {
  switch(lang) {
  case language::input::LANG_CVC4: {
    AntlrInputStream<CvcLexerTraits>* inputStream =
      AntlrInputStream<CvcLexerTraits>::newStringInputStream(str, name);
    return new CvcTraits<void>::InputType(*inputStream);
  }
  case language::input::LANG_SMTLIB_V1: {
    AntlrInputStream<Smt1LexerTraits>* inputStream =
      AntlrInputStream<Smt1LexerTraits>::newStringInputStream(str, name);
    return new Smt1Traits<void>::InputType(*inputStream);
  }
  case language::input::LANG_SMTLIB_V2: {
    AntlrInputStream<Smt2LexerTraits>* inputStream =
      AntlrInputStream<Smt2LexerTraits>::newStringInputStream(str, name);
    return new Smt2Traits<void>::InputType(*inputStream);
  }
  case language::input::LANG_TPTP: {
    AntlrInputStream<TptpLexerTraits>* inputStream =
      AntlrInputStream<TptpLexerTraits>::newStringInputStream(str, name);
    return new TptpTraits<void>::InputType(*inputStream);
  }
  default:
    Warning() << "not supported: " << lang << std::endl;
    assert(false);
  }
}

}/* CVC4::parser namespace */
}/* CVC4 namespace */
