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

}

void Escaper::visit(Let &let) {

}

void Escaper::visit(Identifier &id) {

}

void Escaper::visit(IfThenElse &ite) {

}

void Escaper::visit(VarDecl &decl) {

}

void Escaper::visit(FunDecl &decl) {

}

void Escaper::visit(FunCall &call) {

}

void Escaper::visit(WhileLoop &loop) {

}

void Escaper::visit(ForLoop &loop) {

}

void Escaper::visit(Break &b) {

}

void Escaper::visit(Assign &assign) {

}

} // namespace binder
} // namespace ast
