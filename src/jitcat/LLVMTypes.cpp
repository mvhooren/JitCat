/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/LLVMTypes.h"

using namespace jitcat::LLVM;


llvm::Type* LLVMTypes::doubleType = nullptr;
llvm::Type* LLVMTypes::floatType = nullptr;
llvm::Type* LLVMTypes::intType = nullptr;
llvm::Type* LLVMTypes::longintType = nullptr;
llvm::Type* LLVMTypes::charType = nullptr;
llvm::Type* LLVMTypes::boolType = nullptr;
llvm::Type* LLVMTypes::bool1Type = nullptr;
llvm::PointerType* LLVMTypes::pointerType = nullptr;
llvm::Type* LLVMTypes::pointerTypeAsType = nullptr;
llvm::Type* LLVMTypes::uintPtrType = nullptr;
llvm::Type* LLVMTypes::voidType = nullptr;

llvm::FunctionType* LLVMTypes::functionRetPtrArgPtr = nullptr;
llvm::FunctionType* LLVMTypes::functionRetPtrArgPtr_Ptr = nullptr;
llvm::FunctionType* LLVMTypes::functionRetPtrArgPtr_StringPtr = nullptr;
llvm::FunctionType* LLVMTypes::functionRetPtrArgPtr_Int = nullptr;