/*
  This file is part of the JitCat library.

  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatOperatorNewArray.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatTypeNode.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Tools;


jitcat::AST::CatOperatorNewArray::CatOperatorNewArray(CatTypeNode* arrayItemType, const Tokenizer::Lexeme& lexeme):
	CatTypedExpression(lexeme),
	arrayType(new CatTypeNode(arrayItemType, Reflection::TypeOwnershipSemantics::Value, lexeme)),
	newType(CatGenericType::unknownType)
{
}


jitcat::AST::CatOperatorNewArray::CatOperatorNewArray(const CatOperatorNewArray& other):
	CatTypedExpression(other),
	arrayType(static_cast<CatTypeNode*>(other.arrayType->copy())),
	newType(other.newType)
{
}


CatASTNode* CatOperatorNewArray::copy() const
{
	return new CatOperatorNewArray(*this);
}


void CatOperatorNewArray::print() const
{
	CatLog::log("new ", newType.toString());
}


CatASTNodeType CatOperatorNewArray::getNodeType() const
{
	return CatASTNodeType::OperatorNewArray;
}


std::any CatOperatorNewArray::execute(CatRuntimeContext* runtimeContext)
{
	return newType.getPointeeType()->construct();
}


bool CatOperatorNewArray::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (!arrayType->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		return false;
	}
	newType = arrayType->getType();
	return true;
}


const CatGenericType& CatOperatorNewArray::getType() const
{
	return newType;
}


bool CatOperatorNewArray::isConst() const
{
	return false;
}


CatTypedExpression* CatOperatorNewArray::constCollapse(CatRuntimeContext* compileTimeContext)
{
	return this;
}
