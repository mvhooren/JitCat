/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat::Tokenizer
{

	enum class Identifier
	{
		Identifier,
		Class,
		Struct,
		Inherits,
		Enum,
		Public,
		Protected,
		Private,
		Const,
		Static,
		Virtual,
		New,
		If,
		Then,
		Else,
		While,
		Do,
		For,
		Continue,
		Break,
		Switch,
		Case,
		Default,
		Return,
		Void,
		Unsigned,
		Char,
		Bool,
		Int,
		Long,
		Float,
		Double,
		Vector4,
		Matrix,
		String,
		Null,
		Last
	};

} //End namespace jitcat::Tokenizer