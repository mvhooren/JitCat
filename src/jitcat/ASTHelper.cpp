/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ASTHelper.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatFunctionCall.h"
#include "jitcat/CatTypedExpression.h"
#include "jitcat/ReflectableHandle.h"

#include <cassert>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;


void ASTHelper::updatePointerIfChanged(std::unique_ptr<CatTypedExpression>& uPtr, CatTypedExpression* expression)
{
	if (uPtr.get() != expression)
	{
		uPtr.reset(expression);
	}
}


void ASTHelper::doTypeConversion(std::unique_ptr<CatTypedExpression>& uPtr, const CatGenericType& targetType)
{
	CatGenericType sourceType = uPtr->getType();
	//Currently, only basic type conversions are supported.
	if (sourceType.isBasicType() && targetType.isBasicType()
		&& sourceType != targetType)
	{
		CatTypedExpression* sourceExpression = uPtr.release();
		CatArgumentList* arguments = new CatArgumentList(uPtr->getLexeme());
		arguments->arguments.emplace_back(sourceExpression);
		const char* functionName = nullptr;

		if		(targetType.isIntType())	functionName = "toInt";
		else if (targetType.isFloatType())	functionName = "toFloat";
		else if (targetType.isBoolType())	functionName = "toBool";
		else if (targetType.isStringType()) functionName = "toString";

		assert(functionName != nullptr);
		CatFunctionCall* functionCall = new CatFunctionCall(functionName, arguments, uPtr->getLexeme());
		uPtr.reset(functionCall);
	}
}


void ASTHelper::doAssignment(std::any& target, std::any& source, const CatGenericType& type, AssignableType targetAssignableType)
{
	if (targetAssignableType == AssignableType::Pointer)
	{
		if		(type.isIntType())		*std::any_cast<int*>(target) = std::any_cast<int>(source);
		else if (type.isFloatType())	*std::any_cast<float*>(target) = std::any_cast<float>(source);
		else if (type.isBoolType())		*std::any_cast<bool*>(target) = std::any_cast<bool>(source);
		else if (type.isStringType())	*std::any_cast<std::string*>(target) = std::any_cast<std::string>(source);
		else if (type.isObjectType())	
		{
			//Not supported for now. This would need to call operator= on the target object, not all objects will have implemented this.
		}
		return;
	}
	else if (targetAssignableType == AssignableType::PointerPointer && type.isObjectType())
	{
		*std::any_cast<Reflectable**>(target)		= std::any_cast<Reflectable*>(source);
		return;
	}
	else if (targetAssignableType == AssignableType::HandlePointer  && type.isObjectType())	
	{
		*std::any_cast<ReflectableHandle*>(target)	= std::any_cast<Reflectable*>(source);
		return;
	}

	assert(false);
}
