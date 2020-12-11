/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/TypeMemberInfo.h"
#include "jitcat/MemberInfo.h"

using namespace jitcat;
using namespace jitcat::Reflection;


std::any TypeMemberInfo::getMemberReference(unsigned char* base)
{
	return std::any();
}


std::any TypeMemberInfo::getAssignableMemberReference(unsigned char* base)
{
	return std::any();
}


llvm::Value* TypeMemberInfo::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const
{
	return nullptr;
}


llvm::Value* TypeMemberInfo::generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
{
	return nullptr;
}


llvm::Value* TypeMemberInfo::generateArrayIndexCode(llvm::Value* container, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const
{
	return nullptr;
}


TypeMemberInfo* TypeMemberInfo::toDeferredTypeMemberInfo(TypeMemberInfo* baseMember)
{
	return new DeferredMemberInfo(baseMember, this);
}


const CatGenericType& TypeMemberInfo::getType() const
{
	return catType;
}


const std::string& TypeMemberInfo::getMemberName() const
{
	return memberName;
}


void TypeMemberInfo::setMemberName(const std::string& newMemberName)
{
	memberName = newMemberName;
}


MemberVisibility TypeMemberInfo::getMemberVisibility() const
{
	return visibility;
}


void TypeMemberInfo::setVisibility(MemberVisibility newVisibility)
{
	visibility = newVisibility;
}
