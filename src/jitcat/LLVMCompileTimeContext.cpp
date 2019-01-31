/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "LLVMCompileTimeContext.h"


LLVMCompileTimeContext::LLVMCompileTimeContext(CatRuntimeContext* catContext):
	catContext(catContext),
	currentFunction(nullptr),
	helper(nullptr),
	currentDyLib(nullptr)
{
}