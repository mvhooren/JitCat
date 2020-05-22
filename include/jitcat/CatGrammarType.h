/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

//A value of this enum is passed to a CatGrammar constructor to select what type of grammar to build.
enum class CatGrammarType
{
	Expression,
	Statement,
	Full
};