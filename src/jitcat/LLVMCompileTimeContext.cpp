#include "LLVMCompileTimeContext.h"


LLVMCompileTimeContext::LLVMCompileTimeContext(CatRuntimeContext* catContext):
	catContext(catContext),
	currentFunction(nullptr),
	helper(nullptr),
	currentDyLib(nullptr)
{
}