/*********************                                                        */
/*! \file proof_manager.cpp
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

#include "proof/proof_manager.h"
#include "util/proof.h"
#include "proof/sat_proof.h"
#include "proof/cnf_proof.h"
#include "proof/theory_proof.h"
#include "util/cvc4_assert.h"
#include "smt/smt_engine.h"
#include "smt/smt_engine_scope.h"
#include "theory/output_channel.h"
#include "theory/valuation.h"
#include "util/node_visitor.h"
#include "theory/term_registration_visitor.h"
#include "theory/uf/theory_uf.h"
#include "theory/uf/equality_engine.h"
#include "theory/arrays/theory_arrays.h"
#include "context/context.h"
#include "util/hash.h"

namespace CVC4 {

std::string append(const std::string& str, uint64_t num) {
  std::ostringstream os;
  os << str << num;
  return os.str();
}

ProofManager::ProofManager(ProofFormat format):
  d_satProof(NULL),
  d_cnfProof(NULL),
  d_theoryProof(NULL),
  d_fullProof(NULL),
  d_format(format)
{
}

ProofManager::~ProofManager() {
  delete d_satProof;
  delete d_cnfProof;
  delete d_theoryProof;
  delete d_fullProof;

  for(IdToClause::iterator it = d_inputClauses.begin();
      it != d_inputClauses.end();
      ++it) {
    delete it->second;
  }

  for(OrderedIdToClause::iterator it = d_theoryLemmas.begin();
      it != d_theoryLemmas.end();
      ++it) {
    delete it->second;
  }

  // FIXME: memory leak because there are deleted theory lemmas that
  // were not used in the SatProof
}

ProofManager* ProofManager::currentPM() {
  return smt::currentProofManager();
}

Proof* ProofManager::getProof(SmtEngine* smt) {
  if (currentPM()->d_fullProof != NULL) {
    return currentPM()->d_fullProof;
  }
  Assert (currentPM()->d_format == LFSC);

  currentPM()->d_fullProof = new LFSCProof(smt,
                                           (LFSCSatProof*)getSatProof(),
                                           (LFSCCnfProof*)getCnfProof(),
                                           (LFSCTheoryProof*)getTheoryProof());
  return currentPM()->d_fullProof;
}

SatProof* ProofManager::getSatProof() {
  Assert (currentPM()->d_satProof);
  return currentPM()->d_satProof;
}

CnfProof* ProofManager::getCnfProof() {
  Assert (currentPM()->d_cnfProof);
  return currentPM()->d_cnfProof;
}

TheoryProof* ProofManager::getTheoryProof() {
  Assert (currentPM()->d_theoryProof);
  return currentPM()->d_theoryProof;
}

void ProofManager::initSatProof(Minisat::Solver* solver) {
  Assert (currentPM()->d_satProof == NULL);
  Assert(currentPM()->d_format == LFSC);
  currentPM()->d_satProof = new LFSCSatProof(solver);
}

void ProofManager::initCnfProof(prop::CnfStream* cnfStream) {
  Assert (currentPM()->d_cnfProof == NULL);
  Assert (currentPM()->d_format == LFSC);
  currentPM()->d_cnfProof = new LFSCCnfProof(cnfStream);
}

void ProofManager::initTheoryProof() {
  Assert (currentPM()->d_theoryProof == NULL);
  Assert (currentPM()->d_format == LFSC);
  currentPM()->d_theoryProof = new LFSCTheoryProof();
}

std::string ProofManager::getInputClauseName(ClauseId id) { return append("pb", id); }
std::string ProofManager::getLemmaName(ClauseId id) { return append("lem", id); }
std::string ProofManager::getLemmaClauseName(ClauseId id) { return append("lemc", id); }
std::string ProofManager::getLearntClauseName(ClauseId id) { return append("cl", id); }
std::string ProofManager::getVarName(prop::SatVariable var) { return append("var", var); }
std::string ProofManager::getAtomName(prop::SatVariable var) { return append("atom", var); }
std::string ProofManager::getLitName(prop::SatLiteral lit) { return append("lit", lit.toInt()); }

std::string ProofManager::getAtomName(TNode atom) {
  prop::SatLiteral lit = currentPM()->d_cnfProof->getLiteral(atom);
  Assert(!lit.isNegated());
  return getAtomName(lit.getSatVariable());
}
std::string ProofManager::getLitName(TNode lit) {
  return getLitName(currentPM()->d_cnfProof->getLiteral(lit));
}

class ProofOutputChannel : public theory::OutputChannel {
public:
  Node d_conflict;
  Proof* d_proof;
  Node d_lemma;

  ProofOutputChannel() : d_conflict(), d_proof(NULL) {}

  void conflict(TNode n, Proof* pf) throw() {
    Debug("mgd") << "; CONFLICT: " << n << std::endl;
    Assert(d_conflict.isNull());
    Assert(!n.isNull());
    d_conflict = n;
    Assert(pf != NULL);
    d_proof = pf;
  }
  bool propagate(TNode x) throw() {
    Debug("mgd") << "got a propagation: " << x << std::endl;
    return true;
  }
  theory::LemmaStatus lemma(TNode n, ProofRule rule, bool, bool) throw() {
    //AlwaysAssert(false);
    Debug("mgd") << "new lemma: " << n << std::endl;
    d_lemma = n;
    return theory::LemmaStatus(TNode::null(), 0);
  }
  theory::LemmaStatus splitLemma(TNode, bool) throw() {
    AlwaysAssert(false);
    return theory::LemmaStatus(TNode::null(), 0);
  }
  void requirePhase(TNode n, bool b) throw() {
    Debug("mgd") << "requirePhase " << n << " " << b << std::endl;
    //AlwaysAssert(false);
  }
  bool flipDecision() throw() {
    AlwaysAssert(false);
    return false;
  }
  void setIncomplete() throw() {
    AlwaysAssert(false);
  }
};/* class ProofOutputChannel */

class MyPreRegisterVisitor {
  theory::Theory* d_theory;
  __gnu_cxx::hash_set<TNode, TNodeHashFunction> d_visited;

public:

  typedef void return_type;
  
  MyPreRegisterVisitor(theory::Theory* theory)
  : d_theory(theory)
  , d_visited()
  {}

  /**
   * Returns true is current has already been pre-registered with both current and parent theories.
   */
  bool alreadyVisited(TNode current, TNode parent) { return d_visited.find(current) != d_visited.end(); }

  /**
   * Pre-registeres current with any of the current and parent theories that haven't seen the term yet.
   */
  void visit(TNode current, TNode parent) {
    if(theory::Theory::theoryOf(current) == d_theory->getId()) {
      //Debug("mgd") << "preregister " << current << std::endl;
      d_theory->preRegisterTerm(current);
      d_visited.insert(current);
    }
  }

  /**
   * Marks the node as the starting literal.
   */
  void start(TNode node) { }

  /**
   * Notifies the engine of all the theories used.
   */
  void done(TNode node) { }
};

void ProofManager::printProof(std::ostream& os, TNode n) {
  context::UserContext fakeContext;
  ProofOutputChannel oc;
  theory::Valuation v(NULL);
  //theory::uf::TheoryUF uf(&fakeContext, &fakeContext, oc, v, d_logic);
  theory::arrays::TheoryArrays uf(&fakeContext, &fakeContext, oc, v, d_logic);
  uf.produceProofs();
  MyPreRegisterVisitor preRegVisitor(&uf);
  for(TNode::iterator i = n.begin(); i != n.end(); ++i) {
    Debug("mgd") << "preregistering and asserting " << (*i).negate() << std::endl;
    NodeVisitor<MyPreRegisterVisitor>::run(preRegVisitor, *i);
    uf.assertFact(*i, false);
  }
  uf.check(theory::Theory::EFFORT_FULL);
  if(oc.d_conflict.isNull()) {
    Debug("mgd") << "conflict is null" << std::endl;
    Assert(!oc.d_lemma.isNull());
    Debug("mgd") << "++ but got lemma: " << oc.d_lemma << std::endl;
    Debug("mgd") << "asserting " << oc.d_lemma[1].negate() << std::endl;
    uf.assertFact(oc.d_lemma[1].negate(), false);
    uf.check(theory::Theory::EFFORT_FULL);
  }
  Debug("mgd") << "got conflict " << oc.d_conflict << std::endl
               << "and proof " << oc.d_proof << ":" << std::endl;
Debug("mgd") << "PROOF[[" << std::endl;
((theory::eq::EqProof*)(oc.d_proof))->debug_print("mgd");
Debug("mgd") << "]]" << std::endl;
  oc.d_proof->toStream(os);
}

void ProofManager::addClause(ClauseId id, const prop::SatClause* clause, ClauseKind kind) {
  /*for (unsigned i = 0; i < clause->size(); ++i) {
    prop::SatLiteral lit = clause->operator[](i);
    d_propVars.insert(lit.getSatVariable());
  }*/
  if (kind == INPUT) {
    d_inputClauses.insert(std::make_pair(id, clause));
  } else if(kind == THEORY_LEMMA) {
    d_theoryLemmas.insert(std::make_pair(id, clause));
  } else {
    Assert(kind == THEORY_PROPAGATION);
    d_theoryPropagations.insert(std::make_pair(id, clause));
  }
}

void ProofManager::addAssertion(Expr formula) {
  d_inputFormulas.insert(formula);
}

void ProofManager::setLogic(const LogicInfo& logic) {
  d_logic = logic;
}

LFSCProof::LFSCProof(SmtEngine* smtEngine, LFSCSatProof* sat, LFSCCnfProof* cnf, LFSCTheoryProof* theory)
  : d_satProof(sat)
  , d_cnfProof(cnf)
  , d_theoryProof(theory)
  , d_smtEngine(smtEngine)
{
  d_satProof->constructProof();
}

void LFSCProof::toStream(std::ostream& out) {
  smt::SmtScope scope(d_smtEngine);
  std::ostringstream paren;
  out << "(check\n";
  out << " ;; Declarations\n";
  if (d_theoryProof == NULL) {
    d_theoryProof = new LFSCTheoryProof();
  }
  /*for(LFSCCnfProof::iterator i = d_cnfProof->begin_atom_mapping();
      i != d_cnfProof->end_atom_mapping();
      ++i) {
    d_theoryProof->addDeclaration(*i);
  }*/
  d_theoryProof->printAssertions(out, paren);
  out << " ;; Proof of empty clause follows\n";
  out << "(: (holds cln)\n";
  d_cnfProof->printClauses(out, paren);
  d_satProof->printResolutions(out, paren);
  paren <<")))\n;;";
  out << paren.str();
  out << "\n";
}

} /* CVC4  namespace */
