#include <sstream>

#include "escaper.hh"
#include "../utils/errors.hh"
#include "../utils/nolocation.hh"

using utils::error;
using utils::non_fatal_error;

namespace ast {
namespace escaper {

void Escaper::visit(IntegerLiteral &literal) {
}

void Escaper::visit(StringLiteral &literal) {
}

void Escaper::visit(BinaryOperator &op) {
}

void Escaper::visit(Sequence &seq) {
  for (Expr * expr : seq.get_exprs())
    expr->accept(*this);
}

void Escaper::visit(Let &let) {
  for (Decl * decl : let.get_decls())
    decl->accept(*this);
  let.get_sequence().accept(*this);
}

void Escaper::visit(Identifier &id) {
}

void Escaper::visit(IfThenElse &ite) {
  ite.get_else_part().accept(*this);
  ite.get_then_part().accept(*this);
}

void Escaper::visit(VarDecl &decl) {
  if (decl.get_escapes())
   current_function->get_escaping_decls().push_back(&decl);
}

void Escaper::visit(FunDecl &decl) {
  FunDecl * old_function = current_function;
  current_function = &decl;
  for (VarDecl * varDecl : decl.get_params())
    varDecl->accept(*this);
  decl.get_expr()->accept(*this);
  current_function = old_function;
}

void Escaper::visit(FunCall &call) {
}

void Escaper::visit(WhileLoop &loop) {
  loop.get_body().accept(*this);
}

void Escaper::visit(ForLoop &loop) {
  loop.get_variable().accept(*this);
  loop.get_body().accept(*this);
}

void Escaper::visit(Break &b) {
}

void Escaper::visit(Assign &assign) {
}

} // namespace escaper
} // namespace ast
