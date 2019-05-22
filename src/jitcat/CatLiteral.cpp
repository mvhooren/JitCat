/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatLiteral.h"
#include "jitcat/CatLog.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Tools;


jitcat::AST::CatLiteral::CatLiteral(const std::any& value, CatGenericType type, const Tokenizer::Lexeme& lexeme):
	CatTypedExpression(lexeme),
	value(value), 
	type(type)
{
}


jitcat::AST::CatLiteral::CatLiteral(const std::string& value, const Tokenizer::Lexeme& lexeme): 
	CatTypedExpression(lexeme), 
	value(value), 
	type(CatGenericType::stringType)
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
	if (type.isIntType())			CatLog::log(std::any_cast<int>(value));
	else if (type.isFloatType())	CatLog::log(std::any_cast<float>(value));
	else if (type.isBoolType())		CatLog::log(std::any_cast<bool>(value));
	else if (type.isStringType())	CatLog::log(std::any_cast<std::string>(value));
}


bool CatLiteral::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	return true;
}


const std::any& CatLiteral::getValue() const
{
	return value;
}
