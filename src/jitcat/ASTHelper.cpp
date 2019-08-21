/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ASTHelper.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatAssignableExpression.h"
#include "jitcat/CatBuiltInFunctionCall.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypedExpression.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/ReflectableHandle.h"
#include "jitcat/TypeInfo.h"

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
		CatArgumentList* arguments = new CatArgumentList(sourceExpression->getLexeme(), std::vector<CatTypedExpression*>({sourceExpression}));

		const char* functionName = nullptr;

		if		(targetType.isIntType())	functionName = "toInt";
		else if (targetType.isFloatType())	functionName = "toFloat";
		else if (targetType.isBoolType())	functionName = "toBool";
		else if (targetType.isStringType()) functionName = "toString";

		assert(functionName != nullptr);
		CatBuiltInFunctionCall* functionCall = new CatBuiltInFunctionCall(functionName, sourceExpression->getLexeme(), arguments, sourceExpression->getLexeme());
		uPtr.reset(functionCall);
	}
}


std::any ASTHelper::doAssignment(CatAssignableExpression* target, CatTypedExpression* source, CatRuntimeContext* context)
{
	CatGenericType targetType = target->getAssignableType();
	CatGenericType sourceType = source->getType();

	std::any targetValue = target->executeAssignable(context);
	std::any sourceValue;

	if ((targetType.isPointerToHandleType() || targetType.isPointerToPointerType())
		 &&	(   targetType.getPointeeType()->getOwnershipSemantics() == TypeOwnershipSemantics::Owned
		    || targetType.getPointeeType()->getOwnershipSemantics() == TypeOwnershipSemantics::Shared)
		&& (sourceType.getOwnershipSemantics() == TypeOwnershipSemantics::Owned))
	{
		sourceValue = static_cast<CatAssignableExpression*>(source)->executeAssignable(context);
		sourceType = static_cast<CatAssignableExpression*>(source)->getAssignableType();
	}
	else
	{
		sourceValue = source->execute(context);
	}

	return doAssignment(targetValue, sourceValue, targetType, sourceType);
}


std::any jitcat::AST::ASTHelper::doGetArgument(CatTypedExpression* argument, const CatGenericType& parameterType, CatRuntimeContext* context)
{
	if (!parameterType.isPointerToReflectableObjectType()
		|| (parameterType.getOwnershipSemantics() != TypeOwnershipSemantics::Owned
			&& !(parameterType.getOwnershipSemantics() == TypeOwnershipSemantics::Shared && argument->getType().getOwnershipSemantics() == TypeOwnershipSemantics::Owned))
		|| argument->getType().getOwnershipSemantics() == TypeOwnershipSemantics::Value)
	{
		return argument->execute(context);
	}
	else
	{
		assert(argument->isAssignable());
		assert(argument->getType().getOwnershipSemantics() == TypeOwnershipSemantics::Owned);
		std::any sourceValue = static_cast<CatAssignableExpression*>(argument)->executeAssignable(context);
		const CatGenericType& sourceType = static_cast<CatAssignableExpression*>(argument)->getAssignableType();
		if (sourceType.isPointerToPointerType() && sourceType.getPointeeType()->getPointeeType()->isReflectableObjectType())
		{
			Reflectable** reflectableSource = std::any_cast<Reflectable**>(sourceValue);
			if (reflectableSource != nullptr)
			{
				Reflectable* value = *reflectableSource;
				*reflectableSource = nullptr;
				return value;
			}
			return (Reflectable*)nullptr;
		}
		else if (sourceType.isPointerToHandleType())
		{
			ReflectableHandle* handleSource = std::any_cast<ReflectableHandle*>(sourceValue);
			if (handleSource != nullptr)
			{
				Reflectable* value = handleSource->get();
				*handleSource = nullptr;
				return value;
			}
			return (Reflectable*)nullptr;
		}
		else
		{
			assert(false);
			return (Reflectable*)nullptr;
		}
	}
}


std::any ASTHelper::doAssignment(std::any& target, const std::any& source, const CatGenericType& targetType, const CatGenericType& sourceType)
{
	if (targetType.isPointerType() && targetType.getPointeeType()->isBasicType())
	{
		if (targetType.getPointeeType()->isIntType())
		{
			int* intTarget = std::any_cast<int*>(target);
			if (intTarget != nullptr)
			{
				*intTarget = std::any_cast<int>(source);
			}
		}
		else if (targetType.getPointeeType()->isFloatType())
		{
			float* floatTarget = std::any_cast<float*>(target);
			if (floatTarget != nullptr)
			{
				*floatTarget = std::any_cast<float>(source);
			}
		}
		else if (targetType.getPointeeType()->isBoolType())
		{
			bool* boolTarget = std::any_cast<bool*>(target);
			if (boolTarget != nullptr)
			{
				*boolTarget = std::any_cast<bool>(source);
			}
		}
		else if (targetType.getPointeeType()->isStringType())
		{
			std::string* stringTarget = std::any_cast<std::string*>(target);
			if (stringTarget != nullptr)
			{
				*stringTarget = std::any_cast<std::string>(source);
			}
		}
		else if (targetType.isPointerToReflectableObjectType())
		{
			//Not supported for now. This would need to call operator= on the target object, not all objects will have implemented this.
		}
		return target;
	}
	else if (targetType.isPointerToPointerType() && targetType.getPointeeType()->isPointerToReflectableObjectType())
	{
		Reflectable** reflectableTarget = std::any_cast<Reflectable **>(target);
		
		if (reflectableTarget != nullptr)
		{
			TypeOwnershipSemantics targetOwnership = targetType.getPointeeType()->getOwnershipSemantics();
			if (targetOwnership == TypeOwnershipSemantics::Owned && *reflectableTarget != nullptr)
			{
				targetType.getPointeeType()->getPointeeType()->getObjectType()->destruct(*reflectableTarget);
			}
			if (sourceType.isPointerToReflectableObjectType() || sourceType.isReflectableHandleType())
			{
				*reflectableTarget = std::any_cast<Reflectable*>(source);
			}
			else if (sourceType.isPointerToHandleType())
			{
				TypeOwnershipSemantics sourceOwnership = sourceType.getPointeeType()->getOwnershipSemantics();
				ReflectableHandle* sourceHandle = std::any_cast<ReflectableHandle*>(source);
				if (sourceHandle != nullptr)
				{
					*reflectableTarget = sourceHandle->get();
					if ((targetOwnership == TypeOwnershipSemantics::Owned
						|| targetOwnership == TypeOwnershipSemantics::Shared)
						&& sourceOwnership == TypeOwnershipSemantics::Owned)
					{
						*sourceHandle = nullptr;
					}
				}
				else
				{
					*reflectableTarget = nullptr;
				}
			}
			else if (sourceType.isPointerToPointerType() && sourceType.getPointeeType()->isPointerToReflectableObjectType())
			{
				TypeOwnershipSemantics sourceOwnership = sourceType.getPointeeType()->getOwnershipSemantics();
				Reflectable** sourcePointer = std::any_cast<Reflectable**>(source);
				if (sourcePointer != nullptr)
				{
					*reflectableTarget = *sourcePointer;
					if ((targetOwnership == TypeOwnershipSemantics::Owned
						|| targetOwnership == TypeOwnershipSemantics::Shared)
						&& sourceOwnership == TypeOwnershipSemantics::Owned)
					{
						*sourcePointer = nullptr;
					}
				}
				else
				{
					*reflectableTarget = nullptr;
				}
			}
		}
		return target;
	}
	else if (targetType.isPointerToHandleType())
	{
		ReflectableHandle* handleTarget = std::any_cast<ReflectableHandle*>(target);
		if (handleTarget != nullptr)
		{
			TypeOwnershipSemantics targetOwnership = targetType.getPointeeType()->getOwnershipSemantics();
			if (targetOwnership == TypeOwnershipSemantics::Owned && handleTarget->getIsValid())
			{
				targetType.getPointeeType()->getPointeeType()->getObjectType()->destruct(handleTarget->get());
			}
			if (sourceType.isPointerToReflectableObjectType()
				|| sourceType.isReflectableHandleType())
			{
				*handleTarget = std::any_cast<Reflectable*>(source);
			}
			else if (sourceType.isPointerToHandleType())
			{
				ReflectableHandle* sourceHandle = std::any_cast<ReflectableHandle*>(source);
				if (sourceHandle != nullptr)
				{
					*handleTarget = sourceHandle->get();
					if ((targetOwnership == TypeOwnershipSemantics::Owned
						|| targetOwnership == TypeOwnershipSemantics::Shared)
						&& sourceType.getPointeeType()->getOwnershipSemantics() == TypeOwnershipSemantics::Owned)
					{
						*sourceHandle = nullptr;
					}
				}
				else
				{
					*handleTarget = nullptr;
				}
			}
			else if (sourceType.isPointerToPointerType() && sourceType.getPointeeType()->isPointerToReflectableObjectType())
			{
				Reflectable** sourcePointer = std::any_cast<Reflectable**>(source);
				if (sourcePointer != nullptr)
				{
					*handleTarget = *sourcePointer;
					if ((targetOwnership == TypeOwnershipSemantics::Owned
						|| targetOwnership == TypeOwnershipSemantics::Shared)
						&& sourceType.getPointeeType()->getOwnershipSemantics() == TypeOwnershipSemantics::Owned)
					{
						*sourcePointer = nullptr;
					}
				}
				else
				{
					*handleTarget = nullptr;
				}
			}
		}
		return target;
	}
	assert(false);
	return std::any();
}


bool jitcat::AST::ASTHelper::checkAssignment(const CatTypedExpression* lhs, const CatTypedExpression* rhs, ExpressionErrorManager* errorManager, CatRuntimeContext* context, void* errorSource, const Tokenizer::Lexeme& lexeme)
{
	CatGenericType leftType = lhs->getType();
	CatGenericType rightType = rhs->getType();
	if (!leftType.isWritable() || leftType.isConst() || !lhs->isAssignable())
	{
		errorManager->compiledWithError("Assignment failed because target cannot be assigned.", errorSource, context->getContextName(), lexeme);
		return false;
	}
	else
	{
		if (leftType.compare(rightType, false))
		{
			if (!checkOwnershipSemantics(leftType, rightType, errorManager, context, errorSource, lexeme, "assign"))
			{
				return false;
			}
			return true;
		}
		else
		{
			errorManager->compiledWithError(Tools::append("Cannot assign ", rightType.toString(), " to ", leftType.toString(), "."), errorSource, context->getContextName(), lexeme);
			return false;
		}
	}
}


bool jitcat::AST::ASTHelper::checkOwnershipSemantics(const CatGenericType& targetType, const CatGenericType& sourceType, ExpressionErrorManager* errorManager, CatRuntimeContext* context, void* errorSource, const Tokenizer::Lexeme& lexeme, const std::string& operation)
{
	TypeOwnershipSemantics leftOwnership = targetType.getOwnershipSemantics();
	TypeOwnershipSemantics rightOwnership = sourceType.getOwnershipSemantics();
	if (leftOwnership == TypeOwnershipSemantics::Owned)
	{
		if (rightOwnership == TypeOwnershipSemantics::Shared)
		{
			errorManager->compiledWithError(Tools::append("Cannot ", operation, " shared ownership value to unique ownership value."), errorSource, context->getContextName(), lexeme);
			return false;
		}
		else if (rightOwnership == TypeOwnershipSemantics::Weak)
		{
			errorManager->compiledWithError(Tools::append("Cannot ", operation, " weakly-owned value to unique ownership value."), errorSource, context->getContextName(), lexeme);
			return false;
		}
	}
	else if (leftOwnership == TypeOwnershipSemantics::Shared)
	{
		if (rightOwnership == TypeOwnershipSemantics::Weak)
		{
			errorManager->compiledWithError(Tools::append("Cannot ", operation, " weakly-owned value to shared ownership value."), errorSource, context->getContextName(), lexeme);
			return false;
		}
	}
	/*else if (leftOwnership == TypeOwnershipSemantics::Weak)
	{
		if (rightOwnership == TypeOwnershipSemantics::Value && !sourceType.isNullptrType())
		{
			errorManager->compiledWithError(Tools::append("Cannot ", operation, " owned temporary value to weak ownership value."), errorSource, context->getContextName(), lexeme);
			return false;
		}
	}*/

	if (rightOwnership == TypeOwnershipSemantics::Owned
		&& (leftOwnership == TypeOwnershipSemantics::Owned
			|| leftOwnership == TypeOwnershipSemantics::Shared))
	{
		if (!sourceType.isWritable() || sourceType.isConst())
		{
			errorManager->compiledWithError("Cannot write from owned value because rhs cannot be assigned.", errorSource, context->getContextName(), lexeme);
			return false;
		}
	}
	return true;
}
