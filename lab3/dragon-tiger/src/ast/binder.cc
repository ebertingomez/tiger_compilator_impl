#include <sstream>

#include "binder.hh"
#include "../utils/errors.hh"
#include "../utils/nolocation.hh"

using utils::error;
using utils::non_fatal_error;

namespace ast {
namespace binder {

/* Returns the current scope */
scope_t &Binder::current_scope() { return scopes.back(); }

/* Pushes a new scope on the stack */
void Binder::push_scope() { scopes.push_back(scope_t()); }

/* Pops the current scope from the stack */
void Binder::pop_scope() { scopes.pop_back(); }

/* Enter a declaration in the current scope. Raises an error if the declared name
 * is already defined */
void Binder::enter(Decl &decl) {
  scope_t &scope = current_scope();
  auto previous = scope.find(decl.name);
  if (previous != scope.end()) {
    non_fatal_error(decl.loc,
                    decl.name.get() + " is already defined in this scope");
    error(previous->second->loc, "previous declaration was here");
  }
  scope[decl.name] = &decl;
}

/* Finds the declaration for a given name. The scope stack is traversed
 * front to back starting from the current scope. The first matching
 * declaration is returned. Raises an error, if no declaration matches. */
Decl &Binder::find(const location loc, const Symbol &name) {
  for (auto scope = scopes.crbegin(); scope != scopes.crend(); scope++) {
    auto decl_entry = scope->find(name);
    if (decl_entry != scope->cend()) {
      return *decl_entry->second;
    }
  }
  error(loc, name.get() + " cannot be found in this scope");
}

Binder::Binder() : scopes() {
  /* Create the top-level scope */
  push_scope();

  /* Populate the top-level scope with all the primitive declarations */
  enter_primitive("print_err", "void", {"string"});
  enter_primitive("print", "void", {"string"});
  enter_primitive("print_int", "void", {"int"});
  enter_primitive("flush", "void", {});
  enter_primitive("getchar", "string", {});
  enter_primitive("ord", "int", {"string"});
  enter_primitive("chr", "string", {"int"});
  enter_primitive("size", "int", {"string"});
  enter_primitive("substring", "string", {"string", "int", "int"});
  enter_primitive("concat", "string", {"string", "string"});
  enter_primitive("strcmp", "int", {"string", "string"});
  enter_primitive("streq", "int", {"string", "string"});
  enter_primitive("not", "int", {"int"});
  enter_primitive("exit", "void", {"int"});
}

/* Declares a new primitive into the current scope*/
void Binder::enter_primitive(
    const std::string &name, const std::string &type_name,
    const std::vector<std::string> &argument_typenames) {
  std::vector<VarDecl *> args;
  int counter = 0;
  for (const std::string &tn : argument_typenames) {
    std::ostringstream argname;
    argname << "a_" << counter++;
    args.push_back(
        new VarDecl(utils::nl, Symbol(argname.str()), nullptr, Symbol(tn)));
  }

  FunDecl *fd = new FunDecl(utils::nl, Symbol(name), std::move(args), nullptr,
                            Symbol(type_name), true);
  fd->set_external_name(Symbol("__" + name));
  enter(*fd);
}

/* Sets the parent of a function declaration and computes and sets
 * its unique external name */
void Binder::set_parent_and_external_name(FunDecl &decl) {
  auto parent = functions.empty() ? nullptr : functions.back();
  Symbol external_name;
  if (parent) {
    decl.set_parent(parent);
    external_name = parent->get_external_name().get() + '.' + decl.name.get();
  } else
    external_name = decl.name;
  while (external_names.find(external_name) != external_names.end())
    external_name = Symbol(external_name.get() + '_');
  external_names.insert(external_name);
  decl.set_external_name(external_name);
}

/* Binds a whole program. This method wraps the program inside a top-level main
 * function.  Then, it visits the programs with the Binder visitor; binding
 * each identifier to its declaration and computing depths.*/
FunDecl *Binder::analyze_program(Expr &root) {
  std::vector<VarDecl *> main_params;
  Sequence *const main_body = new Sequence(
      utils::nl,
      std::vector<Expr *>({&root, new IntegerLiteral(utils::nl, 0)}));
  FunDecl *const main = new FunDecl(utils::nl, Symbol("main"), main_params,
                                    main_body, Symbol("int"), true);
  main->accept(*this);
  return main;
}
void Binder::visit(IntegerLiteral &literal) {
}

void Binder::visit(StringLiteral &literal) {
}

void Binder::visit(BinaryOperator &op) {
  op.get_left().accept(*this);
  op.get_right().accept(*this);
}

void Binder::visit(Sequence &seq) {
  const auto exprs = seq.get_exprs();
  for (auto expr = exprs.cbegin(); expr != exprs.cend(); expr++) {
    (*expr)->accept(*this);
  }
}

void Binder::visit(Let &let) {
  push_scope();
  std::vector<FunDecl *> decls;
  for (auto decl : let.get_decls()) {
    FunDecl * func_decl = dynamic_cast<FunDecl *>(decl);
    if  (func_decl != nullptr){
      auto decl_entry = current_scope().find(func_decl->name);
      if (decl_entry != current_scope().cend()) {
        error(func_decl->loc, func_decl->name.get() + " is trying to be declared twice");
      }
      decls.push_back(func_decl);
      func_decl->set_depth(static_cast<int>(scopes.size()));
      enter(*func_decl);
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
  pop_scope();
}

void Binder::visit(Identifier &id) {
  VarDecl * decl = dynamic_cast<VarDecl *>(& find(id.loc, id.name));
  if (decl == nullptr)
    error(id.loc, id.name.get() + " is not a function call");
  id.set_decl(decl);
  id.set_depth(static_cast<int>(scopes.size()));
  if (id.get_depth() - decl->get_depth() > 0)
    decl->set_escapes();
}

void Binder::visit(IfThenElse &ite) {
  ite.get_condition().accept(*this);
  ite.get_then_part().accept(*this);
  ite.get_else_part().accept(*this);
}

void Binder::visit(VarDecl &decl) {
  auto decl_entry = current_scope().find(decl.name);
  if (decl_entry != current_scope().cend()) {
    error(decl.loc, decl.name.get() + " is trying to be declared twice");
  }
  if (auto expr = decl.get_expr()) {
    expr->accept(*this);
  }
  decl.set_depth(static_cast<int>(scopes.size()));
  enter(decl);
}

void Binder::visit(FunDecl &decl) {
  set_parent_and_external_name(decl);
  functions.push_back(&decl);

  push_scope();
  /* Parameters declaration */
  auto params = decl.get_params();
  for (auto param = params.cbegin(); param != params.cend(); param++) {
    (*param)->accept(*this);
  }
  /* Body definition */
  decl.get_expr()->accept(*this);
  pop_scope();

  functions.pop_back();
}

void Binder::visit(FunCall &call) {
  FunDecl * decl = dynamic_cast<FunDecl *>(& find(call.loc, call.func_name));
  if (decl == nullptr)
    error(call.loc, call.func_name.get() + " is not a function call");
  auto args = call.get_args();
  for (auto arg = args.cbegin(); arg != args.cend(); arg++) {
    (*arg)->accept(*this);
  }
  call.set_decl(decl);
  call.set_depth(static_cast<int>(scopes.size()));
}

void Binder::visit(WhileLoop &loop) {
  loop.get_condition().accept(*this);
  
  const auto exprs = ((Sequence * )&loop.get_body())->get_exprs();
  for (auto expr = exprs.cbegin(); expr != exprs.cend(); expr++) {
    Break * b = dynamic_cast<Break *>(*expr);
    if (b != nullptr)
      b->set_loop(&loop);
  }
  loop.get_body().accept(*this);
}

void Binder::visit(ForLoop &loop) {
  push_scope();
  loop.get_variable().accept(*this);
  loop.get_high().accept(*this);

  const auto exprs = ((Sequence * )&loop.get_body())->get_exprs();
  for (auto expr = exprs.cbegin(); expr != exprs.cend(); expr++) {
    Break * b = dynamic_cast<Break *>(*expr);
    if (b != nullptr)
      b->set_loop(&loop);
  }
  loop.get_body().accept(*this);
  pop_scope();
}

void Binder::visit(Break &b) {
  //if (!b.get_loop())
    error( " There is a break outside a loop");
}

void Binder::visit(Assign &assign) {
  assign.get_lhs().accept(*this);
  if (assign.get_lhs().get_decl()->read_only)
    error(assign.get_lhs().get_decl()->loc, assign.get_lhs().get_decl()->name.get() + " is trying to be assigned but is a loop var");
  assign.get_rhs().accept(*this);
}

} // namespace binder
} // namespace ast
