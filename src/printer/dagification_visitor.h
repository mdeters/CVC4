/*********************                                                        */
/*! \file dagification_visitor.h
 ** \verbatim
 ** Original author: Morgan Deters
 ** Major contributors: none
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2013  New York University and The University of Iowa
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief A dagifier for CVC4 expressions
 **
 ** A dagifier for CVC4 expressions.
 **/

#include "cvc4_private.h"

#ifndef __CVC4__PRINTER__DAGIFICATION_VISITOR_H
#define __CVC4__PRINTER__DAGIFICATION_VISITOR_H

#include "expr/node.h"
#include "util/hash.h"

#include <vector>
#include <string>

namespace CVC4 {

namespace context {
  class Context;
}/* CVC4::context namespace */

namespace theory {
  class SubstitutionMap;
}/* CVC4::theory namespace */

namespace printer {

/**
 * This is a visitor class (intended to be used with CVC4's NodeVisitor)
 * that visits an expression looking for common subexpressions that appear
 * more than N times, where N is a configurable threshold.  Afterward,
 * let bindings can be extracted from this visitor and applied to the
 * expression.
 *
 * The main complication is to support binders (e.g., FORALL and EXISTS).
 * If the RHS of a let candidate contains a variable bound by one of these
 * things, the let for that candidate has to be inserted inside the
 * binder.  So, a number of substitutions are kept: one top-level
 * substitution, for let candidates containing no bound variables, and
 * (at most) one substitution for each binder.
 *
 * This dagifier never introduces let bindings for variables, constants,
 * unary-minus exprs over variables or constants, or NOT exprs over
 * variables or constants.  This dagifier never introduces let bindings
 * for types.
 */
class DagificationVisitor {

  /**
   * The threshold for dagification.  Subexprs occurring more than this
   * number of times are dagified.
   */
  const unsigned d_threshold;

  /**
   * Should the produced sequence of LET bindings assume a parallel LET
   * semantics (in which case the LHS of the LET cannot occur in the RHS
   * in the same level sequence).
   */
  const bool d_parallelLet;

  /**
   * The prefix for introduced let bindings.
   */
  const std::string d_letVarPrefix;

  /**
   * A map of subexprs to their occurrence count.
   */
  std::hash_map<TNode, unsigned, TNodeHashFunction> d_nodeCount;

  /**
   * The top-most node we are visiting.
   */
  TNode d_top;

  /**
   * A set of all binders visited.
   */
  std::set<TNode> d_binders;

  /**
   * Type for a mapping of bound vars to their binders.
   */
  typedef std::hash_map<TNode, TNode, TNodeHashFunction> BoundVarMap;

  /**
   * A mapping of bound vars to their binders.
   */
  BoundVarMap d_boundVars;

  /**
   * This class doesn't operate in a context-dependent fashion, but
   * SubstitutionMap does, so we need a context.
   */
  context::Context* d_context;

  /**
   * A map of subexprs to their newly-introduced let bindings.
   */
  theory::SubstitutionMap* d_topLevelSubstitutions;

  /**
   * Type for a mapping between binders and their substitution maps.
   */
  typedef std::hash_map<Node, theory::SubstitutionMap*, NodeHashFunction> BinderSubstitutions;

  /**
   * Binder substitutions.  FORALL and EXISTS expressions inside the
   * visited expression could have some LETs themselves.
   */
  BinderSubstitutions d_binderSubstitutions;

  /**
   * The current count of let bindings.  Used to build unique names
   * for the bindings.
   */
  unsigned d_letVar;

  /**
   * Keep track of whether we are done yet (for assertions---this visitor
   * can only be used one time).
   */
  bool d_done;

  /**
   * If a subexpr occurs uniquely in one parent expr, this map points to
   * it.  An expr not occurring as a key in this map means we haven't
   * seen it yet (and its occurrence count should be zero).  If an expr
   * points to the null expr in this map, it means we've seen more than
   * one parent, so the subexpr doesn't have a unique parent.
   *
   * This information is kept because if a subexpr occurs more than the
   * threshold, it is normally subject to dagification.  But if it occurs
   * only in one unique parent expression, and the parent meets the
   * threshold too, then the parent will be dagified and there's no point
   * in independently dagifying the child.  (If it is beyond the threshold
   * and occurs in more than one parent, we'll independently dagify.)
   */
  std::hash_map<TNode, TNode, TNodeHashFunction> d_uniqueParent;

  /**
   * A list of all nodes that meet the occurrence threshold and therefore
   * *may* be subject to dagification, except for the unique-parent rule
   * mentioned above.
   */
  std::vector<TNode> d_substNodes;

  /**
   * Find bound vars (from the outer context) occurring in n, and
   * return the lowest-level quantifier (i.e., one with lowest node ID)
   * that binds it.  This is used to figure out where it is safe to
   * insert a let, since the RHS of each let-bound variable must occur
   * within its binder.
   */
  TNode findBoundVarsIn(TNode n);

public:

  /** Our visitor doesn't return anything. */
  typedef void return_type;

  /**
   * Construct a dagification visitor with the given threshold and let
   * binding prefix.
   *
   * @param threshold the threshold to apply for dagification (must be > 0)
   * @param parallelLetSemantics should the produced LETs assume parallel
   * let semantics?
   * @param letVarPrefix prefix for let bindings (by default, "_let_")
   */
  DagificationVisitor(unsigned threshold, bool parallelLetSemantics, std::string letVarPrefix = "_let_");

  /**
   * Simple destructor, clean up memory.
   */
  ~DagificationVisitor();

  /**
   * Returns true if "current" has already been visited a sufficient
   * number of times to make it a candidate for dagification, or if
   * it cannot ever be subject to dagification.
   */
  bool alreadyVisited(TNode current, TNode parent);

  /**
   * Visit the expr "current", it might be a candidate for a let binder.
   */
  void visit(TNode current, TNode parent);

  /**
   * Marks the node as the starting literal.
   */
  void start(TNode node);

  /**
   * Called when we're done with all visitation.  Does postprocessing.
   */
  void done(TNode node);

  /**
   * Get the let substitutions.
   *
   * @param binder pass NULL to get top-level lets, or a FORALL or EXISTS
   * node to get the lets for the quantifier body
   */
  const theory::SubstitutionMap* getLets(TNode binder);

  /**
   * Return the let-substituted expression.
   */
  Node getDagifiedBody(TNode binder);

};/* class DagificationVisitor */

}/* CVC4::printer namespace */
}/* CVC4 namespace */

#endif /* __CVC4__PRINTER__DAGIFICATION_VISITOR_H */
