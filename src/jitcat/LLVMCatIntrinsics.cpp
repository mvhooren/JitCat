#include "jitcat/LLVMCatIntrinsics.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/Tools.h"

#include <cmath>

using namespace jitcat;
using namespace jitcat::LLVM;
using namespace jitcat::Reflection;


unsigned char* LLVMCatIntrinsics::getScopePointerFromContext(CatRuntimeContext* context, int scopeId)
{
	return context->getScopeObject((CatScopeID)scopeId);
}


bool LLVMCatIntrinsics::stringEquals(const std::string& left, const std::string& right)
{
	return left == right;
}


bool LLVMCatIntrinsics::stringNotEquals(const std::string& left, const std::string& right)
{
	return left != right;
}


void LLVMCatIntrinsics::stringAssign(std::string* left, const std::string& right)
{
	(*left) = right;
}


bool LLVMCatIntrinsics::stringToBoolean(const std::string& value)
{
	return value == "true" || atoi(value.c_str()) > 0;
}


std::string LLVMCatIntrinsics::stringAppend(const std::string& left, const std::string& right)
{
	return left + right;
}


std::string LLVMCatIntrinsics::floatToString(float number)
{
	return Tools::makeString(number);
}


float LLVMCatIntrinsics::stringToFloat(const std::string& string)
{
	return Tools::convert<float>(string);
}


std::string LLVMCatIntrinsics::intToString(int number)
{
	return Tools::makeString(number);
}


int LLVMCatIntrinsics::stringToInt(const std::string& string)
{
	return Tools::convert<int>(string);
}


std::string LLVMCatIntrinsics::intToPrettyString(int number)
{
	std::string numberString = Tools::makeString(number);
	size_t numberLength = numberString.length();
	int numParts = (((int)numberLength - 1) / 3) + 1;	// so that 1-3 results in 1, 4-6 in 2, etc
	std::string result = "";
	std::string separator = "";// to skip first space, cleaner result

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
		separator = ",";
	}
	return result;
}


std::string LLVMCatIntrinsics::intToFixedLengthString(int number, int stringLength)
{
	std::string numberString = Tools::makeString(number);
	while ((int)numberString.length() < stringLength)
	{
		numberString = "0" + numberString;
	}
	return numberString;
}


void LLVMCatIntrinsics::stringEmptyConstruct(std::string* destination)
{
	new (destination) std::string();
}


void LLVMCatIntrinsics::stringCopyConstruct(std::string* destination, const std::string* string)
{
	if (string != nullptr)
	{
		new (destination) std::string(*string);
	}
	else
	{
		new (destination) std::string("");
	}
}


void LLVMCatIntrinsics::stringDestruct(std::string* target)
{
	if (target != nullptr)
	{
		target->~basic_string();
	}
}


int LLVMCatIntrinsics::findInString(const std::string* text, const std::string* textToFind)
{
	if (text == nullptr || textToFind == nullptr)
	{
		return -1;
	}
	std::size_t pos = text->find(*textToFind);
	if (pos == text->npos)
	{
		return -1;
	}
	else
	{
		return (int)pos;
	}
}


std::string LLVMCatIntrinsics::replaceInString(const std::string* text, const std::string* textToFind, const std::string* replacement)
{
	if (text == nullptr || textToFind == nullptr || replacement == nullptr)
	{
		return "";
	}
	if (*text != "")
	{
		std::string newString = *text;
		size_t startPosition = 0;
		while ((startPosition = newString.find(*textToFind, startPosition)) != std::string::npos)
		{
			newString.replace(startPosition, textToFind->length(), *replacement);
			startPosition += replacement->length(); 
		}
		return newString;
	}
	return *text;
}


int LLVMCatIntrinsics::stringLength(const std::string* text)
{
	if (text != nullptr)
	{
		return (int)text->size();
	}
	else
	{
		return 0;
	}
}


std::string LLVMCatIntrinsics::subString(const std::string* text, int start, int length)
{
	if (text == nullptr || text->size() == 0)
	{
		return "";
	}
	else if ((int)text->size() > start && start >= 0)
	{
		return text->substr((unsigned int)start, (unsigned int)length);
	}
	else
	{
		return "";
	}
}

float LLVMCatIntrinsics::getRandomFloat()
{
	return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
}


bool LLVMCatIntrinsics::getRandomBoolean(bool first, bool second)
{
	return (std::rand() % 2) == 1 ? first : second;
}


int LLVMCatIntrinsics::getRandomInt(int min, int max)
{
	if (min > max)
	{
		std::swap(min, max);
	}
	return min + (std::rand() % (max - min + 1));
}


float LLVMCatIntrinsics::getRandomFloatRange(float min, float max)
{
	if (min > max)
	{
		std::swap(min, max);
	}
	float random = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
	return min + random * (max - min);
}


float LLVMCatIntrinsics::roundFloat(float number, int decimals)
{
	double multiplier = std::pow(10.0f, decimals);
	return (float)(std::floor(number * multiplier + 0.5f) / multiplier);
}


std::string LLVMCatIntrinsics::roundFloatToString(float number, int decimals)
{
	std::stringstream ss;
	ss.precision(decimals);
	ss.setf(std::ios_base::fixed);
	ss.unsetf(std::ios_base::scientific);
	ss << number;
	std::string result = ss.str();
	int discardedCharacters = 0;
	if (result.find('.') != result.npos)
	{
		for (int i = (int)result.length() - 1; i >= 0; i--)
		{
			if (result[(unsigned int)i] == '0')
			{
				discardedCharacters++;
			}
			else if (result[(unsigned int)i] == '.')
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


void jitcat::LLVM::LLVMCatIntrinsics::placementCopyConstructType(unsigned char* target, unsigned char* source, Reflection::TypeInfo* type)
{
	type->copyConstruct(target, type->getTypeSize(), source, type->getTypeSize());
}


void jitcat::LLVM::LLVMCatIntrinsics::placementConstructType(unsigned char* address, Reflection::TypeInfo* type)
{
	type->placementConstruct(address, type->getTypeSize());
}


void jitcat::LLVM::LLVMCatIntrinsics::placementDestructType(unsigned char* address, TypeInfo* type)
{
	type->placementDestruct(address, type->getTypeSize());
}
