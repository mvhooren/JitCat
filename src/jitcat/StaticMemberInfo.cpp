/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/StaticMemberInfo.h"
#include "jitcat/LLVMCatIntrinsics.h"
#include "jitcat/ReflectableHandle.h"

#include <cassert>


using namespace jitcat;
using namespace jitcat::Reflection;


StaticMemberInfo::StaticMemberInfo(const std::string & memberName, const CatGenericType & type, const char * parentTypeName): 
	catType(type), 
	visibility(MemberVisibility::Public), 
	parentTypeName(parentTypeName),
	memberName(memberName)  
{
}


std::any StaticMemberInfo::getMemberReference()
{
	return std::any();
}


std::any StaticMemberInfo::getAssignableMemberReference()
{
	return std::any();
}


llvm::Value* StaticMemberInfo::generateDereferenceCode(LLVM::LLVMCompileTimeContext* context) const
{
	assert(false);
	return nullptr;
}


llvm::Value* StaticMemberInfo::generateAssignCode(llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
{
	assert(false);
	return nullptr;
}


llvm::Value* StaticMemberInfo::generateArrayIndexCode(llvm::Value* container, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const
{
	assert(false);
	return nullptr;
}


StaticClassPointerMemberInfo::StaticClassPointerMemberInfo(const std::string& memberName, unsigned char** memberPointer, const CatGenericType& type, const char* parentTypeName):
	StaticMemberInfo(memberName, type, parentTypeName), 
	memberPointer(memberPointer)
{
	if (JitCat::get()->getHasPrecompiledExpression())
	{
		JitCat::get()->setPrecompiledGlobalVariable(getStaticMemberPointerVariableName(), reinterpret_cast<uintptr_t>(memberPointer));
	}
}


std::any StaticClassPointerMemberInfo::getMemberReference()
{
	return catType.createFromRawPointer(reinterpret_cast<uintptr_t>(memberPointer));
}


std::any StaticClassPointerMemberInfo::getAssignableMemberReference()
{
	return catType.createFromRawPointer(reinterpret_cast<uintptr_t>(memberPointer));
}


llvm::Value* StaticClassPointerMemberInfo::generateDereferenceCode(LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	llvm::Value* staticPtr = context->helper->generateStaticPointerVariable(reinterpret_cast<intptr_t>(memberPointer), context, getStaticMemberPointerVariableName());
	return context->helper->convertToPointer(staticPtr, memberName + "_Ptr", context->helper->toLLVMPtrType(*catType.getPointeeType()));
#else 
	return nullptr;
#endif // ENABLE_LLVM
}


llvm::Value* StaticClassPointerMemberInfo::generateAssignCode(llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM

	llvm::Value* pointerAddress = context->helper->generateStaticPointerVariable(reinterpret_cast<intptr_t>(memberPointer), context, getStaticMemberPointerVariableName());
	llvm::Value* addressValue = context->helper->convertToPointer(pointerAddress, memberName + "_Ptr", context->helper->toLLVMPtrType(catType));
	context->helper->writeToPointer(addressValue, rValue);
	return rValue;
#else
	return nullptr;
#endif // ENABLE_LLVM
}


std::string StaticClassPointerMemberInfo::getStaticMemberPointerVariableName() const
{
	return Tools::append("pointerTo:", parentTypeName, "::", memberName);
}


StaticClassHandleMemberInfo::StaticClassHandleMemberInfo(const std::string& memberName, ReflectableHandle* memberPointer, 
														 const CatGenericType& type, const char* parentTypeName): 
			StaticMemberInfo(memberName, type, parentTypeName), 
			memberPointer(memberPointer) 
{
	if (JitCat::get()->getHasPrecompiledExpression())
	{
		JitCat::get()->setPrecompiledGlobalVariable(getStaticMemberPointerVariableName(), reinterpret_cast<uintptr_t>(memberPointer));
	}
}


std::any StaticClassHandleMemberInfo::getMemberReference()
{
	return catType.createFromRawPointer(reinterpret_cast<uintptr_t>(memberPointer->get()));
}


std::any StaticClassHandleMemberInfo::getAssignableMemberReference()
{
	return catType.createFromRawPointer(reinterpret_cast<uintptr_t>(memberPointer->get()));
}


llvm::Value* StaticClassHandleMemberInfo::generateDereferenceCode(LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	//Create a constant with the pointer to the reflectable handle.
	llvm::Value* reflectableHandle = context->helper->generateStaticPointerVariable(reinterpret_cast<intptr_t>(memberPointer), context, getStaticMemberPointerVariableName());

	//Call function that gets the value
	return context->helper->createIntrinsicCall(context, &LLVM::CatLinkedIntrinsics::_jc_getObjectPointerFromHandle, {reflectableHandle}, "_jc_getObjectPointerFromHandle", true);
#else 
	return nullptr;
#endif //ENABLE_LLVM
}


llvm::Value* StaticClassHandleMemberInfo::generateAssignCode(llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	llvm::Value* reflectableHandle = context->helper->generateStaticPointerVariable(reinterpret_cast<intptr_t>(memberPointer), context, getStaticMemberPointerVariableName());
	//Whether or not the assigned value inherits from reflectable
	llvm::Value* typeInfoConstantAsIntPtr = context->helper->createTypeInfoGlobalValue(context, catType.removeIndirection().getObjectType());
	//Call function that gets the member
	context->helper->createIntrinsicCall(context, &LLVM::CatLinkedIntrinsics::_jc_assignPointerToReflectableHandle, {reflectableHandle, rValue, typeInfoConstantAsIntPtr}, "_jc_assignPointerToReflectableHandle", true);
	return rValue;
#else
	return nullptr;
#endif // ENABLE_LLVM
}


std::string StaticClassHandleMemberInfo::getStaticMemberPointerVariableName() const
{
	return Tools::append("pointerTo:", parentTypeName, "::", memberName);
}


StaticClassObjectMemberInfo::StaticClassObjectMemberInfo(const std::string& memberName, unsigned char* memberPointer, 
														 const CatGenericType& type, const char* parentTypeName): 
			StaticMemberInfo(memberName, type, parentTypeName),
			memberPointer(memberPointer) 
{
	if (JitCat::get()->getHasPrecompiledExpression())
	{
		JitCat::get()->setPrecompiledGlobalVariable(getStaticMemberPointerVariableName(), reinterpret_cast<uintptr_t>(memberPointer));
	}
}


std::any StaticClassObjectMemberInfo::getMemberReference()
{
	return catType.createFromRawPointer(reinterpret_cast<uintptr_t>(memberPointer));
}


std::any StaticClassObjectMemberInfo::getAssignableMemberReference()
{
	return catType.toPointer().createNullPtr();
}


llvm::Value* StaticClassObjectMemberInfo::generateDereferenceCode(LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM

		llvm::Value* objectPointer =  context->helper->generateStaticPointerVariable(reinterpret_cast<intptr_t>(memberPointer), context, getStaticMemberPointerVariableName());

		return context->helper->convertToPointer(objectPointer, memberName);
#else 
	return nullptr;
#endif // ENABLE_LLVM
}


std::string StaticClassObjectMemberInfo::getStaticMemberPointerVariableName() const
{
	return Tools::append("pointerTo:", parentTypeName, "::", memberName);
}

