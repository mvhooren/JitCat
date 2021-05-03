/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatSourceFile.h"
#include "jitcat/CatClassDefinition.h"
#include "jitcat/CatDefinition.h"
#include "jitcat/CatFunctionDefinition.h"
#include "jitcat/CatLib.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatVariableDefinition.h"
#include "jitcat/Configuration.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/Reflectable.h"
#ifdef ENABLE_LLVM
	#include "jitcat/LLVMCodeGenerator.h"
	#include "jitcat/LLVMCompileTimeContext.h"
	#include "jitcat/LLVMJit.h"
#endif

#include <cassert>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


CatSourceFile::CatSourceFile(const std::string& name, std::vector<std::unique_ptr<CatDefinition>>&& definitions, const Tokenizer::Lexeme& lexeme):
	CatASTNode(lexeme),
	name(name),
	definitions(std::move(definitions)),
	staticScopeId(InvalidScopeID),
	scopeType(makeTypeInfo<Reflection::CustomTypeInfo>(this->name.c_str())),
	scopeInstance(scopeType->construct(), scopeType.get())
{
	extractDefinitionLists();
}


CatSourceFile::CatSourceFile(const CatSourceFile& other):
	CatASTNode(other),
	name(other.name),
	staticScopeId(InvalidScopeID),
	scopeType(makeTypeInfo<Reflection::CustomTypeInfo>(this->name.c_str())),
	scopeInstance(scopeType->construct(), scopeType.get())
{
	for (auto& iter : other.definitions)
	{
		definitions.emplace_back(static_cast<CatDefinition*>(iter->copy()));
	}
	extractDefinitionLists();
}


CatSourceFile::~CatSourceFile()
{
}


CatASTNode* CatSourceFile::copy() const
{
	return new CatSourceFile(*this);
}


void CatSourceFile::print() const
{
	for (auto& iter : definitions)
	{
		iter->print();
		CatLog::log("\n\n");
	}
}


CatASTNodeType CatSourceFile::getNodeType() const
{
	return CatASTNodeType::SourceFile;
}


const std::vector<CatClassDefinition*>& CatSourceFile::getClassDefinitions() const
{
	return classDefinitions;
}


const std::vector<CatFunctionDefinition*>& CatSourceFile::getFunctionDefinitions() const
{
	return functionDefinitions;
}


bool CatSourceFile::compile(CatLib& catLib)
{
	CatRuntimeContext* compiletimeContext = catLib.getRuntimeContext();
	staticScopeId = compiletimeContext->addStaticScope(scopeType.get(), scopeInstance.getObject(), name);
	CatScope* previousScope = compiletimeContext->getCurrentScope();
	compiletimeContext->setCurrentScope(this);
	bool noErrors = true;
	std::vector<const CatASTNode*> loopDetectionStack;
	for (auto& iter: definitions)
	{
		noErrors &= iter->typeGatheringCheck(compiletimeContext);
	}
	if (noErrors)
	{
		for (auto& iter: definitions)
		{
			if (iter->getNodeType() == CatASTNodeType::ClassDefinition)
			{
				noErrors &= iter->defineCheck(static_cast<CatClassDefinition*>(iter.get())->getCompiletimeContext() , loopDetectionStack);
			}
			else
			{
				noErrors &= iter->defineCheck(compiletimeContext, loopDetectionStack);
			}
			assert(loopDetectionStack.empty());
		}
	}
	if (noErrors)
	{
		for (auto& iter: definitions)
		{
			if (iter->getNodeType() == CatASTNodeType::ClassDefinition)
			{
				noErrors &= iter->typeCheck(static_cast<CatClassDefinition*>(iter.get())->getCompiletimeContext());
			}
			else
			{
				noErrors &= iter->typeCheck(compiletimeContext);
			}
		}
	}
	compiletimeContext->removeScope(staticScopeId);
	compiletimeContext->setCurrentScope(previousScope);

#ifdef ENABLE_LLVM
	if (noErrors)
	{
		LLVM::LLVMCompileTimeContext llvmContext(compiletimeContext, LLVM::LLVMJit::get().getJitTargetConfig(), false);
		llvmContext.options.enableDereferenceNullChecks = true;
		llvmContext.currentLib = &catLib;
		compiletimeContext->getCodeGenerator()->generate(this, &llvmContext);
		if (compiletimeContext->getPrecompilationContext() != nullptr)
		{
			compiletimeContext->getPrecompilationContext()->precompileSourceFile(this, &catLib, compiletimeContext);
		}

	}
#endif

	return noErrors;
}


CatScopeID CatSourceFile::getScopeId() const
{
	return staticScopeId;
}


Reflection::CustomTypeInfo* CatSourceFile::getCustomType() const
{
	return scopeType.get();
}


unsigned char* jitcat::AST::CatSourceFile::getScopeObjectInstance() const
{
	return scopeInstance.getObject();
}


const std::string& jitcat::AST::CatSourceFile::getFileName() const
{
	return name;
}


void CatSourceFile::extractDefinitionLists()
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
