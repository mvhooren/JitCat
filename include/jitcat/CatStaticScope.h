/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatASTNode.h"
#include "jitcat/TypeInfo.h"
#include "jitcat/TypeOwnershipSemantics.h"

#include <memory>


namespace jitcat::AST
{
	class CatStaticScope: public CatASTNode
	{
	public:
		CatStaticScope(bool isValid, CatStaticScope* parentScope, const std::string& scopeName, const Tokenizer::Lexeme& scopeNameLexeme, const Tokenizer::Lexeme& lexeme);
		CatStaticScope(const CatStaticScope& other);
		virtual ~CatStaticScope();

		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;

		bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext);

		Reflection::TypeInfo* getScopeType() const;

	private:
		bool isValid;
		const std::string scopeName;
		const Tokenizer::Lexeme scopeNameLexeme;
		std::unique_ptr<CatStaticScope> parentScope;
		
		Reflection::TypeInfo* scopeType;

	};
}
