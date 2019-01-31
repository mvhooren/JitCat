/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatBuiltInFunctionType.h"
#include "jitcat/CatTypedExpression.h"
#include "jitcat/CatGenericType.h"

#include <memory>
#include <vector>

namespace jitcat::AST
{
	class CatArgumentList;

	class CatFunctionCall: public CatTypedExpression
	{
	public:
		CatFunctionCall(const std::string& name, CatArgumentList* arguments);
		CatFunctionCall(const CatFunctionCall&) = delete;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;
		virtual CatGenericType typeCheck() override final;
		virtual CatGenericType getType() const override final;
		virtual bool isConst() const override final;
		virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext) override final;
		CatBuiltInFunctionType getFunctionType() const;

		const std::string& getFunctionName() const;
		CatArgumentList* getArgumentList() const;

		static bool isBuiltInFunction(const char* functionName, int numArguments);
		static const std::vector<std::string>& getAllBuiltInFunctions();

	private:
		bool isDeterministic() const;
		bool checkArgumentCount(std::size_t count) const;
		static CatBuiltInFunctionType toFunction(const char* functionName, int numArguments);
		static std::vector<std::string> functionTable;

	private:
		std::unique_ptr<CatArgumentList> arguments;
		std::vector<CatGenericType> argumentTypes;
		const std::string name;
		CatBuiltInFunctionType function;
	};

} //End namespace jitcat::AST