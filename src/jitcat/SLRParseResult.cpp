/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/SLRParseResult.h"
#include "jitcat/ASTNode.h"

using namespace jitcat::Parser;


SLRParseResult::SLRParseResult():
	success(false),
	astRootNode(nullptr)
{
}


SLRParseResult::~SLRParseResult()
{
};
