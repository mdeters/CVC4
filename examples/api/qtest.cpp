/*********************                                                        */
/*! \file qtest.cpp
 ** \verbatim
 ** Original author: mdeters
 ** Major contributors: none
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009-2012  New York University and The University of Iowa
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief A simple implementation of Monniaux's Q-Test from "Quantifier
 ** elimination by lazy model enumeration", CAV 2010:
 ** http://www-verimag.imag.fr/~monniaux/biblio/Monniaux_CAV10.pdf
 **
 ** A simple implementation of Monniaux's Q-Test from "Quantifier
 ** elimination by lazy model enumeration", CAV 2010:
 ** http://www-verimag.imag.fr/~monniaux/biblio/Monniaux_CAV10.pdf
 **/

#include <iostream>
#include <utility>
#include <vector>
#include <set>
#include <functional>
#include <cassert>

#include "smt/smt_engine.h" // for use with make examples
#include "expr/expr_manager.h" // for use with make examples
#include "expr/kind.h" // for use with make examples
//#include <cvc4/cvc4.h> // To follow the wiki

#include "smt_omega.h"

using namespace std;
using namespace CVC4;

unsigned n;
string indent;
vector<Expr> F;
vector<Expr> M;
vector< vector<Expr> > var;
ExprManager em;
SmtEngine cvc4(&em);

template <class T> Expr generalize(Expr C0, T test);
pair<bool, Expr> qTest(unsigned i, Expr C);
Expr pi(unsigned i, Expr e);

Relation* s;

inline Expr negateExpr(Expr e) {
  return e.getKind() == kind::NOT ? e[0] : e.notExpr();
}

// compute F_{i + 1} from F_i
void push(unsigned i) {
  assert(F[i].getKind() == kind::FORALL);
  assert(F[i][1].getKind() != kind::FORALL);
  hash_map<Expr, Expr, ExprHashFunction> vars;
  for(Expr::const_iterator it = F[i][0].begin(); it != F[i][0].end(); ++it) {
    var[i + 1].push_back(vars[*it] = em.mkVar((*it).toString(), (*it).getType()));
  }
  if(F[i][1].getKind() == kind::EXISTS) {
    // push in the negation
    F[i + 1] = em.mkExpr(kind::FORALL, F[i][1][0], negateExpr(F[i][1][1].substitute(vars)));
  } else {
    // e[1] is quant free
    F[i + 1] = negateExpr(F[i][1].substitute(vars));
  }
}

int main() {
  cvc4.setOption("produce-models", "true");
  cvc4.setOption("output-language", "cvc4");
  cvc4.setOption("incremental", "true");
  cvc4.setOption("default-dag-thresh", 0);

  n = 2;
  F.resize(n + 1);
  M.resize(n + 1, em.mkConst(true));
  var.resize(n + 1);

  Expr x = em.mkVar("x", em.integerType());
  var[0].push_back(x);
  Expr y = em.mkBoundVar("y", em.integerType());
  Expr z = em.mkBoundVar("z", em.integerType());
  F[0] = em.mkExpr(kind::FORALL,
                   em.mkExpr(kind::BOUND_VAR_LIST, y),
                   em.mkExpr(kind::EXISTS,
                             em.mkExpr(kind::BOUND_VAR_LIST, z),
                             em.mkExpr(kind::AND,
                                       em.mkExpr(kind::GEQ, z, em.mkConst(Rational(0))),
                                       em.mkExpr(kind::OR,
                                                 em.mkExpr(kind::AND,
                                                           em.mkExpr(kind::GEQ, x, z),
                                                           em.mkExpr(kind::GEQ, y, z)),
                                                 em.mkExpr(kind::LEQ, y, em.mkExpr(kind::MINUS, em.mkConst(Rational(1)), z))))));

  Relation* p = smtToOmega(F[0]);
  p->print();
  cout << "+ is_sat: " << (p->is_satisfiable() ? "true" : "false") << endl;
  *p = Symbolic_Solution(*p);
  cout << "+ solution: ";
  p->print();
  cout << endl;
  delete p;

  cout << "applying Monniaux-style QE to:" << endl << endl
       << "  " << F[0] << endl << endl;

  cout << "F[0]: " << F[0] << endl;
  for(unsigned i = 0; i < n; ++i) {
    push(i);
    cout << "F[" << (i + 1) << "]: " << F[i + 1] << endl;
  }
  cout << endl;
  pair<bool, Expr> result = qTest(0, em.mkConst(true));
  cout << endl
       << "result is: " << (result.first ? "true" : "false") << ", " << result.second << endl;

  return 0;
}

template <class T>
Expr generalize(Expr C0, T test) {
  cout << indent << "generalize: " << C0 << endl;
  indent += "| ";
  if(C0.getKind() != kind::AND) {
    //cout << "trying to remove " << C0 << endl;
    if(test(C0)) {
      indent.resize(indent.size() - 2);
      cout << indent << "+- success (removed atom)" << endl;
#warning fixme - is this right?
      return C0;//return em.mkConst(true);
    } else {
      indent.resize(indent.size() - 2);
      cout << indent << "+- cannot generalize" << endl;
      return C0;
    }
  } else {
    vector<Expr> v = C0.getChildren();
    for(unsigned i = 1; i <= v.size(); ++i) {
      vector<Expr> v2 = v;
      v2.erase(v2.begin() + i - 1);
      //cout << "trying to remove " << v[i - 1] << " from " << C0 << endl;
      if(test(v2.size() == 1 ? v2[0] : em.mkExpr(kind::AND, v2))) {
        v = v2;
        --i;
        //cout << "-- success" << endl;
      }// else cout << "-- fail" << endl;
    }
    assert(v.size() > 0);
    indent.resize(indent.size() - 2);
    if(C0.getNumChildren() == v.size()) {
      cout << indent << "+- cannot generalize" << endl;
      return C0;
    } else {
      cout << indent << "+- removed " << (C0.getNumChildren() - v.size()) << " atoms" << endl;
      return v.size() == 1 ? v[0] : em.mkExpr(kind::AND, v);
    }
  }
  assert(false);
}

template <class T, class U, class V, class W>
struct first : public unary_function< V, T > {
  W d_func;
  first(W f) : d_func(f) { }
  T operator()(const V& v) const {
    return d_func(v).first;
  }
};/* struct first<> */

void atoms(Expr F, set<Expr>& a) {
  switch(F.getKind()) {
  case kind::IFF:
  case kind::AND:
  case kind::OR:
  case kind::XOR:
  case kind::NOT:
  case kind::IMPLIES:
    for(Expr::const_iterator i = F.begin(); i != F.end(); ++i) {
      atoms(*i, a);
    }
    break;

  default:
    a.insert(F);
  }
}

pair<bool, Expr> smtTest(Expr C, Expr F) {
  cout << indent << "smtTest(" << C << ", " << F << ")" << endl;
  Result r = cvc4.checkSat(em.mkExpr(kind::AND, C, F));
  vector<Expr> v;
  if(r.isSat() == Result::SAT) {
    cout << indent << ". model: ";
    bool first = true;
    for(vector< vector<Expr> >::const_iterator i = var.begin(); i != var.end(); ++i) {
      for(vector<Expr>::const_iterator j = (*i).begin(); j != (*i).end(); ++j) {
        if(! first) {
          cout << " , ";
        } else {
          first = false;
        }
        cout << *j << " = " << cvc4.getValue(*j);
      }
    }
    cout << endl;
    set<Expr> a;
    atoms(F, a);
    assert(a.size() > 0);
    for(set<Expr>::const_iterator i = a.begin(); i != a.end(); ++i) {
      if(cvc4.getValue(*i).getConst<bool>()) {
        v.push_back(*i);
      } else {
        v.push_back(negateExpr(*i));
      }
    }
    Expr r = v.size() == 1 ? v[0] : em.mkExpr(kind::AND, v);
    cout << indent << ".. SAT " << r << endl;
    return make_pair(true, r);
  } else {
    cout << indent << ".. UNSAT" << endl;
    return make_pair(false, em.mkConst(false));
  }
}

Expr pi(unsigned i, Expr e) {
  cout << indent << "pi [ ";
  copy(var[i + 1].begin(), var[i + 1].end(), ostream_iterator<Expr>(cout, " "));
  cout << "] " << e << endl;

  Relation* p = smtToOmega(e);
  bool hasOne = false;
  for(vector<Expr>::const_iterator it = var[i + 1].begin(); it != var[i + 1].end(); ++it) {
    Global_Var_ID global = omegaVar(*it);
    if(p->has_local(global)) {
      cout << indent << " -- projecting " << *it << endl;
      *p = Project(*p, global);
      hasOne = true;
    } else {
      cout << indent << " -- does not contain " << *it << endl;
    }
  }
  if(hasOne) {
    *p = Symbolic_Solution(*p);
    e = cvc4.simplify(omegaToSmt(p, &em));
    cout << indent << " -- got " << e << endl;
  } else {
    cout << indent << " -- answer same as input" << endl;
  }
  delete p;
  return e;
}

pair<bool, Expr> qTest(unsigned i, Expr C) {
  cout << indent << "qTest(" << i << ", " << C << ")" << endl;
  indent += "  ";
  if(i == n) {
    pair<bool, Expr> p = smtTest(C, F[n]);
    if(p.first == false) {
      indent.resize(indent.size() - 2);
      cout << indent << "qTest(" << i << ", " << C << ") returns false, false" << endl;
      return make_pair(false, em.mkConst(false));
    } else {
      Expr e = generalize(p.second, not1(first<bool, Expr, Expr, binder2nd<pointer_to_binary_function< Expr, Expr, pair<bool, Expr> > > >(bind2nd(pointer_to_binary_function< Expr, Expr, pair<bool, Expr> >(smtTest), F[n]))));
      indent.resize(indent.size() - 2);
      cout << indent << "qTest(" << i << ", " << C << ") returns true, " << e << endl;
      return make_pair(true, e);
    }
  } else {
    for(;;) {
      pair<bool, Expr> p = smtTest(C, M[i]);
      if(p.first == false) {
        indent.resize(indent.size() - 2);
        cout << indent << "qTest(" << i << ", " << C << ") returns false, false" << endl;
        return make_pair(false, em.mkConst(false));
      } else {
        pair<bool, Expr> pp = qTest(i + 1, p.second);
        if(pp.first == false) {
          Expr e = generalize(p.second, not1(first<bool, Expr, Expr, binder1st<pointer_to_binary_function< unsigned, Expr, pair<bool, Expr> > > >(bind1st(pointer_to_binary_function< unsigned, Expr, pair<bool, Expr> >(qTest), i + 1))));
          indent.resize(indent.size() - 2);
          cout << indent << "qTest(" << i << ", " << C << ") returns true, " << e << endl;
          return make_pair(true, e);
        } else {
          Expr c = negateExpr(pi(i, pp.second));
          M[i] = M[i].andExpr(c);
          cout << indent << "M[" << i << "] conjoined with " << c << endl;
          M[i] = cvc4.simplify(M[i]);
          cout << indent << "M[" << i << "] is now " << M[i] << endl;
        }
      }
    }
  }
  assert(false);
}
