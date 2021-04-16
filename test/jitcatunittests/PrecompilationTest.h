/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/LLVMJit.h"
#include "jitcat/PrecompilationContext.h"

#include <memory>

class Precompilation
{
public:
	static std::shared_ptr<jitcat::PrecompilationContext> precompContext;
};