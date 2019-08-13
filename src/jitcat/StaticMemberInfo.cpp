/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/StaticMemberInfo.h"
#include "jitcat/ReflectableHandle.h"

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
	return nullptr;
}


llvm::Value* StaticMemberInfo::generateAssignCode(llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
{
	return nullptr;
}


llvm::Value* StaticMemberInfo::generateArrayIndexCode(llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const
{
	return nullptr;
}


std::any StaticClassPointerMemberInfo::getMemberReference()
{
	return std::any(*memberPointer);
}


std::any StaticClassPointerMemberInfo::getAssignableMemberReference()
{
	return std::any(memberPointer);
}


llvm::Value* StaticClassPointerMemberInfo::generateDereferenceCode(LLVM::LLVMCompileTimeContext* context) const
{
	return nullptr;
}


llvm::Value* StaticClassPointerMemberInfo::generateAssignCode(llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
{
	return nullptr;
}


std::any jitcat::Reflection::StaticClassHandleMemberInfo::getMemberReference()
{
	return std::any(memberPointer->get());
}


std::any jitcat::Reflection::StaticClassHandleMemberInfo::getAssignableMemberReference()
{
	return std::any(memberPointer);
}


llvm::Value* jitcat::Reflection::StaticClassHandleMemberInfo::generateDereferenceCode(LLVM::LLVMCompileTimeContext* context) const
{
	return nullptr;
}


llvm::Value* jitcat::Reflection::StaticClassHandleMemberInfo::generateAssignCode(llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
{
	return nullptr;
}


std::any StaticClassObjectMemberInfo::getMemberReference()
{
	return std::any(memberPointer);
}


std::any StaticClassObjectMemberInfo::getAssignableMemberReference()
{
	return std::any((Reflectable**)nullptr);
}


llvm::Value* StaticClassObjectMemberInfo::generateDereferenceCode(LLVM::LLVMCompileTimeContext* context) const
{
	return nullptr;
}

