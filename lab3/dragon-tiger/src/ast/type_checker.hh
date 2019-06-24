#ifndef TYPE_CHECKER_HH
#define TYPE_CHECKER_HH

#include "nodes.hh"

namespace ast {
namespace type_checker {

class TypeChecker : public ASTVisitor {

public:
  virtual void visit(IntegerLiteral &) override;
  virtual void visit(StringLiteral &) override;
  virtual void visit(BinaryOperator &) override;
  virtual void visit(Sequence &) override;
  virtual void visit(Let &) override;
  virtual void visit(Identifier &) override;
  virtual void visit(IfThenElse &) override;
  virtual void visit(VarDecl &) override;
  virtual void visit(FunDecl &) override;
  virtual void visit(FunCall &) override;
  virtual void visit(WhileLoop &) override;
  virtual void visit(ForLoop &) override;
  virtual void visit(Break &) override;
  virtual void visit(Assign &) override;
};

} // namespace binder
} // namespace ast

#endif // TYPE_CHECKER_HH
