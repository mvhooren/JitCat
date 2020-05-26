/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatScopeFunctionCall.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatMemberFunctionCall.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatStaticFunctionCall.h"
#include "jitcat/CatStaticScope.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/Tools.h"

#include <cassert>
#include <iostream>


using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


CatScopeFunctionCall::CatScopeFunctionCall(const std::string& name, CatArgumentList* arguments, const Tokenizer::Lexeme& lexeme, const Tokenizer::Lexeme& nameLexeme):
    CatTypedExpression(lexeme),
    name(name),
    nameLexeme(nameLexeme),
    lexeme(lexeme),
    arguments(arguments)
{
    nameLowerCase = Tools::toLowerCase(name);
}


CatScopeFunctionCall::CatScopeFunctionCall(const CatScopeFunctionCall& other):
    CatTypedExpression(other),
    name(other.name),
    nameLexeme(other.nameLexeme),
    lexeme(other.lexeme),
    arguments(static_cast<CatArgumentList*>(other.arguments->copy()))
{
    nameLowerCase = Tools::toLowerCase(name);
}


CatScopeFunctionCall::~CatScopeFunctionCall()
{
}


CatASTNode* CatScopeFunctionCall::copy() const
{
    return new CatScopeFunctionCall(*this);
}


void CatScopeFunctionCall::print() const
{
    CatLog::log(name);
	arguments->print();
}


CatASTNodeType CatScopeFunctionCall::getNodeType() const
{
    return CatASTNodeType::ScopeFunctionCall;
}


std::any CatScopeFunctionCall::execute(CatRuntimeContext* runtimeContext)
{
    return functionCall->execute(runtimeContext);
}


bool CatScopeFunctionCall::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
    if (functionCall != nullptr)
    {
        return functionCall->typeCheck(compiletimeContext, errorManager, errorContext);
    }
    if (!arguments->typeCheck(compiletimeContext, errorManager, errorContext))
    {
        return false;
    }
    CatScopeID scopeId = InvalidScopeID;
    if (MemberFunctionInfo* functionInfo = compiletimeContext->findMemberFunction(this, scopeId); functionInfo != nullptr)
    {
        //This is a member function
        functionCall = std::make_unique<CatMemberFunctionCall>(name, nameLexeme, nullptr, arguments.release(), lexeme);
    }
    else if (StaticFunctionInfo* staticFunctionInfo = compiletimeContext->findStaticFunction(this, scopeId); staticFunctionInfo != nullptr)
    {
        //This is a static function
        auto scope = std::make_unique<CatStaticScope>(true, nullptr, compiletimeContext->getScopeType(scopeId)->getTypeName(), nameLexeme, nameLexeme);
        functionCall = std::make_unique<CatStaticFunctionCall>(scope.release(), name, arguments.release(), lexeme, nameLexeme);
    }
    if (functionCall != nullptr)
    {
        return functionCall->typeCheck(compiletimeContext, errorManager, errorContext);
    }
    else
    {
        //No function found.
		errorManager->compiledWithError(Tools::append("Function not found: ", name, "."), errorContext, compiletimeContext->getContextName(), getLexeme());
		return false;
    }
}


const CatGenericType& CatScopeFunctionCall::getType() const
{
    return functionCall->getType();
}


bool CatScopeFunctionCall::isConst() const
{
    return functionCall->isConst();
}


CatStatement* CatScopeFunctionCall::constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
    ASTHelper::updatePointerIfChanged(functionCall, functionCall->constCollapse(compileTimeContext, errorManager, errorContext));
    return functionCall.release();
}


const std::string& CatScopeFunctionCall::getLowerCaseFunctionName() const
{
    return nameLowerCase;
}


int CatScopeFunctionCall::getNumParameters() const
{
    return (int)arguments->getNumArguments();
}


const CatGenericType& CatScopeFunctionCall::getParameterType(int index) const
{
    return arguments->getArgumentType((std::size_t)index);
}
