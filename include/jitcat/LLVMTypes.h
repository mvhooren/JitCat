/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/Configuration.h"
#include "jitcat/LLVMForwardDeclares.h"
#include "jitcat/TypeTools.h"


#include <string>
#include <type_traits>

namespace jitcat::LLVM
{

	class LLVMTypes
	{
	public:
		LLVMTypes(bool is64BitPlatform, unsigned int sizeOfBoolInBits);
		~LLVMTypes();

	public:
		llvm::Type* doubleType;
		llvm::Type* floatType;
		llvm::Type* vector4fType;
		llvm::Type* intType;
		llvm::Type* longintType;
		llvm::Type* charType;
		llvm::Type* boolType;
		llvm::Type* bool1Type;
		llvm::PointerType* pointerType;
		llvm::Type* pointerTypeAsType;
		llvm::Type* uintPtrType;
		llvm::Type* voidType;

		//A function that takes a pointer and returns a pointer
		llvm::FunctionType* functionRetPtrArgPtr;
		//A function that takes two pointers and returns a pointer
		llvm::FunctionType* functionRetPtrArgPtr_Ptr;
	};
} //End namespace jitcat::LLVM