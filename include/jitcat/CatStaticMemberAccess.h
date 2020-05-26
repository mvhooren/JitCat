/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat::Reflection
{
	struct StaticMemberInfo;
	class TypeInfo;
}
#include "jitcat/CatAssignableExpression.h"
#include "jitcat/CatScopeID.h"

namespace jitcat::AST
{
	class CatClassDefinition;
	class CatTypeNode;
	class CatStaticScope;

	class CatStaticMemberAccess: public CatAssignableExpression
	{
	public:
		CatStaticMemberAccess(CatStaticScope* baseScope, const Tokenizer::Lexeme& identifierLexeme, const Tokenizer::Lexeme& lexeme);
		CatStaticMemberAccess(const CatStaticMemberAccess& other);

		virtual CatASTNode* copy() const override final;
		virtual const CatGenericType& getType() const override final;
		virtual const CatGenericType& getAssignableType() const override final;
		virtual bool isAssignable() const override final;

		virtual void print() const override final;
		virtual bool isConst() const override final;
		virtual CatStatement* constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual CatASTNodeType getNodeType() const override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;
		virtual std::any executeAssignable(CatRuntimeContext* runtimeContext) override final;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;

		Reflection::StaticMemberInfo* getStaticMemberInfo() const;

	private:
		std::string identifier;
		Tokenizer::Lexeme identifierLexeme;

		CatGenericType type;
		CatGenericType assignableType;

		std::unique_ptr<CatStaticScope> baseScope;

		Reflection::StaticMemberInfo* staticMemberInfo;
	};


} //End namespace jitcat::AST