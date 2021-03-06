/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <string>

namespace jitcat
{

	class RuntimeContext
	{
	public:
		RuntimeContext() {};
		virtual ~RuntimeContext() {};
		virtual std::string getContextName() = 0;
	};

} // End namespace jitcat;