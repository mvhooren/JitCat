#include "LLVMCatIntrinsics.h"
#include "CatRuntimeContext.h"
#include "MemberReference.h"
#include "Tools.h"


Reflectable* LLVMCatIntrinsics::getThisPointerFromContext(CatRuntimeContext* context)
{
	return context->getThisReference().getPointer()->getParentObject();
}


Reflectable* LLVMCatIntrinsics::getCustomThisPointerFromContext(CatRuntimeContext* context)
{
	return context->getCustomThisReference().getPointer()->getParentObject();
}


bool LLVMCatIntrinsics::stringEquals(const std::string& left, const std::string& right)
{
	return left == right;
}


bool LLVMCatIntrinsics::stringNotEquals(const std::string& left, const std::string& right)
{
	return left != right;
}


std::string LLVMCatIntrinsics::stringAppend(const std::string& left, const std::string& right)
{
	return left + right;
}


std::string LLVMCatIntrinsics::floatToString(float number)
{
	return Tools::makeString(number);
}


std::string LLVMCatIntrinsics::intToString(int number)
{
	return Tools::makeString(number);
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


void LLVMCatIntrinsics::stringCopyConstruct(std::string* destination, const std::string& string)
{
	new (destination) std::string(string);
}


void LLVMCatIntrinsics::stringDestruct(std::string* target)
{
	target->~basic_string();
}


int LLVMCatIntrinsics::findInString(const std::string& text, const std::string& textToFind)
{
	std::size_t pos = text.find(textToFind);
	if (pos == text.npos)
	{
		return -1;
	}
	else
	{
		return (int)pos;
	}
}


std::string LLVMCatIntrinsics::replaceInString(const std::string& text, const std::string& textToFind, const std::string& replacement)
{
	if (text != "")
	{
		std::string newString = text;
		size_t startPosition = 0;
		while ((startPosition = newString.find(textToFind, startPosition)) != std::string::npos)
		{
			newString.replace(startPosition, textToFind.length(), replacement);
			startPosition += replacement.length(); 
		}
		return newString;
	}
	return text;
}


int LLVMCatIntrinsics::stringLength(const std::string& text)
{
	return (int)text.size();
}


std::string LLVMCatIntrinsics::subString(const std::string& text, int start, int length)
{
	if (text.size() == 0)
	{
		return "";
	}
	else if ((int)text.size() > start && start >= 0)
	{
		return text.substr((unsigned int)start, (unsigned int)length);
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
