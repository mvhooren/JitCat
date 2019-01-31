/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat::AST
{
	class ASTNode;
}
#include <string>


namespace jitcat::Parser
{

	struct SLRParseResult
	{
		SLRParseResult();
		~SLRParseResult();
		bool success;
		AST::ASTNode* astRootNode;
		std::string errorMessage;
		std::size_t errorPosition;
	};

}