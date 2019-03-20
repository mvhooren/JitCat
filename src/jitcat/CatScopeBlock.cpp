/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatScopeBlock.h"
#include "jitcat/CatLog.h"


using namespace jitcat;
using namespace jitcat::AST;


CatScopeBlock::CatScopeBlock(const std::vector<CatStatement*>& statementList)
{
	for (auto& iter : statementList)
	{
		statements.emplace_back(iter);
	}
}


CatScopeBlock::~CatScopeBlock()
{
}


void CatScopeBlock::print() const
{
	Tools::CatLog::log("{\n");
	for (auto& iter : statements)
	{
		iter->print();
		Tools::CatLog::log("\n");
	}
	Tools::CatLog::log("}\n");
}


CatASTNodeType CatScopeBlock::getNodeType()
{
	return CatASTNodeType::ScopeBlock;
}


bool jitcat::AST::CatScopeBlock::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	return false;
}
