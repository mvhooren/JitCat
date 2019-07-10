/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatForLoop.h"
#include "jitcat/CatRange.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatScopeBlock.h"
#include "jitcat/CatLog.h"
#include "jitcat/Configuration.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/Tools.h"
#include "jitcat/CustomTypeMemberInfo.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


CatForLoop::CatForLoop(const Tokenizer::Lexeme& lexeme, const Tokenizer::Lexeme& iteratorLexeme, CatRange* range, CatScopeBlock* loopBody):
	CatStatement(lexeme),
	iteratorLexeme(iteratorLexeme),
	iteratorName(iteratorLexeme),
	range(range),
	loopBody(loopBody),
	loopIteratorScope(InvalidScopeID),
	scopeType(new CustomTypeInfo("loopIterator", false)),
	iteratorMember(nullptr)
{
}


CatForLoop::CatForLoop(const CatForLoop& other):
	CatStatement(other),
	iteratorLexeme(other.iteratorLexeme),
	iteratorName(other.iteratorName),
	range(static_cast<CatRange*>(other.range->copy())),
	loopBody(static_cast<CatScopeBlock*>(other.loopBody->copy()))
{
}


CatForLoop::~CatForLoop()
{
	CustomTypeInfo::destroy(scopeType);
}


CatASTNode* CatForLoop::copy() const
{
	return new CatForLoop(*this);
}


void CatForLoop::print() const
{
	CatLog::log("for ", iteratorName, " in ");
	range->print();
	CatLog::log("\n");
	loopBody->print();
}


CatASTNodeType CatForLoop::getNodeType() const
{
	return CatASTNodeType::ForLoop;
}


bool CatForLoop::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (!range->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		return false;
	}
	CatScopeID scopeId = InvalidScopeID;
	if (compiletimeContext->findVariable(Tools::toLowerCase(iteratorName), scopeId) != nullptr)
	{
		errorManager->compiledWithError(Tools::append("A variable with name \"", iteratorName, "\" already exists."), errorContext, compiletimeContext->getContextName(), iteratorLexeme);
		return false;
	}
	iteratorMember = scopeType->addIntMember(iteratorName, 0, true, false);

	loopIteratorScope = compiletimeContext->addScope(scopeType, nullptr, false);
	CatScope* previousScope = compiletimeContext->getCurrentScope();
	compiletimeContext->setCurrentScope(this);

	if (!loopBody->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		return false;
	}

	compiletimeContext->removeScope(loopIteratorScope);
	compiletimeContext->setCurrentScope(previousScope);

	return true;
}


std::any CatForLoop::execute(jitcat::CatRuntimeContext* runtimeContext)
{
	CatRange::CatRangeIterator iterator;
	unsigned char* scopeMem = reinterpret_cast<unsigned char*>(&iterator.currentValue);
	if constexpr (Configuration::logJitCatObjectConstructionEvents)
	{
		if (scopeType->getTypeSize() > 0)
		{
			std::cout << "(CatForLoop::execute) Stack-allocated buffer of size " << std::dec << scopeType->getTypeSize() << ": " << std::hex << reinterpret_cast<uintptr_t>(scopeMem) << "\n";
		}
	}
	scopeType->placementConstruct(scopeMem, scopeType->getTypeSize());
	loopIteratorScope = runtimeContext->addScope(scopeType, reinterpret_cast<Reflectable*>(scopeMem), false);
	CatScope* previousScope = runtimeContext->getCurrentScope();
	runtimeContext->setCurrentScope(this);

	if (range->begin(iterator, runtimeContext))
	{
		do
		{
			loopBody->execute(runtimeContext);
		} while (range->next(iterator, runtimeContext));
	}

	runtimeContext->removeScope(loopIteratorScope);
	runtimeContext->setCurrentScope(previousScope);
	scopeType->placementDestruct(scopeMem, scopeType->getTypeSize());
	return std::any();
}


std::optional<bool> CatForLoop::checkControlFlow(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext, bool& unreachableCodeDetected) const
{
	auto returns = loopBody->checkControlFlow(compiletimeContext, errorManager, errorContext, unreachableCodeDetected);
	return returns.has_value() && *returns && range->hasAlwaysAtLeastOneIteration(compiletimeContext);
}


CatScopeID CatForLoop::getScopeId() const
{
	return loopIteratorScope;
}


Reflection::CustomTypeInfo* CatForLoop::getCustomType()
{
	return scopeType;
}
