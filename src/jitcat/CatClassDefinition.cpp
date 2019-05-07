/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatClassDefinition.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatFunctionDefinition.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/CatVariableDefinition.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/ExpressionErrorManager.h"
#include <cassert>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


jitcat::AST::CatClassDefinition::CatClassDefinition(const std::string& name, std::vector<std::unique_ptr<CatDefinition>>&& definitions, const Tokenizer::Lexeme& lexeme, const Tokenizer::Lexeme& nameLexeme):
	CatDefinition(lexeme),
	name(name),
	nameLexeme(nameLexeme),
	definitions(std::move(definitions)),
	scopeId(InvalidScopeID),
	customType(new CustomTypeInfo(name.c_str()))
{
	for (auto& iter : this->definitions)
	{
		switch (iter->getNodeType())
		{
			case CatASTNodeType::ClassDefinition:		classDefinitions.push_back(static_cast<CatClassDefinition*>(iter.get())); break;
			case CatASTNodeType::FunctionDefinition:	functionDefinitions.push_back(static_cast<CatFunctionDefinition*>(iter.get())); break;
			case CatASTNodeType::VariableDefinition:	variableDefinitions.push_back(static_cast<CatVariableDefinition*>(iter.get())); break;
			default:
				assert(false);
		}
	}
}


jitcat::AST::CatClassDefinition::~CatClassDefinition()
{
}


void jitcat::AST::CatClassDefinition::print() const
{
	if (definitions.size() == 0)
	{
		CatLog::log("class ", name, "{}");
	}
	else
	{
		CatLog::log("class ", name, "\n{");
		for (auto& iter : definitions)
		{
			iter->print();
			CatLog::log("\n\n");
		}
		CatLog::log("}\n\n");
	}
}


CatASTNodeType jitcat::AST::CatClassDefinition::getNodeType()
{
	return CatASTNodeType::ClassDefinition;
}


bool jitcat::AST::CatClassDefinition::typeCheck(CatRuntimeContext* compileTimeContext)
{
	bool noErrors = true;

	for (auto& iter: classDefinitions)
	{
		noErrors &= iter->typeCheck(compileTimeContext);
	}

	scopeId = compileTimeContext->addCustomTypeScope(customType.get());
	CatScope* previousScope = compileTimeContext->getCurrentScope();
	compileTimeContext->setCurrentScope(this);

	for (auto& iter: variableDefinitions)
	{
		bool valid = iter->typeCheck(compileTimeContext);
		noErrors &= valid;
	}

	for (auto& iter: functionDefinitions)
	{
		bool valid = iter->typeCheck(compileTimeContext);
		noErrors &= valid;
	}

	compileTimeContext->removeScope(scopeId);
	compileTimeContext->setCurrentScope(previousScope);
	if (!compileTimeContext->getCurrentScope()->getCustomType()->addType(customType.get()))
	{
		compileTimeContext->getErrorManager()->compiledWithError(Tools::append("A type with name ", name, " already exists."), this, compileTimeContext->getContextName(), nameLexeme);
		noErrors = false;
	}
	if (noErrors)
	{
		compileTimeContext->getErrorManager()->compiledWithoutErrors(this);
	}
	return noErrors;
}


bool jitcat::AST::CatClassDefinition::isTriviallyCopyable() const
{
	for (const auto& iter : variableDefinitions)
	{
		if (!iter->getType().getType().isTriviallyCopyable())
		{
			return false;
		}
	}
	return true;
}


Reflection::CustomTypeInfo* jitcat::AST::CatClassDefinition::getCustomType()
{
	return customType.get();
}


CatScopeID jitcat::AST::CatClassDefinition::getScopeId() const
{
	return scopeId;
}
