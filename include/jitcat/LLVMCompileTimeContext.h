#pragma once

class CatRuntimeContext;

#include <functional>
#include "LLVMForwardDeclares.h"


struct LLVMCompileTimeContext
{
	LLVMCompileTimeContext(CatRuntimeContext* catContext);

	CatRuntimeContext* catContext;
	llvm::Function* currentFunction;

	std::vector<std::function<llvm::Value*()>> blockDestructorGenerators;
};