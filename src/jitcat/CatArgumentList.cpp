/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatArgumentList.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatTypedExpression.h"

using namespace jitcat::AST;
using namespace jitcat::Tools;


void CatArgumentList::print() const
{
	CatLog::log("(");
	for (unsigned int i = 0; i < arguments.size(); i++)
	{
		if (i != 0)
		{
			CatLog::log(", ");
		}
		arguments[i]->print();
	}
	CatLog::log(")");
}


CatASTNodeType CatArgumentList::getNodeType()
{
	return CatASTNodeType::ParameterList;
}