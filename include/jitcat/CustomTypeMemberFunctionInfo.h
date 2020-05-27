/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatFunctionDefinition.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/TypeInfo.h"
#include "jitcat/MemberFunctionInfo.h"

#include <memory>


namespace jitcat::Reflection
{
	struct CustomTypeMemberFunctionInfo: public MemberFunctionInfo
	{
		CustomTypeMemberFunctionInfo(AST::CatFunctionDefinition* functionDefinition, const CatGenericType& thisType) :
			MemberFunctionInfo(functionDefinition->getFunctionName(), functionDefinition->getReturnTypeNode()->getType()),
			thisType(thisType),
			functionDefinition(functionDefinition),
			nativeAddress(0)
		{
			int numParameters = functionDefinition->getNumParameters(); 
			for (int i = 0; i < numParameters; i++)
			{
				argumentTypes.push_back(functionDefinition->getParameterType(i));
			}
		};


		virtual ~CustomTypeMemberFunctionInfo() 
		{}


		inline virtual std::any call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) override final
		{ 
			if (base.has_value())
			{
				Reflectable* baseReflectable = std::any_cast<Reflectable*>(base);
				runtimeContext->pushStackFrame();
				CatScopeID scope = runtimeContext->addScope(thisType.getObjectType(), reinterpret_cast<unsigned char*>(baseReflectable), false);
				std::any result = functionDefinition->executeFunctionWithArguments(runtimeContext, parameters);
				runtimeContext->removeScope(scope);
				runtimeContext->popStackFrame();
				return result;
			}
			return functionDefinition->getReturnTypeNode()->getType().createDefault();
		}


		inline virtual MemberFunctionCallData getFunctionAddress() const override final
		{
			return MemberFunctionCallData(nativeAddress, reinterpret_cast<uintptr_t>(this), MemberFunctionCallType::ThisCall);
		}

		CatGenericType thisType;
		AST::CatFunctionDefinition* functionDefinition;
		intptr_t nativeAddress;
	};
}