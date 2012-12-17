#include "smt_omega.h"
#include "expr/expr_manager.h"
#include <map>

using namespace omega;
using namespace CVC4;
using namespace std;

map<Expr, Global_Var_ID> cache;
map<Expr, Variable_ID> qcache;
map<Variable_ID, Expr> revCache;
map<Variable_ID, Expr> revQcache;

Variable_ID varid(Expr e, Relation* s) {
  assert(e.getKind() == kind::VARIABLE);
  assert(s->is_set());
  Global_Var_ID& var = cache[e];
  if(var == NULL) {
    var = new Free_Var_Decl(e.toString().c_str());
    Variable_ID id = s->get_local(var);
    revCache[id] = e;
    return id;
  }
  return s->get_local(var);
}

Global_Var_ID omegaVar(Expr e) {
  assert(e.getKind() == kind::VARIABLE);
  Global_Var_ID& var = cache[e];
  if(var == NULL) {
    var = new Free_Var_Decl(e.toString().c_str());
  }
  return var;
}

void smtToOmega(Expr e, GEQ_Handle& g, bool negate, Relation* s) {
  assert(s->is_set());
  switch(e.getKind()) {
  case kind::PLUS:
    for(unsigned i = 0; i < e.getNumChildren(); ++i) {
      smtToOmega(e[i], g, negate, s);
    }
    break;
  case kind::MULT: {
    assert(e.getNumChildren() == 2);// MULT can have more, but probably not in our use case
    assert(e[0].getKind() == kind::CONST_RATIONAL);
    assert(e[1].getKind() == kind::VARIABLE);
    const Rational& r = e[0].getConst<Rational>();
    assert(r.getDenominator() == 1);
    long c = r.getNumerator().getLong();
    g.update_coef(varid(e[1], s), negate ? -c : c);
    break;
  }
  case kind::MINUS:
    smtToOmega(e[0], g, negate, s);
    smtToOmega(e[1], g, !negate, s);
    break;
  case kind::CONST_RATIONAL: {
    const Rational& r = e.getConst<Rational>();
    assert(r.getDenominator() == 1);
    long c = r.getNumerator().getLong();
    g.update_const(negate ? -c : c);
    break;
  }
  case kind::VARIABLE:
    //cout << "looking at var " << e << endl;
    g.update_coef(varid(e, s), negate ? -1 : 1);
    break;
  case kind::BOUND_VARIABLE:
    //cout << "looking at bvar " << e << endl;
    assert(qcache.find(e) != qcache.end());
    g.update_coef(qcache[e], negate ? -1 : 1);
    break;
  case kind::UMINUS:
    smtToOmega(e, g, !negate, s);
    break;
  default:
    cout << "don't yet handle " << e << " kind " << e.getKind() << endl;
    assert(false);// don't handle this yet
  }
}

void smtToOmega(Expr e, F_And* f, Relation* s) {
  assert(s->is_set());
  //cout << "smtToOmega: " << e << endl;
  switch(e.getKind()) {
  case kind::AND:
    f = f->add_and();
    for(unsigned i = 0; i < e.getNumChildren(); ++i) {
      smtToOmega(e[i], f, s);
    }
    break;
  case kind::OR: {
    F_Or* ff = f->add_or();
    for(unsigned i = 0; i < e.getNumChildren(); ++i) {
      smtToOmega(e[i], ff->add_and(), s);
    }
    break;
  }
  case kind::NOT: {
    F_Not* ff = f->add_not();
    smtToOmega(e[0], ff->add_and(), s);
    break;
  }
  case kind::LEQ: {
    GEQ_Handle g = f->add_GEQ();
    smtToOmega(e[1], g, false, s);
    smtToOmega(e[0], g, true, s);
    break;
  }
  case kind::GEQ: {
    GEQ_Handle g = f->add_GEQ();
    smtToOmega(e[0], g, false, s);
    smtToOmega(e[1], g, true, s);
    break;
  }
  case kind::FORALL: {
    F_Forall* q = f->add_forall();
    for(unsigned i = 0; i < e[0].getNumChildren(); ++i) {
      //cout << "declaring bvar " << e[0][i] << endl;
      Variable_ID id = q->declare(e[0][i].toString().c_str());
      assert(qcache.find(e[0][i]) == qcache.end());
      assert(revQcache.find(id) == revQcache.end());
      qcache[e[0][i]] = id;
      revQcache[id] = e[0][i];
    }
    smtToOmega(e[1], q->add_and(), s);
    break;
  }
  case kind::EXISTS: {
    F_Exists* q = f->add_exists();
    for(unsigned i = 0; i < e[0].getNumChildren(); ++i) {
      //cout << "declaring bvar " << e[0][i] << endl;
      Variable_ID id = q->declare(e[0][i].toString().c_str());
      assert(qcache.find(e[0][i]) == qcache.end());
      assert(revQcache.find(id) == revQcache.end());
      qcache[e[0][i]] = id;
      revQcache[id] = e[0][i];
    }
    smtToOmega(e[1], q->add_and(), s);
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

Relation* smtToOmega(Expr e) {
  Relation* r = new Relation(0);
  smtToOmega(e, r->add_and(), r);
  return r;
}

Expr omegaToSmt(Constraint_Handle c, bool eq, ExprManager* em) {
  vector<Expr> v;
  v.push_back(em->mkConst(Rational(long(c.get_const()))));
  for(Constr_Vars_Iter i = c; i; ++i) {
    v.push_back(em->mkExpr(kind::MULT, em->mkConst(Rational(long((*i).coef))), revCache[(*i).var]));
  }
  assert(v.size() > 0);
  return em->mkExpr(eq ? kind::EQUAL : kind::GEQ, (v.size() == 1) ? v[0] : em->mkExpr(kind::PLUS, v), em->mkConst(Rational(0)));
}

Expr omegaToSmt(Conjunct& c, ExprManager* em) {
  vector<Expr> v;
  for(EQ_Iterator i = c.EQs(); i; ++i) {
    v.push_back(omegaToSmt(*i, true, em));
  }
  for(GEQ_Iterator i = c.GEQs(); i; ++i) {
    v.push_back(omegaToSmt(*i, false, em));
  }
  assert(v.size() > 0);
  return (v.size() == 1) ? v[0] : em->mkExpr(kind::AND, v);
}

Expr omegaToSmt(Relation* p, ExprManager* em) {
  assert(p->is_set());
  DNF* d = p->query_DNF();
  vector<Expr> v;
  for(DNF_Iterator i(d); i; ++i) {
    v.push_back(omegaToSmt(**i, em));
  }
  assert(v.size() > 0);
  return (v.size() == 1) ? v[0] : em->mkExpr(kind::OR, v);
}
