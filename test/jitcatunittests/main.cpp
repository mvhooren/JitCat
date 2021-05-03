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
#include "jitcat/Configuration.h"
#include "jitcat/JitCat.h"
#include "jitcat/Tools.h"

#include "PrecompilationTest.h"


int main( int argc, char* argv[] ) 
{
	Catch::Session session; // There must be exactly one instance
  
	bool precompile = false; 
  
	// Build a new parser on top of Catch's
	using namespace Catch::clara;
	auto cli = session.cli() // Get Catch's composite command line parser
		| Opt(precompile) // bind variable to a new option, with a hint string
		["-p"]["--precompile"]    // the option names it will respond to
		("Precompile JitCat expressions to an object file."); // description string for the help output
        
	// Now pass the new composite back to Catch so it uses that
	session.cli(cli); 
  
	// Let Catch (using Clara) parse the command line
	int returnCode = session.applyCommandLine(argc, argv);
	if (returnCode != 0)
	{
		// Indicates a command line error
		return returnCode;
	}
	
	if (jitcat::Configuration::enableLLVM && precompile)
	{
		Precompilation::precompContext = jitcat::JitCat::get()->createPrecompilationContext();
	}

	int result = session.run();

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