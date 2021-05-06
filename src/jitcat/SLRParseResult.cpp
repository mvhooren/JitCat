/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/SLRParseResult.h"
#include "jitcat/ASTNode.h"

using namespace jitcat::Parser;


SLRParseResult::SLRParseResult():
	astRootNode(nullptr),
	success(false)
{
}


SLRParseResult::~SLRParseResult()
{
}


SLRParseResult::SLRParseResult(SLRParseResult&& other):
	astRootNode(std::move(other.astRootNode)),
	success(other.success)
{
}


SLRParseResult& SLRParseResult::operator=(SLRParseResult&& other)
{
	success = other.success;
	astRootNode = std::move(other.astRootNode);
	return *this;
}


SLRParseResult& jitcat::Parser::SLRParseResult::operator=(std::unique_ptr<SLRParseResult>&& other)
{
	SLRParseResult* otherPtr = other.release();
	success = otherPtr->success;
	astRootNode = std::move(otherPtr->astRootNode);
	delete otherPtr;
	return *this;
}


void SLRParseResult::clear()
{
	success = false;
	astRootNode.reset(nullptr);
}

