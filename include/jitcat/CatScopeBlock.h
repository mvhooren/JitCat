/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatStatement.h"
#include "jitcat/CatScope.h"
#include "jitcat/CatScopeID.h"
#include "jitcat/TypeInfoDeleter.h"

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
		virtual CatStatement* constCollapse(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;

		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;

		virtual std::optional<bool> checkControlFlow(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext, bool& unreachableCodeDetected) override final;

		bool containsReturnStatement() const;

		virtual Reflection::CustomTypeInfo* getCustomType() const override final;
		virtual CatScopeID getScopeId() const override final;

		const std::vector<std::unique_ptr<CatStatement>>& getStatements() const;

		void insertStatementFront(CatStatement* statement);
		void addStatement(CatStatement* statement);

	private:
		std::vector<std::unique_ptr<CatStatement>> statements;
		std::unique_ptr<Reflection::CustomTypeInfo, Reflection::TypeInfoDeleter> customType;
		CatScopeID scopeId;
	};

}