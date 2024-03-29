/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat::Tokenizer
{

	enum class OneChar: unsigned short
	{
		Unknown,
		BraceOpen,
		BraceClose,
		ParenthesesOpen,
		ParenthesesClose,
		BracketOpen,
		BracketClose,
		Assignment,
		Plus,
		Times,
		Divide,
		Minus,
		Modulo,
		Smaller,
		Greater,
		Comma,
		Semicolon,
		BitwiseAnd,
		BitwiseOr,
		BitwiseXor,
		Not,
		Dot,
		At
	};

} //End namespace jitcat::Tokenizer