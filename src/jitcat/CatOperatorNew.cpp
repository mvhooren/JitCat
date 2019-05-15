/*
  This file is part of the JitCat library.

  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/


#include "jitcat/CatOperatorNew.h"
#include "jitcat/CatHostClasses.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/CustomTypeInstance.h"
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
	newType(CatGenericType::errorType),
	typeConstructor(nullptr)
{
}


void CatOperatorNew::print() const
{
	CatLog::log("new ", newType.toString());
	arguments->print();
}


CatASTNodeType CatOperatorNew::getNodeType()
{
	return CatASTNodeType::OperatorNew;
}


std::any CatOperatorNew::execute(CatRuntimeContext* runtimeContext)
{
	if (typeConstructor != nullptr)
	{
		CustomTypeInstance* instance = static_cast<CustomTypeInfo*>(newType.getObjectType())->createInstance();
		std::vector<std::any> argumentValues;
		for (std::unique_ptr<CatTypedExpression>& argument : arguments->arguments)
		{
			argumentValues.push_back(argument->execute(runtimeContext));
		}
		std::any instanceValue(static_cast<Reflectable*>(instance));
		typeConstructor->call(runtimeContext, instanceValue, argumentValues);
		return std::any((Reflectable*)instance);
	}
	else if (hostClass != nullptr)
	{
		return std::any(static_cast<Reflectable*>(hostClass->construct()));
	}
	return nullptr;
}


bool CatOperatorNew::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	newType = CatGenericType::errorType;
	if (!type->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		return false;
	}
	newType = type->getType();
	std::size_t numArgumentsSupplied = arguments->arguments.size();
	std::size_t expectedNrOfArguments = 0;
	if (!newType.isObjectType())
	{
		errorManager->compiledWithError(Tools::append("Operator new only supports object types, ", newType.toString(), " not yet supported."), errorContext, compiletimeContext->getContextName(), getLexeme());
		return false;
	}
	else if (!newType.getObjectType()->isCustomType())
	{
		hostClass = compiletimeContext->getHostClasses()->getHostClass(newType.getObjectType()->getTypeName());
		if (hostClass == nullptr || !hostClass->isConstructible())
		{
			errorManager->compiledWithError(Tools::append("Host type cannot be constructed: ", newType.toString(), ", provide a constructor and destructor through the CatHostClasses interface."), errorContext, compiletimeContext->getContextName(), getLexeme());
			return false;
		}
	}
	else
	{
		typeConstructor = newType.getObjectType()->getMemberFunctionInfo("init");
		if (typeConstructor == nullptr)
		{
			//If there is no custom-defined init function, call the auto generated init function if it exists.
			typeConstructor = newType.getObjectType()->getMemberFunctionInfo("__init");
		}
		if (typeConstructor != nullptr)
		{
			expectedNrOfArguments = typeConstructor->getNumberOfArguments();
		}

	}
	
	if (expectedNrOfArguments != numArgumentsSupplied)
	{
		errorManager->compiledWithError(Tools::append("Invalid number of arguments for init function of: ", newType.toString(), " expected ", expectedNrOfArguments, " arguments."), errorContext, compiletimeContext->getContextName(), getLexeme());
		return false;
	}

	for (unsigned int i = 0; i < numArgumentsSupplied; i++)
	{
		if (arguments->arguments[i]->typeCheck(compiletimeContext, errorManager, errorContext))
		{
			if (!(typeConstructor->getArgumentType(i) == arguments->arguments[i]->getType()))
			{
				errorManager->compiledWithError(Tools::append("Invalid argument for init function of: ", newType.toString(), " argument nr: ", i, " expected: ", typeConstructor->getArgumentType(i).toString()), errorContext, compiletimeContext->getContextName(), getLexeme());
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}


CatGenericType CatOperatorNew::getType() const
{
	return newType;
}


bool CatOperatorNew::isConst() const
{
	return false;
}


CatTypedExpression* CatOperatorNew::constCollapse(CatRuntimeContext* compileTimeContext)
{
	return this;
}
