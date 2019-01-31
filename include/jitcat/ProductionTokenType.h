/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once	

namespace jitcat::Grammar
{

enum class ProductionTokenType
{
	Eof,
	Epsilon,
	NonTerminal,
	Terminal,
	TokenSet
};

} // End namespace jitcat::Grammar