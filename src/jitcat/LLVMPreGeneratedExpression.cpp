/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/LLVMPreGeneratedExpression.h"

#include <cassert>


using namespace jitcat;
using namespace jitcat::LLVM;
using namespace jitcat::AST;


LLVMPreGeneratedExpression::LLVMPreGeneratedExpression(llvm::Value* value, CatGenericType type):
    CatTypedExpression(Tokenizer::Lexeme()),
    value(value),
    type(type)
{
}


CatASTNode* LLVMPreGeneratedExpression::copy() const
{
    return new LLVMPreGeneratedExpression(value, type);
}


const CatGenericType& LLVMPreGeneratedExpression::getType() const
{
    return type;
}


void LLVMPreGeneratedExpression::print() const
{
    assert(false);
}


bool LLVMPreGeneratedExpression::isConst() const
{
    return false;
}


CatTypedExpression* LLVMPreGeneratedExpression::constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
    return this;
}


CatASTNodeType LLVMPreGeneratedExpression::getNodeType() const
{
    return CatASTNodeType::LLVMPreGeneratedExpression;
}


std::any LLVMPreGeneratedExpression::execute(CatRuntimeContext* runtimeContext)
{
    assert(false);
    return std::any();
}


bool LLVMPreGeneratedExpression::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
    return true;
}


llvm::Value* LLVMPreGeneratedExpression::getValue() const
{
    return value;
}
