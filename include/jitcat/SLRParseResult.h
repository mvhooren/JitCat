/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class ASTNode;
#include <string>


struct SLRParseResult
{
	SLRParseResult();
	~SLRParseResult();
	bool success;
	ASTNode* astRootNode;
	std::string errorMessage;
	std::size_t errorPosition;
};