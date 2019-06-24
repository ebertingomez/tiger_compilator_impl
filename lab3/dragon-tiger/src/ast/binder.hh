#ifndef BINDER_HH
#define BINDER_HH

#include <unordered_map>
#include <unordered_set>

#include "nodes.hh"

namespace ast {
namespace binder {
/**
 * @brief This class implements the visitor(binder) to bind the identifiers and
 * function calls to their declarations. It checks the consistency of definitions.
 * For instance, using a non declarated variable or function,etc.
 * 
 */
typedef std::unordered_map<Symbol, Decl *> scope_t;

class Binder : public ASTVisitor {
  std::vector<scope_t> scopes;
  std::vector<FunDecl *> functions;
  std::unordered_set<Symbol> external_names;
  void push_scope();
  void pop_scope();
  scope_t &current_scope();
  void enter(Decl &);
  Decl &find(const location loc, const Symbol &name);
  void enter_primitive(const std::string &, const std::string &,
                       const std::vector<std::string> &);
  void set_parent_and_external_name(FunDecl &decl);

public:
  Binder();
  FunDecl *analyze_program(Expr &);
  virtual void visit(IntegerLiteral &) override;
  virtual void visit(StringLiteral &) override;
  virtual void visit(BinaryOperator &) override;
  virtual void visit(Sequence &) override;
  virtual void visit(Let &) override;
  virtual void visit(Identifier &) override;
  virtual void visit(IfThenElse &) override;
  virtual void visit(VarDecl &) override;
  virtual void visit(FunDecl &) override;
  virtual void visit(FunCall &) override;
  virtual void visit(WhileLoop &) override;
  virtual void visit(ForLoop &) override;
  virtual void visit(Break &) override;
  virtual void visit(Assign &) override;
};

} // namespace binder
} // namespace ast

#endif // BINDER_HH
