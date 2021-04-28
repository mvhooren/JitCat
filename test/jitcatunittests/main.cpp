/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

//To prevent the unit test program from exiting immediately after test completion
//add the following to the command line arguments for running the executable:
//--wait-for-keypress exit

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>
#include "jitcat/JitCat.h"

#include "PrecompilationTest.h"


int main( int argc, char* argv[] ) 
{
	#ifdef ENABLE_LLVM
		//Precompilation::precompContext = jitcat::JitCat::get()->createPrecompilationContext();
	#endif
	int result = Catch::Session().run( argc, argv );


	if (Precompilation::precompContext != nullptr)
	{
		//Emit an object file
		Precompilation::precompContext->finishPrecompilation();
		//Make sure the precompContext is destroyed before JitCat::destroy is called.
		Precompilation::precompContext = nullptr;
	}
	jitcat::JitCat::get()->destroy();

	return result;
}