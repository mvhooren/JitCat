/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/LLVMForwardDeclares.h"
#include "jitcat/TypeTools.h"


#include <string>
#include <type_traits>

namespace jitcat::LLVM
{

	class LLVMTypes
	{
		LLVMTypes();
		LLVMTypes(const LLVMTypes&) = delete;
		LLVMTypes& operator=(const LLVMTypes&) = delete;

	public:
		static llvm::Type* floatType;
		static llvm::Type* intType;
		static llvm::Type* charType;
		static llvm::Type* ucharType;
		static llvm::Type* boolType;
		static llvm::PointerType* pointerType;
		static llvm::Type* pointerTypeAsType;
		static llvm::Type* uintPtrType;
		static llvm::Type* voidType;
		static llvm::Type* stringType;
		static llvm::PointerType* stringPtrType;
		static llvm::Type* stringPtrTypeAsType;

		//A function that takes a pointer and returns a pointer
		static llvm::FunctionType* functionRetPtrArgPtr;
		//A function that takes two pointers and returns a pointer
		static llvm::FunctionType* functionRetPtrArgPtr_Ptr;
		//A function that takes a generic pointer and a string pointer and returns a pointer
		static llvm::FunctionType* functionRetPtrArgPtr_StringPtr;
		//A function that takes a pointer and an int and returns a pointer
		static llvm::FunctionType* functionRetPtrArgPtr_Int;

		template<typename T>
		static llvm::Type* getLLVMType();
	};



	template<typename T>
	inline llvm::Type* LLVMTypes::getLLVMType()
	{
		if		constexpr (std::is_same<T, float>::value)									return LLVMTypes::floatType;
		if		constexpr (std::is_same<T, char>::value)									return LLVMTypes::charType;
		if		constexpr (std::is_same<T, unsigned char>::value)							return LLVMTypes::ucharType;
		else if constexpr (std::is_same<T, int>::value)										return LLVMTypes::intType;
		else if constexpr (std::is_same<T, bool>::value)									return LLVMTypes::boolType;
		else if constexpr (std::is_same<typename remove_all<T>::type, std::string>::value)	return LLVMTypes::stringPtrTypeAsType;
		else if constexpr (std::is_same<T, void>::value)									return LLVMTypes::voidType;
		else if constexpr (std::is_pointer<T>::value)										return LLVMTypes::pointerTypeAsType;
		else if constexpr (std::is_reference<T>::value)										return LLVMTypes::pointerTypeAsType;
		else if constexpr (std::is_class<T>::value)											return LLVMTypes::pointerTypeAsType;
		else																				return LLVMTypes::voidType;
	}


} //End namespace jitcat::LLVM