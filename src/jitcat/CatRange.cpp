/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatRange.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/CatLiteral.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/Tools.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Tools;


CatRange::CatRange(CatTypedExpression* rangeMax, const Tokenizer::Lexeme& lexeme):
	CatASTNode(lexeme),
	rangeMax(rangeMax),
	isDefaultMin(true),
	rangeMin(new CatLiteral(0, rangeMax->getLexeme())),
	isDefaultStep(true),
	rangeStep(new CatLiteral(1, rangeMax->getLexeme()))
{
}

CatRange::CatRange(CatTypedExpression* rangeMin, CatTypedExpression* rangeMax, const Tokenizer::Lexeme& lexeme):
	CatASTNode(lexeme),
	rangeMax(rangeMax),
	isDefaultMin(false),
	rangeMin(rangeMin),
	isDefaultStep(true),
	rangeStep(new CatLiteral(1, rangeMax->getLexeme()))
{
}


CatRange::CatRange(CatTypedExpression* rangeMin, CatTypedExpression* rangeMax, CatTypedExpression* rangeStep, const Tokenizer::Lexeme& lexeme):
	CatASTNode(lexeme),
	rangeMax(rangeMax),
	isDefaultMin(false),
	rangeMin(rangeMin),
	isDefaultStep(false),
	rangeStep(rangeStep)
{
}

CatRange::CatRange(const CatRange& other):
	CatASTNode(other),
	rangeMax(static_cast<CatTypedExpression*>(other.rangeMax->copy())),
	isDefaultMin(other.isDefaultMin),
	rangeMin(static_cast<CatTypedExpression*>(other.rangeMin->copy())),
	isDefaultStep(other.isDefaultStep),
	rangeStep(static_cast<CatTypedExpression*>(other.rangeStep->copy()))
{
}

CatRange::~CatRange()
{
}


CatASTNode* CatRange::copy() const
{
	return nullptr;
}


void CatRange::print() const
{
	CatLog::log("range (");
	if (!isDefaultMin)
	{
		rangeMin->print();
		CatLog::log(", ");
	}
	rangeMax->print();
	if (!isDefaultStep)
	{
		CatLog::log(", ");
		rangeStep->print();
	}
	CatLog::log(")");
}


CatASTNodeType CatRange::getNodeType() const
{
	return CatASTNodeType::Range;
}


bool CatRange::begin(CatRangeIterator& iterator, CatRuntimeContext* runtimeContext)
{
	iterator.currentValue =  0;
	if (!isDefaultMin)
	{
		iterator.currentValue = std::any_cast<int>(rangeMin->execute(runtimeContext));
	}
	int currentMax = std::any_cast<int>(rangeMax->execute(runtimeContext));
	return iterator.currentValue < currentMax;
}


bool CatRange::next(CatRangeIterator& iterator, CatRuntimeContext* runtimeContext)
{
	if (!isDefaultStep)
	{
		iterator.currentValue += std::any_cast<int>(rangeStep->execute(runtimeContext));
	}
	else
	{
		iterator.currentValue++;
	}
	int currentMax = std::any_cast<int>(rangeMax->execute(runtimeContext));
	return iterator.currentValue < currentMax;
}


bool CatRange::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (!rangeMax->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		return false;
	}
	if (!rangeMin->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		return false;
	}
	if (!rangeStep->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		return false;
	}

	if (!rangeMax->getType().isIntType())
	{
		errorManager->compiledWithError(Tools::append("Range max expression should be of type int but is of type ", rangeMax->getType().toString(), "."), errorContext, compiletimeContext->getContextName(), rangeMax->getLexeme());
		return false;
	}
	if (!rangeMin->getType().isIntType())
	{
		errorManager->compiledWithError(Tools::append("Range min expression should be of type int but is of type ", rangeMin->getType().toString(), "."), errorContext, compiletimeContext->getContextName(), rangeMin->getLexeme());
		return false;
	}
	if (!rangeStep->getType().isIntType())
	{
		errorManager->compiledWithError(Tools::append("Range step expression should be of type int but is of type ", rangeStep->getType().toString(), "."), errorContext, compiletimeContext->getContextName(), rangeStep->getLexeme());
		return false;
	}

	return true;
}


bool jitcat::AST::CatRange::hasAlwaysAtLeastOneIteration(CatRuntimeContext* compiletimeContext)
{
	if (rangeMax->isConst() 
		&& rangeMin->isConst())
	{
		return std::any_cast<int>(rangeMin->execute(compiletimeContext)) < std::any_cast<int>(rangeMax->execute(compiletimeContext));
	}
	else
	{
		return false;
	}
}


CatASTNode* jitcat::AST::CatRange::constCollapse(CatRuntimeContext* compiletimeContext)
{
	ASTHelper::updatePointerIfChanged(rangeMin, rangeMin->constCollapse(compiletimeContext));
	ASTHelper::updatePointerIfChanged(rangeMax, rangeMax->constCollapse(compiletimeContext));
	ASTHelper::updatePointerIfChanged(rangeStep, rangeStep->constCollapse(compiletimeContext));
	return this;
}
