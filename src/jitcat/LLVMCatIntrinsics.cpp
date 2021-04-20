/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/LLVMCatIntrinsics.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/Configuration.h"
#include "jitcat/Reflectable.h"
#include "jitcat/ReflectableHandle.h"
#include "jitcat/Tools.h"

#include <cmath>

using namespace jitcat;
using namespace jitcat::LLVM;
using namespace jitcat::Reflection;


unsigned char* CatLinkedIntrinsics::_jc_getScopePointerFromContext(CatRuntimeContext* context, int scopeId)
{
	return context->getScopeObject((CatScopeID)scopeId);
}


bool CatLinkedIntrinsics::_jc_stringToBoolean(const Configuration::CatString& value)
{
	return value == Tools::StringConstants<Configuration::CatString>::trueStr || Tools::StringConstants<Configuration::CatString>::stringToInt(value) > 0;
}


double CatLinkedIntrinsics::_jc_stringToDouble(const Configuration::CatString& string)
{
	return Tools::convert<double>(string);
}


float CatLinkedIntrinsics::_jc_stringToFloat(const Configuration::CatString& string)
{
	return Tools::convert<float>(string);
}


int CatLinkedIntrinsics::_jc_stringToInt(const Configuration::CatString& string)
{
	return Tools::convert<int>(string);
}


unsigned int CatLinkedIntrinsics::_jc_stringToUInt(const Configuration::CatString& string)
{
	return Tools::convert<unsigned int>(string);
}


int64_t CatLinkedIntrinsics::_jc_stringToInt64(const Configuration::CatString& string)
{
	return Tools::convert<int64_t>(string);
}


uint64_t CatLinkedIntrinsics::_jc_stringToUInt64(const Configuration::CatString& string)
{
	return Tools::convert<uint64_t>(string);
}


float CatLinkedIntrinsics::_jc_getRandomFloat()
{
	return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
}


bool CatLinkedIntrinsics::_jc_getRandomBoolean(bool first, bool second)
{
	return (std::rand() % 2) == 1 ? first : second;
}


int CatLinkedIntrinsics::_jc_getRandomInt(int min, int max)
{
	if (min > max)
	{
		std::swap(min, max);
	}
	return min + (std::rand() % (max - min + 1));
}


float CatLinkedIntrinsics::_jc_getRandomFloatRange(float min, float max)
{
	if (min > max)
	{
		std::swap(min, max);
	}
	float random = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
	return min + random * (max - min);
}


double CatLinkedIntrinsics::_jc_getRandomDoubleRange(double min, double max)
{
	if (min > max)
	{
		std::swap(min, max);
	}
	double random = static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
	return min + random * (max - min);
}


float CatLinkedIntrinsics::_jc_roundFloat(float number, int decimals)
{
	double multiplier = std::pow(10.0f, decimals);
	return (float)(std::floor(number * multiplier + 0.5f) / multiplier);
}


double CatLinkedIntrinsics::_jc_roundDouble(double number, int decimals)
{
	double multiplier = std::pow(10.0, decimals);
	return std::floor(number * multiplier + 0.5) / multiplier;
}


void CatLinkedIntrinsics::_jc_placementCopyConstructType(unsigned char* target, unsigned char* source, Reflection::TypeInfo* type)
{
	if (target != source)
	{
		type->copyConstruct(target, type->getTypeSize(), source, type->getTypeSize());
	}
}


void CatLinkedIntrinsics::_jc_placementConstructType(unsigned char* address, Reflection::TypeInfo* type)
{
	type->placementConstruct(address, type->getTypeSize());
}


void CatLinkedIntrinsics::_jc_placementDestructType(unsigned char* address, TypeInfo* type)
{
	type->placementDestruct(address, type->getTypeSize());
}


unsigned char* CatLinkedIntrinsics::_jc_allocateMemory(std::size_t size)
{
	return new unsigned char[size];
}


void CatLinkedIntrinsics::_jc_freeMemory(unsigned char* memory)
{
	delete[] memory;
}


unsigned char* CatLinkedIntrinsics::_jc_getObjectPointerFromHandle(const Reflection::ReflectableHandle& handle)
{
	return handle.get();
}


void CatLinkedIntrinsics::_jc_assignPointerToReflectableHandle(Reflection::ReflectableHandle& handle, unsigned char* reflectable, TypeInfo* reflectableType)
{
	handle.setReflectable(reflectable, reflectableType);
}


Configuration::CatString LLVMCatIntrinsics::boolToString(bool boolean)
{
	if (boolean)
	{
		return Tools::StringConstants<Configuration::CatString>::oneStr;
	}
	else
	{
		return Tools::StringConstants<Configuration::CatString>::zeroStr;
	}
}


Configuration::CatString LLVMCatIntrinsics::doubleToString(double number)
{
	return Tools::StringConstants<Configuration::CatString>::makeString(number);
}


Configuration::CatString LLVMCatIntrinsics::floatToString(float number)
{
	return Tools::StringConstants<Configuration::CatString>::makeString(number);
}



Configuration::CatString LLVMCatIntrinsics::intToString(int number)
{
	return Tools::StringConstants<Configuration::CatString>::makeString(number);
}


Configuration::CatString LLVMCatIntrinsics::uIntToString(unsigned int number)
{
	return Tools::StringConstants<Configuration::CatString>::makeString(number);
}


Configuration::CatString LLVMCatIntrinsics::int64ToString(int64_t number)
{
	return Tools::StringConstants<Configuration::CatString>::makeString(number);
}


Configuration::CatString LLVMCatIntrinsics::uInt64ToString(uint64_t number)
{
	return Tools::StringConstants<Configuration::CatString>::makeString(number);
}


Configuration::CatString LLVMCatIntrinsics::intToPrettyString(int number)
{
	Configuration::CatStringStream conversion;
	conversion << number;
	Configuration::CatString numberString = conversion.str();
	size_t numberLength = numberString.length();
	int numParts = (((int)numberLength - 1) / 3) + 1;	// so that 1-3 results in 1, 4-6 in 2, etc
	Configuration::CatString result;
	Configuration::CatString separator;// to skip first space, cleaner result

	for (int i = 0; i < numParts; ++i)
	{
		int substringFirstIndex = (int)numberLength - (3 * (i + 1));
		int substringLength = 3;
		if (substringFirstIndex < 0)
		{
			// if only 2 digits are left, substringFirstIndex will be -1, and substringLength will need to be 2
			// if only 1 digit is left, substringFirstIndex is -2, and substringLength will need to be 1
			substringLength += substringFirstIndex;
			substringFirstIndex = 0;
		}
		result = numberString.substr((unsigned int)substringFirstIndex, (unsigned int)substringLength) + separator + result;
		separator = Tools::StringConstants<Configuration::CatString>::comma;
	}
	return result;
}


Configuration::CatString LLVMCatIntrinsics::intToFixedLengthString(int number, int stringLength)
{
	Configuration::CatStringStream conversion;
	conversion << number;
	Configuration::CatString numberString = conversion.str();
	while ((int)numberString.length() < stringLength)
	{
		numberString = Tools::StringConstants<Configuration::CatString>::zeroStr + numberString;
	}
	return numberString;
}


Configuration::CatString LLVMCatIntrinsics::roundFloatToString(float number, int decimals)
{
	Configuration::CatStringStream ss;
	ss.precision(decimals);
	ss.setf(std::ios_base::fixed);
	ss.unsetf(std::ios_base::scientific);
	ss << number;
	Configuration::CatString result = ss.str();
	return formatRoundString(result);
}


Configuration::CatString LLVMCatIntrinsics::roundDoubleToString(double number, int decimals)
{
	Configuration::CatStringStream ss;
	ss.precision(decimals);
	ss.setf(std::ios_base::fixed);
	ss.unsetf(std::ios_base::scientific);
	ss << number;
	Configuration::CatString result = ss.str();
	return formatRoundString(result);
}


Configuration::CatString LLVMCatIntrinsics::formatRoundString(const Configuration::CatString& result)
{
	int discardedCharacters = 0;
	if (result.find(Tools::StringConstants<Configuration::CatString>::dot) != result.npos)
	{
		for (int i = (int)result.length() - 1; i >= 0; i--)
		{
			if (result[(unsigned int)i] == Tools::StringConstants<Configuration::CatString>::zero)
			{
				discardedCharacters++;
			}
			else if (result[(unsigned int)i] == Tools::StringConstants<Configuration::CatString>::dot)
			{
				discardedCharacters++;
				break;
			}
			else
			{
				break;
			}
		}
	}
	return result.substr(0, result.length() - discardedCharacters);
}
