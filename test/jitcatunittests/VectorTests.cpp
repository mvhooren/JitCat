#include <catch2/catch.hpp>

#include "jitcat/CatLib.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/Configuration.h"
#include "jitcat/JitCat.h"
#include "jitcat/TypeInfo.h"
#include "PrecompilationTest.h"
#include "TestHelperFunctions.h"
#include "TestObjects.h"


using namespace jitcat;
using namespace jitcat::LLVM;
using namespace jitcat::Reflection;
using namespace TestObjects;

//Tests simple variable initialization and access.
TEST_CASE("Vector tests", "[catlib][vector]")
{
	bool enableTest = !JitCat::get()->getHasPrecompiledExpression() && Precompilation::precompContext == nullptr;

	if (!enableTest)
	{
		if (JitCat::get()->getHasPrecompiledExpression())
		{
			WARN("CatLib tests are disabled because precompiled expressions have been found and CatLib does not yet support precompilation");
		}
		else
		{
			WARN("CatLib tests are disabled because there is an active precompilation context and CatLib does not yet support precompilation");
		}
	}
	if (enableTest)
	{
		ReflectedObject reflectedObject;
		ExpressionErrorManager errorManager;

		CatLib library("TestLib", Precompilation::precompContext);
		library.addStaticScope(&reflectedObject, "catlibStaticScope");
		Tokenizer::Document source(
			"class VectorClass\n"
			"{\n"
			"	vector4f testVector = {42.0f, 43.0f, 44.0f, 45.0f};\n"
			"\n"
			"	vector4f getVector() { return testVector;}\n"
			"	//float getFloat() { return testVector.x;}\n"
			"}\n");
		library.addSource("vectortest.jc", source);
		std::vector<const ExpressionErrorManager::Error*> errors;
		library.getErrorManager().getAllErrors(errors);
		for (auto& iter : errors)
		{
			std::cout << iter->contextName << " ERROR: Line: " << iter->errorLine << " Column: " << iter->errorColumn << " Length: " << iter->errorLength << "\n";
			std::cout << iter->message << "\n";
		}
		REQUIRE(library.getErrorManager().getNumErrors() == 0);
		TypeInfo* testClassInfo = library.getTypeInfo("VectorClass");
		REQUIRE(testClassInfo != nullptr);
		unsigned char* testClassInstance = testClassInfo->construct();

		CatRuntimeContext context("jitlib", &errorManager);
		context.setPrecompilationContext(Precompilation::precompContext);
		context.addDynamicScope(testClassInfo, testClassInstance);

		/*SECTION("Vector variable")
		{
			Expression<std::array<float, 4>> testExpression(&context, "testVector");
			std::array<float, 4> expected = {42.0f, 43.0f, 44.0f, 45.0f};
			doChecks(expected, false, false, false, testExpression, context);
		}*/
		/*SECTION("Float variable")
		{
			Expression<float> testExpression(&context, "testFloat");
			doChecks(42.0f, false, false, false, testExpression, context);
		}*/
	}
}