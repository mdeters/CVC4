#pragma once

#include "cvc4_private.h"
#include "expr/node.h"
#include "util/model.h"
#include "util/proof.h"

namespace CVC4 {

class Tactic {
public:
  virtual Node execute(Node) = 0;
  virtual void transform(Model&) = 0;
  virtual void transform(Proof&) = 0;
};/* class Tactic */

}/* CVC4 namespace */
