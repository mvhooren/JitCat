/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatArrayIndex.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/MemberInfo.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeInfo.h"
#include "jitcat/TypeOwnershipSemantics.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Tools;


CatArrayIndex::CatArrayIndex(CatTypedExpression* base, CatTypedExpression* arrayIndex, const Tokenizer::Lexeme& lexeme):
	CatTypedExpression(lexeme),
	array(base),
	index(arrayIndex),
	arrayType(CatGenericType::unknownType),
	indexType(CatGenericType::unknownType),
	containerItemType(CatGenericType::unknownType)
{

}


jitcat::AST::CatArrayIndex::CatArrayIndex(const CatArrayIndex& other):
	CatTypedExpression(other),
	array(static_cast<CatTypedExpression*>(other.array->copy())),
	index(static_cast<CatTypedExpression*>(other.index->copy())),
	arrayType(CatGenericType::unknownType),
	indexType(CatGenericType::unknownType),
	containerItemType(CatGenericType::unknownType)
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
	std::any indexValue = index->execute(runtimeContext);
	if (arrayType.isVectorType() || arrayType.isArrayType())
	{
		return arrayType.getContainerManipulator()->getItemAt(arrayValue, std::any_cast<int>(indexValue));
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
	return containerItemType.createDefault();
}


bool CatArrayIndex::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (array->typeCheck(compiletimeContext, errorManager, errorContext)
		&& index->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		arrayType = array->getType();
		if (arrayType.isPointerType() && arrayType.getPointeeType()->isArrayType())
		{
			arrayType = *arrayType.getPointeeType();
		}
		if (!arrayType.isContainerType())
		{
			errorManager->compiledWithError(Tools::append(arrayType.toString(), " is not a list."), errorContext, compiletimeContext->getContextName(), getLexeme());
			return false;
		}
		CatGenericType keyType = arrayType.getContainerManipulator()->getKeyType();
		indexType = index->getType();
		if (indexType != keyType && !indexType.isIntType())
		{
			errorManager->compiledWithError(Tools::append("Key type ", indexType.toString(), " does not match key type of container. Expected a ", keyType.toString(), " or an int."), errorContext, compiletimeContext->getContextName(), getLexeme());
			return false;
		}
		
		containerItemType = arrayType.getContainerItemType();
		if (containerItemType.isReflectableObjectType())
		{
			containerItemType = containerItemType.toPointer(Reflection::TypeOwnershipSemantics::Weak, false, false);
		}
		return true;
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


CatTypedExpression* CatArrayIndex::constCollapse(CatRuntimeContext* compileTimeContext)
{
	ASTHelper::updatePointerIfChanged(array, array->constCollapse(compileTimeContext));
	ASTHelper::updatePointerIfChanged(index, index->constCollapse(compileTimeContext));
	return this;
}


CatTypedExpression* CatArrayIndex::getBase() const
{
	return array.get();
}


CatTypedExpression* CatArrayIndex::getIndex() const
{
	return index.get();
}
