#include "../utils/errors.hh"
#include "ast_evaluator.hh"

namespace ast {

void ASTEvaluator::visit(const IntegerLiteral &literal) {
    *ostream << literal.value;
}

void ASTEvaluator::visit(const StringLiteral &literal) {
    utils::error("The expression could not be evaluated");
}

void ASTEvaluator::visit(const BinaryOperator &binop) {
    int left = ((IntegerLiteral *)&binop.get_left())->value;
    int right = ((IntegerLiteral *)&binop.get_right())->value;
    if (operator_name[binop.op]=="+")
        *ostream<<left + right;
    else if (operator_name[binop.op]=="-") 
        *ostream<<left - right;
    else if (operator_name[binop.op]=="*") 
        *ostream<<left * right;
    else if (operator_name[binop.op]=="/") 
        *ostream<<left / right;
    else if (operator_name[binop.op]=="=") 
        *ostream<<((left == right)?"True":"False");
    else if (operator_name[binop.op]=="<>") 
        *ostream<<((left != right)?"True":"False");
    else if (operator_name[binop.op]=="<") 
        *ostream<<((left < right)?"True":"False");
    else if (operator_name[binop.op]=="<=") 
        *ostream<<((left <= right)?"True":"False");
    else if (operator_name[binop.op]==">") 
        *ostream<<((left > right)?"True":"False");
    else if (operator_name[binop.op]==">=") 
        *ostream<<((left >= right)?"True":"False");
}

void ASTEvaluator::visit(const Sequence &seqExpr) {
}

void ASTEvaluator::visit(const Let &let) {
    utils::error("The expression could not be evaluated");
}

void ASTEvaluator::visit(const Identifier &id) {
    utils::error("The expression could not be evaluated");
}

void ASTEvaluator::visit(const IfThenElse &ite) {
}

void ASTEvaluator::visit(const VarDecl &decl) {
    utils::error("The expression could not be evaluated");
}

void ASTEvaluator::visit(const FunDecl &decl) {
    utils::error("The expression could not be evaluated");
}

void ASTEvaluator::visit(const FunCall &call) {
    utils::error("The expression could not be evaluated");
}

void ASTEvaluator::visit(const WhileLoop &loop) {
    utils::error("The expression could not be evaluated");
}

void ASTEvaluator::visit(const ForLoop &loop) {
    utils::error("The expression could not be evaluated");
}

void ASTEvaluator::visit(const Break &brk) {
    utils::error("The expression could not be evaluated");
}

void ASTEvaluator::visit(const Assign &assign) {
    utils::error("The expression could not be evaluated");
}

} // namespace ast
