/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/LLVMCompileTimeContext.h"

using namespace jitcat;
using namespace jitcat::LLVM;


LLVMCompileTimeContext::LLVMCompileTimeContext(CatRuntimeContext* catContext, const LLVMTargetConfig* targetConfig, bool isPrecompilationContext):
	catContext(catContext),
	isPrecompilationContext(isPrecompilationContext),
	currentLib(nullptr),
	currentClass(nullptr),
	currentFunctionDefinition(nullptr),
	currentFunction(nullptr),
	currentScope(nullptr),
	helper(nullptr)
{
}


void LLVMCompileTimeContext::clearState()
{
	catContext = nullptr;
	currentLib = nullptr;
	currentClass = nullptr;
	currentScope = nullptr;
	currentFunctionDefinition = nullptr;
	currentFunction = nullptr;
	helper = nullptr;
	blockDestructorGenerators.clear();
	scopeValues.clear();
}
