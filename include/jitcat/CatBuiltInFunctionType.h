/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

enum class CatBuiltInFunctionType
{
	ToVoid,
	ToInt,
	ToFloat,
	ToBool,
	ToString,
	ToPrettyString,
	ToFixedLengthString,
	Sin,
	Cos,
	Tan,
	Random,
	RandomRange,
	Round,
	StringRound,
	Abs,
	Cap,
	Min,
	Max,
	Log,
	Sqrt,
	Pow,
	Ceil,
	Floor,
	FindInString,
	ReplaceInString,
	StringLength,
	SubString,
	Select,
	Count,
	Invalid
};