#include "LLVMTypes.h"
#include "LLVMJit.h"

#include <llvm/IR/Type.h>

llvm::Type* LLVMTypes::floatType = nullptr;
llvm::Type* LLVMTypes::intType = nullptr;
llvm::Type* LLVMTypes::boolType = nullptr;
llvm::Type* LLVMTypes::pointerType = nullptr;
llvm::Type* LLVMTypes::uintPtrType = nullptr;
llvm::Type* LLVMTypes::voidType = nullptr;
llvm::Type* LLVMTypes::stringType = nullptr;
llvm::Type* LLVMTypes::stringPtrType = nullptr;

llvm::FunctionType* LLVMTypes::functionRetPtrArgPtr = nullptr;
llvm::FunctionType* LLVMTypes::functionRetPtrArgPtr_Ptr = nullptr;
llvm::FunctionType* LLVMTypes::functionRetPtrArgPtr_StringPtr = nullptr;
llvm::FunctionType* LLVMTypes::functionRetPtrArgPtr_Int = nullptr;