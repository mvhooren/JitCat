/*
  This file is part of the JitCat library.

  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat::Reflection
{
	enum class TypeOwnershipSemantics: unsigned char
	{
		None,
		Value,
		Owned,
		Shared,
		Weak,
		Count
	};

}