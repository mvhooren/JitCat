/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat
{
	class CatGenericType;
	class CatRuntimeContext;
	namespace Reflection
	{
		class CustomTypeInfo;
		struct MemberFunctionInfo;
	}
}
#include "jitcat/CatASTNodesDeclares.h"
#include "jitcat/LLVMForwardDeclares.h"
#include "jitcat/CatScopeID.h"

#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/ErrorHandling.h>
#include <memory>
#include <string>


namespace jitcat::LLVM
{
	class LLVMCodeGeneratorHelper;
	struct LLVMCompileTimeContext;
	class LLVMMemoryManager;

	//A code generator for the LLVM backend. 
	//Create one code generator per module / translation unit.
	//Not thread safe. Create multiple code generators for different threads instead.
	class LLVMCodeGenerator
	{
	public:
		LLVMCodeGenerator(const std::string& name);
		~LLVMCodeGenerator();

		void generate(const AST::CatSourceFile* sourceFile, LLVMCompileTimeContext* context);
		llvm::Value* generate(const AST::CatTypedExpression* expression, LLVMCompileTimeContext* context);

	
		//Wraps an expression into a function that returns the expression's computed value.
		//The function has one parameter, the CatRuntimeContext. 
		//If the function returns a string, it will have 2 parameters where the first parameter is a pointer to a pre-allocated string. (marked sret)
		llvm::Function* generateExpressionFunction(const AST::CatTypedExpression* expression, LLVMCompileTimeContext* context, const std::string& name);

		llvm::Function* generateExpressionAssignFunction(const AST::CatAssignableExpression* expression, LLVMCompileTimeContext* context, const std::string& name);
	
		//Generates a function that returns the value of the expression.
		intptr_t generateAndGetFunctionAddress(const AST::CatTypedExpression* expression, LLVMCompileTimeContext* context);

		//Generates a function that takes a parameter that will be assigned to the result of the expression. Expression must be of an assignable type (lValue).
		intptr_t generateAndGetAssignFunctionAddress(const jitcat::AST::CatAssignableExpression* expression, LLVMCompileTimeContext* context);

	private:
		llvm::Value* generate(const AST::CatBuiltInFunctionCall* functionCall, LLVMCompileTimeContext* context);
		llvm::Value* generate(const AST::CatIndirectionConversion* indirectionConversion, LLVMCompileTimeContext* context);
		llvm::Value* generate(const AST::CatInfixOperator* infixOperator, LLVMCompileTimeContext* context);
		llvm::Value* generate(const AST::CatAssignmentOperator* assignmentOperator, LLVMCompileTimeContext* context);
		llvm::Value* generate(const AST::CatLiteral* literal, LLVMCompileTimeContext* context);
		llvm::Value* generate(const AST::CatMemberAccess* memberAccess, LLVMCompileTimeContext* context);
		llvm::Value* generate(const AST::CatMemberFunctionCall* memberFunctionCall, LLVMCompileTimeContext* context);
		llvm::Value* generate(const AST::CatStaticFunctionCall* staticFunctionCall, LLVMCompileTimeContext* context);
		llvm::Value* generate(const AST::CatStaticMemberAccess* staticIdentifier, LLVMCompileTimeContext* context);
		llvm::Value* generate(const AST::CatPrefixOperator* prefixOperator, LLVMCompileTimeContext* context);
		llvm::Value* generate(const AST::CatScopeRoot* scopeRoot, LLVMCompileTimeContext* context);
		void generate(const AST::CatScopeBlock* scopeBlock, LLVMCompileTimeContext* context);
		llvm::Value* generate(const AST::CatReturnStatement* returnStatement, LLVMCompileTimeContext* context);

		void generate(const AST::CatStatement* statement, LLVMCompileTimeContext* context);

		void generate(const AST::CatDefinition* definition, LLVMCompileTimeContext* context);
		void generate(const AST::CatClassDefinition* classDefinition, LLVMCompileTimeContext* context);
		llvm::Function* generate(const AST::CatFunctionDefinition* functionDefinition, LLVMCompileTimeContext* context);

		llvm::Value* generateAssign(const AST::CatAssignableExpression* expression, llvm::Value* rValue, LLVMCompileTimeContext* context);
		llvm::Value* generateAssign(const AST::CatMemberAccess* memberAccess, llvm::Value* rValue, LLVMCompileTimeContext* context);
		llvm::Value* generateAssign(const AST::CatStaticMemberAccess* staticMemberAccess, llvm::Value* rValue, LLVMCompileTimeContext* context);

		llvm::Value* generateFPMath(const char* name, float(*floatVariant)(float), double(*doubleVariant)(double), 
								    const AST::CatArgumentList* argumentList, LLVMCompileTimeContext* context);

		llvm::Value* generateFPMath(const char* name, float(*floatVariant)(float, float), double(*doubleVariant)(double, double), 
								    const AST::CatArgumentList* argumentList, LLVMCompileTimeContext* context);

		llvm::Value* getBaseAddress(CatScopeID source, LLVMCompileTimeContext* context);

		llvm::Value* generateMemberFunctionCall(Reflection::MemberFunctionInfo* memberFunction, const AST::CatTypedExpression* base, 
											    const std::vector<const AST::CatTypedExpression*>& arguments, 
												LLVMCompileTimeContext* context);


		void initContext(LLVMCompileTimeContext* context);
		void createNewModule(LLVMCompileTimeContext* context);
		std::string getNextFunctionName(LLVMCompileTimeContext* context);
		llvm::Function* verifyAndOptimizeFunction(llvm::Function* function);

		llvm::Expected<llvm::JITEvaluatedSymbol> findSymbol(const std::string& name, llvm::orc::JITDylib& dyLib) const;
		llvm::JITTargetAddress getSymbolAddress(const std::string& name, llvm::orc::JITDylib& dyLib) const;

		llvm::Function* generateFunctionPrototype(const std::string& functionName, bool isThisCall, const CatGenericType& returnType, const std::vector<CatGenericType>& parameterTypes, const std::vector<std::string>& parameterNames);
		void generateFunctionReturn(const CatGenericType& returnType, llvm::Value* expressionValue, llvm::Function* function, LLVMCompileTimeContext* context);

		void link(Reflection::CustomTypeInfo* customType);

	private:
		//ExecutionSession represents a running JIT program
		std::unique_ptr<llvm::orc::ExecutionSession> executionSession;
		//A module represents a "translation unit".
		std::unique_ptr<llvm::Module> currentModule;
		//
		llvm::orc::JITDylib* dylib;
		std::unique_ptr<llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>> builder;
		//Can be used to add object files to the JIT.
		std::unique_ptr<llvm::orc::RTDyldObjectLinkingLayer> objectLinkLayer;
		//Mangles symbol names
		std::unique_ptr<llvm::orc::MangleAndInterner> mangler;
		//Takes an LLVM IR module and creates an object file that is linked into the JIT using the objectLinkLayer
		std::unique_ptr<llvm::orc::IRCompileLayer> compileLayer;
		std::unique_ptr<llvm::legacy::FunctionPassManager> passManager;
		std::unique_ptr<LLVMCodeGeneratorHelper> helper;
		//The runtime library dylib
		llvm::orc::JITDylib* runtimeLibraryDyLib;

		static std::unique_ptr<LLVMMemoryManager> memoryManager;
	};

} //End namespace jitcat::LLVM