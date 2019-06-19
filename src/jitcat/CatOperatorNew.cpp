/*
  This file is part of the JitCat library.

  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/


#include "jitcat/CatOperatorNew.h"
#include "jitcat/CatLiteral.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatMemberFunctionCall.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeInfo.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


CatOperatorNew::CatOperatorNew(CatMemberFunctionCall* functionCall, const std::string& typeName, const Tokenizer::Lexeme& lexeme):
	CatTypedExpression(lexeme),
	functionCall(functionCall),
	newType(CatGenericType::unknownType),
	typeName(typeName)
{
}


jitcat::AST::CatOperatorNew::CatOperatorNew(const CatOperatorNew& other):
	CatTypedExpression(other),
	functionCall(static_cast<CatMemberFunctionCall*>(other.functionCall->copy())),
	newType(CatGenericType::unknownType),
	typeName(other.typeName)
{
}


CatASTNode* jitcat::AST::CatOperatorNew::copy() const
{
	return new CatOperatorNew(*this);
}


void CatOperatorNew::print() const
{
	CatLog::log("new ", newType.toString());
	functionCall->getArguments()->print();
}


CatASTNodeType CatOperatorNew::getNodeType() const
{
	return CatASTNodeType::OperatorNew;
}


std::any CatOperatorNew::execute(CatRuntimeContext* runtimeContext)
{
	Reflectable* instance = static_cast<CustomTypeInfo*>(newType.getPointeeType()->getObjectType())->construct();
	if (functionCall != nullptr)
	{
		functionCall->executeWithBase(runtimeContext, instance);
	}
	return std::any((Reflectable*)instance);
}


bool CatOperatorNew::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	newType = CatGenericType::unknownType;
	type.reset(new CatTypeNode(typeName, TypeOwnershipSemantics::Value, functionCall->getNameLexeme()));

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

		return true;
	}
	else
	{
		MemberFunctionInfo* typeConstructor = newType.getPointeeType()->getObjectType()->getMemberFunctionInfo("init");
		if (typeConstructor == nullptr)
		{
			//If there is no custom-defined init function, call the auto generated init function if it exists.
			functionCall->setFunctionName("__init");
		}
		else
		{
			functionCall->setFunctionName("init");
		}
		functionCall->setBase(std::make_unique<CatLiteral>(std::any((Reflectable*)nullptr), newType, functionCall->getNameLexeme()));
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


CatTypedExpression* CatOperatorNew::constCollapse(CatRuntimeContext* compileTimeContext)
{
	functionCall->constCollapse(compileTimeContext);
	return this;
}
