/*********************                                                        */
/*! \file type_node.h
 ** \verbatim
 ** Original author: dejan
 ** Major contributors: mdeters
 ** Minor contributors (to current version): taking
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010, 2011  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Reference-counted encapsulation of a pointer to node information.
 **
 ** Reference-counted encapsulation of a pointer to node information.
 **/

#include "cvc4_private.h"

// circular dependency
#include "expr/node_value.h"

#ifndef __CVC4__TYPE_NODE_H
#define __CVC4__TYPE_NODE_H

#include <vector>
#include <string>
#include <iostream>
#include <stdint.h>

#include "expr/kind.h"
#include "expr/metakind.h"
#include "util/Assert.h"
#include "util/cardinality.h"

namespace CVC4 {

class NodeManager;

namespace expr {
  class NodeValue;
}/* CVC4::expr namespace */

/**
 * Encapsulation of an NodeValue pointer for Types. The reference count is
 * maintained in the NodeValue.
 */
class TypeNode {

public:

  // for hash_maps, hash_sets..
  struct HashFunction {
    size_t operator()(TypeNode node) const {
      return (size_t) node.getId();
    }
  };/* struct HashFunction */

private:

  /**
   * The NodeValue has access to the private constructors, so that the
   * iterators can can create new types.
   */
  friend class expr::NodeValue;

  /** A convenient null-valued encapsulated pointer */
  static TypeNode s_null;

  /** The referenced NodeValue */
  expr::NodeValue* d_nv;

  /**
   * This constructor is reserved for use by the TypeNode package.
   */
  explicit TypeNode(const expr::NodeValue*);

  friend class NodeManager;

  template <unsigned nchild_thresh>
  friend class NodeBuilder;

  /**
   * Assigns the expression value and does reference counting. No
   * assumptions are made on the expression, and should only be used
   * if we know what we are doing.
   *
   * @param ev the expression value to assign
   */
  void assignNodeValue(expr::NodeValue* ev);

  /**
   * Cache-aware, recursive version of substitute() used by the public
   * member function with a similar signature.
   */
  TypeNode substitute(const TypeNode& type, const TypeNode& replacement,
                      std::hash_map<TypeNode, TypeNode, HashFunction>& cache) const;

  /**
   * Cache-aware, recursive version of substitute() used by the public
   * member function with a similar signature.
   */
  template <class Iterator1, class Iterator2>
  TypeNode substitute(Iterator1 typesBegin, Iterator1 typesEnd,
                      Iterator2 replacementsBegin, Iterator2 replacementsEnd,
                      std::hash_map<TypeNode, TypeNode, HashFunction>& cache) const;

public:

  /** Default constructor, makes a null expression. */
  TypeNode() : d_nv(&expr::NodeValue::s_null) { }

  /** Copy constructor */
  TypeNode(const TypeNode& node);

  /**
   * Destructor. If ref_count is true it will decrement the reference count
   * and, if zero, collect the NodeValue.
   */
  ~TypeNode() throw(AssertionException);

  /**
   * Assignment operator for nodes, copies the relevant information from node
   * to this node.
   *
   * @param typeNode the node to copy
   * @return reference to this node
   */
  TypeNode& operator=(const TypeNode& typeNode);

  /**
   * Return the null node.
   *
   * @return the null node
   */
  static TypeNode null() {
    return s_null;
  }

  /**
   * Substitution of TypeNodes.
   */
  inline TypeNode
  substitute(const TypeNode& type, const TypeNode& replacement) const;

  /**
   * Simultaneous substitution of TypeNodes.
   */
  template <class Iterator1, class Iterator2>
  inline TypeNode
  substitute(Iterator1 typesBegin, Iterator1 typesEnd,
             Iterator2 replacementsBegin, Iterator2 replacementsEnd) const;

  /**
   * Structural comparison operator for expressions.
   *
   * @param typeNode the type node to compare to
   * @return true if expressions are equal, false otherwise
   */
  bool operator==(const TypeNode& typeNode) const {
    return
      d_nv == typeNode.d_nv ||
      (typeNode.isReal() && this->isReal());
  }

  /**
   * Structural comparison operator for expressions.
   *
   * @param typeNode the type node to compare to
   * @return true if expressions are equal, false otherwise
   */
  bool operator!=(const TypeNode& typeNode) const {
    return !(*this == typeNode);
  }

  /**
   * We compare by expression ids so, keeping things deterministic and having
   * that subexpressions have to be smaller than the enclosing expressions.
   *
   * @param typeNode the node to compare to
   * @return true if this expression is smaller
   */
  inline bool operator<(const TypeNode& typeNode) const {
    return d_nv->d_id < typeNode.d_nv->d_id;
  }

  /**
   * Returns the i-th child of this node.
   *
   * @param i the index of the child
   * @return the node representing the i-th child
   */
  inline TypeNode operator[](int i) const {
    return TypeNode(d_nv->getChild(i));
  }

  /**
   * Returns the unique id of this node
   *
   * @return the id
   */
  inline unsigned getId() const {
    return d_nv->getId();
  }

  /**
   * Returns the kind of this type node.
   *
   * @return the kind
   */
  inline Kind getKind() const {
    return Kind(d_nv->d_kind);
  }

  /**
   * Returns the metakind of this type node.
   *
   * @return the metakind
   */
  inline kind::MetaKind getMetaKind() const {
    return kind::metaKindOf(getKind());
  }

  /**
   * Returns the number of children this node has.
   *
   * @return the number of children
   */
  inline size_t getNumChildren() const;

  /**
   * If this is a CONST_* TypeNode, extract the constant from it.
   */
  template <class T>
  inline const T& getConst() const;

  /**
   * Returns the value of the given attribute that this has been attached.
   *
   * @param attKind the kind of the attribute
   * @return the value of the attribute
   */
  template <class AttrKind>
  inline typename AttrKind::value_type
  getAttribute(const AttrKind& attKind) const;

  // Note that there are two, distinct hasAttribute() declarations for
  // a reason (rather than using a pointer-valued argument with a
  // default value): they permit more optimized code in the underlying
  // hasAttribute() implementations.

  /**
   * Returns true if this node has been associated an attribute of
   * given kind.  Additionally, if a pointer to the value_kind is
   * give, and the attribute value has been set for this node, it will
   * be returned.
   *
   * @param attKind the kind of the attribute
   * @return true if this node has the requested attribute
   */
  template <class AttrKind>
  inline bool hasAttribute(const AttrKind& attKind) const;

  /**
   * Returns true if this node has been associated an attribute of given kind.
   * Additionaly, if a pointer to the value_kind is give, and the attribute
   * value has been set for this node, it will be returned.
   *
   * @param attKind the kind of the attribute
   * @param value where to store the value if it exists
   * @return true if this node has the requested attribute
   */
  template <class AttrKind>
  inline bool getAttribute(const AttrKind& attKind,
                           typename AttrKind::value_type& value) const;

  /**
   * Sets the given attribute of this node to the given value.
   *
   * @param attKind the kind of the atribute
   * @param value the value to set the attribute to
   */
  template <class AttrKind>
  inline void setAttribute(const AttrKind& attKind,
                           const typename AttrKind::value_type& value);

  /** Iterator allowing for scanning through the children. */
  typedef expr::NodeValue::iterator<TypeNode> iterator;
  /** Constant iterator allowing for scanning through the children. */
  typedef expr::NodeValue::iterator<TypeNode> const_iterator;

  /**
   * Returns the iterator pointing to the first child.
   *
   * @return the iterator
   */
  inline iterator begin() {
    return d_nv->begin<TypeNode>();
  }

  /**
   * Returns the iterator pointing to the end of the children (one
   * beyond the last one.
   *
   * @return the end of the children iterator.
   */
  inline iterator end() {
    return d_nv->end<TypeNode>();
  }

  /**
   * Returns the const_iterator pointing to the first child.
   *
   * @return the const_iterator
   */
  inline const_iterator begin() const {
    return d_nv->begin<TypeNode>();
  }

  /**
   * Returns the const_iterator pointing to the end of the children
   * (one beyond the last one.
   *
   * @return the end of the children const_iterator.
   */
  inline const_iterator end() const {
    return d_nv->end<TypeNode>();
  }

  /**
   * Converts this type into a string representation.
   *
   * @return the string representation of this type.
   */
  inline std::string toString() const {
    return d_nv->toString();
  }

  /**
   * Converts this node into a string representation and sends it to the
   * given stream
   *
   * @param out the stream to serialize this node to
   * @param toDepth the depth to which to print this expression, or -1 to
   * print it fully
   * @param types set to true to ascribe types to the output expressions
   * (might break language compliance, but good for debugging expressions)
   * @param language the language in which to output
   */
  inline void toStream(std::ostream& out, int toDepth = -1, bool types = false,
                       OutputLanguage language = language::output::LANG_AST) const {
    d_nv->toStream(out, toDepth, types, language);
  }

  /**
   * Very basic pretty printer for Node.
   *
   * @param out output stream to print to.
   * @param indent number of spaces to indent the formula by.
   */
  void printAst(std::ostream& out, int indent = 0) const;

  /**
   * Returns true if this type is a null type.
   *
   * @return true if null
   */
  bool isNull() const {
    return d_nv == &expr::NodeValue::s_null;
  }

  /**
   * Convert this TypeNode into a Type using the currently-in-scope
   * manager.
   */
  inline Type toType();

  /**
   * Convert a Type into a TypeNode.
   */
  inline static TypeNode fromType(const Type& t);

  /**
   * Returns the cardinality of this type.
   *
   * @return a finite or infinite cardinality
   */
  Cardinality getCardinality() const;

  /**
   * Returns whether this type is well-founded.  A type is
   * well-founded if there exist ground terms.
   *
   * @return true iff the type is well-founded
   */
  bool isWellFounded() const;

  /**
   * Construct and return a ground term of this type.  If the type is
   * not well founded, this function throws an exception.
   *
   * @return a ground term of the type
   */
  Node mkGroundTerm() const;

  /** Is this the Boolean type? */
  bool isBoolean() const;

  /** Is this the Integer type? */
  bool isInteger() const;

  /** Is this the Real type? */
  bool isReal() const;

  /** Is this an array type? */
  bool isArray() const;

  /** Get the index type (for array types) */
  TypeNode getArrayIndexType() const;

  /** Get the element type (for array types) */
  TypeNode getArrayConstituentType() const;

  /** Get the return type (for constructor types) */
  TypeNode getConstructorRangeType() const;

  /**
   * Is this a function type?  Function-like things (e.g. datatype
   * selectors) that aren't actually functions are NOT considered
   * functions, here.
   */
  bool isFunction() const;

  /**
   * Is this a function-LIKE type?  Function-like things
   * (e.g. datatype selectors) that aren't actually functions ARE
   * considered functions, here.  The main point is that this is used
   * to avoid anything higher-order: anything function-like cannot be
   * the argument or return value for anything else function-like.
   *
   * Arrays are explicitly *not* function-like for the purposes of
   * this test.  However, functions still cannot contain anything
   * function-like.
   */
  bool isFunctionLike() const;

  /**
   * Get the argument types of a function, datatype constructor,
   * datatype selector, or datatype tester.
   */
  std::vector<TypeNode> getArgTypes() const;

  /**
   * Get the paramater types of a parameterized datatype.  Fails an
   * assertion if this type is not a parametric datatype.
   */
  std::vector<TypeNode> getParamTypes() const;

  /**
   * Get the range type (i.e., the type of the result) of a function,
   * datatype constructor, datatype selector, or datatype tester.
   */
  TypeNode getRangeType() const;

  /**
   * Is this a predicate type?  NOTE: all predicate types are also
   * function types (so datatype testers are NOT considered
   * "predicates" for the purpose of this function).
   */
  bool isPredicate() const;

  /** Is this a tuple type? */
  bool isTuple() const;

  /** Get the constituent types of a tuple type */
  std::vector<TypeNode> getTupleTypes() const;

  /** Is this a bit-vector type */
  bool isBitVector() const;

  /** Is this a bit-vector type of size <code>size</code> */
  bool isBitVector(unsigned size) const;

  /** Is this a datatype type */
  bool isDatatype() const;

  /** Is this a parameterized datatype type */
  bool isParametricDatatype() const;

  /** Is this a fully instantiated datatype type */
  bool isInstantiatedDatatype() const;

  /** Is this an instantiated datatype parameter */
  bool isParameterInstantiatedDatatype(unsigned n) const;

  /** Is this a constructor type */
  bool isConstructor() const;

  /** Is this a selector type */
  bool isSelector() const;

  /** Is this a tester type */
  bool isTester() const;

  /** Get the size of this bit-vector type */
  unsigned getBitVectorSize() const;

  /** Is this a sort kind */
  bool isSort() const;

  /** Is this a sort constructor kind */
  bool isSortConstructor() const;

  /** Is this a kind type (i.e., the type of a type)? */
  bool isKind() const;

private:

  /**
   * Indents the given stream a given amount of spaces.
   *
   * @param out the stream to indent
   * @param indent the number of spaces
   */
  static void indent(std::ostream& out, int indent) {
    for(int i = 0; i < indent; i++) {
      out << ' ';
    }
  }

};/* class TypeNode */

/**
 * Serializes a given node to the given stream.
 *
 * @param out the output stream to use
 * @param n the node to output to the stream
 * @return the stream
 */
inline std::ostream& operator<<(std::ostream& out, const TypeNode& n) {
  n.toStream(out,
             Node::setdepth::getDepth(out),
             Node::printtypes::getPrintTypes(out),
             Node::setlanguage::getLanguage(out));
  return out;
}

typedef TypeNode::HashFunction TypeNodeHashFunction;

}/* CVC4 namespace */

#include <ext/hash_map>

#include "expr/node_manager.h"

namespace CVC4 {

inline Type TypeNode::toType() {
  return NodeManager::currentNM()->toType(*this);
}

inline TypeNode TypeNode::fromType(const Type& t) {
  return NodeManager::fromType(t);
}

inline TypeNode
TypeNode::substitute(const TypeNode& type,
                     const TypeNode& replacement) const {
  std::hash_map<TypeNode, TypeNode, HashFunction> cache;
  return substitute(type, replacement, cache);
}

template <class Iterator1, class Iterator2>
inline TypeNode
TypeNode::substitute(Iterator1 typesBegin,
                     Iterator1 typesEnd,
                     Iterator2 replacementsBegin,
                     Iterator2 replacementsEnd) const {
  std::hash_map<TypeNode, TypeNode, HashFunction> cache;
  return substitute(typesBegin, typesEnd,
                    replacementsBegin, replacementsEnd, cache);
}

template <class Iterator1, class Iterator2>
TypeNode TypeNode::substitute(Iterator1 typesBegin,
                              Iterator1 typesEnd,
                              Iterator2 replacementsBegin,
                              Iterator2 replacementsEnd,
                              std::hash_map<TypeNode, TypeNode, HashFunction>& cache) const {
  // in cache?
  std::hash_map<TypeNode, TypeNode, HashFunction>::const_iterator i = cache.find(*this);
  if(i != cache.end()) {
    return (*i).second;
  }

  // otherwise compute
  Assert( typesEnd - typesBegin == replacementsEnd - replacementsBegin,
          "Substitution iterator ranges must be equal size" );
  Iterator1 j = find(typesBegin, typesEnd, *this);
  if(j != typesEnd) {
    TypeNode tn = *(replacementsBegin + (j - typesBegin));
    cache[*this] = tn;
    return tn;
  } else if(getNumChildren() == 0) {
    cache[*this] = *this;
    return *this;
  } else {
    NodeBuilder<> nb(getKind());
    if(getMetaKind() == kind::metakind::PARAMETERIZED) {
      // push the operator
      nb << TypeNode(d_nv->d_children[0]);
    }
    for(TypeNode::const_iterator i = begin(),
          iend = end();
        i != iend;
        ++i) {
      nb << (*i).substitute(typesBegin, typesEnd,
                            replacementsBegin, replacementsEnd, cache);
    }
    TypeNode tn = nb.constructTypeNode();
    cache[*this] = tn;
    return tn;
  }
}

inline size_t TypeNode::getNumChildren() const {
  return d_nv->getNumChildren();
}

template <class T>
inline const T& TypeNode::getConst() const {
  return d_nv->getConst<T>();
}

inline TypeNode::TypeNode(const expr::NodeValue* ev) :
  d_nv(const_cast<expr::NodeValue*> (ev)) {
  Assert(d_nv != NULL, "Expecting a non-NULL expression value!");
  d_nv->inc();
}

inline TypeNode::TypeNode(const TypeNode& typeNode) {
  Assert(typeNode.d_nv != NULL, "Expecting a non-NULL expression value!");
  d_nv = typeNode.d_nv;
  d_nv->inc();
}

inline TypeNode::~TypeNode() throw(AssertionException) {
  Assert(d_nv != NULL, "Expecting a non-NULL expression value!");
  d_nv->dec();
}

inline void TypeNode::assignNodeValue(expr::NodeValue* ev) {
  d_nv = ev;
  d_nv->inc();
}

inline TypeNode& TypeNode::operator=(const TypeNode& typeNode) {
  Assert(d_nv != NULL, "Expecting a non-NULL expression value!");
  Assert(typeNode.d_nv != NULL,
         "Expecting a non-NULL expression value on RHS!");
  if(EXPECT_TRUE( d_nv != typeNode.d_nv )) {
    d_nv->dec();
    d_nv = typeNode.d_nv;
    d_nv->inc();
  }
  return *this;
}

template <class AttrKind>
inline typename AttrKind::value_type TypeNode::
getAttribute(const AttrKind&) const {
  Assert( NodeManager::currentNM() != NULL,
          "There is no current CVC4::NodeManager associated to this thread.\n"
          "Perhaps a public-facing function is missing a NodeManagerScope ?" );
  return NodeManager::currentNM()->getAttribute(d_nv, AttrKind());
}

template <class AttrKind>
inline bool TypeNode::
hasAttribute(const AttrKind&) const {
  Assert( NodeManager::currentNM() != NULL,
          "There is no current CVC4::NodeManager associated to this thread.\n"
          "Perhaps a public-facing function is missing a NodeManagerScope ?" );
  return NodeManager::currentNM()->hasAttribute(d_nv, AttrKind());
}

template <class AttrKind>
inline bool TypeNode::getAttribute(const AttrKind&, typename AttrKind::value_type& ret) const {
  Assert( NodeManager::currentNM() != NULL,
          "There is no current CVC4::NodeManager associated to this thread.\n"
          "Perhaps a public-facing function is missing a NodeManagerScope ?" );
  return NodeManager::currentNM()->getAttribute(d_nv, AttrKind(), ret);
}

template <class AttrKind>
inline void TypeNode::
setAttribute(const AttrKind&, const typename AttrKind::value_type& value) {
  Assert( NodeManager::currentNM() != NULL,
          "There is no current CVC4::NodeManager associated to this thread.\n"
          "Perhaps a public-facing function is missing a NodeManagerScope ?" );
  NodeManager::currentNM()->setAttribute(d_nv, AttrKind(), value);
}

inline void TypeNode::printAst(std::ostream& out, int indent) const {
  d_nv->printAst(out, indent);
}

#ifdef CVC4_DEBUG
/**
 * Pretty printer for use within gdb.  This is not intended to be used
 * outside of gdb.  This writes to the Warning() stream and immediately
 * flushes the stream.
 *
 * Note that this function cannot be a template, since the compiler
 * won't instantiate it.  Even if we explicitly instantiate.  (Odd?)
 * So we implement twice.  We mark as __attribute__((used)) so that
 * GCC emits code for it even though static analysis indicates it's
 * never called.
 *
 * Tim's Note: I moved this into the node.h file because this allows gdb
 * to find the symbol, and use it, which is the first standard this code needs
 * to meet. A cleaner solution is welcomed.
 */
static void __attribute__((used)) debugPrintTypeNode(const TypeNode& n) {
  Warning() << Node::setdepth(-1)
            << Node::setlanguage(language::output::LANG_AST)
            << n << std::endl;
  Warning().flush();
}
static void __attribute__((used)) debugPrintRawTypeNode(const TypeNode& n) {
  n.printAst(Warning(), 0);
  Warning().flush();
}
#endif /* CVC4_DEBUG */

}/* CVC4 namespace */

#endif /* __CVC4__NODE_H */
