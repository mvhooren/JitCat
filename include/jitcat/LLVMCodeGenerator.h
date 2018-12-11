/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class CatRuntimeContext;
class LLVMCodeGeneratorHelper;
struct LLVMCompileTimeContext;
class Reflectable;
#include "CatASTNodesDeclares.h"
#include "CatType.h"
#include "LLVMForwardDeclares.h"
#include "RootTypeSource.h"

#include <memory>
#include <string>

//A code generator for the LLVM backend. 
//Create one code generator per module / translation unit.
//Not thread safe. Create multiple code generators for different threads instead.
class LLVMCodeGenerator
{
public:
	LLVMCodeGenerator();
	~LLVMCodeGenerator();

	llvm::Value* generate(CatTypedExpression* expression, LLVMCompileTimeContext* context);
	
	//Wraps an expression into a function that returns the expression's computed value.
	//The function has one parameter, the CatRuntimeContext. 
	//If the function returns a string, it will have 2 parameters where the first parameter is a pointer to a pre-allocated string. (marked sret)
	llvm::Function* generateExpressionFunction(CatTypedExpression* expression, LLVMCompileTimeContext* context, const std::string& name);
	
	intptr_t generateAndGetFunctionAddress(CatTypedExpression* expression, LLVMCompileTimeContext* context);

	void generateAndDump(CatTypedExpression* expression, LLVMCompileTimeContext* context, const std::string& functionName);

	void compileAndTest(CatRuntimeContext* context, const std::string& functionName);

private:
	llvm::Value* generate(CatIdentifier* identifier, LLVMCompileTimeContext* context);
	llvm::Value* generate(CatFunctionCall* functionCall, LLVMCompileTimeContext* context);
	llvm::Value* generate(CatInfixOperator* infixOperator, LLVMCompileTimeContext* context);
	llvm::Value* generate(CatLiteral* literal, LLVMCompileTimeContext* context);
	llvm::Value* generate(CatMemberAccess* memberAccess, LLVMCompileTimeContext* context);
	llvm::Value* generate(CatMemberFunctionCall* memberFunctionCall, LLVMCompileTimeContext* context);
	llvm::Value* generate(CatPrefixOperator* prefixOperator, LLVMCompileTimeContext* context);
	llvm::Value* generate(CatArrayIndex* arrayIndex, LLVMCompileTimeContext* context);
	llvm::Value* generate(CatScopeRoot* scopeRoot, LLVMCompileTimeContext* context);

	llvm::Value* getBaseAddress(RootTypeSource source, LLVMCompileTimeContext* context);

private:
	llvm::LLVMContext& llvmContext;
	std::unique_ptr<llvm::Module> currentModule;
	std::unique_ptr<llvm::IRBuilder<>> builder;
	std::unique_ptr<llvm::legacy::FunctionPassManager> passManager;
	std::unique_ptr<LLVMCodeGeneratorHelper> helper;
};