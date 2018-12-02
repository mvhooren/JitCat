#pragma once

class CatRuntimeContext;

#include "LLVMForwardDeclares.h"


struct LLVMCompileTimeContext
{
	LLVMCompileTimeContext(CatRuntimeContext* catContext);

	LLVMCompileTimeContext* getPointer();

	CatRuntimeContext* catContext;
	llvm::Function* currentFunction;
};