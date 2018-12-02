#pragma once

#include <iostream>

template<typename ...Arguments>
inline llvm::Value* LLVMJit::logError(Arguments ...arguments)
{
	int dummy[] = { 0, ( std::cout << arguments, 0) ... };
	std::cout << "\n";
	return nullptr;
}
