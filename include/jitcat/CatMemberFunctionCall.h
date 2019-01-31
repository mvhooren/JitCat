/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat::Reflection
{
	struct MemberFunctionInfo;
}
#include "jitcat/CatTypedExpression.h"

#include <memory>

namespace jitcat::AST
{
	class CatArgumentList;


	class CatMemberFunctionCall: public CatTypedExpression
	{
	public:
		CatMemberFunctionCall(const std::string& name, CatTypedExpression* base, CatArgumentList* arguments);
		CatMemberFunctionCall(const CatMemberFunctionCall&) = delete;
		// Inherited via CatTypedExpression
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;
		virtual CatGenericType typeCheck() override final;
		virtual CatGenericType getType() const override final;
		virtual bool isConst() const override final;
		virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext) override final;
	
		Reflection::MemberFunctionInfo* getMemberFunctionInfo() const;
		CatTypedExpression* getBase() const;
		CatArgumentList* getArguments() const;

	private:
		Reflection::MemberFunctionInfo* memberFunctionInfo;
		const std::string functionName;
		std::unique_ptr<CatTypedExpression> base;
		std::unique_ptr<CatArgumentList> arguments;
	};


} // End namespace jitcat::AST