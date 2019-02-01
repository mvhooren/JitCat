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


int main( int argc, char* argv[] ) 
{
	int result = Catch::Session().run( argc, argv );

	jitcat::JitCat::get()->cleanup();

	return result;
}