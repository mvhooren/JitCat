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
	memberInfo(nullptr),
	type(CatType::Error)
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
		else
		{
			assert(false);
		}
	}
	else
	{
		assert(false);
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


CatValue CatMemberAccess::execute(CatRuntimeContext* runtimeContext)
{
	CatValue baseValue = base->execute(runtimeContext);
	if (baseValue.getValueType() == CatType::Error)
	{
		return baseValue;
	}
	if (memberInfo != nullptr && runtimeContext != nullptr)
	{

		MemberReferencePtr rootReference = baseValue.getCustomTypeValue();
		MemberReferencePtr reference = memberInfo->getMemberReference(rootReference);
		if (!reference.isNull())
		{
			switch (type.getCatType())
			{
				case CatType::Int:		
				case CatType::Float:	
				case CatType::String:	
				case CatType::Bool:	
				case CatType::Object:	return CatValue(reference);
				case CatType::Void:	return CatValue();
			}
		}
	}
	return CatValue(CatError(std::string("Member not found:") + memberName));
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
		return new CatLiteral(execute(compileTimeContext));
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
