/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatStatement.h"
#include "jitcat/CatScope.h"
#include "jitcat/CatScopeID.h"

#include <any>
#include <memory>
#include <vector>

namespace jitcat
{
	namespace Reflection
	{
		class CustomTypeInfo;
	}
}

namespace jitcat::AST
{
	class CatStatement;

	class CatScopeBlock: public CatStatement, public CatScope
	{
	public:
		CatScopeBlock(const std::vector<CatStatement*>& statementList, const Tokenizer::Lexeme& lexeme);
		CatScopeBlock(const CatScopeBlock& other);
		virtual ~CatScopeBlock();

		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;

		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;

		virtual std::optional<bool> checkControlFlow(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext, bool& unreachableCodeDetected) const override final;

		bool containsReturnStatement() const;

		virtual Reflection::CustomTypeInfo* getCustomType() override final;
		virtual CatScopeID getScopeId() const override final;

		void insertStatementFront(CatStatement* statement);

	private:
		std::vector<std::unique_ptr<CatStatement>> statements;
		std::unique_ptr<Reflection::CustomTypeInfo> customType;
		CatScopeID scopeId;
	};

}