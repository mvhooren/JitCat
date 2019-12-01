/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatArrayIndex.h"
#include "jitcat/ContainerManipulator.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/FunctionSignature.h"
#include "jitcat/MemberInfo.h"
#include "jitcat/MemberFunctionInfo.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeInfo.h"
#include "jitcat/TypeOwnershipSemantics.h"

#include <sstream>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


CatArrayIndex::CatArrayIndex(CatTypedExpression* base, CatTypedExpression* arrayIndex, const Tokenizer::Lexeme& lexeme):
	CatAssignableExpression(lexeme),
	array(base),
	index(arrayIndex),
	arrayType(CatGenericType::unknownType),
	indexType(CatGenericType::unknownType),
	containerItemType(CatGenericType::unknownType),
	assignableItemType(CatGenericType::unknownType),
	isReflectedArrayType(false),
	arrayIndexFunction(nullptr)
{

}


jitcat::AST::CatArrayIndex::CatArrayIndex(const CatArrayIndex& other):
	CatAssignableExpression(other),
	array(static_cast<CatTypedExpression*>(other.array->copy())),
	index(static_cast<CatTypedExpression*>(other.index->copy())),
	arrayType(CatGenericType::unknownType),
	indexType(CatGenericType::unknownType),
	containerItemType(CatGenericType::unknownType),
	assignableItemType(CatGenericType::unknownType),
	isReflectedArrayType(false),
	arrayIndexFunction(nullptr)
{	
}


CatASTNode* jitcat::AST::CatArrayIndex::copy() const
{
	return new CatArrayIndex(*this);
}


void CatArrayIndex::print() const
{
	array->print();
	CatLog::log("[");
	index->print();
	CatLog::log("]");
}


CatASTNodeType CatArrayIndex::getNodeType() const
{
	return CatASTNodeType::ArrayIndex;
}


std::any CatArrayIndex::execute(CatRuntimeContext* runtimeContext)
{
	std::any arrayValue = array->execute(runtimeContext);
	Reflectable* testReflectable = std::any_cast<Reflectable*>(arrayValue);
	std::any indexValue = index->execute(runtimeContext);
	if (!isReflectedArrayType)
	{
		if (arrayType.isVectorType())
		{
			return arrayType.getContainerManipulator()->getItemAt(arrayValue, std::any_cast<int>(indexValue));
		}
		else if (arrayType.isArrayType())
		{
			return static_cast<ArrayManipulator*>(arrayType.getObjectType())->getItemAt(arrayValue, std::any_cast<int>(indexValue));
		}
		else if (arrayType.isMapType())
		{
			if (indexType.isIntType() && !arrayType.getContainerManipulator()->getKeyType().isIntType())
			{
				return arrayType.getContainerManipulator()->getItemAt(arrayValue, std::any_cast<int>(indexValue));
			}
			else 
			{
				return arrayType.getContainerManipulator()->getItemAt(arrayValue, indexValue);
			}
		}
	}
	else 
	{
		return arrayIndexFunction->call(runtimeContext, arrayValue, {indexValue});
	}
	return containerItemType.createDefault();
}


bool CatArrayIndex::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (array->typeCheck(compiletimeContext, errorManager, errorContext)
		&& index->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		arrayType = array->getType();
		indexType = index->getType();

		if ((arrayType.isPointerToReflectableObjectType() || arrayType.isReflectableHandleType()) && !arrayType.getPointeeType()->isArrayType())
		{
			return typeCheckOperatorArray(compiletimeContext, errorManager, errorContext);
		}

		if ((arrayType.isPointerType() || arrayType.isReflectableHandleType()) && arrayType.getPointeeType()->isArrayType())
		{
			arrayType = *arrayType.getPointeeType();
		}
		if (!arrayType.isContainerType() && !arrayType.isArrayType())
		{
			errorManager->compiledWithError(Tools::append(arrayType.toString(), " is not a list."), errorContext, compiletimeContext->getContextName(), getLexeme());
			return false;
		}
		else
		{
			return typeCheckIntrinsicArray(compiletimeContext, errorManager, errorContext);
		}
	}
	return false;
}


const CatGenericType& CatArrayIndex::getType() const
{
	return containerItemType;
}


bool CatArrayIndex::isConst() const
{
	return false;
}


CatTypedExpression* CatArrayIndex::constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	ASTHelper::updatePointerIfChanged(array, array->constCollapse(compileTimeContext, errorManager, errorContext));
	ASTHelper::updatePointerIfChanged(index, index->constCollapse(compileTimeContext, errorManager, errorContext));
	return this;
}


bool jitcat::AST::CatArrayIndex::isAssignable() const
{
	return arrayType.isArrayType();
}


const CatGenericType& jitcat::AST::CatArrayIndex::getAssignableType() const
{
	return assignableItemType;
}


std::any jitcat::AST::CatArrayIndex::executeAssignable(CatRuntimeContext* runtimeContext)
{
	std::any arrayValue = array->execute(runtimeContext);
	std::any indexValue = index->execute(runtimeContext);
	if (arrayType.isArrayType())
	{
		return static_cast<ArrayManipulator*>(arrayType.getObjectType())->getAssignableItemAt(arrayValue, std::any_cast<int>(indexValue));
	}
	else
	{
		assert(false);
	}
	return assignableItemType.createDefault();
}


CatTypedExpression* CatArrayIndex::getBase() const
{
	return array.get();
}


CatTypedExpression* CatArrayIndex::getIndex() const
{
	return index.get();
}


bool jitcat::AST::CatArrayIndex::isReflectedArray() const
{
	return isReflectedArrayType;
}


jitcat::Reflection::MemberFunctionInfo* jitcat::AST::CatArrayIndex::getArrayIndexOperatorFunction() const
{
	return arrayIndexFunction;
}


bool jitcat::AST::CatArrayIndex::typeCheckIntrinsicArray(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	CatGenericType keyType;
	if (arrayType.isArrayType())
	{
		keyType = static_cast<ArrayManipulator*>(arrayType.getObjectType())->getKeyType();
	}
	else 
	{
		keyType = arrayType.getContainerManipulator()->getKeyType();
	}
	if (indexType != keyType && !indexType.isIntType())
	{
		errorManager->compiledWithError(Tools::append("Key type ", indexType.toString(), " does not match key type of container. Expected a ", keyType.toString(), " or an int."), errorContext, compiletimeContext->getContextName(), getLexeme());
		return false;
	}
	if (arrayType.isArrayType())
	{
		containerItemType = static_cast<ArrayManipulator*>(arrayType.getObjectType())->getValueType();
		assignableItemType = static_cast<ArrayManipulator*>(arrayType.getObjectType())->getValueType().toPointer();
	}
	else
	{
		containerItemType = arrayType.getContainerItemType();
	}
	if (containerItemType.isReflectableObjectType())
	{
		containerItemType = containerItemType.toPointer(Reflection::TypeOwnershipSemantics::Weak, false, false);
	}
	return true;
}


bool jitcat::AST::CatArrayIndex::typeCheckOperatorArray(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	//Find an operator[] that matches the index type
	TypeInfo* objectType = arrayType.getPointeeType()->getObjectType();
	arrayIndexFunction = objectType->getMemberFunctionInfo(SearchFunctionSignature("[]", {indexType}));
	isReflectedArrayType = false;
	if (arrayIndexFunction == nullptr)
	{
		//An operator was not found for the index type, check if there is any other operator[].
		MemberFunctionInfo* firstArrayIndexFunction = objectType->getFirstMemberFunctionInfo("[]");
		if (firstArrayIndexFunction == nullptr)
		{
			//The object does not have an operator []
			errorManager->compiledWithError(Tools::append(arrayType.getPointeeType()->toString(), " does not implement operator []."), errorContext, compiletimeContext->getContextName(), getLexeme());
			return false;
		}
		else
		{
			//The object has at least one operator [] 
			std::stringstream errorMessage;
			auto& allMemberFunctions = objectType->getMemberFunctions();
			bool first = true;
			errorMessage << "Key type " << indexType.toString() << " does not match key type of container. Expected a ";
			for (auto& iter = allMemberFunctions.lower_bound("[]"); iter != allMemberFunctions.upper_bound("[]"); ++iter)
			{
				if (!first)
				{
					errorMessage << " or a ";
				}
				errorMessage << iter->second->getArgumentType(0).toString();
			}
			errorManager->compiledWithError(errorMessage.str(), errorContext, compiletimeContext->getContextName(), getLexeme());
			return false;
		}
	}
	else
	{
		//A matching operator[] was found
		assert(arrayIndexFunction->getNumberOfArguments() == 1);
		containerItemType = arrayIndexFunction->returnType;
		isReflectedArrayType = true;
		return true;
	}
}
