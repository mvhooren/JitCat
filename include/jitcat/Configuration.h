/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat
{

class Configuration
{
public:
	static constexpr bool sretBeforeThis = 
#ifdef WIN32
		false;
#else
		true;
#endif

	static constexpr bool useThisCall = 
#ifdef WIN32
		true;
#else
		false;
#endif

	static constexpr int basicMemberFunctionPointerSize = sizeof(uintptr_t);

	static constexpr bool dumpFunctionIR = 
#ifdef _DEBUG
		false;
#else
		//function IR cannot be dumped in release builds
		false;
#endif

	static constexpr bool enableSymbolSearchWorkaround =
#ifdef WIN32
		true;
#else
		false;
#endif

	static constexpr bool enableLLVM =
#ifdef ENABLE_LLVM
		true;
#else
		false;
#endif
};

} //namespace jitcat