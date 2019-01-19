/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CatLiteral.h"
#include "CatLog.h"


void CatLiteral::print() const
{
	switch(type.getCatType())
	{
		case CatType::Int:		CatLog::log(std::any_cast<int>(value)); return;
		case CatType::Float:	CatLog::log(std::any_cast<float>(value)); return;
		case CatType::Bool:		CatLog::log(std::any_cast<bool>(value)); return;
		case CatType::String:	CatLog::log(std::any_cast<std::string>(value)); return;
	}
}


CatGenericType CatLiteral::typeCheck()
{
	return type;
}


const std::any& CatLiteral::getValue() const
{
	return value;
}
