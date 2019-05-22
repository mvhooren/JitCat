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
#include "jitcat/CatScopeID.h"

namespace jitcat::AST
{


	class CatIdentifier: public CatAssignableExpression
	{
	public:
		CatIdentifier(const std::string& name, const Tokenizer::Lexeme& lexeme);
		CatIdentifier(const CatIdentifier& other);

		virtual CatASTNode* copy() const override final;
		virtual CatGenericType getType() const override final;
		virtual void print() const override final;
		virtual bool isConst() const override final;
		virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext) override final;
		virtual CatASTNodeType getNodeType() const override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;
		virtual std::any executeAssignable(CatRuntimeContext* runtimeContext, Reflection::AssignableType& assignableType) override final;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		CatScopeID getScopeId() const;
		const Reflection::TypeMemberInfo* getMemberInfo() const;

		const std::string& getName() const;

	public:
		std::string name;
		CatGenericType type;
		Reflection::TypeMemberInfo* memberInfo;
		CatScopeID scopeId;
	};


} //End namespace jitcat::AST