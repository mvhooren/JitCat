/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CustomTypeMemberFunctionInfo.h"
#include "jitcat/CatFunctionDefinition.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/CustomObject.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/FunctionNameMangler.h"
#include "jitcat/TypeInfo.h"


using namespace jitcat;
using namespace jitcat::Reflection;


CustomTypeMemberFunctionInfo::CustomTypeMemberFunctionInfo(AST::CatFunctionDefinition* functionDefinition, const CatGenericType& thisType) :
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
}


 CustomTypeMemberFunctionInfo::~CustomTypeMemberFunctionInfo()
 {}


 std::any CustomTypeMemberFunctionInfo::call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) const
 {
	 if (base.has_value())
	 {
		 CustomObject* baseReflectable = std::any_cast<CustomObject*>(base);
		 runtimeContext->pushStackFrame();
		 CatScopeID scope = runtimeContext->addScope(thisType.getObjectType(), reinterpret_cast<unsigned char*>(baseReflectable), false);
		 std::any result = functionDefinition->executeFunctionWithArguments(runtimeContext, parameters);
		 runtimeContext->removeScope(scope);
		 runtimeContext->popStackFrame();
		 return result;
	 }
	 return functionDefinition->getReturnTypeNode()->getType().createDefault();
 }


 MemberFunctionCallData CustomTypeMemberFunctionInfo::getFunctionAddress() const
 {
	 return MemberFunctionCallData(nativeAddress, reinterpret_cast<uintptr_t>(this), nullptr, MemberFunctionCallType::ThisCall, true);
 }


 std::string CustomTypeMemberFunctionInfo::getMangledName() const
 {
	return functionDefinition->getMangledFunctionName();
 }


 const AST::CatFunctionDefinition*CustomTypeMemberFunctionInfo::getFunctionDefinition() const
 {
	 return functionDefinition;
 }


 void CustomTypeMemberFunctionInfo::setFunctionNativeAddress(intptr_t functionNativeAddress)
 {
	 nativeAddress = functionNativeAddress;
 }
