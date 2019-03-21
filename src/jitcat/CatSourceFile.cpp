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

#include <cassert>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Tools;


jitcat::AST::CatSourceFile::CatSourceFile(const std::string& name, std::vector<std::unique_ptr<CatDefinition>>&& definitions, const Tokenizer::Lexeme& lexeme):
	CatASTNode(lexeme),
	name(name),
	definitions(std::move(definitions))
{
	for (auto& iter : this->definitions)
	{
		switch (iter->getNodeType())
		{
			case CatASTNodeType::ClassDefinition:		classDefinitions.push_back(static_cast<CatClassDefinition*>(iter.get())); break;
			case CatASTNodeType::FunctionDefinition:	functionDefinitions.push_back(static_cast<CatFunctionDefinition*>(iter.get())); break;
			default:
				assert(false);

		}

	}
}


jitcat::AST::CatSourceFile::~CatSourceFile()
{
}


void jitcat::AST::CatSourceFile::print() const
{
	for (auto& iter : definitions)
	{
		iter->print();
		CatLog::log("\n\n");
	}
}


CatASTNodeType jitcat::AST::CatSourceFile::getNodeType()
{
	return CatASTNodeType::SourceFile;
}


const std::vector<const CatClassDefinition*>& jitcat::AST::CatSourceFile::getClassDefinitions() const
{
	return classDefinitions;
}


const std::vector<const CatFunctionDefinition*>& jitcat::AST::CatSourceFile::getFunctionDefinitions() const
{
	return functionDefinitions;
}


bool jitcat::AST::CatSourceFile::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	bool anyErrors = false;
	for (auto& iter: definitions)
	{
		anyErrors |= iter->typeCheck(compiletimeContext);
	}
	return !anyErrors;
}
