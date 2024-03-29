/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once
namespace jitcat::Reflection
{
	class StaticFunctionInfo;
}
#include "jitcat/CatTypedExpression.h"
#include "jitcat/FunctionSignature.h"

#include <memory>
#include <string>
#include <vector>

namespace jitcat::AST
{
	class CatArgumentList;
	class CatStaticScope;

	class CatStaticFunctionCall: public CatTypedExpression, public Reflection::FunctionSignature
	{
	public:
		CatStaticFunctionCall(CatStaticScope* parentScope, const std::string& name, CatArgumentList* arguments, const Tokenizer::Lexeme& lexeme, const Tokenizer::Lexeme& nameLexeme);
		CatStaticFunctionCall(const CatStaticFunctionCall& other);
		
		const CatArgumentList* getArguments() const;
		uintptr_t getFunctionAddress() const;
		bool getFunctionNeverReturnsNull() const;
		const std::string& getFunctionName() const;
		std::string getMangledFunctionName(bool sRetBeforeThis) const;

		// Inherited via CatTypedExpression
		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual const CatGenericType& getType() const override final;
		virtual bool isConst() const override final;
		virtual CatStatement* constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
	
		const CatGenericType& getFunctionParameterType(std::size_t index) const;

		const std::vector<CatGenericType>& getExpectedParameterTypes() const;
		const std::vector<int>& getArgumentsToCheckForNull() const;

	private:
		Reflection::StaticFunctionInfo* staticFunctionInfo;
		
		std::unique_ptr<CatStaticScope> parentScope;
		std::string name;
		std::string lowerCaseName;

		Tokenizer::Lexeme nameLexeme;
		std::unique_ptr<CatArgumentList> arguments;
		CatGenericType returnType;
		
		std::vector<int> argumentsToCheckForNull;

		// Inherited via FunctionSignature
		virtual const std::string& getLowerCaseFunctionName() const override;
		virtual int getNumParameters() const override;
		virtual const CatGenericType& getParameterType(int index) const override;
	};
};