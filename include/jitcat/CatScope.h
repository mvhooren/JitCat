/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "CatScopeID.h"

namespace jitcat::Reflection
{
	class CustomTypeInfo;
}

namespace jitcat
{
	class CatScope
	{
	public:
		virtual CatScopeID getScopeId() const = 0;
		virtual Reflection::CustomTypeInfo* getCustomType() = 0;
	};
}