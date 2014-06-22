/*********************                                                        */
/*! \file dagification_visitor.cpp
 ** \verbatim
 ** Original author: Morgan Deters
 ** Major contributors: none
 ** Minor contributors (to current version): Andrew Reynolds
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2013  New York University and The University of Iowa
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Implementation of a dagifier for CVC4 expressions
 **
 ** Implementation of a dagifier for CVC4 expressions.
 **/

#include "printer/dagification_visitor.h"

#include "context/context.h"
#include "theory/substitutions.h"

#include <sstream>

namespace CVC4 {
namespace printer {

DagificationVisitor::DagificationVisitor(unsigned threshold, bool parallelLetSemantics, std::string letVarPrefix) :
  d_threshold(threshold),
  d_parallelLet(parallelLetSemantics),
  d_letVarPrefix(letVarPrefix),
  d_nodeCount(),
  d_top(),
  d_binders(),
  d_boundVars(),
  d_context(NULL),
  d_topLevelSubstitutions(NULL),
  d_binderSubstitutions(),
  d_letVar(0),
  d_done(false),
  d_uniqueParent(),
  d_substNodes() {

  // 0 doesn't make sense
  AlwaysAssertArgument(threshold > 0, threshold);
}

DagificationVisitor::~DagificationVisitor() {
  delete d_topLevelSubstitutions;
  std::set<theory::SubstitutionMap*> deletions;
  for(BinderSubstitutions::iterator i = d_binderSubstitutions.begin();
      i != d_binderSubstitutions.end();
      ++i) {
    deletions.insert((*i).second);
  }
  for(std::set<theory::SubstitutionMap*>::iterator i = deletions.begin();
      i != deletions.end();
      ++i) {
    delete *i;
  }
  delete d_context;
}

bool DagificationVisitor::alreadyVisited(TNode current, TNode parent) {
  // don't visit variables, constants, or those exprs that we've
  // already seen more than the threshold: if we've increased
  // the count beyond the threshold already, we've done the same
  // for all subexpressions, so it isn't useful to traverse and
  // increment again (they'll be dagified anyway).
  return current.isVar() ||
         current.getMetaKind() == kind::metakind::CONSTANT ||
         current.getNumChildren()==0 ||
         ( ( current.getKind() == kind::NOT ||
             current.getKind() == kind::UMINUS ) &&
           ( current[0].isVar() ||
             current[0].getMetaKind() == kind::metakind::CONSTANT ) ) ||
         current.getKind() == kind::SORT_TYPE ||
         d_nodeCount[current] > d_threshold;
}

void DagificationVisitor::visit(TNode current, TNode parent) {

#ifdef CVC4_TRACING
#  ifdef CVC4_DEBUG
  // turn off dagification for Debug stream while we're doing this work
  Node::dag::Scope scopeDebug(Debug.getStream(), false);
#  endif /* CVC4_DEBUG */
  // turn off dagification for Trace stream while we're doing this work
  Node::dag::Scope scopeTrace(Trace.getStream(), false);
#endif /* CVC4_TRACING */

  // binders treated specially
  if(current.getKind() == kind::FORALL || current.getKind() == kind::EXISTS) {
    for(TNode::iterator i = current[0].begin(); i != current[0].end(); ++i) {
      d_boundVars[*i] = current;
    }
    d_binders.insert(current);
  }

  if(d_uniqueParent.find(current) != d_uniqueParent.end()) {
    // we've seen this expr before

    TNode& uniqueParent = d_uniqueParent[current];

    if(!uniqueParent.isNull() && uniqueParent != parent) {
      // there is not a unique parent for this expr, mark it
      uniqueParent = TNode::null();
    }

    // increase the count
    const unsigned count = ++d_nodeCount[current];

    if(count > d_threshold) {
      // candidate for a let binder
      Debug("dag") << "found dagification candidate: " << current << std::endl;
      d_substNodes.push_back(current);
    }
  } else {
    // we haven't seen this expr before
    Assert(d_nodeCount[current] == 0);
    d_nodeCount[current] = 1;
    d_uniqueParent[current] = parent;
  }
}

void DagificationVisitor::start(TNode node) {
  AlwaysAssert(!d_done, "DagificationVisitor cannot be re-used");
  d_top = node;
}

TNode DagificationVisitor::findBoundVarsIn(TNode n) {
  TNode found = TNode::null();

  for(TNode::iterator i = n.begin(); i != n.end(); ++i) {
    if((*i).getMetaKind() == kind::metakind::VARIABLE) {
      BoundVarMap::iterator binder = d_boundVars.find(*i);
      if(binder != d_boundVars.end() &&
         ( found.isNull() ||
           found.getId() > (*binder).second.getId() )) {
        found = (*binder).second;
      }
    } else {
      TNode binder;

      if((*i).getKind() == kind::FORALL ||
         (*i).getKind() == kind::EXISTS) {
        for(TNode::iterator j = (*i)[0].begin(); j != (*i)[0].end(); ++j) {
          Assert(d_boundVars[*j] == *i);
          d_boundVars.erase(*j);
        }
        binder = findBoundVarsIn(n[1]);
        for(TNode::iterator j = (*i)[0].begin(); j != (*i)[0].end(); ++j) {
          d_boundVars[*j] = *i;
        }
      } else {
        binder = findBoundVarsIn(*i);
      }

      if(!binder.isNull() &&
         ( found.isNull() ||
           found.getId() > binder.getId() )) {
        found = binder;
      }
    }
  }

  return found;
}

void DagificationVisitor::done(TNode node) {
  AlwaysAssert(!d_done);

  d_done = true;
  d_context = new context::Context();
  d_topLevelSubstitutions = new theory::SubstitutionMap(d_context);

#ifdef CVC4_TRACING
#  ifdef CVC4_DEBUG
  // turn off dagification for Debug stream while we're doing this work
  Node::dag::Scope scopeDebug(Debug.getStream(), false);
#  endif /* CVC4_DEBUG */
  // turn off dagification for Trace stream while we're doing this work
  Node::dag::Scope scopeTrace(Trace.getStream(), false);
#endif /* CVC4_TRACING */

  // letify subexprs before parents (cascading LETs)
  std::sort(d_substNodes.begin(), d_substNodes.end());

  for(std::vector<TNode>::iterator i = d_substNodes.begin();
      i != d_substNodes.end();
      ++i) {
    Assert(d_nodeCount[*i] > d_threshold);
    TNode parent = d_uniqueParent[*i];
    if(!parent.isNull() && d_nodeCount[parent] > d_threshold) {
      // no need to letify this expr, because it only occurs in
      // a single super-expression, and that one will be letified
      continue;
    }

    // if the RHS has a bound var in it, we have to ensure that we don't
    // pull the let outside of the binder.  This function returns the
    // binder-level where it should be letified, or the null node if there
    // are no bound variables used in *i.
    TNode binder = findBoundVarsIn(*i);

    theory::SubstitutionMap*& subs = binder.isNull() ? d_topLevelSubstitutions : d_binderSubstitutions[binder];
    if(subs == NULL) {
      subs = new theory::SubstitutionMap(d_context);
    }

    // construct the let binder
    std::stringstream ss;
    ss << d_letVarPrefix << d_letVar++;
    Node letvar = NodeManager::currentNM()->mkSkolem(ss.str(), (*i).getType(), "dagification", NodeManager::SKOLEM_NO_NOTIFY | NodeManager::SKOLEM_EXACT_NAME);

    // apply previous substitutions to the rhs, enabling cascading LETs
    Node n = subs->apply(*i);
    Assert(! subs->hasSubstitution(n));
    subs->addSubstitution(n, letvar);
  }

  for(std::set<TNode>::iterator i = d_binders.begin();
      i != d_binders.end();
      ++i) {
    Debug("dag") << "\nlooking at binder " << *i << "\n";
    Node nn = d_topLevelSubstitutions->apply(*i);
    for(std::set<TNode>::iterator j = d_binders.begin();
        j != i;
        ++j) {
      nn = d_binderSubstitutions[*j]->apply(nn);
    }
    d_binderSubstitutions[nn] = d_binderSubstitutions[*i];
    Debug("dag") << "\n  turned into     " << nn << "\n";
  }
}

const theory::SubstitutionMap* DagificationVisitor::getLets(TNode binder) {
  AlwaysAssert(d_done, "DagificationVisitor must be used as a visitor before getting the dagified version out!");
  return binder.isNull() ? d_topLevelSubstitutions : d_binderSubstitutions[binder];
}

Node DagificationVisitor::getDagifiedBody(TNode binder) {
  AlwaysAssert(d_done, "DagificationVisitor must be used as a visitor before getting the dagified version out!");

#ifdef CVC4_TRACING
#  ifdef CVC4_DEBUG
  // turn off dagification for Debug stream while we're doing this work
  Node::dag::Scope scopeDebug(Debug.getStream(), false);
#  endif /* CVC4_DEBUG */
  // turn off dagification for Trace stream while we're doing this work
  Node::dag::Scope scopeTrace(Trace.getStream(), false);
#endif /* CVC4_TRACING */

  if(binder.isNull()) {
    return d_topLevelSubstitutions->apply(d_top);
  } else {
    Assert( binder.getKind() == kind::FORALL ||
            binder.getKind() == kind::EXISTS );
Debug("dag") << "requested for binder " << Expr::setlanguage(language::output::LANG_AST) << binder << std::endl;
    if(d_binderSubstitutions[binder] != NULL) {
Debug("dag") << "==> substitution is " << d_binderSubstitutions[binder]->apply(binder[1]) << std::endl;
      return d_binderSubstitutions[binder]->apply(binder[1]);
    } else {
Debug("dag") << "==> no substitution" << std::endl;
      return binder[1];
    }
  }
}

}/* CVC4::printer namespace */
}/* CVC4 namespace */
