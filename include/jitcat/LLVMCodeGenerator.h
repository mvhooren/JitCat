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

#include <memory>
#include <set>
#include <string>
#include <vector>


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
		friend class LLVMCodeGeneratorHelper;
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
		intptr_t generateAndGetFunctionAddress(const AST::CatTypedExpression* expression, const std::string& expressionStr, LLVMCompileTimeContext* context);

		//Generates a function that takes a parameter that will be assigned to the result of the expression. Expression must be of an assignable type (lValue).
		intptr_t generateAndGetAssignFunctionAddress(const jitcat::AST::CatAssignableExpression* expression, const std::string& expressionStr, LLVMCompileTimeContext* context);

		void emitModuleToObjectFile(const std::string& objectFileName);

		std::string getUniqueExpressionFunctionName(const std::string& expression, LLVMCompileTimeContext* context, bool isAssignExpression);

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
		void generate(const AST::CatConstruct* constructor, LLVMCompileTimeContext* context);
		void generate(const AST::CatDestruct* destructor, LLVMCompileTimeContext* context);
		llvm::Value* generate(const AST::CatReturnStatement* returnStatement, LLVMCompileTimeContext* context);
		void generate(const AST::CatVariableDeclaration* variableDeclaration, LLVMCompileTimeContext* context);
		void generate(const AST::CatIfStatement* ifStatement, LLVMCompileTimeContext* context);
		void generate(const AST::CatForLoop* forLoop, LLVMCompileTimeContext* context);

		void generate(const AST::CatStatement* statement, LLVMCompileTimeContext* context);

		void generate(const AST::CatDefinition* definition, LLVMCompileTimeContext* context);
		void generate(const AST::CatClassDefinition* classDefinition, LLVMCompileTimeContext* context);
		llvm::Function* generate(const AST::CatFunctionDefinition* functionDefinition, LLVMCompileTimeContext* context);

		llvm::Value* generateAssign(const AST::CatAssignableExpression* expression, llvm::Value* rValue, LLVMCompileTimeContext* context);
		llvm::Value* generateAssign(const AST::CatMemberAccess* memberAccess, llvm::Value* rValue, LLVMCompileTimeContext* context);
		llvm::Value* generateAssign(const AST::CatStaticMemberAccess* staticMemberAccess, llvm::Value* rValue, LLVMCompileTimeContext* context);
		llvm::Value* generateAssign(const AST::CatMemberFunctionCall* memberFunction, llvm::Value* rValue, LLVMCompileTimeContext* context);

		llvm::Value* generateFPMath(const char* floatName, float(*floatVariant)(float), const char* doubleName, double(*doubleVariant)(double), 
								    const AST::CatArgumentList* argumentList, LLVMCompileTimeContext* context);

		llvm::Value* generateFPMath(const char* floatName, float(*floatVariant)(float, float), const char* doubleName, double(*doubleVariant)(double, double), 
								    const AST::CatArgumentList* argumentList, LLVMCompileTimeContext* context);

		llvm::Value* getBaseAddress(CatScopeID source, LLVMCompileTimeContext* context);

		void initContext(LLVMCompileTimeContext* context);
		void createNewModule(LLVMCompileTimeContext* context);
		
		llvm::Function* verifyAndOptimizeFunction(llvm::Function* function);

		uint64_t getSymbolAddress(const std::string& name, llvm::orc::JITDylib& dyLib) const;

		llvm::FunctionType* createFunctionType(bool isThisCall, const CatGenericType& returnType, const std::vector<CatGenericType>& parameterTypes);
		llvm::Function* generateFunctionPrototype(const std::string& functionName, bool isThisCall, const CatGenericType& returnType, const std::vector<CatGenericType>& parameterTypes, const std::vector<std::string>& parameterNames);
		llvm::Function* generateFunctionPrototype(const std::string& functionName, llvm::FunctionType* functionType, bool isThisCall, const CatGenericType& returnType, const std::vector<std::string>& parameterNames);
		void generateFunctionReturn(const CatGenericType& returnType, llvm::Value* expressionValue, llvm::Function* function, LLVMCompileTimeContext* context);

		void link(Reflection::CustomTypeInfo* customType);

		llvm::Module* getCurrentModule() const;
		llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>* getBuilder() const;

		llvm::Value* booleanCast(llvm::Value* boolean);

	private:
		//ExecutionSession represents a running JIT program
		std::unique_ptr<llvm::orc::ExecutionSession> executionSession;
		//A module represents a "translation unit".
		std::unique_ptr<llvm::Module> currentModule;
		//
		llvm::orc::JITDylib* dylib;
		std::set<const llvm::orc::JITDylib*> linkedLibs;

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