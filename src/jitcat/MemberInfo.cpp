/*
  This file is part of the JitCat library.

  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/
#include "jitcat/MemberInfo.h"

using namespace jitcat;
using namespace jitcat::Reflection;


std::any DeferredMemberInfo::getMemberReference(unsigned char* base)
{
	return deferredMember->getMemberReference(reinterpret_cast<unsigned char*>(baseMember->getType().getRawPointer(baseMember->getMemberReference(base))));
}


std::any DeferredMemberInfo::getAssignableMemberReference(unsigned char* base)
{
	return deferredMember->getAssignableMemberReference(reinterpret_cast<unsigned char*>(baseMember->getType().getRawPointer(baseMember->getMemberReference(base))));
}


llvm::Value* DeferredMemberInfo::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const
{
	llvm::Value* parent = baseMember->generateDereferenceCode(parentObjectPointer, context);
	return deferredMember->generateDereferenceCode(parent, context);
}


llvm::Value* DeferredMemberInfo::generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
{
	llvm::Value* parent = baseMember->generateDereferenceCode(parentObjectPointer, context);
	return deferredMember->generateAssignCode(parent, rValue, context);
}


llvm::Value* DeferredMemberInfo::generateArrayIndexCode(llvm::Value* container, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const
{
	llvm::Value* parent = baseMember->generateDereferenceCode(container, context);
	return deferredMember->generateArrayIndexCode(parent, index, context);
}


unsigned long long jitcat::Reflection::DeferredMemberInfo::getOrdinal() const
{
	return baseMember->getOrdinal() + deferredMember->getOrdinal();
}
