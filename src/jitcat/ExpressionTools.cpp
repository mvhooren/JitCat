/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "ExpressionTools.h"
#include "CatASTNodes.h"
#include "CatError.h"
#include "CatGenericType.h"
#include "Document.h"
#include "ExpressionAny.h"
#include "ExpressionErrorManager.h"
#include "JitCat.h"
#include "MemberInfo.h"
#include "MemberReference.h"
#include "MemberReferencePtr.h"
#include "SLRParseResult.h"
#include "Tools.h"


CatValue ExpressionTools::executeExpression(const std::string& expression, CatRuntimeContext* context)
{
	Document document(expression.c_str(), expression.length());
	SLRParseResult* parseResult = JitCat::get()->parse(&document, context);
	if (parseResult->success)
	{
		CatTypedExpression* typedExpression = static_cast<CatTypedExpression*>(parseResult->astRootNode);
		CatValue value = typedExpression->execute(context);
		delete parseResult;
		return value;
	}
	else
	{
		CatError error = CatError(parseResult->errorMessage);
		delete parseResult;
		return CatValue(error);
	}
}


bool ExpressionTools::assignIfPossible(CatValue& targetValue, const CatValue& sourceValue)
{
	if (targetValue.isReference())
	{
		MemberReferencePtr targetRef = targetValue.getCustomTypeValue();
		if (!targetRef.isNull())
		{
			if (isBasicType(targetValue.getValueType()))
			{
				switch (targetValue.getValueType())
				{
					case CatType::Float:	targetRef->setFloat(sourceValue.toFloatValue());	return true;
					case CatType::Int:		targetRef->setInt(sourceValue.toIntValue());		return true;
					case CatType::Bool:	targetRef->setBool(sourceValue.toBoolValue());		return true;
					case CatType::String:	targetRef->setString(sourceValue.toStringValue());	return true;
				}
			}
			else if (targetRef->getType() == SpecificMemberType::NestedType)
			{
				if (sourceValue.isReference())
				{
					MemberReferencePtr sourceRef = sourceValue.getCustomTypeValue();
					if (!sourceRef.isNull()
						&& sourceRef->getType() == SpecificMemberType::NestedType
						//intentional comparison of pointer type
						&& targetRef->getCustomTypeName() == sourceRef->getCustomTypeName())
					{
						if (targetRef.getOriginalReference() != nullptr)
						{
							MemberReferencePtr* originalReference = targetRef.getOriginalReference();
							*originalReference = sourceRef.getPointer();
							return true;
						}
						else if (targetRef->getMemberInfo()->isWritable)
						{
							Reflectable* reflectableToCopyTo = targetRef->getParentObject();
							Reflectable* reflectableToCopy = sourceRef->getParentObject();
							if (reflectableToCopyTo != nullptr)
							{
								reflectableToCopyTo->copyFrom(reflectableToCopy);
							}
						}
					}

				}
			}
		}
	}
	return false;
}


void ExpressionTools::checkAssignment(ExpressionAny& target, ExpressionAny& source, CatRuntimeContext* context, void* errorSource)
{
	const CatGenericType targetType = target.getType();
	const CatGenericType sourceType = source.getType();
	if (!targetType.isValidType())
	{
		context->getErrorManager()->compiledWithError(Tools::append("ERROR in: ", context->getContextName(), ": Assignment failed because VariableToAssign is invalid."), errorSource);
	}
	else if (!sourceType.isValidType())
	{
		context->getErrorManager()->compiledWithError(Tools::append("ERROR in: ", context->getContextName(), ": Assignment failed because of an error in the source/expression."), errorSource);
	}
	else if (!(targetType.isContainerType() || sourceType.isContainerType())
			 && (targetType == sourceType
			     || (targetType.isBasicType() && sourceType.isBasicType())))
	{
		context->getErrorManager()->compiledWithoutErrors(errorSource);
	}
	else
	{
		context->getErrorManager()->compiledWithError(Tools::append("ERROR in: ", context->getContextName(), ": Unable to assign: ", source.getExpression(), " to ", target.getExpression()), errorSource);
	}
}
