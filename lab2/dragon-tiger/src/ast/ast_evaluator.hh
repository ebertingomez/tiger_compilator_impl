#ifndef AST_EVALUATOR_HH
#define AST_EVALUATOR_HH

#include <ostream>

#include "nodes.hh"

namespace ast {
/**
 * @brief Implementation of a visitor to evaluate an AST. It determines
 *        the numerical value of it if possible. Otherwise, it raises an error
 * 
 */
class ASTEvaluator : public ConstASTIntVisitor {
  std::ostream *ostream;
  bool verbose;
  unsigned indent_level = 0;

public:
  ASTEvaluator(std::ostream *_ostream, bool _verbose)
      : ostream(_ostream), verbose(_verbose) {}
  void nl() {
    *ostream << std::endl;
    for (unsigned i = 0; i < indent_level; i++)
      *ostream << "  ";
  };
  int32_t visit(const IntegerLiteral &) override ;
  int32_t visit(const StringLiteral &) override ;
  int32_t visit(const BinaryOperator &) override ;
  int32_t visit(const Sequence &) override ;
  int32_t visit(const Let &) override ;
  int32_t visit(const Identifier &) override ;
  int32_t visit(const IfThenElse &) override ;
  int32_t visit(const VarDecl &) override ;
  int32_t visit(const FunDecl &) override ;
  int32_t visit(const FunCall &) override ;
  int32_t visit(const WhileLoop &) override ;
  int32_t visit(const ForLoop &) override ;
  int32_t visit(const Break &) override ;
  int32_t visit(const Assign &) override ;
};

} // namespace ast

#endif // _AST_EVALUATOR_HH
