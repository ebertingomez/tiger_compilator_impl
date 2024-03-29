#include <cstdlib>  // For exit
#include <iostream> // For std::cerr
#include "irgen.hh"

#include "llvm/Support/raw_ostream.h"

namespace {

} // namespace

namespace irgen {

llvm::Value *IRGenerator::visit(const IntegerLiteral &literal) {
  return Builder.getInt32(literal.value);
}

llvm::Value *IRGenerator::visit(const StringLiteral &literal) {
  return Builder.CreateGlobalStringPtr(literal.value.get());
}

llvm::Value *IRGenerator::visit(const BinaryOperator &op) {
  llvm::Value *l = op.get_left().accept(*this);
  llvm::Value *r = op.get_right().accept(*this);

  if (op.get_left().get_type() == t_string) {
    auto const strcmp = Mod->getOrInsertFunction(
        "__strcmp", Builder.getInt32Ty(), Builder.getInt8PtrTy(),
        Builder.getInt8PtrTy(), nullptr);
    l = Builder.CreateCall(strcmp, {l, r});
    r = Builder.getInt32(0);
  }

  switch(op.op) {
    case o_plus: return Builder.CreateBinOp(llvm::Instruction::Add, l, r);
    case o_minus: return Builder.CreateBinOp(llvm::Instruction::Sub, l, r);
    case o_times: return Builder.CreateBinOp(llvm::Instruction::Mul, l, r);
    case o_divide: return Builder.CreateBinOp(llvm::Instruction::SDiv, l, r);
    default: break;
  }

  // Comparisons return an i1 result which needs to be
  // casted to i32, as Tiger might use that as an integer.
  llvm::Value *cmp;

  switch(op.op) {
    case o_eq: cmp = Builder.CreateICmpEQ(l, r); break;
    case o_neq: cmp = Builder.CreateICmpNE(l, r); break;
    case o_gt: cmp = Builder.CreateICmpSGT(l, r); break;
    case o_lt: cmp = Builder.CreateICmpSLT(l, r); break;
    case o_ge: cmp = Builder.CreateICmpSGE(l, r); break;
    case o_le: cmp = Builder.CreateICmpSLE(l, r); break;
    default: assert(false); __builtin_unreachable();
  }

  return Builder.CreateIntCast(cmp, Builder.getInt32Ty(), true);
}

llvm::Value *IRGenerator::visit(const Sequence &seq) {
  llvm::Value *result = nullptr;

  for (auto expr : seq.get_exprs())
    result = expr->accept(*this);
  // An empty sequence should return () but the result
  // will never be used anyway, so nullptr is fine.
  return result;
}

llvm::Value *IRGenerator::visit(const Let &let) {
  for (auto decl : let.get_decls())
    decl->accept(*this);

  return let.get_sequence().accept(*this);
}

llvm::Value *IRGenerator::visit(const IfThenElse &ite) {
  llvm::Value * pointer;
  if (ite.get_type()!=t_void)
    pointer = alloca_in_entry(llvm_type(ite.get_type()),"if_result");
  // Creation of the block and the condition
  llvm::BasicBlock *const if_then =
      llvm::BasicBlock::Create(Context, "if_then", current_function);
  llvm::BasicBlock *const if_else =
      llvm::BasicBlock::Create(Context, "if_else", current_function);
  llvm::BasicBlock *const if_end =
      llvm::BasicBlock::Create(Context, "if_end", current_function);

  llvm::Value * cond_value = ite.get_condition().accept(*this);
  llvm::Value * cond = Builder.CreateICmpNE(cond_value,Builder.getInt32(0));
  // If the condition is verified, we go to the if_then block, otherwise to the if_else one
  Builder.CreateCondBr(cond,if_then,if_else);
  
  llvm::Value * value;
  Builder.SetInsertPoint(if_then);
  value = ite.get_then_part().accept(*this);

  if (ite.get_type()!=t_void)
    Builder.CreateStore(value, pointer);

  Builder.CreateBr(if_end);

  Builder.SetInsertPoint(if_else);
  value = ite.get_else_part().accept(*this);

  if (ite.get_type()!=t_void)
    Builder.CreateStore(value, pointer);

  Builder.CreateBr(if_end);
  
  // To the end ifthenelse segment after executing the precedent blocks
  Builder.SetInsertPoint(if_end);

  if (ite.get_type()==t_void)
    return nullptr;
  
  return Builder.CreateLoad(llvm_type(ite.get_type()),pointer);
}

llvm::Value *IRGenerator::visit(const VarDecl &decl) {
  llvm::Value * pointer = generate_vardecl(decl);
  llvm::Value * value = decl.get_expr()->accept(*this);

  if (value != nullptr)
    Builder.CreateStore(value,pointer);
  return nullptr;
}

llvm::Value *IRGenerator::visit(const FunDecl &decl) {
  std::vector<llvm::Type *> param_types;
  
  // If the function is internal and has a parent, we store a pointer to the
  //  parent's frame in the first position of the current frame
  if (!decl.is_external && decl.get_parent()){
    llvm::StructType * parent_struc = frame_type[&decl.get_parent().get()];
    param_types.push_back(parent_struc->getPointerTo());
  }
  
  for (auto param_decl : decl.get_params()) {
    param_types.push_back(llvm_type(param_decl->get_type()));
  }
  llvm::Type *return_type = llvm_type(decl.get_type());

  llvm::FunctionType *ft =
      llvm::FunctionType::get(return_type, param_types, false);

  llvm::Function::Create(ft,
                         decl.is_external ? llvm::Function::ExternalLinkage
                                          : llvm::Function::InternalLinkage,
                         decl.get_external_name().get(), Mod.get());

  if (decl.get_expr())
    pending_func_bodies.push_front(&decl);
  return nullptr;
}

llvm::Value *IRGenerator::visit(const Identifier &id) {
  llvm::Type * type = llvm_type(id.get_type());
  llvm::Value * pointer = address_of(id);
  return Builder.CreateLoad(type,pointer);

}

llvm::Value *IRGenerator::visit(const FunCall &call) {
  // Look up the name in the global module table.
  const FunDecl &decl = call.get_decl().get();
  llvm::Function *callee =
      Mod->getFunction(decl.get_external_name().get());

  if (!callee) {
    // This should only happen for primitives whose Decl is out of the AST
    // and has not yet been handled
    assert(!decl.get_expr());
    decl.accept(*this);
    callee = Mod->getFunction(decl.get_external_name().get());
  }

  std::vector<llvm::Value *> args_values;
  
  // If the call function declaration is internal and is in another frame, we look for it
  if (call.get_decl()){
    if (!call.get_decl().get().is_external){
      int depth_diff = call.get_depth() - call.get_decl().get().depth;
      llvm::Value * v = frame_up(depth_diff).second;
      args_values.push_back(v);
    }
  }    
  
  for (auto expr : call.get_args()) {
    args_values.push_back(expr->accept(*this));
  }

  if (decl.get_type() == t_void) {
    Builder.CreateCall(callee, args_values);
    return nullptr;
  }
  return Builder.CreateCall(callee, args_values, "call");
}

llvm::Value *IRGenerator::visit(const WhileLoop &loop) {
  // Creation the the blocks and of the condition
  llvm::BasicBlock *const test_block =
      llvm::BasicBlock::Create(Context, "while_test", current_function);
  llvm::BasicBlock *const body_block =
      llvm::BasicBlock::Create(Context, "while_body", current_function);
  llvm::BasicBlock *const end_block =
      llvm::BasicBlock::Create(Context, "while_end", current_function);

  Builder.CreateBr(test_block);
  loop_exit_bbs.insert(std::pair<const Loop *, llvm::BasicBlock *>(&loop,end_block));

  // We test the condition in each iteration via this block.
  Builder.SetInsertPoint(test_block);
  llvm::Value * cond_value = loop.get_condition().accept(*this);
  Builder.CreateCondBr(Builder.CreateICmpNE(cond_value,Builder.getInt32(0)),
                       body_block, end_block);
  
  Builder.SetInsertPoint(body_block);
  loop.get_body().accept(*this);
  // After jumping to the body and executing it. we go back to the test block
  Builder.CreateBr(test_block);

  Builder.SetInsertPoint(end_block);
  return nullptr;
}

llvm::Value *IRGenerator::visit(const ForLoop &loop) {
  llvm::BasicBlock *const test_block =
          llvm::BasicBlock::Create(Context, "loop_test", current_function);
  llvm::BasicBlock *const body_block =
          llvm::BasicBlock::Create(Context, "loop_body", current_function);
  llvm::BasicBlock *const end_block =
          llvm::BasicBlock::Create(Context, "loop_end", current_function);

  llvm::Value *const index = loop.get_variable().accept(*this);
  llvm::Value *const high = loop.get_high().accept(*this);
  Builder.CreateBr(test_block);
  loop_exit_bbs.insert(std::pair<const Loop *, llvm::BasicBlock *>(&loop,end_block));

  Builder.SetInsertPoint(test_block);
  Builder.CreateCondBr(Builder.CreateICmpSLE(Builder.CreateLoad(index), high),
                       body_block, end_block);

  Builder.SetInsertPoint(body_block);
  loop.get_body().accept(*this);
  
  Builder.CreateStore(
      Builder.CreateAdd(Builder.CreateLoad(index), Builder.getInt32(1)), index);
  Builder.CreateBr(test_block);

  Builder.SetInsertPoint(end_block);
  return nullptr;
}

llvm::Value *IRGenerator::visit(const Break &b) {
  llvm::BasicBlock * exit_block = loop_exit_bbs[b.get_loop().get_ptr()];
  Builder.CreateBr(exit_block);
  return nullptr;
}

llvm::Value *IRGenerator::visit(const Assign &assign) {
  llvm::Value * value = assign.get_rhs().accept(*this);
  if (value != nullptr) {
      Builder.CreateStore(value,address_of(assign.get_lhs()));
  }
  return nullptr;
}

} // namespace irgen
