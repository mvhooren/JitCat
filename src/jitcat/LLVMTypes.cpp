/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "LLVMTypes.h"

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