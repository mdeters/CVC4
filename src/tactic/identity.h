#include "tactic/tactic.h"

namespace CVC4 {
namespace tactic {

class Identity : public Tactic {
public:
  Identity();
  Node execute(Node) { return Node(); }
  void transform(Model&) {}
  void transform(Proof&) {}
};/* class Identity */

/* CVC4::tactic namespace */
/* CVC4 namespace */
