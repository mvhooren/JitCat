/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatGenericType.h"
#include "jitcat/FunctionSignature.h"
#include "jitcat/MemberFunctionInfo.h"

#include <vector>


namespace jitcat::Reflection
{
	class ArrayTypeInfo;

	struct ArrayMemberFunctionInfo: public MemberFunctionInfo
	{
	public:
		enum class Operation
		{
			Add,
			Index,
			Remove,
			Size
		};

		static const char* toString(Operation operation);

		ArrayMemberFunctionInfo(Operation operation, ArrayTypeInfo* arrayTypeInfo);

		virtual std::any call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) override final;

		virtual MemberFunctionCallData getFunctionAddress() const override final;

	private:
		Operation operation;
		ArrayTypeInfo* arrayTypeInfo;
	};
}