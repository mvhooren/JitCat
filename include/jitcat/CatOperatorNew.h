/*
  This file is part of the JitCat library.

  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/
#pragma once

#include "jitcat/CatArgumentList.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/CatMemberFunctionCall.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/CatTypedExpression.h"

#include <memory>
#include <vector>

namespace jitcat
{
	class CatHostClass;
}
namespace jitcat::Reflection
{
	struct MemberFunctionInfo;
}

namespace jitcat::AST
{
	class CatOperatorNew: public CatTypedExpression
	{
	public:
		CatOperatorNew(CatMemberFunctionCall* functionCall, const Tokenizer::Lexeme& lexeme);
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual CatGenericType getType() const override final;
		virtual bool isConst() const override final;
		virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext) override final;


	private:
		std::unique_ptr<CatMemberFunctionCall> functionCall;
		std::unique_ptr<CatTypeNode> type;
		std::string typeName;
		CatGenericType newType;
		//may be nullptr
		CatHostClass* hostClass;
	};
}