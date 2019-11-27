/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat
{

namespace Configuration
{
	//Determines the ordering of the 'this' argument and the 'sret' argument in a member function.
	//Sret is used when a function returns a structure by value. Om windows/msvc a class member funtion's
	//first argument will be the 'this' pointer and the second argument will be the sret pointer if applicable.
	//On linux/clang/gcc this is the other way around.
	static constexpr bool sretBeforeThis = 
#ifdef WIN32
		false;
#else
		true;
#endif

	//Sets the calling convention for member function calls. Similar to the sretBeforeThis option.
	static constexpr bool useThisCall = 
#ifdef WIN32
		true;
#else
		false;
#endif

	//The assumed size of a normal member function pointer.
	//Member function pointers can differ in size in the case of (virtual)inheritance.
	static constexpr int basicMemberFunctionPointerSize = sizeof(uintptr_t);

	//Dump LLVM IR whenever a function's code is being generated.
	static constexpr bool dumpFunctionIR = 
#ifdef _DEBUG
		false;
#else
		//function IR cannot be dumped in release builds
		false;
#endif

	//Enable some workarounds on Windows/MSVC required for finding function symbols after their code has been generated.
	static constexpr bool enableSymbolSearchWorkaround =
#ifdef WIN32
		true;
#else
		false;
#endif

	//Enable the LLVM code generator backend.
	static constexpr bool enableLLVM =
#ifdef ENABLE_LLVM
		true;
#else
		false;
#endif

	//Whenever a floating point number is divided by zero, normally a NaN or (+-)Infinity is returned.
	//Dividing an integer by zero is undefined behaviour. The program will probably abort.
	//By enabling this flag, dividing by zero within an expression will return 0, preventing the 
	//spread of NaNs or program abort but breaking mathematical correctness.
	static constexpr bool divisionByZeroYieldsZero = false;

};

} //namespace jitcat