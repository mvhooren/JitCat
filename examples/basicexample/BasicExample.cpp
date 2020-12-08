/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/Configuration.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/Expression.h"
#include "jitcat/ExpressionAny.h"
#include "jitcat/ObjectInstance.h"
#include "jitcat/TypeInfoDeleter.h"
#include "ReflectionTestRoot.h"

#include <memory>
#include <string>
#ifdef WIN32
	#include <tchar.h>
	#define MAIN _tmain
#else
	#define MAIN main
#endif
#include <iostream>

using namespace jitcat;
using namespace jitcat::Reflection;


int MAIN(int argc, char* argv[])
{
	//#####################################
	//# Route any debug output to std out #
	//#####################################
	std::unique_ptr<Tools::CatLogListener> logMethod = std::make_unique<Tools::CatLogStdOut>();
	Tools::CatLog::addListener(logMethod.get());

	//###############################
	//# A simple consant expression #
	//###############################

	//Create an expression that does some integer math.
	Expression<int> myConstantExpression("41 + 1");
	//Compile it, providing nullptr for the context because we don't use any variables.
	myConstantExpression.compile(nullptr);
	//Print the result. (Again providing nullptr for the context).
	std::cout << "myConstantExpression: " << myConstantExpression.getValue(nullptr) << "\n";

	//############################
	//# Providing some variables #
	//############################

	//Create a context that we will use later on to provide variables for our expressions
	CatRuntimeContext context("myContext");

	//This reflectable object will provide some variables and functions
	ReflectionTestRoot exampleObject;
	context.addScope(&exampleObject, true);

	//Create a floating point expression and execute it.
	//test.aFloat comes from the testObject member inside the exampleObject	and pi comes from the exampleObject itself
	Expression<float> aFloatingPointExpression(&context, "test.aFloat * pi");
	std::cout << "aFloatingPointExpression: " << aFloatingPointExpression.getValue(&context) << "\n";


	//###################################################
	//# Providing variables from a runtime-defined type #
	//###################################################

	//Create a runtime-defined type
	const char* customTypeName = "MyCustomType";
	std::unique_ptr<CustomTypeInfo, TypeInfoDeleter> customType = makeTypeInfo<CustomTypeInfo>(customTypeName);

	//Add some fields to the type and some default values
	customType->addFloatMember("aFloat", 666.0f);
	customType->addIntMember("anInt", 43);
	customType->addStringMember("aString", "ThisIsAString");
	//Register a new type and use it to add an object-type member
	TypeInfo* exampleObjectType = TypeRegistry::get()->registerType<ReflectionTestRoot>();
	customType->addObjectMember("anObject", &exampleObject, exampleObjectType, TypeOwnershipSemantics::Weak, false);

	//Create an instance of the runtime-defined type
	ObjectInstance customTypeInstance(customType.get());

	//Add the type to the context so we can access the variables in an expression
	context.addScope(customTypeInstance, false);

	//This time we do not provide the context in the constructor so we need to call compile before calling getValue.
	//We use the built-in function abs. A list of all built-in functions can be found in the CatBuiltInFunctionType header.
	//Here aFloat comes from our custom type and test.aFloat comes from the testObject member inside the exampleObject.
	Expression<float> anotherFloatingPointExpression("abs(aFloat - test.aFloat)");
	anotherFloatingPointExpression.compile(&context);
	std::cout << "anotherFloatingPointExpression: " << anotherFloatingPointExpression.getValue(&context) << "\n";


	//#####################
	//# returning objects #
	//#####################

	//It is possible for expressions to return objects
	//test3 comes from the unique_ptr testObject3 inside our exampleObject.
	Expression<ReflectionTestObject*> objectTypedExpression(&context, "test3");
	//Normally, you should check for nullptr on the returned object but we just print its address here.
	ReflectionTestObject* object = objectTypedExpression.getValue(&context);
	std::cout << "objectTypedExpression: " << object << "\n";


	//###############################
	//# Expressions return any type #
	//###############################
	
	//You may not always know in advance what type an expression will return or perhaps your expression is based on 
	//user input and can contain any type.
	//For that case there is ExpressionAny.
	//This expression calls the getTest2 function on the testObject member inside the exampleObject and accesses the 'what' 
	//std::string member of the returned ReflectionTestObject2 object.
	ExpressionAny anyExpression(&context, "test.getTest2().what");
	//Now getValue returns a std::any
	std::any result = anyExpression.getValue(&context);
	//Cast the result based on the type returned by the expression
	if (anyExpression.getType().isStringValueType())
	{
		//using std::any_cast, we can cast it to the string that it returned
		std::cout << "anyExpression: " << std::any_cast<Configuration::CatString>(result) << "\n";
	}
	else if (anyExpression.getType().isStringPtrType())
	{
		//using std::any_cast, we can cast it to the string that it returned
		std::cout << "anyExpression: " << *std::any_cast<Configuration::CatString*>(result) << "\n";
	}
	return 0;
}

