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

#include <omega.h>

using namespace std;
using namespace CVC4;

unsigned n = 2;
Expr F[3];
Expr M[3];
Expr var[3];
ExprManager em;
SmtEngine cvc4(&em);

template <class T> Expr generalize(Expr C0, T test);
pair<bool, Expr> qTest(unsigned i, Expr C);
Expr pi(unsigned i, Expr e);

int main() {
  cvc4.setOption("produce-models", "true");
  cvc4.setOption("output-language", "cvc4");
  cvc4.setOption("incremental", "true");
  cvc4.setOption("default-dag-thresh", 0);

  M[0] = M[1] = M[2] = em.mkConst(true);

  Expr x = var[0] = em.mkVar("x", em.integerType());
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

  y = var[1] = em.mkVar("y", em.integerType());
  z = em.mkBoundVar("z", em.integerType());
  F[1] = em.mkExpr(kind::FORALL,
                   em.mkExpr(kind::BOUND_VAR_LIST, z),
                   em.mkExpr(kind::NOT,
                             em.mkExpr(kind::AND,
                                       em.mkExpr(kind::GEQ, z, em.mkConst(Rational(0))),
                                       em.mkExpr(kind::OR,
                                                 em.mkExpr(kind::AND,
                                                           em.mkExpr(kind::GEQ, x, z),
                                                           em.mkExpr(kind::GEQ, y, z)),
                                                 em.mkExpr(kind::LEQ, y, em.mkExpr(kind::MINUS, em.mkConst(Rational(1)), z))))));

  z = var[2] = em.mkVar("z", em.integerType());
  F[2] = em.mkExpr(kind::AND,
                   em.mkExpr(kind::GEQ, z, em.mkConst(Rational(0))),
                   em.mkExpr(kind::OR,
                             em.mkExpr(kind::AND,
                                       em.mkExpr(kind::GEQ, x, z),
                                       em.mkExpr(kind::GEQ, y, z)),
                             em.mkExpr(kind::LEQ, y, em.mkExpr(kind::MINUS, em.mkConst(Rational(1)), z))));

  cout << "applying Monniaux-style QE to:" << endl
       << F[0] << endl;
  pair<bool, Expr> result = qTest(0, em.mkConst(true));
  cout << "result is: " << (result.first ? "true" : "false") << " , " << result.second << endl;

  return 0;
}

template <class T>
Expr generalize(Expr C0, T test) {
  //cout << "generalize: " << C0 << endl;
  if(C0.getKind() != kind::AND) {
    //cout << "trying to remove " << C0 << endl;
    if(test(C0)) {
      //cout << "-- success" << endl;
#warning fixme - is this right?
      return C0;//return em.mkConst(true);
    } else {
      //cout << "-- fail" << endl;
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
    return (C0.getNumChildren() == v.size()) ? C0 :
            v.size() == 1 ? v[0] : em.mkExpr(kind::AND, v);
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
  cout << "smtTest(" << C << " , " << F << ")" << endl;
  Result r = cvc4.checkSat(em.mkExpr(kind::AND, C, F));
  vector<Expr> v;
  if(r.isSat() == Result::SAT) {
    cout << "SAT" << endl;
    set<Expr> a;
    atoms(F, a);
    assert(a.size() > 0);
    //cout << "atoms of " << F << " :";
    for(set<Expr>::const_iterator i = a.begin(); i != a.end(); ++i) {
      if(cvc4.getValue(*i).getConst<bool>()) {
        v.push_back(*i);
      } else {
        v.push_back((*i).notExpr());
      }
      //cout << " " << v.back();
    }
    //cout << endl;
    return make_pair(true, v.size() == 1 ? v[0] : em.mkExpr(kind::AND, v));
  } else {
    cout << "UNSAT" << endl;
    return make_pair(false, em.mkConst(false));
  }
}

Relation* s;

map<Expr, Global_Var_ID> cache;
map<Variable_ID, Expr> revCache;

Variable_ID varid(Expr e) {
  assert(e.getKind() == kind::VARIABLE);
  Global_Var_ID& var = cache[e];
  if(var == NULL) {
    var = new Free_Var_Decl(e.toString().c_str());
    Variable_ID id = s->get_local(var);
    revCache[id] = e;
    return id;
  }
  return s->get_local(var);
}

void smtToOmega(Expr e, GEQ_Handle& g, bool negate) {
  switch(e.getKind()) {
  case kind::PLUS:
    for(unsigned i = 0; i < e.getNumChildren(); ++i) {
      smtToOmega(e[i], g, negate);
    }
    break;
  case kind::MULT: {
    assert(e.getNumChildren() == 2);// MULT can have more, but probably not in our use case
    assert(e[0].getKind() == kind::CONST_RATIONAL);
    assert(e[1].getKind() == kind::VARIABLE);
    const Rational& r = e[0].getConst<Rational>();
    assert(r.getDenominator() == 1);
    long c = r.getNumerator().getLong();
    g.update_coef(varid(e[1]), negate ? -c : c);
    break;
  }
  case kind::MINUS:
    smtToOmega(e[0], g, negate);
    smtToOmega(e[1], g, !negate);
    break;
  case kind::CONST_RATIONAL: {
    const Rational& r = e.getConst<Rational>();
    assert(r.getDenominator() == 1);
    long c = r.getNumerator().getLong();
    g.update_const(negate ? -c : c);
    break;
  }
  case kind::VARIABLE:
    g.update_coef(varid(e), negate ? -1 : 1);
    break;
  case kind::UMINUS:
    smtToOmega(e, g, !negate);
    break;
  default:
    cout << "don't yet handle " << e << " kind " << e.getKind() << endl;
    assert(false);// don't handle this yet
  }
}

void smtToOmega(Expr e, F_And* f) {
  switch(e.getKind()) {
  case kind::AND:
    f = f->add_and();
    for(unsigned i = 0; i < e.getNumChildren(); ++i) {
      smtToOmega(e[i], f);
    }
    break;
  case kind::OR: {
    F_Or* ff = f->add_or();
    for(unsigned i = 0; i < e.getNumChildren(); ++i) {
      smtToOmega(e[i], ff->add_and());
    }
    break;
  }
  case kind::NOT: {
    F_Not* ff = f->add_not();
    smtToOmega(e[0], ff->add_and());
    break;
  }
  case kind::LEQ: {
    GEQ_Handle g = f->add_GEQ();
    smtToOmega(e[1], g, false);
    smtToOmega(e[0], g, true);
    break;
  }
  case kind::GEQ: {
    GEQ_Handle g = f->add_GEQ();
    smtToOmega(e[0], g, false);
    smtToOmega(e[1], g, true);
    break;
  }
  case kind::XOR:
  case kind::IFF:
  case kind::IMPLIES:
  default:
    cout << "don't yet handle " << e.getKind() << endl;
    assert(false);// don't handle this yet
  }
}

Expr omegaToSmt(Constraint_Handle c, bool eq) {
  vector<Expr> v;
  v.push_back(em.mkConst(Rational(long(c.get_const()))));
  for(Constr_Vars_Iter i = c; i; ++i) {
    v.push_back(em.mkExpr(kind::MULT, em.mkConst(Rational(long((*i).coef))), revCache[(*i).var]));
  }
  assert(v.size() > 0);
  return em.mkExpr(eq ? kind::EQUAL : kind::GEQ, (v.size() == 1) ? v[0] : em.mkExpr(kind::PLUS, v), em.mkConst(Rational(0)));
}

Expr omegaToSmt(Conjunct& c) {
  vector<Expr> v;
  for(EQ_Iterator i = c.EQs(); i; ++i) {
    v.push_back(omegaToSmt(*i, true));
  }
  for(GEQ_Iterator i = c.GEQs(); i; ++i) {
    v.push_back(omegaToSmt(*i, false));
  }
  assert(v.size() > 0);
  return (v.size() == 1) ? v[0] : em.mkExpr(kind::AND, v);
}

Expr omegaToSmt(Relation& p) {
  DNF* d = p.query_DNF();
  vector<Expr> v;
  for(DNF_Iterator i(d); i; ++i) {
    v.push_back(omegaToSmt(**i));
  }
  assert(v.size() > 0);
  return (v.size() == 1) ? v[0] : em.mkExpr(kind::OR, v);
}

Expr pi(unsigned i, Expr e) {
  //cout << "pi " << var[i + 1] << " " << e << endl;
  s = new Relation(0);
  smtToOmega(e, s->add_and());
  //s->print(); cout << endl;
  Global_Var_ID global = cache[var[i + 1]];
  if(s->has_local(global)) {
    Relation p = Symbolic_Solution(Project(*s, global));
    //p.print(); cout << endl;
    e = cvc4.simplify(omegaToSmt(p));
    //cout << " -- got " << e << endl;
  } else {
    //cout << " -- does not contain " << var[i + 1] << endl;
  }
  delete s;
  return e;
}

pair<bool, Expr> qTest(unsigned i, Expr C) {
  cout << "qTest(" << i << " , " << C << ")" << endl;
  if(i == n) {
    pair<bool, Expr> p = smtTest(C, F[n]);
    if(p.first == false) {
      cout << "qTest(" << i << " , " << C << ") returns false, false" << endl;
      return make_pair(false, em.mkConst(false));
    } else {
      Expr e = generalize(p.second, not1(first<bool, Expr, Expr, binder2nd<pointer_to_binary_function< Expr, Expr, pair<bool, Expr> > > >(bind2nd(pointer_to_binary_function< Expr, Expr, pair<bool, Expr> >(smtTest), F[n]))));
      cout << "qTest(" << i << " , " << C << ") returns true, " << e << endl;
      return make_pair(true, e);
    }
  } else {
    for(;;) {
      pair<bool, Expr> p = smtTest(C, M[i]);
      if(p.first == false) {
        cout << "qTest(" << i << " , " << C << ") returns false, false" << endl;
        return make_pair(false, em.mkConst(false));
      } else {
        pair<bool, Expr> pp = qTest(i + 1, p.second);
        if(pp.first == false) {
          Expr e = generalize(p.second, not1(first<bool, Expr, Expr, binder1st<pointer_to_binary_function< unsigned, Expr, pair<bool, Expr> > > >(bind1st(pointer_to_binary_function< unsigned, Expr, pair<bool, Expr> >(qTest), i + 1))));
          cout << "qTest(" << i << " , " << C << ") returns true, " << e << endl;
          return make_pair(true, e);
        } else {
          M[i] = M[i].andExpr(pi(i, pp.second).notExpr());
          cout << "M[" << i << "] gets " << M[i] << endl;
          M[i] = cvc4.simplify(M[i]);
          cout << "M[" << i << "] now " << M[i] << endl;
        }
      }
    }
  }
  assert(false);
}
