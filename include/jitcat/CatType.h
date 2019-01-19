/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

enum class CatType
{
	Int,
	Float,
	String,
	Bool,
	Void,
	Object,
	Unknown,
	Count
};


const char* toString(CatType type);
CatType toCatType(const char* value);
bool isScalar(CatType type);
bool isBasicType(CatType type);