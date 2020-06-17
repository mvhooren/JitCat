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

	//An identifier refers to any type of member of a scope object.
	//What kind of member it is, is determined during type checking.
	//During const-collapse, CatIdentifier is replaced with an AST node representing the specific type of Identifier.
	//The different types are:
	//- A member variable (Collapses to CatMemberAccess)
	//- A static variable (Collapses to CatStaticIdentifier)
	//- A constant variable (Collapses to CatLiteral)
	class CatIdentifier: public CatAssignableExpression
	{
	public:
		CatIdentifier(const std::string& name, const Tokenizer::Lexeme& lexeme);
		CatIdentifier(const CatIdentifier& other);

		virtual CatASTNode* copy() const override final;
		virtual const CatGenericType& getType() const override final;
		virtual bool isAssignable() const override final;
		virtual const CatGenericType& getAssignableType() const override final;
		virtual std::optional<std::string> getAssignableVariableName() const override final;

		virtual void print() const override final;
		virtual bool isConst() const override final;
		virtual CatStatement* constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual CatASTNodeType getNodeType() const override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;
		virtual std::any executeAssignable(CatRuntimeContext* runtimeContext) override final;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;

	public:
		std::string name;
		std::unique_ptr<CatTypedExpression> disambiguatedIdentifier;
	};


} //End namespace jitcat::AST