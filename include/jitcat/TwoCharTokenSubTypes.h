/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

enum class TwoChar
{
	Unknown,
	Equals,
	NotEquals,
	SmallerOrEqual,
	GreaterOrEqual,
	LogicalAnd,
	LogicalOr,
	PlusAssign,
	MinusAssign,
	TimesAssign,
	DivideAssign,
	BitwiseOrAssign,
	BitwiseAndAssign,
	BitwiseXorAssign,
	BitshiftLeft,
	BitshiftRight,
	Increment,
	Decrement
};