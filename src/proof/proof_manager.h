/*********************                                                        */
/*! \file proof_manager.h
 ** \verbatim
 ** Original author: Liana Hadarean
 ** Major contributors: Morgan Deters
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2014  New York University and The University of Iowa
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief A manager for Proofs
 **
 ** A manager for Proofs.
 **/

#include "cvc4_private.h"

#ifndef __CVC4__PROOF_MANAGER_H
#define __CVC4__PROOF_MANAGER_H

#include <iostream>
#include <map>
#include "proof/proof.h"
#include "util/proof.h"
#include "expr/node.h"
#include "theory/logic_info.h"

// forward declarations
namespace Minisat {
  class Solver;
}/* Minisat namespace */

namespace CVC4 {

namespace prop {
  class CnfStream;
}/* CVC4::prop namespace */

class SmtEngine;

typedef int ClauseId;

class Proof;
class SatProof;
class CnfProof;
class TheoryProof;

class LFSCSatProof;
class LFSCCnfProof;
class LFSCTheoryProof;

namespace prop {
  typedef uint64_t SatVariable;
  class SatLiteral;
  typedef std::vector<SatLiteral> SatClause;
}/* CVC4::prop namespace */

// different proof modes
enum ProofFormat {
  LFSC,
  NATIVE
};/* enum ProofFormat */

std::string append(const std::string& str, uint64_t num);

typedef __gnu_cxx::hash_map < ClauseId, const prop::SatClause* > IdToClause;
typedef std::map < ClauseId, const prop::SatClause* > OrderedIdToClause;
typedef __gnu_cxx::hash_set<prop::SatVariable > VarSet;
typedef __gnu_cxx::hash_set<Expr, ExprHashFunction > ExprSet;

typedef int ClauseId;

enum ClauseKind {
  INPUT,
  THEORY_LEMMA,
  THEORY_PROPAGATION,
  LEARNT
};/* enum ClauseKind */

enum ProofRule {
  RULE_GIVEN,       /* input assertion */
  RULE_DERIVED,     /* a "macro" rule */
  RULE_RECONSTRUCT, /* prove equivalence using another method */
  RULE_TRUST,       /* trust without evidence (escape hatch until proofs are fully supported) */
  RULE_INVALID,     /* assert-fail if this is ever needed in proof; use e.g. for split lemmas */
  RULE_CONFLICT,    /* re-construct as a conflict */

  RULE_ARRAYS_EXT,  /* arrays, extensional */
};/* enum ProofRules */

class ProofManager {
  SatProof*    d_satProof;
  CnfProof*    d_cnfProof;
  TheoryProof* d_theoryProof;

  // information that will need to be shared across proofs
  IdToClause d_inputClauses;
  OrderedIdToClause d_theoryLemmas;
  IdToClause d_theoryPropagations;
  ExprSet    d_inputFormulas;
  //VarSet     d_propVars;

  Proof* d_fullProof;
  ProofFormat d_format; // used for now only in debug builds

  int d_nextId;

protected:
  LogicInfo d_logic;

public:
  ProofManager(ProofFormat format = LFSC);
  ~ProofManager();

  static ProofManager* currentPM();

  // initialization
  static void         initSatProof(Minisat::Solver* solver);
  static void         initCnfProof(CVC4::prop::CnfStream* cnfStream);
  static void         initTheoryProof();

  static Proof*       getProof(SmtEngine* smt);
  static SatProof*    getSatProof();
  static CnfProof*    getCnfProof();
  static TheoryProof* getTheoryProof();

  // iterators over data shared by proofs
  typedef IdToClause::const_iterator clause_iterator;
  typedef OrderedIdToClause::const_iterator ordered_clause_iterator;
  typedef ExprSet::const_iterator assertions_iterator;
  typedef VarSet::const_iterator var_iterator;

  clause_iterator begin_input_clauses() const { return d_inputClauses.begin(); }
  clause_iterator end_input_clauses() const { return d_inputClauses.end(); }
  size_t num_input_clauses() const { return d_inputClauses.size(); }

  ordered_clause_iterator begin_lemmas() const { return d_theoryLemmas.begin(); }
  ordered_clause_iterator end_lemmas() const { return d_theoryLemmas.end(); }
  size_t num_lemmas() const { return d_theoryLemmas.size(); }

  assertions_iterator begin_assertions() const { return d_inputFormulas.begin(); }
  assertions_iterator end_assertions() const { return d_inputFormulas.end(); }
  size_t num_assertions() const { return d_inputFormulas.size(); }

  //var_iterator begin_vars() const { return d_propVars.begin(); }
  //var_iterator end_vars() const { return d_propVars.end(); }
  //size_t num_vars() const { return d_propVars.size(); }

  void printProof(std::ostream& os, TNode n);

  void addAssertion(Expr formula);
  void addClause(ClauseId id, const prop::SatClause* clause, ClauseKind kind);

  int nextId() { return d_nextId++; }

  // variable prefixes
  static std::string getInputClauseName(ClauseId id);
  static std::string getLemmaName(ClauseId id);
  static std::string getLemmaClauseName(ClauseId id);
  static std::string getLearntClauseName(ClauseId id);

  static std::string getVarName(prop::SatVariable var);
  static std::string getAtomName(prop::SatVariable var);
  static std::string getAtomName(TNode atom);
  static std::string getLitName(prop::SatLiteral lit);
  static std::string getLitName(TNode lit);

  void setLogic(const LogicInfo& logic);
  const std::string getLogic() const { return d_logic.getLogicString(); }

};/* class ProofManager */

class LFSCProof : public Proof {
  LFSCSatProof* d_satProof;
  LFSCCnfProof* d_cnfProof;
  LFSCTheoryProof* d_theoryProof;
  SmtEngine* d_smtEngine;
public:
  LFSCProof(SmtEngine* smtEngine, LFSCSatProof* sat, LFSCCnfProof* cnf, LFSCTheoryProof* theory);
  virtual void toStream(std::ostream& out);
  virtual ~LFSCProof() {}
};/* class LFSCProof */

}/* CVC4 namespace */

#endif /* __CVC4__PROOF_MANAGER_H */
