/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat::Reflection
{
	struct TypeMemberInfo;
}
#include "jitcat/CatAssignableExpression.h"

#include <any>
#include <memory>
#include <string>

namespace jitcat::AST
{

	class CatMemberAccess: public CatAssignableExpression
	{
	public:
		CatMemberAccess(CatTypedExpression* base, const std::string& memberName, const Tokenizer::Lexeme& lexeme);
		CatMemberAccess(const CatMemberAccess& other);

		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;
		virtual std::any executeAssignable(CatRuntimeContext* runtimeContext) override final;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual const CatGenericType& getType() const override final;
		virtual const CatGenericType& getAssignableType() const override final;
		virtual bool isConst() const override final;
		virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;

		void setTypeAndMemberInfo(Reflection::TypeMemberInfo* newMemberInfo, const CatGenericType& newMemberType);

		CatTypedExpression* getBase() const;
		Reflection::TypeMemberInfo* getMemberInfo() const;

	private:
		std::unique_ptr<CatTypedExpression> base;
		Reflection::TypeMemberInfo* memberInfo;
		CatGenericType type;
		CatGenericType assignableType;
		const std::string memberName;
	};

} //End namespace jitcat::AST