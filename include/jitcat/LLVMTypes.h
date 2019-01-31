/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "LLVMForwardDeclares.h"

#include <string>
#include <type_traits>


class LLVMTypes
{
	LLVMTypes();
	LLVMTypes(const LLVMTypes&) = delete;
	LLVMTypes& operator=(const LLVMTypes&) = delete;

public:
	static llvm::Type* floatType;
	static llvm::Type* intType;
	static llvm::Type* boolType;
	static llvm::Type* pointerType;
	static llvm::Type* uintPtrType;
	static llvm::Type* voidType;
	static llvm::Type* stringType;
	static llvm::Type* stringPtrType;

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


template<class T, class U=
  typename std::remove_cv<
  typename std::remove_pointer<
  typename std::remove_reference<
  typename std::remove_extent<
  T
  >::type
  >::type
  >::type
  >::type
  > struct remove_all : remove_all<U> {};
template<class T> struct remove_all<T, T> { typedef T type; };

template<typename T>
inline llvm::Type* LLVMTypes::getLLVMType()
{
	if		constexpr (std::is_same<T, float>::value)									return LLVMTypes::floatType;
	else if constexpr (std::is_same<T, int>::value)										return LLVMTypes::intType;
	else if constexpr (std::is_same<T, bool>::value)									return LLVMTypes::boolType;
	else if constexpr (std::is_same<typename remove_all<T>::type, std::string>::value)	return LLVMTypes::stringPtrType;
	else if constexpr (std::is_same<T, void>::value)									return LLVMTypes::voidType;
	else if constexpr (std::is_pointer<T>::value)										return LLVMTypes::pointerType;
	else if constexpr (std::is_reference<T>::value)										return LLVMTypes::pointerType;
	else if constexpr (std::is_class<T>::value)											return LLVMTypes::pointerType;
	else																				return LLVMTypes::voidType;
}
