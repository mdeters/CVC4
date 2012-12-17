/*********************                                                        */
/*! \file smt_omega.h
 ** \verbatim
 ** Original author: mdeters
 ** Major contributors: none
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009-2012  New York University and The University of Iowa
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Interface functions for CVC4 <-> Omega Library conversions
 **
 ** Interface functions for CVC4 <-> Omega Library conversions.  These
 ** conversions allow you to convert from a CVC4 expression into an
 ** Omega Library "Relation" (really a set) and back again.
 **/

#include "expr/expr_manager.h"
#include <omega.h>

CVC4::Expr omegaToSmt(omega::Relation* p, CVC4::ExprManager* em);
omega::Relation* smtToOmega(CVC4::Expr e);
omega::Global_Var_ID omegaVar(CVC4::Expr e);
