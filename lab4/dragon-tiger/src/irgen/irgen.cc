#include "irgen.hh"
#include "../utils/errors.hh"

#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"

using utils::error;

namespace irgen {

IRGenerator::IRGenerator() : Builder(Context) {
  Mod = llvm::make_unique<llvm::Module>("tiger", Context);
}

llvm::Type *IRGenerator::llvm_type(const ast::Type ast_type) {
  switch (ast_type) {
  case t_int:
    return Builder.getInt32Ty();
  case t_string:
    return Builder.getInt8PtrTy();
  case t_void:
    return Builder.getVoidTy();
  default:
    assert(false); __builtin_unreachable();
  }
}

llvm::Value *IRGenerator::alloca_in_entry(llvm::Type *Ty,
                                          const std::string &name) {
  llvm::IRBuilderBase::InsertPoint const saved = Builder.saveIP();
  Builder.SetInsertPoint(&current_function->getEntryBlock());
  llvm::Value *const value = Builder.CreateAlloca(Ty, nullptr, name);
  Builder.restoreIP(saved);
  return value;
}

void IRGenerator::print_ir(std::ostream *ostream) {
  // FIXME: This is inefficient. Should probably take a filename
  // and use directly LLVM raw stream interface
  std::string buffer;
  llvm::raw_string_ostream OS(buffer);
  OS << *Mod;
  OS.flush();
  *ostream << buffer;
}

llvm::Value *IRGenerator::address_of(const Identifier &id) {
  assert(id.get_decl());
  const VarDecl &decl = dynamic_cast<const VarDecl &>(id.get_decl().get());
  int depth_diff = id.get_depth() - decl.get_depth();
  if (depth_diff==0)
    return allocations[&decl];
  else{
    std::pair<llvm::StructType *, llvm::Value *> pair = frame_up(depth_diff);
    llvm::StructType * st = pair.first;
    int position = frame_position[&id.get_decl().get()];
    return Builder.CreateStructGEP(st->getPointerTo(),(llvm::Value *)st, position );
  }
}

void IRGenerator::generate_program(FunDecl *main) {
  main->accept(*this);

  while (!pending_func_bodies.empty()) {
    generate_function(*pending_func_bodies.back());
    pending_func_bodies.pop_back();
  }
}

void IRGenerator::generate_function(const FunDecl &decl) {
  // Reinitialize common structures.
  allocations.clear();
  loop_exit_bbs.clear();

  // Set current function
  current_function = Mod->getFunction(decl.get_external_name().get());
  current_function_decl = &decl;
  std::vector<VarDecl *> params = decl.get_params();

  // Create a new basic block to insert allocation insertion
  llvm::BasicBlock *bb1 =
      llvm::BasicBlock::Create(Context, "entry", current_function);

  // Create a second basic block for body insertion
  llvm::BasicBlock *bb2 =
      llvm::BasicBlock::Create(Context, "body", current_function);

  Builder.SetInsertPoint(bb2);
  generate_frame();

  // Set the name for each argument and register it in the allocations map
  // after storing it in an alloca.

  unsigned i = 0;
  for (auto &arg : current_function->args()) {
    if (!decl.is_external && i==0){
      Builder.CreateStore(frame,&arg);
      continue;
    }
    arg.setName(params[i]->name.get());
    llvm::Value *const shadow = generate_vardecl(*params[i]);
    Builder.CreateStore(&arg, shadow);
    i++;
  }

  // Visit the body
  llvm::Value *expr = decl.get_expr()->accept(*this);

  // Finish off the function.
  if (decl.get_type() == t_void)
    Builder.CreateRetVoid();
  else
    Builder.CreateRet(expr);

  // Jump from entry to body
  Builder.SetInsertPoint(bb1);
  Builder.CreateBr(bb2);

  // Validate the generated code, checking for consistency.
  llvm::verifyFunction(*current_function);
}

void IRGenerator::generate_frame(){
  std::vector<llvm::Type *> types;
  if (current_function_decl->get_parent()){
    const llvm::StructType * parent_struc = frame_type[&current_function_decl->get_parent().get()];
    types.push_back(parent_struc->getPointerTo());
  }
  for (const VarDecl * var : current_function_decl->get_escaping_decls()){
    types.push_back(llvm_type(var->get_type()));
  }
  std::string name = "ft_"+current_function_decl->get_external_name().get();
  llvm::StructType * struct_type = llvm::StructType::create(Context,types,name);
  frame_type.insert(std::pair<const FunDecl *, llvm::StructType *>(current_function_decl,struct_type));
  frame = Builder.CreateAlloca(struct_type,nullptr);
}

std::pair<llvm::StructType *, llvm::Value *> IRGenerator::frame_up(int levels){
  const FunDecl * fun = current_function_decl;
  llvm::Value * sl = frame;
  for (int i=0; i<levels;i++){
    if (!fun->get_parent())
      break;
    sl = Builder.CreateStructGEP((llvm::StructType *)frame_type[fun]->getPointerTo(),
                                (llvm::Value *)frame_type[fun], 0 );
    fun = &fun->get_parent().get();

  }
  return std::pair<llvm::StructType *, llvm::Value *>(frame_type[fun],sl);
}

llvm::Value * IRGenerator::generate_vardecl(const VarDecl &decl){
  llvm::Value * pointer;
  if (decl.get_escapes()){
    int position = 0;
    for (const VarDecl * v : current_function_decl->get_escaping_decls()){
      if (v->name.get() == decl.name.get())
        break;
      position++;
    }
    if (current_function_decl->get_parent())
      position++;
    
    frame_position.insert(std::pair<const VarDecl *, int>(&decl,position));

    
    //std::cout<<decl.name.get()<<std::endl;
    pointer = Builder.CreateStructGEP(
              (llvm::StructType *)frame_type[current_function_decl]->getPointerTo(),
              (llvm::Value *)frame_type[current_function_decl], 2 );
    //std::cout<<decl.name.get()<<std::endl;
  }
  else
    pointer = alloca_in_entry(llvm_type(decl.get_type()),decl.name.get());

  allocations.insert(std::pair<const VarDecl *, llvm::Value *>(&decl,pointer));

  return pointer;
}

} // namespace irgen
