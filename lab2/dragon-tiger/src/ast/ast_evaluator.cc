#include "../utils/errors.hh"
#include "ast_evaluator.hh"

namespace ast {

/* Evaluates an Integer, returns its value */
int32_t ASTEvaluator::visit(const IntegerLiteral &literal) {
    return literal.value;
}

int32_t ASTEvaluator::visit(const StringLiteral &literal) {
    utils::error("The expression could not be evaluated");
    return 0;
}

/* Evaluates both members of the operator and computes the result in function
of the operator */
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
    return 0;
}

/* Evaluates each element of the sequence and return the last value of the sequence
. If it is empty or there is an empty expression in the sequence, it raises and error */
int32_t ASTEvaluator::visit(const Sequence &seqExpr) {

    if (seqExpr.get_exprs().size() > 0) {
        const auto exprs = seqExpr.get_exprs();

        for (auto expr = exprs.cbegin(); expr != exprs.cend(); expr++) {
            if (*expr == NULL)
                utils::error("Early error in the sequence");
        }

        Expr * e;
        for (auto expr = exprs.cbegin(); expr != exprs.cend(); expr++) {
            (*expr)->accept(*this);
            e = *expr;      
        }
        return e->accept(*this);
    } else {
        utils::error("Empty sequence");
    }
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

/* Evaluates the condition and analyzes either the then branch or the else one 
accordingly. Return the result of the branch */
int32_t ASTEvaluator::visit(const IfThenElse &ite) {

    if (ite.get_condition().accept(*this))
        return ite.get_then_part().accept(*this);
    else {
        const Expr * expr = &ite.get_else_part();
        Sequence * seq = (Sequence *)expr;

        if (seq->get_exprs().size() > 0)
            return ite.get_else_part().accept(*this);
        else
            utils::error("Empty else");
    }
    return -1;

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
