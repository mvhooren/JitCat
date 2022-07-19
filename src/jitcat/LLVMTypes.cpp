/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/LLVMTypes.h"
#include "jitcat/LLVMJit.h"

using namespace jitcat::LLVM;


LLVMTypes::LLVMTypes(bool is64BitPlatform, unsigned int sizeOfBoolInBits)
{
	llvm::LLVMContext& llvmContext = LLVMJit::get().getContext();
	doubleType = llvm::Type::getDoubleTy(llvmContext);
	floatType = llvm::Type::getFloatTy(llvmContext);
	vector4fType = llvm::ArrayType::get(llvm::Type::getFloatTy(llvmContext), 4);
	intType = llvm::Type::getInt32Ty(llvmContext);
	longintType = llvm::Type::getInt64Ty(llvmContext);
	charType = llvm::Type::getInt8Ty(llvmContext);
	boolType = llvm::Type::getIntNTy(llvmContext, sizeOfBoolInBits);
	bool1Type = llvm::Type::getInt1Ty(llvmContext);
	pointerType = llvm::Type::getInt8PtrTy(llvmContext);
	pointerTypeAsType = static_cast<llvm::Type*>(pointerType);
	if (is64BitPlatform)
	{
		uintPtrType = llvm::Type::getInt64Ty(llvmContext);
	}
	else
	{
		uintPtrType = llvm::Type::getInt32Ty(llvmContext);
	}
	voidType = llvm::Type::getVoidTy(llvmContext);

	functionRetPtrArgPtr = llvm::FunctionType::get(pointerType, {pointerType}, false);
	functionRetPtrArgPtr_Ptr = llvm::FunctionType::get(pointerType, {pointerType, pointerType}, false);
}


LLVMTypes::~LLVMTypes()
{
}
