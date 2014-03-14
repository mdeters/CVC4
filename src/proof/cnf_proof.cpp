/*********************                                                        */
/*! \file cnf_proof.cpp
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

#include "proof/cnf_proof.h"
#include "proof/theory_proof.h"
#include "proof/proof_manager.h"
#include "prop/sat_solver_types.h"
#include "prop/minisat/minisat.h"
#include "prop/cnf_stream.h"

using namespace CVC4::prop;

namespace CVC4 {

CnfProof::CnfProof(CnfStream* stream)
  : d_cnfStream(stream)
{}

CnfProof::~CnfProof() {
}

Expr CnfProof::getAtom(prop::SatVariable var) {
  prop::SatLiteral lit (var);
  Node node = d_cnfStream->getNode(lit);
  Expr atom = node.toExpr();
  return atom;
}

prop::SatLiteral CnfProof::getLiteral(TNode atom) {
  return d_cnfStream->getLiteral(atom);
}

LFSCCnfProof::iterator LFSCCnfProof::begin_atom_mapping() {
  return iterator(*this, ProofManager::currentPM()->begin_vars());
}

LFSCCnfProof::iterator LFSCCnfProof::end_atom_mapping() {
  return iterator(*this, ProofManager::currentPM()->end_vars());
}

void LFSCCnfProof::printAtomMapping(std::ostream& os, std::ostream& paren) {
  ProofManager::var_iterator it = ProofManager::currentPM()->begin_vars();
  ProofManager::var_iterator end = ProofManager::currentPM()->end_vars();

  for (;it != end;  ++it) {
    os << "(decl_atom ";

    if (ProofManager::currentPM()->getLogic().compare("QF_UF") == 0 ||
        ProofManager::currentPM()->getLogic().compare("QF_SAT") == 0) {
      Expr atom = getAtom(*it);
      LFSCTheoryProof::printTerm(atom, os);
    } else {
      // print fake atoms for all other logics
      os << "true ";
    }

    os << " (\\ " << ProofManager::getVarName(*it) << " (\\ " << ProofManager::getAtomName(*it) << "\n";
    paren << ")))";
  }
}

void LFSCCnfProof::printClauses(std::ostream& os, std::ostream& paren) {
  printInputClauses(os, paren);
  printTheoryLemmas(os, paren);
}

void LFSCCnfProof::printInputClauses(std::ostream& os, std::ostream& paren) {
  os << " ;; Input Clauses \n";
  ProofManager::clause_iterator it = ProofManager::currentPM()->begin_input_clauses();
  ProofManager::clause_iterator end = ProofManager::currentPM()->end_input_clauses();

  for (; it != end; ++it) {
    ClauseId id = it->first;
    const prop::SatClause* clause = it->second;
    os << "(satlem _ _ ";
    std::ostringstream clause_paren;
    printClause(*clause, os, clause_paren);
    os << "(clausify_false trust)" << clause_paren.str()
       << " (\\ " << ProofManager::getInputClauseName(id) << "\n";
    paren << "))";
  }
}

void LFSCCnfProof::printTheoryLemmas(std::ostream& os, std::ostream& paren) {
  os << " ;; Theory Lemmas \n";
  ProofManager::clause_iterator it = ProofManager::currentPM()->begin_lemmas();
  ProofManager::clause_iterator end = ProofManager::currentPM()->end_lemmas();

  for(size_t n = 0; it != end; ++it, ++n) {
    if(n % 100 == 0) {
      Chat() << "proving theory lemmas...(" << n << "/" << ProofManager::currentPM()->num_lemmas() << ")" << std::endl;
    }

    ClauseId id = it->first;
    const prop::SatClause* clause = it->second;
    os << "(satlem _ _ ";
    std::ostringstream clause_paren;
    printClause(*clause, os, clause_paren);

    NodeBuilder<> c(kind::AND);
    for(unsigned i = 0; i < clause->size(); ++i) {
      prop::SatLiteral lit = (*clause)[i];
      prop::SatVariable var = lit.getSatVariable();
      if(lit.isNegated()) {
        c << Node::fromExpr(getAtom(var));
      } else {
        c << Node::fromExpr(getAtom(var)).notNode();
      }
    }
    Node cl = c;
    //os << "\n;; need a proof of " << cl << "\n";
    ProofManager::currentPM()->printProof(os, cl);
    os << clause_paren.str()
       << " (\\ " << ProofManager::getLemmaClauseName(id) << "\n";
    paren << "))";
  }
}

void LFSCCnfProof::printClause(const prop::SatClause& clause, std::ostream& os, std::ostream& paren) {
  for (unsigned i = 0; i < clause.size(); ++i) {
    prop::SatLiteral lit = clause[i];
    prop::SatVariable var = lit.getSatVariable();
    if (lit.isNegated()) {
      os << "(ast _ _ _ " << ProofManager::getAtomName(var) << " (\\ " << ProofManager::getLitName(lit) << " ";
      paren << "))";
    } else {
      os << "(asf _ _ _ " << ProofManager::getAtomName(var) << " (\\ " << ProofManager::getLitName(lit) << " ";
      paren << "))";
    }
  }
}

} /* CVC4 namespace */
