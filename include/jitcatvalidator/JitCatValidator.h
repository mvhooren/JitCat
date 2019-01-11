/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the JITCATVALIDATOR_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// JITCATVALIDATOR_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#if defined(WIN32)
#ifdef JITCATVALIDATOR_EXPORTS
#define JITCATVALIDATOR_API __declspec(dllexport)
#else
#define JITCATVALIDATOR_API __declspec(dllimport)
#endif
#else 
#define JITCATVALIDATOR_API
#endif

#include <cstddef>

//Result structure that should be passed to the validateExpression function
struct ValidationResult
{
	bool isValid;
	bool isLiteral;
	bool isConstant;
	std::size_t errorOffset;
	const char* errorMessage;
	const char* typeName;
};


struct CodeCompletionSuggestion
{
	const char* newExpression;
	const char* autoCompletionValue;
	bool isPrefixSuggestion;
	std::size_t newCursorPosition;
};

struct CodeCompletionSuggestions
{
	//Array of objects
	CodeCompletionSuggestion** suggestions;
	std::size_t numSuggestions;
};

//Validates the expression and fills the result structure with information about the expression.
//Type information for use of variables can be loaded using the loadTypeInfo function.
//Various type names can be passed for variable lookup. These types should all exist in a previously loaded type info file.
//Return codes:
//1:  expression compilation succeeded
//0:  expression compilation failed, see error message in the result structure
//-1: result was null
//-2: One of the provided types does not exist
extern "C" JITCATVALIDATOR_API int validateExpression(const char* expression, const char* globalsTypeName, 
										   const char* localsTypeName, const char* customLocalsTypeName, 
										   const char* customGlobalsTypeName, ValidationResult* result);

//Frees any memory allocated by the result
extern "C" JITCATVALIDATOR_API void destroyValidationResult(ValidationResult* result);

//Returns code completions suggestions based on an expression and a cursor position within the expression.
//Various type names can be passed for variable lookup. These types should all exist in a previously loaded type info file.
//Return codes:
//1:  one or more code suggestions exist
//0:  no code suggestions could be found
//-1: results was null
//-2: One of the provided types does not exist
extern "C" JITCATVALIDATOR_API int codeCompleteExpression(const char* expression, int cursorPosition, const char* globalsTypeName, 
											   const char* localsTypeName, const char* customLocalsTypeName, 
											   const char* customGlobalsTypeName, CodeCompletionSuggestions* results);

//Frees any memory allocated by the codeCompleteExpression function in the CodeCompletionSuggestions struct;
extern "C" JITCATVALIDATOR_API void destroyCompletionResult(CodeCompletionSuggestions* result);

//Loads a type information XML file that was previously exported from the TypeRegistry
//This will erase all previously loaded type information
//Return codes:
//1:  type info was loaded successfully
//0:  failed to load the type info from the provided path. File may not exist or may be in a bad format.
extern "C" JITCATVALIDATOR_API int loadTypeInfo(const char* path);
