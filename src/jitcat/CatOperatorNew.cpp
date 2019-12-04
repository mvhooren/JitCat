/*
  This file is part of the JitCat library.

  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/


#include "jitcat/CatOperatorNew.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatLiteral.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatMemberFunctionCall.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeInfo.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


CatOperatorNew::CatOperatorNew(CatTypeNode* type, CatArgumentList* arguments, const Tokenizer::Lexeme& lexeme):
	CatTypedExpression(lexeme),
	type(type),
	arguments(arguments),
	newType(CatGenericType::unknownType)
{
}


jitcat::AST::CatOperatorNew::CatOperatorNew(const CatOperatorNew& other):
	CatTypedExpression(other),
	type(static_cast<CatTypeNode*>(other.type->copy())),
	arguments(static_cast<CatArgumentList*>(other.arguments->copy())),
	newType(CatGenericType::unknownType)
{
}


CatASTNode* jitcat::AST::CatOperatorNew::copy() const
{
	return new CatOperatorNew(*this);
}


void CatOperatorNew::print() const
{
	CatLog::log("new ");
	type->print();
	arguments->print();
}


CatASTNodeType CatOperatorNew::getNodeType() const
{
	return CatASTNodeType::OperatorNew;
}


std::any CatOperatorNew::execute(CatRuntimeContext* runtimeContext)
{
	std::any instance = newType.getPointeeType()->construct();
	if (functionCall != nullptr)
	{
		std::any result = functionCall->executeWithBase(runtimeContext, instance);
	}
	return instance;
}


bool CatOperatorNew::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	newType = CatGenericType::unknownType;
	if (!type->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		return false;
	}
	newType = type->getType().toPointer(TypeOwnershipSemantics::Value);

	if (!newType.isPointerToReflectableObjectType())
	{
		errorManager->compiledWithError(Tools::append("Operator new only supports object types, ", newType.toString(), " not yet supported."), errorContext, compiletimeContext->getContextName(), getLexeme());
		return false;
	}
	else if (!newType.getPointeeType()->getObjectType()->getAllowConstruction())
	{
		errorManager->compiledWithError(Tools::append("Construction of ", newType.toString(), " is not allowed."), errorContext, compiletimeContext->getContextName(), getLexeme());
		return false;
	}
	else if (!newType.getPointeeType()->getObjectType()->isCustomType())
	{
		functionCall = nullptr;
		return true;
	}
	else
	{
		std::string initName = "init";
		MemberFunctionInfo* typeConstructor = newType.getPointeeType()->getObjectType()->getFirstMemberFunctionInfo(initName);
		if (typeConstructor == nullptr)
		{
			//If there is no custom-defined init function, call the auto generated init function if it exists.
			initName = "__init";
		}
		
		functionCall = std::make_unique<CatMemberFunctionCall>(initName, type->getLexeme(), new CatLiteral(std::any(nullptr), newType, type->getLexeme()), static_cast<CatArgumentList*>(arguments->copy()), lexeme);

		if (!functionCall->typeCheck(compiletimeContext, errorManager, errorContext))
		{
			return false;
		}
	}
	return true;
}


const CatGenericType& CatOperatorNew::getType() const
{
	return newType;
}


bool CatOperatorNew::isConst() const
{
	return false;
}


CatTypedExpression* CatOperatorNew::constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	functionCall->constCollapse(compileTimeContext, errorManager, errorContext);
	return this;
}
