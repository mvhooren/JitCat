/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat::Tokenizer
{

	enum class ConstantType
	{
		Integer,
		DoubleFloatingPoint,
		FloatingPoint,
		String,
		Char,
		Bool,
		NoType
	};

} //End namespace jitcat::Tokenizer