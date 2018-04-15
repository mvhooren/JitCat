/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "SLRParseResult.h"
#include "ASTNode.h"


SLRParseResult::SLRParseResult():
	success(false),
	astRootNode(nullptr),
	errorPosition(0)
{
}


SLRParseResult::~SLRParseResult()
{
	delete astRootNode;
};
