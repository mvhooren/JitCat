/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CatMemberAccess.h"
#include "CatLiteral.h"
#include "CatLog.h"
#include "MemberInfo.h"
#include "TypeInfo.h"

#include <cassert>

CatMemberAccess::CatMemberAccess(CatTypedExpression* base, const std::string& memberName):
	base(base),
	memberName(memberName),
	memberInfo(nullptr)
{
	if (base != nullptr
		&& base->getType() == CatType::Object)
	{
		CatGenericType baseMemberInfo = base->getType();
		if (baseMemberInfo.isValidType()
			&& (baseMemberInfo.isObjectType()
 			    || baseMemberInfo.isContainerType()))
		{
			memberInfo = baseMemberInfo.getObjectType()->getMemberInfo(Tools::toLowerCase(memberName));
		}

		if (memberInfo != nullptr)
		{
			type = memberInfo->toGenericType();
		}
	}
}


void CatMemberAccess::print() const
{
	base->print();
	CatLog::log(".");
	CatLog::log(memberName);
}


CatASTNodeType CatMemberAccess::getNodeType()
{
	return CatASTNodeType::MemberAccess;
}


std::any CatMemberAccess::execute(CatRuntimeContext* runtimeContext)
{
	std::any baseValue = base->execute(runtimeContext);
	if (memberInfo != nullptr && runtimeContext != nullptr)
	{
		return memberInfo->getMemberReference(baseValue);
	}
	assert(false);
	return std::any();
}


CatGenericType CatMemberAccess::typeCheck()
{
	CatGenericType baseType = base->typeCheck();
	if (!baseType.isValidType())
	{
		return baseType;
	}
	else if (baseType.isContainerType())
	{
		return CatGenericType(Tools::append("Invalid operation on container:", memberName));
	}
	else if (memberInfo != nullptr)
	{
		return memberInfo->toGenericType();
	}
	else
	{
		return CatGenericType(Tools::append("Member not found:", memberName));
	}
}


CatGenericType CatMemberAccess::getType() const
{
	return type;
}


bool CatMemberAccess::isConst() const
{
	if (memberInfo != nullptr)
	{
		return memberInfo->isConst && base->isConst();
	}
	return false;
}


CatTypedExpression* CatMemberAccess::constCollapse(CatRuntimeContext* compileTimeContext)
{
	if (type.isValidType() && isConst())
	{
		return new CatLiteral(execute(compileTimeContext), getType());
	}
	return this;
}


CatTypedExpression* CatMemberAccess::getBase() const
{
	return base.get();
}


TypeMemberInfo* CatMemberAccess::getMemberInfo() const
{
	return memberInfo;
}
