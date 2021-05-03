/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2021
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


namespace jitcat::LLVM
{
	//These are the targets that JitCat has pre-defined configurations for.
	//An exception is LLVMTarget::CurrentMachine and CurrentMachineJIT, the configuration of these are determined at runtime.
	enum class LLVMTarget
	{
		//JIT target:
		CurrentMachineJIT,
		//Precompilation targets:
		CurrentMachine,
		Windows_X64,
		Playstation4,
		XboxOne
	};
}