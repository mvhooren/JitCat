/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include <catch2/catch.hpp>
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/TypeInfo.h"
#include "jitcat/TypeInfoDeleter.h"
#include "jitcat/TypeRegistry.h"
#include "PrecompilationTest.h"
#include "TestHelperFunctions.h"
#include "TestObjects.h"

#include <fstream>
#include <string>

using namespace jitcat;
using namespace jitcat::LLVM;
using namespace jitcat::Reflection;
using namespace TestObjects;


TEST_CASE("Type Serialization Tests", "[serialization][.]" ) 
{
	//clear the type registry
	//TypeRegistry::get()->recreate();

	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("Serialization", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "serializationStaticScope");

	TypeInfo* objectTypeInfo = TypeRegistry::get()->registerType<ReflectedObject>();
	const char* customTypeName = "SerializationType";
	TypeRegistry::get()->removeType(customTypeName);
	std::unique_ptr<CustomTypeInfo, TypeInfoDeleter> customType = makeTypeInfo<CustomTypeInfo>(customTypeName);
	TypeRegistry::get()->registerType(customTypeName, customType.get());
	customType->addFloatMember("myFloat", 0.001f);
	customType->addIntMember("myInt", 54321);
	customType->addStringMember("myString", "foo");
	customType->addBoolMember("myBoolean", true);
	customType->addObjectMember("myObject", &reflectedObject, objectTypeInfo);
	customType->addObjectMember("myNullObject", &reflectedObject, objectTypeInfo);
	ObjectInstance typeInstance(customType.get());
	context.addDynamicScope(typeInstance);

	const char* customStaticTypeName = "MyStaticAssignType";
	TypeRegistry::get()->removeType(customStaticTypeName);
	std::unique_ptr<CustomTypeInfo, TypeInfoDeleter> customStaticType = makeTypeInfo<CustomTypeInfo>(customStaticTypeName);
	customStaticType->addObjectMember("myStaticObject", &reflectedObject, objectTypeInfo);
	customStaticType->addObjectMember("myStaticCustomObject", typeInstance.getObject(), customType.get());
	ObjectInstance staticTypeInstance(customStaticType.get());
	context.addDynamicScope(staticTypeInstance);

	//Save the type information to an XML file
	TypeRegistry::get()->exportRegistyToXML("Test_TypeInfo.xml");

	//Clear the type registry again
	TypeRegistry::get()->recreate();

	//Load the type info from the XML file
	TypeRegistry::get()->loadRegistryFromXML("Test_TypeInfo.xml");

	//Save again
	TypeRegistry::get()->exportRegistyToXML("Test_TypeInfo2.xml");

	SECTION("Match serialized files")
	{
		//Compare the files, they should be identical
		std::ifstream firstFile("Test_TypeInfo.xml", std::ios::in | std::ios::binary);
		CHECK(firstFile);
		if (firstFile)
		{
			std::string firstFileContents;
			firstFile.seekg(0, std::ios::end);
			firstFileContents.resize(firstFile.tellg());
			firstFile.seekg(0, std::ios::beg);
			firstFile.read(&firstFileContents[0], firstFileContents.size());
			firstFile.close();
			std::ifstream secondFile("Test_TypeInfo2.xml", std::ios::in | std::ios::binary);
			CHECK(secondFile);
			if (secondFile)
			{
				std::string secondFileContents;
				secondFile.seekg(0, std::ios::end);
				secondFileContents.resize(secondFile.tellg());
				secondFile.seekg(0, std::ios::beg);
				secondFile.read(&secondFileContents[0], secondFileContents.size());
				secondFile.close();

				bool contentsEqual = firstFileContents == secondFileContents;

				CHECK(contentsEqual);
			}
		}
	}
}
