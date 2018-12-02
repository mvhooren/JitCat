#include "LLVMTypes.h"
#include "LLVMJit.h"

//QQQ not portable, fix
#pragma warning(push, 0)        
//Disable warnings from llvm includes
#include <llvm\IR\Type.h>

#pragma warning(pop)


llvm::Type* LLVMTypes::floatType = nullptr;
llvm::Type* LLVMTypes::intType = nullptr;
llvm::Type* LLVMTypes::boolType = nullptr;
llvm::Type* LLVMTypes::pointerType = nullptr;
llvm::Type* LLVMTypes::uintPtrType = nullptr;
llvm::Type* LLVMTypes::voidType = nullptr;

llvm::FunctionType* LLVMTypes::functionRetPtrArgPtr = nullptr;
llvm::FunctionType* LLVMTypes::functionRetPtrArgPtr_Ptr = nullptr;
llvm::FunctionType* LLVMTypes::functionRetPtrArgPtr_Int = nullptr;