/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatASTNode.h"

#include <optional>
#include <vector>


namespace jitcat::AST
{
	class CatDefinition: public CatASTNode
	{
	public:
		CatDefinition(const Tokenizer::Lexeme& lexeme): CatASTNode(lexeme) {};
		CatDefinition(const CatDefinition& other): CatASTNode(other) {}

		virtual ~CatDefinition() {};

		//Recursively gathers all types. The gathered types are not fully defined after the pass is completed.
		//Detects duplicate type definitions.
		virtual bool typeGatheringCheck(CatRuntimeContext* compileTimeContext) = 0;
		//Gathers all definitions and builds the type structure.
		//Checks for things like duplicate definitions.
		virtual bool defineCheck(CatRuntimeContext* compileTimeContext, std::vector<const CatASTNode*>& loopDetectionStack) = 0;
		
		//Type checks all the definitions and their implementations and const-collapses them.
		virtual bool typeCheck(CatRuntimeContext* compileTimeContext) = 0;
	};

}
