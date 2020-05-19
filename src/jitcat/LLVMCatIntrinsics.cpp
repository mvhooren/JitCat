#include "jitcat/LLVMCatIntrinsics.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/Configuration.h"
#include "jitcat/Reflectable.h"
#include "jitcat/Tools.h"

#include <cmath>

using namespace jitcat;
using namespace jitcat::LLVM;
using namespace jitcat::Reflection;


Reflectable* LLVMCatIntrinsics::getScopePointerFromContext(CatRuntimeContext* context, int scopeId)
{
	return reinterpret_cast<Reflectable*>(context->getScopeObject((CatScopeID)scopeId));
}


bool LLVMCatIntrinsics::stringEquals(const Configuration::CatString& left, const Configuration::CatString& right)
{
	return left == right;
}


bool LLVMCatIntrinsics::stringNotEquals(const Configuration::CatString& left, const Configuration::CatString& right)
{
	return left != right;
}


void LLVMCatIntrinsics::stringAssign(Configuration::CatString* left, const Configuration::CatString& right)
{
	(*left) = right;
}


bool LLVMCatIntrinsics::stringToBoolean(const Configuration::CatString& value)
{
	return value == Tools::StringConstants<Configuration::CatString>::trueStr || Tools::StringConstants<Configuration::CatString>::stringToInt(value) > 0;
}


Configuration::CatString LLVMCatIntrinsics::stringAppend(const Configuration::CatString& left, const Configuration::CatString& right)
{
	return left + right;
}


Configuration::CatString jitcat::LLVM::LLVMCatIntrinsics::boolToString(bool boolean)
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


Configuration::CatString jitcat::LLVM::LLVMCatIntrinsics::doubleToString(double number)
{
	return Tools::StringConstants<Configuration::CatString>::makeString(number);
}


Configuration::CatString LLVMCatIntrinsics::floatToString(float number)
{
	return Tools::StringConstants<Configuration::CatString>::makeString(number);
}

double jitcat::LLVM::LLVMCatIntrinsics::stringToDouble(const Configuration::CatString& string)
{
	return Tools::convert<double>(string);
}


float LLVMCatIntrinsics::stringToFloat(const Configuration::CatString& string)
{
	return Tools::convert<float>(string);
}


Configuration::CatString LLVMCatIntrinsics::intToString(int number)
{
	return Tools::StringConstants<Configuration::CatString>::makeString(number);
}


int LLVMCatIntrinsics::stringToInt(const Configuration::CatString& string)
{
	return Tools::convert<int>(string);
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


int LLVMCatIntrinsics::findInString(const Configuration::CatString* text, const Configuration::CatString* textToFind)
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


Configuration::CatString LLVMCatIntrinsics::replaceInString(const Configuration::CatString* text, const Configuration::CatString* textToFind, const Configuration::CatString* replacement)
{
	if (text == nullptr || textToFind == nullptr || replacement == nullptr)
	{
		return Configuration::CatString();
	}
	if (*text != Configuration::CatString())
	{
		Configuration::CatString newString = *text;
		size_t startPosition = 0;
		while ((startPosition = newString.find(*textToFind, startPosition)) != Configuration::CatString::npos)
		{
			newString.replace(startPosition, textToFind->length(), *replacement);
			startPosition += replacement->length(); 
		}
		return newString;
	}
	return *text;
}


int LLVMCatIntrinsics::stringLength(const Configuration::CatString* text)
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


Configuration::CatString LLVMCatIntrinsics::subString(const Configuration::CatString* text, int start, int length)
{
	if (text == nullptr || text->size() == 0)
	{
		return Configuration::CatString();
	}
	else if ((int)text->size() > start && start >= 0)
	{
		return text->substr((unsigned int)start, (unsigned int)length);
	}
	else
	{
		return Configuration::CatString();
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


Configuration::CatString LLVMCatIntrinsics::roundFloatToString(float number, int decimals)
{
	Configuration::CatStringStream ss;
	ss.precision(decimals);
	ss.setf(std::ios_base::fixed);
	ss.unsetf(std::ios_base::scientific);
	ss << number;
	Configuration::CatString result = ss.str();
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


void jitcat::LLVM::LLVMCatIntrinsics::placementCopyConstructType(Reflectable* target, Reflectable* source, Reflection::TypeInfo* type)
{
	type->copyConstruct(reinterpret_cast<unsigned char*>(target), type->getTypeSize(), reinterpret_cast<unsigned char*>(source), type->getTypeSize());
}


void jitcat::LLVM::LLVMCatIntrinsics::placementConstructType(Reflectable* address, Reflection::TypeInfo* type)
{
	type->placementConstruct(reinterpret_cast<unsigned char*>(address), type->getTypeSize());
}


void jitcat::LLVM::LLVMCatIntrinsics::placementDestructType(Reflectable* address, TypeInfo* type)
{
	type->placementDestruct(reinterpret_cast<unsigned char*>(address), type->getTypeSize());
}
