/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatSourceFile.h"
#include "jitcat/CatClassDefinition.h"
#include "jitcat/CatDefinition.h"
#include "jitcat/CatFunctionDefinition.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatVariableDefinition.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/Reflectable.h"

#include <cassert>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Tools;


jitcat::AST::CatSourceFile::CatSourceFile(const std::string& name, std::vector<std::unique_ptr<CatDefinition>>&& definitions, const Tokenizer::Lexeme& lexeme):
	CatASTNode(lexeme),
	name(name),
	definitions(std::move(definitions)),
	staticScopeId(InvalidScopeID),
	scopeType(new Reflection::CustomTypeInfo(this->name.c_str())),
	scopeInstance(scopeType->construct(), scopeType.get())
{
	extractDefinitionLists();
}


jitcat::AST::CatSourceFile::CatSourceFile(const CatSourceFile& other):
	CatASTNode(other),
	name(other.name),
	staticScopeId(InvalidScopeID),
	scopeType(new Reflection::CustomTypeInfo(this->name.c_str())),
	scopeInstance(scopeType->construct(), scopeType.get())
{
	for (auto& iter : other.definitions)
	{
		definitions.emplace_back(static_cast<CatDefinition*>(iter->copy()));
	}
	extractDefinitionLists();
}


jitcat::AST::CatSourceFile::~CatSourceFile()
{
}


CatASTNode* jitcat::AST::CatSourceFile::copy() const
{
	return new CatSourceFile(*this);
}


void jitcat::AST::CatSourceFile::print() const
{
	for (auto& iter : definitions)
	{
		iter->print();
		CatLog::log("\n\n");
	}
}


CatASTNodeType jitcat::AST::CatSourceFile::getNodeType() const
{
	return CatASTNodeType::SourceFile;
}


const std::vector<CatClassDefinition*>& jitcat::AST::CatSourceFile::getClassDefinitions() const
{
	return classDefinitions;
}


const std::vector<CatFunctionDefinition*>& jitcat::AST::CatSourceFile::getFunctionDefinitions() const
{
	return functionDefinitions;
}


bool jitcat::AST::CatSourceFile::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	staticScopeId = compiletimeContext->addScope(scopeType.get(), scopeInstance.getReflectable(), true);
	CatScope* previousScope = compiletimeContext->getCurrentScope();
	compiletimeContext->setCurrentScope(this);
	bool noErrors = true;
	for (auto& iter: definitions)
	{
		noErrors &= iter->typeCheck(compiletimeContext);
	}
	compiletimeContext->removeScope(staticScopeId);
	compiletimeContext->setCurrentScope(previousScope);
	return noErrors;
}


CatScopeID jitcat::AST::CatSourceFile::getScopeId() const
{
	return staticScopeId;
}


Reflection::CustomTypeInfo* jitcat::AST::CatSourceFile::getCustomType()
{
	return scopeType.get();
}


void jitcat::AST::CatSourceFile::extractDefinitionLists()
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
