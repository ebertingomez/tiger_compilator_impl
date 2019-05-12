#include "../utils/errors.hh"
#include "ast_evaluator.hh"

namespace ast {

int32_t ASTEvaluator::visit(const IntegerLiteral &literal) {
    return literal.value;
}

int32_t ASTEvaluator::visit(const StringLiteral &literal) {
    utils::error("The expression could not be evaluated");
    return 0;
}

int32_t ASTEvaluator::visit(const BinaryOperator &binop) {

    if (operator_name[binop.op]=="+")
        return binop.get_left().accept(*this) + binop.get_right().accept(*this);
    else if (operator_name[binop.op]=="-") 
        return binop.get_left().accept(*this) - binop.get_right().accept(*this);
    else if (operator_name[binop.op]=="*")
        return binop.get_left().accept(*this) * binop.get_right().accept(*this);
    else if (operator_name[binop.op]=="/") 
        return binop.get_left().accept(*this) / binop.get_right().accept(*this);
    else if (operator_name[binop.op]=="=") 
        return ((binop.get_left().accept(*this) == binop.get_right().accept(*this))?1:0);
    else if (operator_name[binop.op]=="<>") 
        return ((binop.get_left().accept(*this) != binop.get_right().accept(*this))?1:0);
    else if (operator_name[binop.op]=="<") 
        return ((binop.get_left().accept(*this) < binop.get_right().accept(*this))?1:0);
    else if (operator_name[binop.op]=="<=") 
        return ((binop.get_left().accept(*this) <= binop.get_right().accept(*this))?1:0);
    else if (operator_name[binop.op]==">") 
        return ((binop.get_left().accept(*this) > binop.get_right().accept(*this))?1:0);
    else if (operator_name[binop.op]==">=") 
        return ((binop.get_left().accept(*this) >= binop.get_right().accept(*this))?1:0);
    else if (operator_name[binop.op]=="|") 
        return ((binop.get_left().accept(*this) || binop.get_right().accept(*this))?1:0);
    else if (operator_name[binop.op]=="&") 
        return ((binop.get_left().accept(*this) && binop.get_right().accept(*this))?1:0);
}

int32_t ASTEvaluator::visit(const Sequence &seqExpr) {
    return 0;
}

int32_t ASTEvaluator::visit(const Let &let) {
    utils::error("The expression could not be evaluated");
    return 0;
}

int32_t ASTEvaluator::visit(const Identifier &id) {
    utils::error("The expression could not be evaluated");
    return 0;
}

int32_t ASTEvaluator::visit(const IfThenElse &ite) {
    
    if (ite.get_condition().accept(*this))
        return ite.get_then_part().accept(*this);
    else
        return ite.get_else_part().accept(*this);
}

int32_t ASTEvaluator::visit(const VarDecl &decl) {
    utils::error("The expression could not be evaluated");
    return 0;
}

int32_t ASTEvaluator::visit(const FunDecl &decl) {
    utils::error("The expression could not be evaluated");
    return 0;
}

int32_t ASTEvaluator::visit(const FunCall &call) {
    utils::error("The expression could not be evaluated");
    return 0;
}

int32_t ASTEvaluator::visit(const WhileLoop &loop) {
    utils::error("The expression could not be evaluated");
    return 0;
}

int32_t ASTEvaluator::visit(const ForLoop &loop) {
    utils::error("The expression could not be evaluated");
    return 0;
}

int32_t ASTEvaluator::visit(const Break &brk) {
    utils::error("The expression could not be evaluated");
    return 0;
}

int32_t ASTEvaluator::visit(const Assign &assign) {
    utils::error("The expression could not be evaluated");
    return 0;
}

} // namespace ast
