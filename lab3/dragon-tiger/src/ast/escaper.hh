#ifndef ESCAPER_HH
#define ESCAPER_HH

#include <unordered_map>
#include <unordered_set>

#include "nodes.hh"

namespace ast {
namespace escaper {

class Escaper : public ASTVisitor {

public:
  Escaper();
  virtual void visit(IntegerLiteral &);
  virtual void visit(StringLiteral &);
  virtual void visit(BinaryOperator &);
  virtual void visit(Sequence &);
  virtual void visit(Let &);
  virtual void visit(Identifier &);
  virtual void visit(IfThenElse &);
  virtual void visit(VarDecl &);
  virtual void visit(FunDecl &);
  virtual void visit(FunCall &);
  virtual void visit(WhileLoop &);
  virtual void visit(ForLoop &);
  virtual void visit(Break &);
  virtual void visit(Assign &);
};

} // namespace binder
} // namespace ast

#endif // ESCAPER_HH
