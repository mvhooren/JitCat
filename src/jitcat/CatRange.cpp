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
	isDefaultMin(true),
	rangeMin(std::make_unique<CatLiteral>(0, rangeMax->getLexeme())),
	rangeMax(rangeMax),
	isDefaultStep(true),
	rangeStep(std::make_unique<CatLiteral>(1, rangeMax->getLexeme()))
{
}

CatRange::CatRange(CatTypedExpression* rangeMin, CatTypedExpression* rangeMax, const Tokenizer::Lexeme& lexeme):
	CatASTNode(lexeme),
	isDefaultMin(false),
	rangeMin(rangeMin),
	rangeMax(rangeMax),
	isDefaultStep(true),
	rangeStep(std::make_unique<CatLiteral>(1, rangeMax->getLexeme()))
{
}


CatRange::CatRange(CatTypedExpression* rangeMin, CatTypedExpression* rangeMax, CatTypedExpression* rangeStep, const Tokenizer::Lexeme& lexeme):
	CatASTNode(lexeme),
	isDefaultMin(false),
	rangeMin(rangeMin),
	rangeMax(rangeMax),
	isDefaultStep(false),
	rangeStep(rangeStep)
{
}

CatRange::CatRange(const CatRange& other):
	CatASTNode(other),
	isDefaultMin(other.isDefaultMin),
	rangeMin(static_cast<CatTypedExpression*>(other.rangeMin->copy())),
	rangeMax(static_cast<CatTypedExpression*>(other.rangeMax->copy())),
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

	if (rangeStep->getType().isConst())
	{
		int step = std::any_cast<int>(rangeMin->execute(compiletimeContext));
		if (step == 0)
		{
			errorManager->compiledWithError(Tools::append("Infinite loop: step is always 0."), errorContext, compiletimeContext->getContextName(), rangeStep->getLexeme());
			return false;
		}
		if (rangeMin->getType().isConst()
			&& rangeMax->getType().isConst())
		{
			int min = std::any_cast<int>(rangeMin->execute(compiletimeContext));
			int max = std::any_cast<int>(rangeMin->execute(compiletimeContext));

			if (isDefaultStep && min > max)
			{
				rangeStep = std::make_unique<CatLiteral>(-1, rangeMax->getLexeme());
				rangeStep->typeCheck(compiletimeContext, errorManager, errorContext);
				step = -1;
			}

			if ((min < max && step < 0)
				|| (min > max && step > 0))
			{
				errorManager->compiledWithError(Tools::append("Infinite loop. Step goes in the wrong direction."), errorContext, compiletimeContext->getContextName(), rangeStep->getLexeme());
				return false;
			}
		}
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


CatASTNode* jitcat::AST::CatRange::constCollapse(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	ASTHelper::updatePointerIfChanged(rangeMin, rangeMin->constCollapse(compiletimeContext, errorManager, errorContext));
	ASTHelper::updatePointerIfChanged(rangeMax, rangeMax->constCollapse(compiletimeContext, errorManager, errorContext));
	ASTHelper::updatePointerIfChanged(rangeStep, rangeStep->constCollapse(compiletimeContext, errorManager, errorContext));
	return this;
}


const CatTypedExpression* jitcat::AST::CatRange::getRangeMin() const
{
	return rangeMin.get();
}


const CatTypedExpression* jitcat::AST::CatRange::getRangeMax() const
{
	return rangeMax.get();
}


const CatTypedExpression* jitcat::AST::CatRange::getRangeStep() const
{
	return rangeStep.get();
}
