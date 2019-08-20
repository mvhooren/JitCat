/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/
#include "jitcat/CatStaticFunctionCall.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/Tools.h"


using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;


CatStaticFunctionCall::CatStaticFunctionCall(CatTypeNode* parentType, const std::string& name, CatArgumentList* arguments, const Tokenizer::Lexeme& lexeme, const Tokenizer::Lexeme& nameLexeme):
	CatTypedExpression(lexeme),
	staticFunctionInfo(nullptr),
	parentType(parentType),
	name(name),
	nameLexeme(nameLexeme),
	arguments(arguments),
	returnType(CatGenericType::unknownType)
{
}


CatStaticFunctionCall::CatStaticFunctionCall(const CatStaticFunctionCall& other):
	CatTypedExpression(other),
	staticFunctionInfo(nullptr),
	name(other.name),
	nameLexeme(other.nameLexeme),
	parentType(static_cast<CatTypeNode*>(other.parentType->copy())),
	arguments(static_cast<CatArgumentList*>(other.arguments->copy())),
	returnType(CatGenericType::unknownType)
{
}


CatASTNode* CatStaticFunctionCall::copy() const
{
	return new CatStaticFunctionCall(*this);
}


void CatStaticFunctionCall::print() const
{
	parentType->print();
	arguments->print();
}


CatASTNodeType CatStaticFunctionCall::getNodeType() const
{
	return CatASTNodeType::StaticFunctionCall;
}


std::any CatStaticFunctionCall::execute(CatRuntimeContext* runtimeContext)
{
	return std::any();
}


bool CatStaticFunctionCall::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (!parentType->typeCheck(compiletimeContext, errorManager, errorContext)
		|| !arguments->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		return false;
	}

	return true;
}


const CatGenericType& CatStaticFunctionCall::getType() const
{
	return returnType;
}


bool CatStaticFunctionCall::isConst() const
{
	return false;
}


CatTypedExpression* CatStaticFunctionCall::constCollapse(CatRuntimeContext* compileTimeContext)
{
	arguments->constCollapse(compileTimeContext);
	return this;
}
