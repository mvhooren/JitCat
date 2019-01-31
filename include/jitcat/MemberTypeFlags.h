/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat::Reflection
{

	enum MemberTypeFlags
	{
		MTF_IS_CONST = 1,
		MTF_IS_STATIC_CONST = 2,
		MTF_IS_WRITABLE = 4
	};

} //End namespace jitcat::Reflection