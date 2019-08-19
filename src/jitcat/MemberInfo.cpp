/*
  This file is part of the JitCat library.

  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/
#include "jitcat/MemberInfo.h"

using namespace jitcat;
using namespace jitcat::Reflection;



inline std::any TypeMemberInfo::getMemberReference(Reflectable* base)
{
	return std::any();
}


inline std::any TypeMemberInfo::getAssignableMemberReference(Reflectable* base)
{
	return std::any();
}


inline llvm::Value* TypeMemberInfo::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const
{
	return nullptr;
}


inline llvm::Value* TypeMemberInfo::generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
{
	return nullptr;
}


inline llvm::Value* TypeMemberInfo::generateArrayIndexCode(llvm::Value* container, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const
{
	return nullptr;
}


TypeMemberInfo* TypeMemberInfo::toDeferredTypeMemberInfo(TypeMemberInfo* baseMember)
{
	return new DeferredMemberInfo(baseMember, this);
}


inline std::any DeferredMemberInfo::getMemberReference(Reflectable* base)
{
	return deferredMember->getMemberReference(std::any_cast<Reflectable*>(baseMember->getMemberReference(base)));
}


inline std::any DeferredMemberInfo::getAssignableMemberReference(Reflectable* base)
{
	return deferredMember->getAssignableMemberReference(std::any_cast<Reflectable*>(baseMember->getMemberReference(base)));
}


inline llvm::Value* DeferredMemberInfo::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const
{
	llvm::Value* parent = baseMember->generateDereferenceCode(parentObjectPointer, context);
	return deferredMember->generateDereferenceCode(parent, context);
}


inline llvm::Value* DeferredMemberInfo::generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
{
	llvm::Value* parent = baseMember->generateDereferenceCode(parentObjectPointer, context);
	return deferredMember->generateAssignCode(parent, rValue, context);
}


inline llvm::Value* DeferredMemberInfo::generateArrayIndexCode(llvm::Value* container, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const
{
	llvm::Value* parent = baseMember->generateDereferenceCode(container, context);
	return deferredMember->generateArrayIndexCode(parent, index, context);
}


inline unsigned long long jitcat::Reflection::DeferredMemberInfo::getOrdinal() const
{
	return baseMember->getOrdinal() + deferredMember->getOrdinal();
}
