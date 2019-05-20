#ifndef TYPE_CHECKER_HH
#define TYPE_CHECKER_HH

#include <unordered_map>
#include <unordered_set>

#include "nodes.hh"

namespace ast {
namespace type_checker {

class TypeChecker : public ASTVisitor {

public:
  TypeChecker();
  void analyze_program(FunDecl &);
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

#endif // TYPE_CHECKER_HH
