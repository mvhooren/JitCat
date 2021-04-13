/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatForLoop.h"
#include "jitcat/ASTHelper.h"
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
	loopIteratorScope(InvalidScopeID),
	scopeType(makeTypeInfo<CustomTypeInfo>("loopIterator", HandleTrackingMethod::None)),
	iteratorMember(nullptr),
	range(range),
	loopBody(loopBody)
{
}


CatForLoop::CatForLoop(const CatForLoop& other):
	CatStatement(other),
	iteratorLexeme(other.iteratorLexeme),
	iteratorName(other.iteratorName),
	loopIteratorScope(InvalidScopeID),
	scopeType(nullptr),
	iteratorMember(nullptr),
	range(static_cast<CatRange*>(other.range->copy())),
	loopBody(static_cast<CatScopeBlock*>(other.loopBody->copy()))
{
}


CatForLoop::~CatForLoop()
{
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

	loopIteratorScope = compiletimeContext->addScope(scopeType.get(), nullptr, false);
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

CatStatement* jitcat::AST::CatForLoop::constCollapse(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	range->constCollapse(compiletimeContext, errorManager, errorContext);

	CatScopeID iteratorScope = compiletimeContext->addScope(scopeType.get(), nullptr, false);
	assert(iteratorScope == loopIteratorScope);
	CatScope* previousScope = compiletimeContext->getCurrentScope();
	compiletimeContext->setCurrentScope(this);

	ASTHelper::updatePointerIfChanged(loopBody, loopBody->constCollapse(compiletimeContext, errorManager, errorContext));

	compiletimeContext->removeScope(iteratorScope);
	compiletimeContext->setCurrentScope(previousScope);

	return this;
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
	loopIteratorScope = runtimeContext->addScope(scopeType.get(), scopeMem, false);
	CatScope* previousScope = runtimeContext->getCurrentScope();
	runtimeContext->setCurrentScope(this);
	std::any returnValue;

	if (range->begin(iterator, runtimeContext))
	{
		do
		{
			std::any value = loopBody->execute(runtimeContext);
			if (runtimeContext->getIsReturning())
			{
				returnValue = value;
				break;
			}
		} while (range->next(iterator, runtimeContext));
	}

	runtimeContext->removeScope(loopIteratorScope);
	runtimeContext->setCurrentScope(previousScope);
	scopeType->placementDestruct(scopeMem, scopeType->getTypeSize());
	return returnValue;
}


std::optional<bool> CatForLoop::checkControlFlow(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext, bool& unreachableCodeDetected)
{
	auto returns = loopBody->checkControlFlow(compiletimeContext, errorManager, errorContext, unreachableCodeDetected);
	allControlPathsReturn = returns.has_value() && *returns && range->hasAlwaysAtLeastOneIteration(compiletimeContext);
	return allControlPathsReturn;
}


CatScopeID CatForLoop::getScopeId() const
{
	return loopIteratorScope;
}


Reflection::CustomTypeInfo* CatForLoop::getCustomType() const
{
	return scopeType.get();
}


const CatRange* jitcat::AST::CatForLoop::getRange() const
{
	return range.get();
}


const CatStatement* jitcat::AST::CatForLoop::getBody() const
{
	return loopBody.get();
}
