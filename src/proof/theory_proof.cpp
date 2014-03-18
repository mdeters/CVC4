/*********************                                                        */
/*! \file theory_proof.cpp
 ** \verbatim
 ** Original author: Liana Hadarean
 ** Major contributors: Morgan Deters
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2014  New York University and The University of Iowa
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief [[ Add one-line brief description here ]]
 **
 ** [[ Add lengthier description here ]]
 ** \todo document this file
 **/

#include "proof/theory_proof.h"
#include "proof/proof_manager.h"
using namespace CVC4;

TheoryProof::TheoryProof()
  : d_termDeclarations()
  , d_sortDeclarations()
  , d_declarationCache()
{}

void TheoryProof::addDeclaration(Expr term) {
  if (d_declarationCache.count(term)) {
    return;
  }

  Type type = term.getType();
  if (type.isSort())
    d_sortDeclarations.insert(type);
  if (term.getKind() == kind::APPLY_UF) {
    Expr function = term.getOperator();
    d_termDeclarations.insert(function);
  } else if (term.isVariable()) {
    //Assert (type.isSort() || type.isBoolean());
    d_termDeclarations.insert(term);
  }
  // recursively declare all other terms
  for (unsigned i = 0; i < term.getNumChildren(); ++i) {
    addDeclaration(term[i]);
  }
  d_declarationCache.insert(term);
}

std::string toLFSCKind(Kind kind) {
  switch(kind) {
  case kind::OR : return "or";
  case kind::AND: return "and";
  case kind::XOR: return "xor";
  case kind::EQUAL: return "=";
  case kind::IFF: return "iff";
  case kind::IMPLIES: return "impl";
  case kind::NOT: return "not";
  default:
    Unreachable();
  }
}

// sanitize identifiers for LFSC
inline static void sanitize(std::string& name) {
  Assert(name.size() > 0);
  if(name[0] == '_') {
    // sanitize the name
    name = "lfsc" + name;
  }
}
inline static std::string sanitize(Expr v) {
  Assert(v.isVariable());
  std::stringstream ss;
  ss << v;
  std::string name = ss.str();
  sanitize(name);
  return name;
}
inline static std::string sanitize(Type s) {
  Assert(s.isSort());
  std::stringstream ss;
  ss << s;
  std::string name = ss.str();
  sanitize(name);
  return name;
}

void LFSCTheoryProof::printTerm(Expr term, std::ostream& os) {
  if (term.isVariable()) {
    if(ProofManager::currentPM()->hasOp(Node::fromExpr(term))) {
      printTerm(ProofManager::currentPM()->lookupOp(Node::fromExpr(term)).toExpr(), os);
    } else if(term.getType().isBoolean()) {
      os << "(p_app " << sanitize(term) << ")";
    } else {
      os << sanitize(term);
    }
    return;
  }

  switch(Kind k = term.getKind()) {
  case kind::APPLY_UF: {
    if(term.getType().isBoolean()) {
      os << "(p_app ";
    }
    Expr func = term.getOperator();
    for (unsigned i = 0; i < term.getNumChildren(); ++i) {
      os << "(apply _ _ ";
    }
    os << func << " ";
    for (unsigned i = 0; i < term.getNumChildren(); ++i) {
      printTerm(term[i], os);
      os << ")";
      if(i + 1 < term.getNumChildren()) {
        os << " ";
      }
    }
    if(term.getType().isBoolean()) {
      os << ")";
    }
    return;
  }

  case kind::ITE:
    os << (term.getType().isBoolean() ? "(ifte " : "(ite _ ");
    printTerm(term[0], os);
    os << " ";
    printTerm(term[1], os);
    os << " ";
    printTerm(term[2], os);
    os << ")";
    return;

  case kind::EQUAL:
    os << "(";
    os << "= ";
    printSort(term[0].getType(), os);
    os << " ";
    printTerm(term[0], os);
    os << " ";
    printTerm(term[1], os);
    os << ")";
    return;

  case kind::DISTINCT:
    os << "(not (= ";
    printSort(term[0].getType(), os);
    os << " ";
    printTerm(term[0], os);
    os << " ";
    printTerm(term[1], os);
    os << "))";
    return;

  case kind::OR:
  case kind::AND:
  case kind::XOR:
  case kind::IFF:
  case kind::IMPLIES:
  case kind::NOT:
    // print the Boolean operators
    os << "(" << toLFSCKind(k);
    if(term.getNumChildren() > 2) {
      // LFSC doesn't allow declarations with variable numbers of
      // arguments, so we have to flatten these N-ary versions.
      std::ostringstream paren;
      for (unsigned i = 0; i < term.getNumChildren(); ++i) {
        os << " ";
        printTerm(term[i], os);
        if(i < term.getNumChildren() - 2) {
          os << " (" << toLFSCKind(k);
          paren << ")";
        }
      }
      os << paren.str() << ")";
    } else {
      // this is for binary and unary operators
      for (unsigned i = 0; i < term.getNumChildren(); ++i) {
        os << " ";
        printTerm(term[i], os);
      }
      os << ")";
    }
    return;

  case kind::CONST_BOOLEAN:
    os << (term.getConst<bool>() ? "true" : "false");
    return;

  case kind::CHAIN: {
    // LFSC doesn't allow declarations with variable numbers of
    // arguments, so we have to flatten chained operators, like =.
    Kind op = term.getOperator().getConst<Chain>().getOperator();
    size_t n = term.getNumChildren();
    std::ostringstream paren;
    for(size_t i = 1; i < n; ++i) {
      if(i + 1 < n) {
        os << "(" << toLFSCKind(kind::AND) << " ";
        paren << ")";
      }
      os << "(" << toLFSCKind(op) << " ";
      printTerm(term[i - 1], os);
      os << " ";
      printTerm(term[i], os);
      os << ")";
      if(i + 1 < n) {
        os << " ";
      }
    }
    os << paren.str();
    return;
  }

  /* Arrays */
  case kind::SELECT:
    os << "(apply _ _ (apply _ _ (read ";
    printSort(ArrayType(term[0].getType()).getIndexType(), os);
    os << " ";
    printSort(ArrayType(term[0].getType()).getConstituentType(), os);
    os << ") ";
    printTerm(term[0], os);
    os << ") ";
    printTerm(term[1], os);
    os << ")";
    return;
  case kind::STORE:
    os << "(apply _ _ (apply _ _ (apply _ _ (write ";
    printSort(ArrayType(term[0].getType()).getIndexType(), os);
    os << " ";
    printSort(ArrayType(term[0].getType()).getConstituentType(), os);
    os << ") ";
    printTerm(term[0], os);
    os << ") ";
    printTerm(term[1], os);
    os << ") ";
    printTerm(term[2], os);
    os << ")";
    return;

  case kind::BUILTIN:
    switch(k = term.getConst<Kind>()) {
    case kind::SELECT:
      os << "(read _ _)";
      break;
    case kind::STORE:
      os << "(write _ _)";
      break;
    default:
      Unhandled(k);
    }
    return;

  default:
    Debug("mgdx") << "unhandled partial operator application? " << k << std::endl << term << std::endl;
    Unhandled(k);
    return;
  }

  Unreachable();
}

void LFSCTheoryProof::printAssertions(std::ostream& os, std::ostream& paren) {
  unsigned counter = 0;
  ProofManager::assertions_iterator it = ProofManager::currentPM()->begin_assertions();
  ProofManager::assertions_iterator end = ProofManager::currentPM()->end_assertions();

  // collect declarations first
  for(; it != end; ++it) {
    addDeclaration(*it);
  }
  printDeclarations(os, paren);

  it = ProofManager::currentPM()->begin_assertions();
  for (; it != end; ++it) {
    os << "(% A" << counter++ << " (th_holds ";
    printTerm(*it,  os);
    os << ")\n";
    paren << ")";
  }
}

void LFSCTheoryProof::printSort(Type type, std::ostream& os) {
  if(type.isFunction()) {
    std::ostringstream fparen;
    FunctionType ftype = (FunctionType)type;
    std::vector<Type> args = ftype.getArgTypes();
    args.push_back(ftype.getRangeType());
    os << "(arrow";
    for(unsigned i = 0; i < args.size(); i++) {
      Type arg_type = args[i];
      //Assert (arg_type.isSort() || arg_type.isBoolean());
      os << " " << arg_type;
      if(i < args.size() - 2) {
        os << " (arrow";
        fparen << ")";
      }
    }
    os << fparen.str() << ")";
  } else if(type.isArray()) {
    ArrayType arrtype = type;
    os << "(array " << arrtype.getIndexType() << " " << arrtype.getConstituentType() << ")";
  } else {
    os << type;
  }
}

void LFSCTheoryProof::printDeclarations(std::ostream& os, std::ostream& paren) {
  // declaring the sorts
  for (SortSet::const_iterator it = d_sortDeclarations.begin(); it != d_sortDeclarations.end(); ++it) {
    os << "(% " << sanitize(*it) << " sort\n";
    paren << ")";
  }

  // declaring the terms
  for (ExprSet::const_iterator it = d_termDeclarations.begin(); it != d_termDeclarations.end(); ++it) {
    Expr term = *it;

    os << "(% " << sanitize(term) << " ";
    os << "(term ";
    printSort(term.getType(), os);
    os << ")\n";
    paren << ")";
  }
}
