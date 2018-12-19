#pragma once

class CatRuntimeContext;
class LLVMCodeGeneratorHelper;
#include <functional>
#include "LLVMCompileOptions.h"
#include "LLVMForwardDeclares.h"

#include <vector>


struct LLVMCompileTimeContext
{
	LLVMCompileTimeContext(CatRuntimeContext* catContext);

	CatRuntimeContext* catContext;
	llvm::Function* currentFunction;

	LLVMCodeGeneratorHelper* helper;
	std::vector<std::function<llvm::Value*()>> blockDestructorGenerators;


	LLVMCompileOptions options;
};