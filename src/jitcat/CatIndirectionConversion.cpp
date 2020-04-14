/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ASTHelper.h"
#include "jitcat/CatIndirectionConversion.h"

using namespace jitcat;
using namespace jitcat::AST;


CatIndirectionConversion::CatIndirectionConversion(const Tokenizer::Lexeme& lexeme, const CatGenericType& outType, 
                                                   IndirectionConversionMode conversionMode, std::unique_ptr<CatTypedExpression> expressionToConvert_): 
    CatTypedExpression(lexeme),
    expressionToConvert(std::move(expressionToConvert_)),
    conversionMode(conversionMode),
    convertedType(outType),
    typeWithoutIndirection(outType.removeIndirection())
{
}


CatIndirectionConversion::CatIndirectionConversion(const CatIndirectionConversion& other): 
    CatTypedExpression(other),
    expressionToConvert(static_cast<CatTypedExpression*>(other.expressionToConvert->copy())),
    conversionMode(other.conversionMode),
    convertedType(other.convertedType),
    typeWithoutIndirection(other.typeWithoutIndirection)
{
}


const CatGenericType& CatIndirectionConversion::getType() const
{
    return convertedType;
}


bool CatIndirectionConversion::isConst() const
{
    return expressionToConvert->isConst();
}


bool CatIndirectionConversion::isAssignable() const
{
    return convertedType.isAssignableType();
}


CatTypedExpression* CatIndirectionConversion::constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
    ASTHelper::updatePointerIfChanged(expressionToConvert, expressionToConvert->constCollapse(compileTimeContext, errorManager, errorContext));
    return this;
}


bool CatIndirectionConversion::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
    return expressionToConvert->typeCheck(compiletimeContext, errorManager, errorContext);
}


std::any CatIndirectionConversion::execute(jitcat::CatRuntimeContext* runtimeContext)
{
    return typeWithoutIndirection.doIndirectionConversion(expressionToConvert->execute(runtimeContext), conversionMode);
}


std::optional<bool> CatIndirectionConversion::checkControlFlow(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext, bool& unreachableCodeDetected) const
{
    return expressionToConvert->checkControlFlow(compiletimeContext, errorManager, errorContext, unreachableCodeDetected);
}


CatASTNode* jitcat::AST::CatIndirectionConversion::copy() const
{
    return new CatIndirectionConversion(*this);
}


void jitcat::AST::CatIndirectionConversion::print() const
{
    expressionToConvert->print();
}


CatASTNodeType jitcat::AST::CatIndirectionConversion::getNodeType() const
{
    return CatASTNodeType::IndirectionConversion;
}


IndirectionConversionMode jitcat::AST::CatIndirectionConversion::getIndirectionConversionMode() const
{
	return conversionMode;
}


const CatTypedExpression* jitcat::AST::CatIndirectionConversion::getExpressionToConvert() const
{
    return expressionToConvert.get();
}
