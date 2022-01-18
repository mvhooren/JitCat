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
	class ReflectableHandle;
	class TypeInfo;
}

#include "jitcat/Configuration.h"
#include <string>
#include <vector>


namespace jitcat::LLVM
{
	struct LLVMCompileTimeContext;
	class LLVMCodeGeneratorHelper;

	//This class contains static functions that are called directly from jitted code. 
	//They represent some of the built-in/intrinsic functions available in JitCat.
	//It is a run-time library of sorts.

	namespace CatLinkedIntrinsics
	{
		//The CatLinkedIntrinsics are linked directly from the precompiled object file, while the intrinsics
		//contained in LLVMCatIntrinsics are indirectly linked through a global function pointer.
		//This is because the functions in LLVMCatIntrinsics return an object by value and this is not supported
		//with extern "C". Linking agains the mangled C++ symbols is difficult and they are therefore "manually"
		//linked through a set of global function pointers.
		extern "C" unsigned char* _jc_getScopePointerFromContext(CatRuntimeContext* context, int scopeId);
		extern "C" bool _jc_stringToBoolean(const Configuration::CatString& value);
		extern "C" double _jc_stringToDouble(const Configuration::CatString& string);
		extern "C" float _jc_stringToFloat(const Configuration::CatString& string);
		extern "C" int _jc_stringToInt(const Configuration::CatString& string);
		extern "C" unsigned int _jc_stringToUInt(const Configuration::CatString& string);
		extern "C" int64_t _jc_stringToInt64(const Configuration::CatString& string);
		extern "C" uint64_t _jc_stringToUInt64(const Configuration::CatString& string);
		extern "C" float _jc_getRandomFloat();
		extern "C" bool _jc_getRandomBoolean(bool first, bool second);
		extern "C" int _jc_getRandomInt(int min, int max);
		extern "C" float _jc_getRandomFloatRange(float min, float max);
		extern "C" double _jc_getRandomDoubleRange(double min, double max);
		extern "C" float _jc_roundFloat(float number, int decimals);
		extern "C" double _jc_roundDouble(double number, int decimals);
		extern "C" void _jc_placementCopyConstructType(unsigned char* target, unsigned char* source, Reflection::TypeInfo* type);
		extern "C" void _jc_placementMoveConstructType(unsigned char* target, unsigned char* source, Reflection::TypeInfo * type);
		extern "C" void _jc_placementConstructType(unsigned char* address, Reflection::TypeInfo* type);
		extern "C" void _jc_placementDestructType(unsigned char* address, Reflection::TypeInfo* type);
		extern "C" unsigned char* _jc_allocateMemory(std::size_t size);
		extern "C" void _jc_freeMemory(unsigned char* memory);
		extern "C" unsigned char* _jc_getObjectPointerFromHandle(const Reflection::ReflectableHandle& handle);
		extern "C" void _jc_assignPointerToReflectableHandle(Reflection::ReflectableHandle& handle, unsigned char* reflectable, Reflection::TypeInfo* reflectableType);
	};

	class LLVMCatIntrinsics
	{
		LLVMCatIntrinsics();
		~LLVMCatIntrinsics() = delete;

	public:
		static Configuration::CatString boolToString(bool boolean);
		static Configuration::CatString doubleToString(double number);
		static Configuration::CatString floatToString(float number);
		static Configuration::CatString intToString(int number);
		static Configuration::CatString uIntToString(unsigned int number);
		static Configuration::CatString int64ToString(int64_t number);
		static Configuration::CatString uInt64ToString(uint64_t number);
		static Configuration::CatString intToPrettyString(int number);
		static Configuration::CatString intToFixedLengthString(int number, int stringLength);
		static Configuration::CatString roundFloatToString(float number, int decimals);
		static Configuration::CatString roundDoubleToString(double number, int decimals);

	private:
		static Configuration::CatString formatRoundString(const Configuration::CatString& string);
	};


} //End namespace jitcat::LLVM