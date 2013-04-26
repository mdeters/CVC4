/*********************                                                        */
/*! \file interpreted_attr.h
 ** \verbatim
 ** Original author: Morgan Deters
 ** Major contributors: none
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2013  New York University and The University of Iowa
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief [[ Add one-line brief description here ]]
 **
 ** [[ Add lengthier description here ]]
 ** \todo document this file
 **/

#include "cvc4_private.h"

#ifndef __CVC4__SMT__INTERPRETED_ATTR_H
#define __CVC4__SMT__INTERPRETED_ATTR_H

namespace CVC4 {
namespace smt {

namespace attr {
  struct FullyInterpretedTag {};
  struct InterpretedTag {};
}/* CVC4::smt::attr namespace */

typedef expr::Attribute<attr::InterpretedTag, Node> InterpretedAttr;
typedef expr::Attribute<attr::FullyInterpretedTag, Node> FullyInterpretedAttr;

}/* CVC4::smt namespace */
}/* CVC4 namespace */

#endif /* __CVC4__SMT__INTERPRETED_ATTR_H */
