/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class CatArgumentList;
#include "CatTypedExpression.h"

#include <memory>
#include <vector>


class CatFunctionCall: public CatTypedExpression
{
private:
	enum class BuiltInFunction
	{
		ToInt,
		ToFloat,
		ToBool,
		ToString,
		ToPrettyString,
		ToFixedLengthString,
		Sin,
		Cos,
		Tan,
		Random,
		RandomRange,
		Round,
		StringRound,
		Abs,
		Cap,
		Min,
		Max,
		Log,
		Sqrt,
		Pow,
		Ceil,
		Floor,
		FindInString,
		ReplaceInString,
		StringLength,
		SubString,
		Select,
		Count,
		Invalid
	};
public:
	CatFunctionCall(const std::string& name, CatArgumentList* arguments);
	CatFunctionCall(const CatFunctionCall&) = delete;
	virtual void print() const override final;
	virtual CatASTNodeType getNodeType() override final;
	virtual CatValue execute(CatRuntimeContext* runtimeContext) override final;
	virtual CatGenericType typeCheck() override final;
	virtual CatGenericType getType() const override final;
	virtual bool isConst() const override final;
	virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext) override final;

	static bool isBuiltInFunction(const char* functionName, int numArguments);
	static const std::vector<std::string>& getAllBuiltInFunctions();

private:
	bool isDeterministic() const;
	bool checkArgumentCount(std::size_t count) const;
	static BuiltInFunction toFunction(const char* functionName, int numArguments);
	static std::vector<std::string> functionTable;

private:
	std::unique_ptr<CatArgumentList> arguments;
	const std::string name;
	BuiltInFunction function;
};