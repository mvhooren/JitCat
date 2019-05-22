/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatArgumentList.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatTypedExpression.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Tools;


jitcat::AST::CatArgumentList::CatArgumentList(const Tokenizer::Lexeme& lexeme, std::vector<CatTypedExpression*>& argumentList): 
	CatASTNode(lexeme)
{
	for (auto& iter : argumentList)
	{
		arguments.emplace_back(iter);
	}
}


jitcat::AST::CatArgumentList::CatArgumentList(const CatArgumentList& other):
	CatASTNode(other)
{
	for (auto& iter : other.arguments)
	{
		arguments.emplace_back(static_cast<CatTypedExpression*>(iter->copy()));
	}
}


CatASTNode* jitcat::AST::CatArgumentList::copy() const
{
	return new CatArgumentList(*this);
}


void CatArgumentList::print() const
{
	CatLog::log("(");
	for (unsigned int i = 0; i < arguments.size(); i++)
	{
		if (i != 0)
		{
			CatLog::log(", ");
		}
		arguments[i]->print();
	}
	CatLog::log(")");
}


CatASTNodeType CatArgumentList::getNodeType() const
{
	return CatASTNodeType::ParameterList;
}


bool jitcat::AST::CatArgumentList::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	argumentTypes.clear();
	bool noErrors = true;
	for (auto& iter : arguments)
	{
		if (iter->typeCheck(compiletimeContext, errorManager, errorContext))
		{
			argumentTypes.push_back(iter->getType());
		}
		else
		{
			argumentTypes.push_back(CatGenericType::errorType);
			noErrors = false;
		}
	}
	return noErrors;
}


void jitcat::AST::CatArgumentList::constCollapse(CatRuntimeContext* compileTimeContext)
{
	for (auto& iter : arguments)
	{
		ASTHelper::updatePointerIfChanged(iter, iter->constCollapse(compileTimeContext));
	}
}


bool jitcat::AST::CatArgumentList::getAllArgumentsAreConst() const
{
	for (auto& iter : arguments)
	{
		if (!iter->isConst())
		{
			return false;
		}
	}
	return true;
}


bool jitcat::AST::CatArgumentList::getArgumentIsConst(std::size_t argumentIndex) const
{
	return arguments[argumentIndex]->isConst();
}


Tokenizer::Lexeme jitcat::AST::CatArgumentList::getArgumentLexeme(std::size_t argumentIndex) const
{
	return arguments[argumentIndex]->getLexeme();
}


CatTypedExpression* jitcat::AST::CatArgumentList::releaseArgument(std::size_t argumentIndex)
{
	return arguments[argumentIndex].release();
}


const CatTypedExpression* jitcat::AST::CatArgumentList::getArgument(std::size_t argumentIndex) const
{
	return arguments[argumentIndex].get();
}


void jitcat::AST::CatArgumentList::addArgument(std::unique_ptr<CatTypedExpression> argument)
{
	arguments.emplace_back(std::move(argument));
}


std::any jitcat::AST::CatArgumentList::executeArgument(std::size_t argumentIndex, CatRuntimeContext* context)
{
	return arguments[argumentIndex]->execute(context);
}


void jitcat::AST::CatArgumentList::executeAllArguments(std::vector<std::any>& values, const std::vector<CatGenericType>& expectedTypes, CatRuntimeContext* context)
{
	std::size_t index = 0;
	for (auto& iter : arguments)
	{
		std::any value = ASTHelper::doGetArgument(iter.get(), expectedTypes[index], context);
		values.push_back(value);
		index++;
	}
}


const CatGenericType& jitcat::AST::CatArgumentList::getArgumentType(std::size_t argumentIndex) const
{
	return argumentTypes[argumentIndex];
}


std::size_t jitcat::AST::CatArgumentList::getNumArguments() const
{
	return arguments.size();
}