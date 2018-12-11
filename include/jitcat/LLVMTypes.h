#pragma once

#include "LLVMForwardDeclares.h"

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
};