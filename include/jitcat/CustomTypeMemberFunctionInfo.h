/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatFunctionDefinition.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/MemberFunctionInfo.h"

#include <memory>


namespace jitcat::Reflection
{
	struct CustomTypeMemberFunctionInfo: public MemberFunctionInfo
	{
		CustomTypeMemberFunctionInfo(AST::CatFunctionDefinition* functionDefinition): 
			MemberFunctionInfo(functionDefinition->getFunctionName(), functionDefinition->getReturnTypeNode()->getType()),
			functionDefinition(functionDefinition)
		{
			int numParameters = functionDefinition->getNumParameters(); 
			for (int i = 0; i < numParameters; i++)
			{
				argumentTypes.push_back(functionDefinition->getParameterType(i)->getType());
			}
		};


		virtual ~CustomTypeMemberFunctionInfo() 
		{}


		inline virtual std::any call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) override final
		{ 
			return functionDefinition->executeFunctionWithArguments(runtimeContext, parameters);
		}


		inline virtual MemberFunctionCallData getFunctionAddress() const override final
		{
			return MemberFunctionCallData();
		}


		AST::CatFunctionDefinition* functionDefinition;
	};
}