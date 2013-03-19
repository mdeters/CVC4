/*********************                                                        */
/*! \file antlr_input.h
 ** \verbatim
 ** Original author: Christopher L. Conway
 ** Major contributors: Morgan Deters
 ** Minor contributors (to current version): Tim King, Francois Bobot, Dejan Jovanovic
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2014  New York University and The University of Iowa
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Base for ANTLR parser classes
 **
 ** Base for ANTLR parser classes.
 **/

#include <antlr3.h>

// ANTLR3 headers define these in our space :(
// undef them so that we don't get multiple-definition warnings
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include "cvc4parser_private.h"

#ifndef __CVC4__PARSER__ANTLR_INPUT_H
#define __CVC4__PARSER__ANTLR_INPUT_H

#include <antlr3.hpp>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cassert>

#include "parser/parser_exception.h"
#include "parser/input.h"

#include "util/bitvector.h"
#include "util/integer.h"
#include "util/rational.h"
#include "util/output.h"

namespace CVC4 {

class Command;
class Type;
class FunctionType;

namespace parser {

/** Wrapper around an ANTLR3 input stream. */
template <class ImplTraits>
class AntlrInputStream : public InputStream {
  typename ImplTraits::InputStreamType* d_input;

  AntlrInputStream(std::string name,
                   typename ImplTraits::InputStreamType* input,
                   bool fileIsTemporary = false);

  /* This is private and unimplemented, because you should never use it. */
  AntlrInputStream(const AntlrInputStream& inputStream) CVC4_UNUSED;

  /* This is private and unimplemented, because you should never use it. */
  AntlrInputStream& operator=(const AntlrInputStream& inputStream) CVC4_UNUSED;

public:

  virtual ~AntlrInputStream();

  typename ImplTraits::InputStreamType* getAntlr3InputStream() const;

  /** Create a file input.
   *
   * @param name the path of the file to read
   * @param useMmap <code>true</code> if the input should use memory-mapped I/O; otherwise, the
   * input will use the standard ANTLR3 I/O implementation.
   */
  static AntlrInputStream* newFileInputStream(const std::string& name, 
                                              bool useMmap = false)
    throw (InputStreamException);

  /** Create an input from an istream. */
  static AntlrInputStream* newStreamInputStream(std::istream& input, 
                                                const std::string& name,
                                                bool lineBuffered = false)
    throw (InputStreamException);

  /** Create a string input.
   *
   * @param input the string to read
   * @param name the "filename" to use when reporting errors
   */
  static AntlrInputStream* newStringInputStream(const std::string& input, 
                                                const std::string& name)
    throw (InputStreamException);
};/* class AntlrInputStream */

class Parser;

/**
 * An input to be parsed. The static factory methods in this class (e.g.,
 * <code>newFileInput</code>, <code>newStringInput</code>) create a parser
 * for the given input language and attach it to an input source of the
 * appropriate type.
 */
template <class ImplTraits>
class AntlrInput : public Input {
  /** The token lookahead used to lex and parse the input. This should usually be equal to
   * <code>K</code> for an LL(k) grammar. */
  unsigned int d_lookahead;

  /** The ANTLR3 lexer associated with this input. This will be <code>NULL</code> initially. It
   *  must be set by a call to <code>setLexer</code>, preferably in the subclass constructor. */
  typename ImplTraits::BaseLexerType* d_lexer;

  /** The ANTLR3 parser associated with this input. This will be <code>NULL</code> initially. It
   *  must be set by a call to <code>setParser</code>, preferably in the subclass constructor.
   *  The <code>super</code> field of <code>d_parser</code> will be set to <code>this</code> and
   *  <code>reportError</code> will be set to <code>Input::reportError</code>. */
  typename ImplTraits::BaseParserType* d_parser;

  /** The ANTLR3 input stream associated with this input. */
  typename ImplTraits::InputStreamType* d_antlr3InputStream;

  /** The ANTLR3 bounded token buffer associated with this input.
   *  We only need this so we can free it on exit.
   *  This is set by <code>setLexer</code>.
   *  NOTE: We assume that we <em>can</em> free it on exit. No sharing! */
  //pBOUNDED_TOKEN_BUFFER d_tokenBuffer;

  /** Turns an ANTLR3 exception into a message for the user and calls <code>parseError</code>. */
  static void reportError(typename ImplTraits::template RecognizerType<typename ImplTraits::TokenStreamType>* recognizer);

  /** Builds a message for a lexer error and calls <code>parseError</code>. */
  static void lexerError(typename ImplTraits::template RecognizerType<typename ImplTraits::TokenStreamType>* recognizer);

  /** Returns the next available lexer token from the current input stream. */
  /* - auxillary function */
  static typename ImplTraits::CommonTokenType*
  nextTokenStr (typename ImplTraits::TokenSourceType* toksource);
  /* - main function */
  static typename ImplTraits::CommonTokenType*
  nextToken (typename ImplTraits::TokenSourceType* toksource);

  /* Since we own d_tokenStream and it needs to be freed, we need to prevent
   * copy construction and assignment.
   */
  AntlrInput(const AntlrInput& input);
  AntlrInput& operator=(const AntlrInput& input);

public:

  /** Destructor. Frees the token stream and closes the input. */
  virtual ~AntlrInput();

  /** Retrieve the text associated with a token. */
  static std::string tokenText(const typename ImplTraits::CommonTokenType* token);

  /** Retrieve a substring of the text associated with a token.
   *
   * @param token the token
   * @param index the index of the starting character of the substring
   * @param n the size of the substring. If <code>n</code> is 0, then all of the
   * characters up to the end of the token text will be included. If <code>n</code>
   * would make the substring span past the end of the token text, only those
   * characters up to the end of the token text will be included.
   */
  static std::string tokenTextSubstr(const typename ImplTraits::CommonTokenType* token, size_t index, size_t n = 0);

  /** Retrieve an unsigned from the text of a token */
  static unsigned tokenToUnsigned(const typename ImplTraits::CommonTokenType* token);

  /** Retrieve an Integer from the text of a token */
  static Rational tokenToInteger(const typename ImplTraits::CommonTokenType* token);

  /** Retrieve a Rational from the text of a token */
  static Rational tokenToRational(const typename ImplTraits::CommonTokenType* token);

  /** Get a bitvector constant from the text of the number and the size token */
  static BitVector tokenToBitvector(const typename ImplTraits::CommonTokenType* number, const typename ImplTraits::CommonTokenType* size);

  /** Retrieve the remaining text in this input. */
  std::string getUnparsedText();

  /** Get the ANTLR3 lexer for this input. */
  typename ImplTraits::BaseLexerType* getAntlr3Lexer() { return d_lexer; }

  typename ImplTraits::InputStreamType* getAntlr3InputStream() { return d_antlr3InputStream; }
protected:
  /** Create an input. This input takes ownership of the given input stream,
   * and will delete it at destruction time.
   *
   * @param inputStream the input stream to use
   * @param lookahead the lookahead needed to parse the input (i.e., k for
   * an LL(k) grammar)
   */
  AntlrInput(AntlrInputStream<ImplTraits>& inputStream, unsigned int lookahead);

  /** Retrieve the token stream for this parser. Must not be called before
   * <code>setLexer()</code>. */
  typename ImplTraits::TokenStreamType* getTokenStream();

  /**
   * Issue a non-fatal warning to the user with file, line, and column info.
   */
  void warning(const std::string& msg);

  /**
   * Throws a <code>ParserException</code> with the given message.
   */
  void parseError(const std::string& msg, bool eofException = false)
    throw (ParserException);

  /** Set the ANTLR3 lexer for this input. */
  void setAntlr3Lexer(typename ImplTraits::BaseLexerType* pLexer);

  /** Set the ANTLR3 parser implementation for this input. */
  void setAntlr3Parser(typename ImplTraits::BaseParserType* pParser);

  /** Set the Parser object for this input. */
  void setParser(Parser& parser);
};/* class AntlrInput */

template <class ImplTraits>
inline std::string AntlrInput<ImplTraits>::getUnparsedText() {
  const char* base = (const char*) d_antlr3InputStream->get_data();
  const char* cur = (const char*) d_antlr3InputStream->get_nextChar();

  return std::string(cur, d_antlr3InputStream->get_sizeBuf() - (cur - base));
}

template <class ImplTraits>
inline std::string AntlrInput<ImplTraits>::tokenText(const typename ImplTraits::CommonTokenType* token) {
  if(token->get_type() == ImplTraits::CommonTokenType::TOKEN_EOF) {
    return "<<EOF>>";
  }

  ANTLR_MARKER start = token->get_startIndex();
  ANTLR_MARKER end = token->get_stopIndex();
  /* start and end are boundary pointers. The text is a string
   * of (end-start+1) bytes beginning at start. */
  std::string txt((const char*) start, end - start + 1);
  Debug("parser-extra") << "tokenText: start=" << start << std::endl
                        <<  "end=" << end << std::endl
                        <<  "txt='" << txt << "'" << std::endl;
  return txt;
}

template <class ImplTraits>
inline std::string AntlrInput<ImplTraits>::tokenTextSubstr(const typename ImplTraits::CommonTokenType* token,
                                                           size_t index,
                                                           size_t n) {

  ANTLR_MARKER start = token->get_startIndex();
  // Its the last character of the token (not the one just after)
  ANTLR_MARKER end = token->get_stopIndex();
  assert( start < end );
  if( index > (size_t) end - start ) {
    std::stringstream ss;
    ss << "Out-of-bounds substring index: " << index;
    throw std::invalid_argument(ss.str());
  }
  start += index;
  if( n==0 || n > (size_t) end - start ) {
    return std::string( (const char*) start, end-start+1 );
  } else {
    return std::string( (const char*) start, n );
  }
}

template <class ImplTraits>
inline unsigned AntlrInput<ImplTraits>::tokenToUnsigned(const typename ImplTraits::CommonTokenType* token) {
  unsigned result;
  std::stringstream ss;
  ss << tokenText(token);
  ss >> result;
  return result;
}

template <class ImplTraits>
inline Rational AntlrInput<ImplTraits>::tokenToInteger(const typename ImplTraits::CommonTokenType* token) {
  return Rational( tokenText(token) );
}

template <class ImplTraits>
inline Rational AntlrInput<ImplTraits>::tokenToRational(const typename ImplTraits::CommonTokenType* token) {
  return Rational::fromDecimal( tokenText(token) );
}

template <class ImplTraits>
inline BitVector AntlrInput<ImplTraits>::tokenToBitvector(const typename ImplTraits::CommonTokenType* number, const typename ImplTraits::CommonTokenType* size) {
  std::string number_str = tokenTextSubstr(number, 2);
  unsigned sz = tokenToUnsigned(size);
  Integer val(number_str);
  if(val.modByPow2(sz) != val) {
    std::stringstream ss;
    ss << "Overflow in bitvector construction (specified bitvector size " << sz << " too small to hold value " << tokenText(number) << ")";
    throw std::invalid_argument(ss.str());
  }
  return BitVector(sz, val);
}

}/* CVC4::parser namespace */
}/* CVC4 namespace */

#include "parser/cvc/cvc_input.h"
#include "parser/smt1/smt1_input.h"
#include "parser/smt2/smt2_input.h"
#include "parser/tptp/tptp_input.h"

namespace CVC4 {
namespace parser {

template <class ImplTraits>
AntlrInputStream<ImplTraits>::AntlrInputStream(std::string name,
                                               typename ImplTraits::InputStreamType* input,
                                               bool fileIsTemporary) :
  InputStream(name, fileIsTemporary),
  d_input(input) {
  assert(input != NULL);
  input->set_fileName(name);
}

template <class ImplTraits>
AntlrInputStream<ImplTraits>::~AntlrInputStream() {
  delete d_input;
}

template <class ImplTraits>
typename ImplTraits::InputStreamType* AntlrInputStream<ImplTraits>::getAntlr3InputStream() const {
  return d_input;
}

template <class ImplTraits>
AntlrInputStream<ImplTraits>*
AntlrInputStream<ImplTraits>::newFileInputStream(const std::string& name,
                                     bool useMmap)
  throw (InputStreamException) {
#ifdef _WIN32
  if(useMmap) {
    useMmap = false;
  }
#endif
  typename ImplTraits::InputStreamType* input = NULL;
  if(useMmap) {
#warning support mmap?
    //input = MemoryMappedInputBufferNew(name);
  } else {
    // libantlr3c v3.2 isn't source-compatible with v3.4
    input = new typename ImplTraits::InputStreamType((ANTLR_UINT8*) name.c_str(), ANTLR_ENC_8BIT);
  }
  if(input == NULL) {
    throw InputStreamException("Couldn't open file: " + name);
  }
  return new AntlrInputStream<ImplTraits>(name, input);
}

template <class ImplTraits>
AntlrInputStream<ImplTraits>*
AntlrInputStream<ImplTraits>::newStreamInputStream(std::istream& input,
                                                   const std::string& name,
                                                   bool lineBuffered)
  throw (InputStreamException) {

  typename ImplTraits::InputStreamType* inputStream = NULL;

#warning line buffered
  /*
  if(lineBuffered) {
    inputStream =
      antlr3LineBufferedStreamNew(input,
                                  ANTLR_ENC_8BIT,
                                  (ANTLR_UINT8*) strdup(name.c_str()));
  } else {
  */
    // Since these are all NULL on entry, realloc will be called
    char *basep = NULL, *boundp = NULL, *cp = NULL;
    /* 64KB seems like a reasonable default size. */
    size_t bufSize = 0x10000;

    /* Keep going until we can't go no more. */
    while( !input.eof() && !input.fail() ) {

      if( cp == boundp ) {
        /* We ran out of room in the buffer. Realloc at double the size. */
        ptrdiff_t offset = cp - basep;
        basep = (char *) realloc(basep, bufSize);
        if( basep == NULL ) {
          throw InputStreamException("Failed buffering input stream: " + name);
        }
        cp = basep + offset;
        boundp = basep + bufSize;
        bufSize *= 2;
      }

      /* Read as much as we have room for. */
      input.read( cp, boundp - cp );
      cp += input.gcount();
    }

    /* Make sure the fail bit didn't get set. */
    if(!input.eof()) {
      throw InputStreamException("Stream input failed: " + name);
    }

    /* Create an ANTLR input backed by the buffer. */
    inputStream =
      new typename ImplTraits::InputStreamType((ANTLR_UINT8*) basep,
                                               ANTLR_ENC_8BIT,
                                               cp - basep,
                                               (ANTLR_UINT8*) strdup(name.c_str()));
  //}

  if(inputStream == NULL) {
    throw InputStreamException("Couldn't initialize input: " + name);
  }

  return new AntlrInputStream<ImplTraits>(name, inputStream);
}


template <class ImplTraits>
AntlrInputStream<ImplTraits>*
AntlrInputStream<ImplTraits>::newStringInputStream(const std::string& input,
                                                   const std::string& name)
  throw (InputStreamException) {
  char* inputStr = strdup(input.c_str());
  char* nameStr = strdup(name.c_str());
  assert( inputStr!=NULL && nameStr!=NULL );
  typename ImplTraits::InputStreamType* inputStream =
    new typename ImplTraits::InputStreamType((ANTLR_UINT8*) inputStr,
                                             ANTLR_ENC_8BIT,
                                             input.size(),
                                             (ANTLR_UINT8*) nameStr);
  if(inputStream == NULL) {
    throw InputStreamException("Couldn't initialize string input: '" + input + "'");
  }
  return new AntlrInputStream<ImplTraits>(name, inputStream);
}

template <class ImplTraits>
AntlrInput<ImplTraits>::AntlrInput(AntlrInputStream<ImplTraits>& inputStream, unsigned int lookahead) :
    Input(inputStream),
    d_lookahead(lookahead),
    d_lexer(NULL),
    d_parser(NULL),
    d_antlr3InputStream(inputStream.getAntlr3InputStream())
    /*d_tokenBuffer(NULL)*/ {
}

template <class ImplTraits>
AntlrInput<ImplTraits>::~AntlrInput() {
  //BoundedTokenBufferFree(d_tokenBuffer);
}

template <class ImplTraits>
typename ImplTraits::TokenStreamType* AntlrInput<ImplTraits>::getTokenStream() {
  return new typename ImplTraits::TokenStreamType(4, d_lexer->get_tokSource());
}

template <class ImplTraits>
void AntlrInput<ImplTraits>::lexerError(typename ImplTraits::template RecognizerType<typename ImplTraits::TokenStreamType>* recognizer) {
  typename ImplTraits::LexerType* lexer = recognizer->super;
  assert(lexer != NULL);
  Parser* parser = lexer->cvc4parser;
  assert(parser != NULL);
  AntlrInput<ImplTraits>* input = (AntlrInput<ImplTraits>*) parser->getInput();
  assert(input != NULL);

  /* Call the error display routine *if* there's not already a
   * parse error pending.  If a parser error is pending, this
   * error is probably less important, so we just drop it. */
  if(!input->d_parser->rec->state->get_error()) {
    input->parseError("Error finding next token.");
  }
}

template <class ImplTraits>
void AntlrInput<ImplTraits>::warning(const std::string& message) {
  Warning() << getInputStream()->getName()
            << ':' << d_lexer->getLine()
            << '.' << d_lexer->getCharPositionInLine()
            << ": " << message << std::endl;
}

template <class ImplTraits>
void AntlrInput<ImplTraits>::parseError(const std::string& message, bool eofException)
  throw (ParserException) {
  Debug("parser") << "Throwing exception: "
      << d_lexer->get_tokSource()->get_fileName() << ":"
      << d_lexer->getLine() << "."
      << d_lexer->getCharPositionInLine() << ": "
      << message << std::endl;
  if(eofException) {
    throw ParserEndOfFileException(message,
                                   d_lexer->get_tokSource()->get_fileName(),
                                   d_lexer->getLine(),
                                   d_lexer->getCharPositionInLine());

  } else {
    throw ParserException(message,
                          d_lexer->get_tokSource()->get_fileName(),
                          d_lexer->getLine(),
                          d_lexer->getCharPositionInLine());
  }
}

template <class ImplTraits>
void AntlrInput<ImplTraits>::setAntlr3Lexer(typename ImplTraits::BaseLexerType* pLexer) {
  d_lexer = pLexer;

  /*
  pANTLR3_TOKEN_FACTORY pTokenFactory = d_lexer->rec->state->tokFactory;
  if(pTokenFactory != NULL) {
    pTokenFactory->close(pTokenFactory);
  }
  */

  /* 2*lookahead should be sufficient, but we give ourselves some breathing room. */
  /*
  pTokenFactory = BoundedTokenFactoryNew(d_antlr3InputStream, 2 * d_lookahead);
  if( pTokenFactory == NULL ) {
    throw InputStreamException("Couldn't create token factory.");
  }
  d_lexer->rec->state->tokFactory = pTokenFactory;

  pBOUNDED_TOKEN_BUFFER buffer = BoundedTokenBufferSourceNew(d_lookahead, d_lexer->rec->state->tokSource);
  if( buffer == NULL ) {
    throw InputStreamException("Couldn't create token buffer.");
  }

  d_tokenBuffer = buffer;
  */

  // Override default lexer error reporting
  //d_lexer->rec->reportError = &lexerError;
  // Override default nextToken function, just to prevent exceptions escaping.
  //d_lexer->rec->state->tokSource->nextToken = &nextToken;
}

template <class ImplTraits>
void AntlrInput<ImplTraits>::setParser(Parser& parser) {
  // ANTLR isn't using super in the lexer or the parser, AFAICT.
  // We could also use @lexer/parser::context to add a field to the generated
  // objects, but then it would have to be declared separately in every
  // language's grammar and we'd have to in the address of the field anyway.
  d_lexer->super = (typename ImplTraits::BaseParserType::SuperType*) &parser;
  d_parser->super = (typename ImplTraits::BaseParserType::SuperType*) &parser;
}

template <class ImplTraits>
void AntlrInput<ImplTraits>::setAntlr3Parser(typename ImplTraits::BaseParserType* pParser) {
  d_parser = pParser;
//  d_parser->rec->match = &match;
  //d_parser->rec->reportError = &reportError;
  /* Don't try to recover from a parse error. */
  // [chris 4/5/2010] Not clear on why this cast is necessary, but I get an error if I remove it.
  //d_parser->rec->recoverFromMismatchedToken =
    //      (void* (*)(ANTLR3_BASE_RECOGNIZER_struct*, ANTLR3_UINT32, ANTLR3_BITSET_LIST_struct*))
  //d_parser->rec->mismatch;
}

}/* CVC4::parser namespace */
}/* CVC4 namespace */

#endif /* __CVC4__PARSER__ANTLR_INPUT_H */
