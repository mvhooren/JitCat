/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include <catch2/catch.hpp>
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/TypeInfo.h"
#include "TestHelperFunctions.h"
#include "TestObjects.h"

using namespace jitcat;
using namespace jitcat::LLVM;
using namespace jitcat::Reflection;
using namespace TestObjects;


//This test case is meant to test things out while developing
//Do not commit tests in this file.
//To enable this test, remove the '.' in [.development] to enable the test by default or
//add [.development] to the command line arguments of JitCatUnitTests
TEST_CASE("DevelopmentTests", "[.development]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("development_tests", &errorManager);
	context.addStaticScope(&reflectedObject, "devStaticScope");	

	SECTION("HelloWorld")
	{
		Expression<std::string> testExpression(&context, "\"Hello World!\"");
		doChecks(std::string("Hello World!"), false, true, true, testExpression, context);
	}
}