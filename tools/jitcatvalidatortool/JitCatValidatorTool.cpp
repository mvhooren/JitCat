/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include <algorithm>
#include <iostream>
#include <JitCatValidator.h>
#include <locale>
#include <string>


int main(int argc, char* argv[])
{
    if (argc == 7 || argc == 8)
	{
		std::string typeInformationPath = argv[1];
		std::string globalTypeName = argv[2];
		std::string localTypeName = argv[3];
		std::string customLocalsTypeName = argv[4];
		std::string customGlobalsTypeName = argv[5];
		std::string expression = argv[6];

		bool isCodeCompletion = argc == 8;
		int cursorPos = 0;
		if (isCodeCompletion)
		{
			cursorPos = atoi(argv[7]);
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
			int error = validateExpression(expression.c_str(), globalTypeName.c_str(), localTypeName.c_str(), customLocalsTypeName.c_str(), customGlobalsTypeName.c_str(), &result);
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
			int error = codeCompleteExpression(expression.c_str(), cursorPos, globalTypeName.c_str(), localTypeName.c_str(), customLocalsTypeName.c_str(), customGlobalsTypeName.c_str(), &results);
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
		std::cout << "Usage: <PathToTypeInformation> <GlobalsTypeName> <LocalsTypeName> <CustomLocalsTypeName> <CustomGlobalsTypeName> <Expression>\n";
		std::cout << "To not use type information, use \"none\"\n";
	}
}