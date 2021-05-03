/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2021
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


namespace jitcat::LLVM
{
	//These are the targets that JitCat has pre-defined configurations.
	//An exception is LLVMTarget::CurrentMachine, the configuration of which is determined at runtime.
	enum class LLVMTarget
	{
		CurrentMachine,
		Windows_X64,
		Playstation4,
		XboxOne
	};
}