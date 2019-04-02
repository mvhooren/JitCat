/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatIfStatement.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatScopeBlock.h"
#include "jitcat/CatTypedExpression.h"
#include "jitcat/ExpressionErrorManager.h"

using namespace jitcat;
using namespace jitcat::AST;


CatIfStatement::CatIfStatement(CatTypedExpression* condition, CatScopeBlock* ifBody, const Tokenizer::Lexeme& lexeme, CatASTNode* elseNode):
	CatStatement(lexeme),
	condition(condition),
	ifBody(ifBody),
	elseNode(elseNode)
{

}


void CatIfStatement::print() const
{
	Tools::CatLog::log("if (");
	condition->print();
	Tools::CatLog::log(")\n");
	ifBody->print();
	if (elseNode != nullptr)
	{
		Tools::CatLog::log("else");
		if (elseNode->getNodeType() == CatASTNodeType::ScopeBlock)
		{
			Tools::CatLog::log("\n");
		}
		elseNode->print();
	}
}


CatASTNodeType CatIfStatement::getNodeType()
{
	return CatASTNodeType::IfStatement;
}


bool jitcat::AST::CatIfStatement::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	bool conditionOk = condition->typeCheck(compiletimeContext, errorManager, errorContext);
	bool ifBodyOk = ifBody->typeCheck(compiletimeContext, errorManager, errorContext);
	bool elseBodyOk = true;
	if (elseNode != nullptr)
	{
		elseBodyOk = static_cast<CatStatement*>(elseNode.get())->typeCheck(compiletimeContext, errorManager, errorContext);
	}
	bool conditionIsBool = condition->getType().isBoolType();
	if (!conditionIsBool)
	{
		errorManager->compiledWithError("Condition expression does not evaluate to a boolean.", errorContext, compiletimeContext->getContextName(), getLexeme());
	}
	return conditionOk && ifBodyOk && elseBodyOk && conditionIsBool;
}


std::any jitcat::AST::CatIfStatement::execute(CatRuntimeContext* runtimeContext)
{
	bool result = std::any_cast<bool>(condition->execute(runtimeContext));
	if (result)
	{
		return ifBody->execute(runtimeContext);
	}
	else if (elseNode != nullptr)
	{
		return static_cast<CatStatement*>(elseNode.get())->execute(runtimeContext);
	}
	return std::any();
}
