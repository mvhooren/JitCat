/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


#include <locale>
#include <sstream>
#include <string>


namespace jitcat
{

namespace Configuration
{
	//This determines the default underlying string type.
	//Can be changed to, for example, use a different string allocator.
	using CatString			= std::string;
	using CatStringStream	= std::stringstream;
	using CatStringOStream	= std::ostringstream;

	//Defines whether or not to look for precompiled expressions that have been linked into the program.
	static constexpr bool usePreCompiledExpressions = false;

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

	//Determines who is responsible for destroying temporary argument values that are passed to a function.
	//In the Windows/MSVC ABI, the callee is responsible for destroying temporaries.
	//On Linux/GCC/Clang, which use the Itanium C++ ABI, the caller is responsible.
	static constexpr bool callerDestroysTemporaryArguments = 
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
	//This only works with debug builds and when linking against a debug build of LLVM.
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

	//Enable the registration of function unwind tables on Windows
	//This will improve callstacks while debugging and profiling jitted code.
	static constexpr bool enableWin32ExperimentalUnwindTableRegistration =
#ifdef WIN32
		false;
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

	//Enables very verbose logging of the JitCat grammar
	static constexpr bool debugGrammar = 
#ifdef DEBUG_GRAMMAR
		true;
#else
		false;
#endif

	//Enables verbose logging of the construction and destruction of objects from within jitcat code.
	static constexpr bool logJitCatObjectConstructionEvents = 
#ifdef _DEBUG
		false;
#else
		false;
#endif

	//Whenever ReflectableHandles are changed, verify the linked list of handles.
	//This can be very slow, but catches issues with corrupted handles.
	//Only works if assertions are enabled.
	static constexpr bool enableHandleVerificationAsserts = 
#ifdef _DEBUG
		false;
#else
		false;
#endif

	//Whenever a floating point number is divided by zero, normally a NaN or (+-)Infinity is returned.
	//Dividing an integer by zero is undefined behaviour. The program will probably abort.
	//By enabling this flag, dividing by zero within an expression will return 0, preventing the 
	//spread of NaNs or program abort but breaking mathematical correctness.
	static constexpr bool divisionByZeroYieldsZero = false;

	//Weither or not floating point literals are doubles by default
	static constexpr bool defaultFloatingPointLiteralIsDouble = false;

	//The locale for when numbers are converted to strings.
	static std::locale localeForStringConversions = std::locale("");
};

} //namespace jitcat