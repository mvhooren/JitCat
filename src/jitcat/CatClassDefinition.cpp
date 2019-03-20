/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatClassDefinition.h"
#include "jitcat/CatLog.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Tools;


jitcat::AST::CatClassDefinition::CatClassDefinition(const std::string& name):
	name(name)
{
}


jitcat::AST::CatClassDefinition::~CatClassDefinition()
{
}


void jitcat::AST::CatClassDefinition::print() const
{
	CatLog::log("class ", name, "{}");
}


CatASTNodeType jitcat::AST::CatClassDefinition::getNodeType()
{
	return CatASTNodeType::ClassDefinition;
}


bool jitcat::AST::CatClassDefinition::typeCheck(CatRuntimeContext* compileTimeContext)
{
	return false;
}
