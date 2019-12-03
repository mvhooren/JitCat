/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat
{
	class CatRuntimeContext;
}
namespace jitcat::Reflection
{
	class Reflectable;
}
#include "jitcat/LLVMForwardDeclares.h"
#include "jitcat/LLVMTypes.h"
#include <string>
#include <vector>


namespace jitcat::LLVM
{
	struct LLVMCompileTimeContext;
	class LLVMCodeGeneratorHelper;

	//This class contains static functions that are called directly from jitted code. 
	//They represent some of the built-in/intrinsic functions available in JitCat.
	//It is a run-time library of sorts.
	class LLVMCatIntrinsics
	{
		LLVMCatIntrinsics();
		~LLVMCatIntrinsics() = delete;

	public:
		static unsigned char* getScopePointerFromContext(CatRuntimeContext* context, int scopeId);
		static bool stringEquals(const std::string& left, const std::string& right);
		static bool stringNotEquals(const std::string& left, const std::string& right);
		static void stringAssign(std::string* left, const std::string& right);
		static bool stringToBoolean(const std::string& value);
		static std::string stringAppend(const std::string& left, const std::string& right);
		static std::string floatToString(float number);
		static float stringToFloat(const std::string& string);
		static std::string intToString(int number);
		static int stringToInt(const std::string& string);
		static std::string intToPrettyString(int number);
		static std::string intToFixedLengthString(int number, int stringLength);
		static void stringEmptyConstruct(std::string* destination);
		static void stringCopyConstruct(std::string* destination, const std::string* string);
		static void stringDestruct(std::string* target);
		static int findInString(const std::string* text, const std::string* textToFind);
		static std::string replaceInString(const std::string* text, const std::string* textToFind, const std::string* replacement);
		static int stringLength(const std::string* text);
		static	std::string subString(const std::string* text, int start, int length);
		static float getRandomFloat();
		static bool getRandomBoolean(bool first, bool second);
		static int getRandomInt(int min, int max);
		static float getRandomFloatRange(float min, float max);
		static float roundFloat(float number, int decimals);
		static std::string roundFloatToString(float number, int decimals);
	};


} //End namespace jitcat::LLVM