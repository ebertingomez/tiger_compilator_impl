#include <sstream>

#include "type_checker.hh"
#include "../utils/errors.hh"
#include "../utils/nolocation.hh"

using utils::error;
using utils::non_fatal_error;

namespace ast {
namespace type_checker {

/* Binds a whole program. This method wraps the program inside a top-level main
 * function.  Then, it visits the programs with the TypeChecker visitor; binding
 * each identifier to its declaration and computing depths.*/
void TypeChecker::visit(IntegerLiteral &literal) {
  literal.set_type(t_int);
}

void TypeChecker::visit(StringLiteral &literal) {
  literal.set_type(t_string);
}

void TypeChecker::visit(Sequence &seq) {
  if (seq.get_exprs().size()>0){
    const auto exprs = seq.get_exprs();
    Expr * last = exprs.front();
    for (auto expr : exprs) {
      expr->accept(*this);
      last = expr;
    }
    seq.set_type(last->get_type());
  } else
    seq.set_type(t_void);
}

void TypeChecker::visit(IfThenElse &ite) {
  ite.get_condition().accept(*this);
  ite.get_then_part().accept(*this);
  ite.get_else_part().accept(*this);

  if (ite.get_condition().get_type() != t_int){
    error(ite.loc, "The condition of the ifthenelse is not valid" );
  }
  if (ite.get_then_part().get_type() != ite.get_then_part().get_type()){
    error(ite.loc, "The if and else expression do nor have the same type" );
  }
  ite.set_type(ite.get_then_part().get_type());

}

void TypeChecker::visit(BinaryOperator &op) {
  op.get_left().accept(*this);
  op.get_right().accept(*this);
}

void TypeChecker::visit(Let &let) {
  std::vector<FunDecl *> decls;
  for (auto decl : let.get_decls()) {
    FunDecl * func_decl = dynamic_cast<FunDecl *>(decl);
    if  (func_decl != nullptr){
      decls.push_back(func_decl);
    } 
    else {
      while (!decls.empty()){
        decls.back()->accept(*this);
        decls.pop_back();
      }
      decl->accept(*this);
    }
  }
  while (!decls.empty()){
    decls.back()->accept(*this);
    decls.pop_back();
  }
  let.get_sequence().accept(*this);

}

void TypeChecker::visit(Identifier &id) {

}

void TypeChecker::visit(VarDecl &decl) {
  if (auto expr = decl.get_expr()) {
    expr->accept(*this);
  }

}

void TypeChecker::visit(FunDecl &decl) {

  /* Parameters declaration */
  for (auto param : decl.get_params()) {
    if (param->name == decl.name)
      error(decl.loc, decl.name.get() + " has a parameter with the same name");
    param->accept(*this);
  }
  /* Body definition */
  if (auto expr = decl.get_expr()) {
    decl.get_expr()->accept(*this);
  }
}

void TypeChecker::visit(FunCall &call) {

  for (auto arg : call.get_args()) {
    arg->accept(*this);
  }

}

void TypeChecker::visit(WhileLoop &loop) {
  loop.get_condition().accept(*this);

  loop.get_body().accept(*this);

}

void TypeChecker::visit(ForLoop &loop) {
  loop.get_variable().accept(*this);
  loop.get_high().accept(*this);

  loop.get_body().accept(*this);
}

void TypeChecker::visit(Break &b) {

}

void TypeChecker::visit(Assign &assign) {
  assign.get_lhs().accept(*this);
  if (assign.get_lhs().get_decl()->read_only)
    error(assign.get_lhs().get_decl()->loc, assign.get_lhs().get_decl()->name.get() + " is trying to be assigned but is a loop var");
  assign.get_rhs().accept(*this);
}

} // namespace TypeChecker
} // namespace ast
