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
	class TypeInfo;
}
#include "jitcat/Configuration.h"
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
		static Reflection::Reflectable* getScopePointerFromContext(CatRuntimeContext* context, int scopeId);
		static bool stringEquals(const Configuration::CatString& left, const Configuration::CatString& right);
		static bool stringNotEquals(const Configuration::CatString& left, const Configuration::CatString& right);
		static void stringAssign(Configuration::CatString* left, const Configuration::CatString& right);
		static bool stringToBoolean(const Configuration::CatString& value);
		static Configuration::CatString stringAppend(const Configuration::CatString& left, const Configuration::CatString& right);
		static Configuration::CatString boolToString(bool boolean);
		static Configuration::CatString doubleToString(double number);
		static Configuration::CatString floatToString(float number);
		static double stringToDouble(const Configuration::CatString& string);
		static float stringToFloat(const Configuration::CatString& string);
		static Configuration::CatString intToString(int number);
		static int stringToInt(const Configuration::CatString& string);
		static Configuration::CatString intToPrettyString(int number);
		static Configuration::CatString intToFixedLengthString(int number, int stringLength);
		static float getRandomFloat();
		static bool getRandomBoolean(bool first, bool second);
		static int getRandomInt(int min, int max);
		static float getRandomFloatRange(float min, float max);
		static double getRandomDoubleRange(double min, double max);
		static float roundFloat(float number, int decimals);
		static double roundDouble(double number, int decimals);
		static Configuration::CatString roundFloatToString(float number, int decimals);
		static Configuration::CatString roundDoubleToString(double number, int decimals);
		static void placementCopyConstructType(Reflection::Reflectable* target, Reflection::Reflectable* source,  Reflection::TypeInfo* type);
		static void placementConstructType(Reflection::Reflectable* address, Reflection::TypeInfo* type);
		static void placementDestructType(Reflection::Reflectable* address, Reflection::TypeInfo* type);

	private:
		static Configuration::CatString formatRoundString(const Configuration::CatString& string);
	};


} //End namespace jitcat::LLVM