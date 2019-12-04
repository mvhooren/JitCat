/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatScope.h"
#include "jitcat/CatStatement.h"
#include "jitcat/TypeInfoDeleter.h"

#include <memory>

namespace jitcat::Reflection
{
	class CustomTypeInfo;
	struct TypeMemberInfo;
}

namespace jitcat::AST
{
	class CatScopeBlock;
	class CatRange;

	class CatForLoop: public CatStatement, public CatScope
	{
	public:
		CatForLoop(const Tokenizer::Lexeme& lexeme, const Tokenizer::Lexeme& iteratorLexeme, CatRange* range, CatScopeBlock* loopBody);
		CatForLoop(const CatForLoop& other);

		virtual ~CatForLoop();

		//From CatASTNode
		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;

		//From CatStatement
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual std::any execute(jitcat::CatRuntimeContext* runtimeContext) override final;
		virtual std::optional<bool> checkControlFlow(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext, bool& unreachableCodeDetected) const override final;

		// Inherited via CatScope
		virtual CatScopeID getScopeId() const override final;
		virtual Reflection::CustomTypeInfo* getCustomType() override final;

	private:
		Tokenizer::Lexeme iteratorLexeme;
		std::string iteratorName;
		CatScopeID loopIteratorScope;

		std::unique_ptr<Reflection::CustomTypeInfo, Reflection::TypeInfoDeleter> scopeType;
		Reflection::TypeMemberInfo* iteratorMember;

		std::unique_ptr<CatRange> range;
		std::unique_ptr<CatScopeBlock> loopBody;
		

	};
}
