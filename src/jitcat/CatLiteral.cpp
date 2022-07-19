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


CatLiteral::CatLiteral(const std::any& value, CatGenericType type, const Tokenizer::Lexeme& lexeme):
	CatTypedExpression(lexeme),
	type(type),
	value(value)
{
}


CatLiteral::CatLiteral(const Configuration::CatString& value, const Tokenizer::Lexeme& lexeme): 
	CatTypedExpression(lexeme), 
	type(CatGenericType::stringConstantValuePtrType),
	value(TypeTraits<const Configuration::CatString*>::getCatValue(StringConstantPool::getString(value)))
{
}


CatLiteral::CatLiteral(float floatValue, const Tokenizer::Lexeme& lexeme):
	CatTypedExpression(lexeme), 
	type(CatGenericType::floatType),
	value(floatValue)
{
}


CatLiteral::CatLiteral(std::array<float, 4> vectorValue, const Tokenizer::Lexeme& lexeme):
	CatTypedExpression(lexeme),
	type(CatGenericType::vector4fType),
	value(vectorValue)
{
}


CatLiteral::CatLiteral(double doubleValue, const Tokenizer::Lexeme& lexeme):
	CatTypedExpression(lexeme), 
	type(CatGenericType::doubleType),
	value(doubleValue)
{
}


jitcat::AST::CatLiteral::CatLiteral(int intValue, const Tokenizer::Lexeme& lexeme):
	CatTypedExpression(lexeme), 
	type(CatGenericType::intType),
	value(intValue)
{
}


jitcat::AST::CatLiteral::CatLiteral(bool boolValue, const Tokenizer::Lexeme& lexeme): 
	CatTypedExpression(lexeme), 
	type(CatGenericType::boolType),
	value(boolValue)
{
}


jitcat::AST::CatLiteral::CatLiteral(const CatLiteral& other):
	CatTypedExpression(other),
	type(other.type),
	value(other.value)
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
