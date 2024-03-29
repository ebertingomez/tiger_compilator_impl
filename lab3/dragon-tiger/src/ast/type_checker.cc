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

/* Sets the type of the StringLiteral to string */
void TypeChecker::visit(StringLiteral &literal) {
  literal.set_type(t_string);
}

/* Checks for type consistency and sets the type of each expression of the
sequence. Its type is the one of the last analyzed expression. Otherwise it is void */
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

/* Checks for type consistency of the different parts of the ifthenelse node.
the condition has to be an int, and the then else parts must have the same type.
The type of the node is the same of either the else or then part.*/
void TypeChecker::visit(IfThenElse &ite) {
  ite.get_condition().accept(*this);
  ite.get_then_part().accept(*this);
  ite.get_else_part().accept(*this);

  if (ite.get_condition().get_type() != t_int)
    error(ite.loc, "The condition of the ifthenelse is not valid" );
  if (ite.get_then_part().get_type() != ite.get_else_part().get_type())
    error(ite.loc, "The if and else expression do nor have the same type");
  ite.set_type(ite.get_then_part().get_type());

}

/* Checks for type consistency of each declaration and of the body. The type is the same
as the sequence analyzed. */
void TypeChecker::visit(Let &let) {
  for (auto decl : let.get_decls())
    decl->accept(*this);
  let.get_sequence().accept(*this);
  let.set_type(let.get_sequence().get_type());
}

/* Checks the type of the vardecl or inferes it in function of the expression.
Checks for consitency between the explicit type (if any) and the expr type. */
void TypeChecker::visit(VarDecl &decl) {
  Type type = t_undef;
  if (decl.get_expr()){
    decl.get_expr()->accept(*this);
    type = decl.get_expr()->get_type();
  }

  if (decl.type_name){
    Type text_type;
    if (decl.type_name.get().get() == "int")
      text_type = t_int;
    else if (decl.type_name.get().get() == "string")
      text_type = t_string;
    else error(decl.loc, decl.name.get()+": Invalid type");

    if (type != t_undef){
      if (text_type == type)
        decl.set_type(type);
      else
        error(decl.loc, decl.name.get()+": Invalid type");
    }
    else
       decl.set_type(text_type);
  }
  else{
    if (type != t_void)
      decl.set_type(type);
    else
      error(decl.loc, decl.name.get()+": The type of the variable is void");
  }
}

/* Checks for consistency of the to members of the node according to the operator. */
void TypeChecker::visit(BinaryOperator &binop) {
  Expr * left = &binop.get_left();
  Expr * right = &binop.get_right();
  std::string op = operator_name[binop.op];
  left->accept(*this);
  right->accept(*this);
  if (left->get_type() == right->get_type()){
    if ((op=="+" || op=="-" || op=="*" || op=="/" ) && left->get_type()==t_int)
      binop.set_type(t_int);
    else if ((op==">=" || op=="<" || op=="<=" || op==">") && left->get_type() != t_void)
      binop.set_type(t_int);
    else if ((op=="|" || op=="&") && left->get_type()==t_int) 
      binop.set_type(t_int);
    else if ((op=="=" || op=="<>") && left->get_type() != t_void)
      binop.set_type(t_int);
    else
      error(binop.loc, ": Incorrect operand");
  } else {
    error(binop.loc, ": Incorrect comparison operand ");
  }
}

/* Sets the type as being the same as its declaration */
void TypeChecker::visit(Identifier &id) {
  if(!id.get_decl())
    error(id.loc, id.name.get()+": No declaration operand");
  id.set_type(id.get_decl()->get_type());
}

/* Sets the type of the funcDecl if it has not been done yet. Checks if there is an explicit 
type and set it. Otherwise, it sets void as type. It checks the function body too. It checks
if the explicit type correspond to the one of the body*/
void TypeChecker::visit(FunDecl &decl) {
  if (decl.get_type() != t_undef)
    return;
  
  /* Parameters declaration */
  for (auto param : decl.get_params())
    param->accept(*this);


  Type text_type;
  if (decl.type_name){
    if (decl.type_name.get().get() == "int")
      text_type = t_int;
    else if (decl.type_name.get().get() == "string")
      text_type = t_string;
    else if (decl.type_name.get().get() == "void" && decl.is_external)
      text_type = t_void;
    else
      error(decl.loc, decl.name.get()+":  unknown type");
  }
  else
    text_type = t_void;

  decl.set_type(text_type);

  Type expr_type;
  /* Body definition */
  if (auto expr = decl.get_expr()) {
    expr->accept(*this);
    expr_type = expr->get_type();
  }
  else
    expr_type = t_void;
  
  if (decl.is_external)
      return;
  if (text_type != expr_type)
    error(decl.loc, decl.name.get()+":  Type mismatch"); 
}

/* Checks for type consistency of the parameters of the function according to its definition.
Its type is the same is its declaration. */
void TypeChecker::visit(FunCall &call) {
  if(!call.get_decl())
    error(call.loc, call.func_name.get()+": No declaration in this call");
  
  for (auto arg : call.get_args())
    arg->accept(*this);

  if (call.get_decl()->get_type() == t_undef)
    call.get_decl()->accept(*this);

  if (call.get_type() != t_undef)
    return;
  
  call.set_type(call.get_decl()->get_type());

  if (call.get_args().size() != call.get_decl()->get_params().size())
    error(call.loc, call.get_decl()->name.get()+": number of arguments and parameters mismatch");
  
  std::vector<Expr *> args(call.get_args());
  std::vector<VarDecl *> params(call.get_decl()->get_params());
  while (!args.empty()){
    if (args.back()->get_type() != params.back()->get_type())
      error(call.loc, call.get_decl()->name.get()+": arguments and parameters type mismatch: "+std::to_string(args.back()->get_type())+" "+std::to_string(params.back()->get_type()));
    args.pop_back();
    params.pop_back();
  }
}

/* Checks for type consistency of the loop elements. The condition has to be 
int and the body void. The loop type is void*/
void TypeChecker::visit(WhileLoop &loop) {
  loop.get_condition().accept(*this);
  loop.get_body().accept(*this);
  if (loop.get_condition().get_type() != t_int)
    error(loop.loc, ": Condition type mismatch");
  if (loop.get_body().get_type() != t_void)
    error(loop.loc, ": Body type mismatch");

  loop.set_type(t_void);
}

/* Checks for type consistency of the loop elements. The variable and the high values
have to be integers. The loop body and the loop have to be void */
void TypeChecker::visit(ForLoop &loop) {
  loop.get_variable().accept(*this);
  loop.get_high().accept(*this);
  loop.get_body().accept(*this);

  if (loop.get_high().get_type() != t_int)
    error(loop.loc, ": high type mismatch");
  if (loop.get_variable().get_type() != t_int)
    error(loop.loc, ": index type mismatch");
  if (loop.get_body().get_type() != t_void)
    error(loop.loc, ": Body type mismatch");

  loop.set_type(t_void);
}

void TypeChecker::visit(Break &b) {
  b.set_type(t_void);
}

/* Checks for type consistency of the identifier and the expression of the assing. They
have to be the same. The assign type is void */
void TypeChecker::visit(Assign &assign) {
  assign.get_lhs().accept(*this);
  assign.get_rhs().accept(*this);
  if (assign.get_lhs().get_type() != assign.get_rhs().get_type())
    error(assign.loc, " has a variable - expression mismatch");
  assign.set_type(t_void);
}

} // namespace TypeChecker
} // namespace ast
