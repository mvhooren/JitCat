/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
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

	class CatStaticIdentifier: public CatAssignableExpression
	{
	public:
		CatStaticIdentifier(CatTypeNode* baseType, const Tokenizer::Lexeme& nameLexeme, const Tokenizer::Lexeme& lexeme);
		CatStaticIdentifier(CatStaticIdentifier* idBaseType, const Tokenizer::Lexeme& nameLexeme, const Tokenizer::Lexeme& lexeme);
		CatStaticIdentifier(const CatStaticIdentifier& other);

		virtual CatASTNode* copy() const override final;
		virtual const CatGenericType& getType() const override final;
		virtual const CatGenericType& getAssignableType() const override final;
		virtual bool isAssignable() const override final;

		virtual void print() const override final;
		virtual bool isConst() const override final;
		virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext) override final;
		virtual CatASTNodeType getNodeType() const override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;
		virtual std::any executeAssignable(CatRuntimeContext* runtimeContext) override final;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;

		bool isNestedType() const;
		bool isStaticMember() const;

		const Reflection::StaticMemberInfo* getMemberInfo() const;
		const Reflection::TypeInfo* getTypeInfo() const;


		const std::string& getName() const;

	private:
		std::string name;
		Tokenizer::Lexeme nameLexeme;

		CatGenericType type;
		CatGenericType assignableType;

		//Either of these must be non-null;
		std::unique_ptr<CatTypeNode> baseType;
		std::unique_ptr<CatStaticIdentifier> idBaseType;

		//A static identifier refers to either a class or a static class member.
		//In the future it might also refer to a namespace.
		Reflection::StaticMemberInfo* memberInfo;
		Reflection::TypeInfo* nestedType;
	};


} //End namespace jitcat::AST