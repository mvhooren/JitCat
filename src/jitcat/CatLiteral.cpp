/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatLiteral.h"
#include "jitcat/CatLog.h"
#include "jitcat/StringConstantPool.h"
#include "jitcat/TypeTraits.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Tools;


jitcat::AST::CatLiteral::CatLiteral(const std::any& value, CatGenericType type, const Tokenizer::Lexeme& lexeme):
	CatTypedExpression(lexeme),
	value(value), 
	type(type)
{
}


jitcat::AST::CatLiteral::CatLiteral(const Configuration::CatString& value, const Tokenizer::Lexeme& lexeme): 
	CatTypedExpression(lexeme), 
	value(TypeTraits<const Configuration::CatString*>::getCatValue(StringConstantPool::getString(value))), 
	type(CatGenericType::stringConstantValuePtrType)
{
}


jitcat::AST::CatLiteral::CatLiteral(float floatValue, const Tokenizer::Lexeme& lexeme):
	CatTypedExpression(lexeme), 
	value(floatValue), 
	type(CatGenericType::floatType)
{
}


jitcat::AST::CatLiteral::CatLiteral(int intValue, const Tokenizer::Lexeme& lexeme):
	CatTypedExpression(lexeme), 
	value(intValue), 
	type(CatGenericType::intType)
{
}


jitcat::AST::CatLiteral::CatLiteral(bool boolValue, const Tokenizer::Lexeme& lexeme): 
	CatTypedExpression(lexeme), 
	value(boolValue), 
	type(CatGenericType::boolType)
{
}


jitcat::AST::CatLiteral::CatLiteral(const CatLiteral& other):
	CatTypedExpression(other),
	value(other.value),
	type(other.type)
{
}


CatASTNode* jitcat::AST::CatLiteral::copy() const
{
	return new CatLiteral(*this);
}


void CatLiteral::print() const
{
	CatLog::log(CatGenericType::convertToString(value, type));
}


bool CatLiteral::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	return true;
}


const std::any& CatLiteral::getValue() const
{
	return value;
}
