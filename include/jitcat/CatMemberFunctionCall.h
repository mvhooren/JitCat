/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat::Reflection
{
	struct MemberFunctionInfo;
}
#include "jitcat/CatTypedExpression.h"

#include <memory>

namespace jitcat::AST
{
	class CatArgumentList;


	class CatMemberFunctionCall: public CatTypedExpression
	{
	public:
		CatMemberFunctionCall(const std::string& name, const Tokenizer::Lexeme& nameLexeme, CatTypedExpression* base, CatArgumentList* arguments, const Tokenizer::Lexeme& lexeme);
		CatMemberFunctionCall(const CatMemberFunctionCall& other);
		
		// Inherited via CatTypedExpression
		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;
		std::any executeWithBase(CatRuntimeContext* runtimeContext, std::any baseValue);
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual const CatGenericType& getType() const override final;
		virtual bool isConst() const override final;
		virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext) override final;
	
		Reflection::MemberFunctionInfo* getMemberFunctionInfo() const;
		CatTypedExpression* getBase() const;
		CatArgumentList* getArguments() const;

		const std::string& getFunctionName() const;
		void setFunctionName(const std::string& name);
		void setBase(std::unique_ptr<CatTypedExpression> newBase);
		const Tokenizer::Lexeme& getNameLexeme() const;

	private:
		Reflection::MemberFunctionInfo* memberFunctionInfo;
		std::string functionName;
		Tokenizer::Lexeme nameLexeme;
		std::unique_ptr<CatTypedExpression> base;
		std::unique_ptr<CatArgumentList> arguments;
		CatGenericType returnType;
	};


} // End namespace jitcat::AST