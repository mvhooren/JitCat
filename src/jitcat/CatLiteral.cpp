/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CatLiteral.h"
#include "CatLog.h"


void CatLiteral::print() const
{
	if (type.isIntType())			CatLog::log(std::any_cast<int>(value));
	else if (type.isFloatType())	CatLog::log(std::any_cast<float>(value));
	else if (type.isBoolType())		CatLog::log(std::any_cast<bool>(value));
	else if (type.isStringType())	CatLog::log(std::any_cast<std::string>(value));
}


CatGenericType CatLiteral::typeCheck()
{
	return type;
}


const std::any& CatLiteral::getValue() const
{
	return value;
}
