#ifndef AST_EVALUATOR_HH
#define AST_EVALUATOR_HH

#include <ostream>

#include "nodes.hh"

namespace ast {

class ASTEvaluator : public ConstASTVisitor {
  std::ostream *ostream;
  bool verbose;

public:
  ASTEvaluator(std::ostream *_ostream, bool _verbose)
      : ostream(_ostream), verbose(_verbose) {}
  void visit(const IntegerLiteral &) override ;
  void visit(const StringLiteral &) override ;
  void visit(const BinaryOperator &) override ;
  void visit(const Sequence &) override ;
  void visit(const Let &) override ;
  void visit(const Identifier &) override ;
  void visit(const IfThenElse &) override ;
  void visit(const VarDecl &) override ;
  void visit(const FunDecl &) override ;
  void visit(const FunCall &) override ;
  void visit(const WhileLoop &) override ;
  void visit(const ForLoop &) override ;
  void visit(const Break &) override ;
  void visit(const Assign &) override ;
};

} // namespace ast

#endif // _AST_EVALUATOR_HH
