/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

template<typename ...Arguments>
inline llvm::Value* LLVMJit::logError(Arguments ...arguments)
{
	int dummy[] = { 0, ( std::cout << arguments, 0) ... };
	//To silence unused variable warnings.
	(void)dummy;
	std::cout << "\n";
	return nullptr;
}
