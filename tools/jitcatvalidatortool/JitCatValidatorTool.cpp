/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include <algorithm>
#include <iostream>
#include <jitcatvalidator/JitCatValidator.h>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

std::string replaceAll(std::string str, const std::string& from, const std::string& to) 
{
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) 
	{
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}


int main(int argc, char* argv[])
{
    if (argc == 4 || argc == 5)
	{
		std::string typeInformationPath = argv[1];
		std::string rootScopeTypes = argv[2];
		std::string expression = argv[3];
		rootScopeTypes = replaceAll(rootScopeTypes, ":", " ");

		bool isCodeCompletion = argc == 4;
		int cursorPos = 0;
		if (isCodeCompletion)
		{
			cursorPos = atoi(argv[5]);
		}
		std::size_t length = typeInformationPath.length();
		for (std::size_t i = 0; i < length; i++)
		{
			typeInformationPath[i] = std::tolower(typeInformationPath[i], std::locale());
		}
		if (typeInformationPath != "none")
		{
			int errorResult = loadTypeInfo(typeInformationPath.c_str());
			if (errorResult == 0)
			{
				std::cout << "Failed to load type information.\n";
			}
		}
		if (!isCodeCompletion)
		{
			ValidationResult result;
			int error = validateExpression(expression.c_str(), rootScopeTypes.c_str(), &result);
			if (error == 1)
			{
				std::cout << "Success.\nconst: " << result.isConstant << "\nliteral: " << result.isLiteral << "\ntypename: " << result.typeName << "\n";
			}
			else if (error == 0)
			{
				std::cout << "Compile error.\n" << result.errorMessage << "\noffset: " << result.errorOffset << "\n";
			}
			else if (error == -2)
			{
				std::cout << "A type was not found.";
			}
			destroyValidationResult(&result);
		}
		else
		{
			CodeCompletionSuggestions results;
			int error = codeCompleteExpression(expression.c_str(), cursorPos, rootScopeTypes.c_str(), &results);
			if (error == 1)
			{
				for (std::size_t i = 0; i < results.numSuggestions; i++)
				{
					std::cout << results.suggestions[i]->autoCompletionValue << "\n";
					std::cout << results.suggestions[i]->newExpression << "\n";
					std::cout << results.suggestions[i]->newCursorPosition << "\n";
					std::cout << results.suggestions[i]->isPrefixSuggestion << "\n\n";
				}
				destroyCompletionResult(&results);
			}
			else if (error == 0)
			{
				std::cout << "No suggestions found.";
			}
			else if (error == -2)
			{
				std::cout << "A type was not found.";
			}
		}
	}
	else
	{
		std::cout << "Invalid number of arguments.\n";
		std::cout << "Usage: <PathToTypeInformation> <RootScopeTypes> <Expression>\n";
		std::cout << "Where <RootScopeTypes> is a list of typenames separated by a colon.\n";
		std::cout << "To not use type information, use \"none\" for PathToTypeInformation\n";
	}
}