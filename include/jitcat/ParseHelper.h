/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat::Tokenizer
{

	class ParseHelper
	{
	public:
		static bool isValidStringChar(const char symbol);
		static bool isAlphaNumeric(const char symbol);
		static bool isNumber(const char symbol);
		static bool isNonZeroNumber(const char symbol);
		static bool isOctNumber(const char symbol);
		static bool isHexDigit(const char symbol);
	};

} //End namespace jitcat::Tokenizer