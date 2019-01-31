/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CatMemberFunctionCall.h"
#include "CatArgumentList.h"
#include "CatLog.h"
#include "MemberInfo.h"
#include "ASTHelper.h"
#include "TypeInfo.h"

#include <cassert>


CatMemberFunctionCall::CatMemberFunctionCall(const std::string& name, CatTypedExpression* base, CatArgumentList* arguments):
	functionName(name),
	arguments(arguments),
	base(base),
	memberFunctionInfo(nullptr)
{
	if (base != nullptr
		&& base->getType().isObjectType())
	{
		CatGenericType baseMemberInfo = base->getType();
		if (baseMemberInfo.isValidType()
			&& (baseMemberInfo.isObjectType()
 			    ||  baseMemberInfo.isContainerType()))
		{
			memberFunctionInfo = baseMemberInfo.getObjectType()->getMemberFunctionInfo(Tools::toLowerCase(name));
		}
	}

}


void CatMemberFunctionCall::print() const
{
	base->print();
	CatLog::log(".");
	CatLog::log(functionName);
	arguments->print();
}


CatASTNodeType CatMemberFunctionCall::getNodeType()
{
	return CatASTNodeType::MemberFunctionCall;
}


std::any CatMemberFunctionCall::execute(CatRuntimeContext* runtimeContext)
{
	std::any baseValue = base->execute(runtimeContext);
	if (memberFunctionInfo != nullptr && runtimeContext != nullptr)
	{
		std::vector<std::any> argumentValues;
		for (std::unique_ptr<CatTypedExpression>& argument : arguments->arguments)
		{
			argumentValues.push_back(argument->execute(runtimeContext));
		}
		return memberFunctionInfo->call(baseValue, argumentValues);
	}
	assert(false);
	return std::any();

}


CatGenericType CatMemberFunctionCall::typeCheck()
{
	CatGenericType baseType = base->typeCheck();
	if (!baseType.isValidType())
	{
		return baseType;
	}
	else if (memberFunctionInfo != nullptr)
	{
		std::size_t numArgumentsSupplied = arguments->arguments.size();
		if (numArgumentsSupplied != memberFunctionInfo->getNumberOfArguments())
		{
			return CatGenericType(Tools::append("Invalid number of arguments for function: ", functionName, " expected ", memberFunctionInfo->getNumberOfArguments(), " arguments."));
		}
		std::vector<CatGenericType> argumentList;
		for (unsigned int i = 0; i < numArgumentsSupplied; i++)
		{
			argumentList.push_back(arguments->arguments[i]->typeCheck());
			if (!argumentList[i].isValidType())
			{
				return argumentList[i];
			}
			else if (!(memberFunctionInfo->getArgumentType(i) == argumentList[i]))
			{
				return CatGenericType(Tools::append("Invalid argument for function: ", functionName, " argument nr: ", i, " expected: ", memberFunctionInfo->getArgumentType(i).toString()));
			}
		}
		return memberFunctionInfo->returnType;
	}
	else
	{
		return CatGenericType(Tools::append("Member function not found: ", functionName));
	}
}


CatGenericType CatMemberFunctionCall::getType() const
{
	if (memberFunctionInfo != nullptr)
	{
		return memberFunctionInfo->returnType;
	}
	else 
	{
		return CatGenericType::unknownType;
	}
}


bool CatMemberFunctionCall::isConst() const
{
	return false;
}


CatTypedExpression* CatMemberFunctionCall::constCollapse(CatRuntimeContext* compileTimeContext)
{
	ASTHelper::updatePointerIfChanged(base, base->constCollapse(compileTimeContext));
	for (auto& iter: arguments->arguments)
	{
		ASTHelper::updatePointerIfChanged(iter, iter->constCollapse(compileTimeContext));
	}
	return this;
}


MemberFunctionInfo* CatMemberFunctionCall::getMemberFunctionInfo() const
{
	return memberFunctionInfo;
}


CatTypedExpression* CatMemberFunctionCall::getBase() const
{
	return base.get();
}

CatArgumentList* CatMemberFunctionCall::getArguments() const
{
	return arguments.get();
}
