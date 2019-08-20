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

#include <memory>
#include <string>


namespace jitcat::AST
{
	class CatArgumentList;
	class CatTypeNode;

	class CatStaticFunctionCall: public CatTypedExpression
	{
	public:
		CatStaticFunctionCall(CatTypeNode* parentType, const std::string& name, CatArgumentList* arguments, const Tokenizer::Lexeme& lexeme, const Tokenizer::Lexeme& nameLexeme);
		CatStaticFunctionCall(const CatStaticFunctionCall& other);
		
		// Inherited via CatTypedExpression
		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual const CatGenericType& getType() const override final;
		virtual bool isConst() const override final;
		virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext) override final;
	
	private:
		Reflection::StaticFunctionInfo* staticFunctionInfo;
		
		std::unique_ptr<CatTypeNode> parentType;
		std::string name;
		Tokenizer::Lexeme nameLexeme;

		std::unique_ptr<CatArgumentList> arguments;
		CatGenericType returnType;
	};
};