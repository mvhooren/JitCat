/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/StaticMemberInfo.h"
#include "jitcat/ReflectableHandle.h"

#include <cassert>


using namespace jitcat;
using namespace jitcat::Reflection;


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


StaticClassPointerMemberInfo::StaticClassPointerMemberInfo(const std::string& memberName, unsigned char** memberPointer, const CatGenericType& type):
	StaticMemberInfo(memberName, type), memberPointer(memberPointer)
{
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
	return context->helper->createPtrConstant(reinterpret_cast<intptr_t>(memberPointer), "pointerTo_" + memberName, context->helper->toLLVMPtrType(*catType.getPointeeType()));
#else 
	return nullptr;
#endif // ENABLE_LLVM
}


llvm::Value* StaticClassPointerMemberInfo::generateAssignCode(llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	llvm::Constant* pointerAddress = context->helper->createIntPtrConstant(reinterpret_cast<intptr_t>(memberPointer), "pointerTo_" + memberName);
	llvm::Value* addressValue = context->helper->convertToPointer(pointerAddress, memberName + "_Ptr", context->helper->toLLVMPtrType(catType));
	context->helper->writeToPointer(addressValue, rValue);
	return rValue;
#else
	return nullptr;
#endif // ENABLE_LLVM
}


std::any jitcat::Reflection::StaticClassHandleMemberInfo::getMemberReference()
{
	return catType.createFromRawPointer(reinterpret_cast<uintptr_t>(memberPointer->get()));
}


std::any jitcat::Reflection::StaticClassHandleMemberInfo::getAssignableMemberReference()
{
	return catType.createFromRawPointer(reinterpret_cast<uintptr_t>(memberPointer->get()));
}


llvm::Value* jitcat::Reflection::StaticClassHandleMemberInfo::generateDereferenceCode(LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	//Create a constant with the pointer to the reflectable handle.
	llvm::Value* reflectableHandle = context->helper->createPtrConstant(reinterpret_cast<uintptr_t>(memberPointer), "ReflectableHandle");
	//Call function that gets the value
	std::string mangledName = "unsigned char* __getReflectable(const ReflectableHandle& handle)";
	context->helper->defineWeakSymbol(reinterpret_cast<uintptr_t>(&ReflectableHandle::staticGet), mangledName);
	return context->helper->createCall(LLVM::LLVMTypes::functionRetPtrArgPtr, {reflectableHandle}, false, mangledName, "getReflectable");
#else 
	return nullptr;
#endif //ENABLE_LLVM
}


llvm::Value* jitcat::Reflection::StaticClassHandleMemberInfo::generateAssignCode(llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	//Create a constant with the pointer to the reflectable handle.
	llvm::Value* reflectableHandle = context->helper->createPtrConstant(reinterpret_cast<uintptr_t>(memberPointer), "ReflectableHandle");
	//Wether or not the assigned value inherits from reflectable
	llvm::Constant* typeInfoConstant = context->helper->createIntPtrConstant(reinterpret_cast<uintptr_t>(catType.removeIndirection().getObjectType()), Tools::append(catType.removeIndirection().getObjectTypeName(), "_typeInfo"));
	llvm::Value* typeInfoConstantAsIntPtr = context->helper->convertToPointer(typeInfoConstant, Tools::append(catType.removeIndirection().getObjectTypeName(), "_typeInfoPtr"));
	//Call function that gets the member
	context->helper->createIntrinsicCall(context, &ReflectableHandle::staticAssign, {reflectableHandle, rValue, typeInfoConstantAsIntPtr}, "staticAssign");
	return rValue;
#else
	return nullptr;
#endif // ENABLE_LLVM
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
		llvm::Constant* objectPointer = context->helper->createIntPtrConstant(reinterpret_cast<intptr_t>(memberPointer), "pointerTo_" + memberName);
		return context->helper->convertToPointer(objectPointer, memberName);
#else 
	return nullptr;
#endif // ENABLE_LLVM
}

