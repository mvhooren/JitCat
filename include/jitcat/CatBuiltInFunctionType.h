/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat::AST
{

	enum class CatBuiltInFunctionType
	{
		ToVoid,
		ToInt,
		ToDouble,
		ToFloat,
		ToBool,
		ToString,
		ToPrettyString,
		ToFixedLengthString,
		Sin,
		Cos,
		Tan,
		Asin,
		Acos,
		Atan,
		Sinh,
		Cosh,
		Tanh,
		Asinh,
		Acosh,
		Atanh,
		Atan2,
		Hypot,
		Random,
		RandomRange,
		Round,
		StringRound,
		Abs,
		Cap,
		Min,
		Max,
		Log10,
		Ln,
		Exp,
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


} //End namespace jitcat::AST