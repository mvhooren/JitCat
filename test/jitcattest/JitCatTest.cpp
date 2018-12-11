/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "JitCat.h"
#include "AutoCompletion.h"
#include "CatASTNodes.h"
#include "CatGrammar.h"
#include "CatLog.h"
#include "CatRuntimeContext.h"
#include "CatTokenizer.h"
#include "CommentToken.h"
#include "CustomTypeInfo.h"
#include "CustomTypeInstance.h"
#include "Document.h"
#include "Expression.h"
#include "IdentifierToken.h"
#include "Lexeme.h"
#include "LLVMCodeGenerator.h"
#include "LLVMCompileTimeContext.h"
#include "ObjectMemberReference.h"
#include "OneCharToken.h"
#include "SLRParser.h"
#include "ParseToken.h"
#include "ReflectionTestObject.h"
#include "ReflectionTestObject2.h"
#include "ReflectionTestRoot.h"
#include "Timer.h"
#include "Tools.h"
#include "TypeRegistry.h"
#include "WhitespaceToken.h"

#include <iomanip>
#include <string>
#ifdef WIN32
	#include <tchar.h>
	#define MAIN _tmain
#else
	#define MAIN main
#endif
#include <vector>
#include <iostream>

class LogListener: public CatLogListener
{
public:
	virtual void catLog(const char* message)
	{
		std::cout << message;
	}
};

template<typename T>
void testExpression(CatRuntimeContext& context, const std::string& expression)
{
	Expression<T> testFloatExpression(&context, expression);
	T value = testFloatExpression.getValue(&context);
	T valueInterpreted = testFloatExpression.getInterpretedValue(&context);

	const int iterations = 10000;
	Timer timer;
	timer.getTimeSincePreviousCall();
	for (int i = 0; i < iterations; i++)
	{
		valueInterpreted = testFloatExpression.getInterpretedValue(&context);
		if (valueInterpreted != value)
		{
			assert(false);
			std::cout << "ERROR!\n";
		}
	}
	float interpretedTime = timer.getTimeSincePreviousCall();
	for (int i = 0; i < iterations; i++)
	{
		value = testFloatExpression.getValue(&context);
		if (valueInterpreted != value)
		{
			assert(false);
			std::cout << "ERROR!\n";
		}
	}
	float nativeTime = timer.getTimeSincePreviousCall();
	for (int i = 0; i < iterations; i++)
	{
		value = testFloatExpression.getValue2(&context);
		if (valueInterpreted != value)
		{
			assert(false);
			std::cout << "ERROR!\n";
		}
	}
	float nativeTime2 = timer.getTimeSincePreviousCall();
	std::cout << "Testing '" << expression << "': native: " << value << " interpreted: " << valueInterpreted << " " << iterations << " iterations, native time: " << std::setprecision(9) << nativeTime << " interpreted time: " << std::setprecision(9) << interpretedTime << " (" << interpretedTime / nativeTime <<  "x) native2 time: " << std::setprecision(9) << nativeTime2 << " (" << interpretedTime / nativeTime2 <<  "x)\n";


	if (value == valueInterpreted)
	{
		std::cout << "PASSED\n";
	}
	else
	{
		std::cout << "FAILED\n";
	}
}


int MAIN(int argc, char* argv[])
{
	TypeRegistry::get()->registerType<ReflectionTestRoot>();
	TypeRegistry::get()->registerType<ReflectionTestObject>();
	TypeRegistry::get()->registerType<ReflectionTestObject2>();

	LogListener* listener = new LogListener();
	CatLog::addListener(listener);

	CatTokenizer tokenizer;
	Document* document = nullptr;
	OneCharToken* eofToken = nullptr;
	SLRParseResult* result = nullptr;
	std::vector<ParseToken*> tokens;

	CatGrammar grammar(&tokenizer);

	ReflectionTestObject2* localObject = new ReflectionTestObject2();
	localObject->aLot = 101010101.0f;
	localObject->what = "I can't hear you!";

	const char* customRootName = "GlobalsRoot";
	CustomTypeInfo* globalsRoot = new CustomTypeInfo(customRootName);
	CustomTypeInstance* globalsInstance = globalsRoot->getDefaultInstance();

	const char* customTypeName = "MyType";
	CustomTypeInfo* customTypeGlobals = new CustomTypeInfo(customTypeName);
	TypeRegistry::get()->registerType(customTypeName, customTypeGlobals);
	customTypeGlobals->addFloatMember("globalFloat", 666.0f);
	customTypeGlobals->addIntMember("globalInt", 43);
	customTypeGlobals->addStringMember("globalString", "ThisIsAString");
	CustomTypeInstance* typeGlobalsInstance = customTypeGlobals->createInstance();
	globalsRoot->addIntMember("globalInt", 42);
	globalsRoot->addFloatMember("globalFloat", 66.0f);
	globalsRoot->addObjectMember(customTypeName, customTypeName,
								 new ObjectMemberReference<CustomTypeInstance>(typeGlobalsInstance, nullptr, customTypeGlobals));
	
	const char* customTypeName2 = "MyType2";
	CustomTypeInfo* customType = new CustomTypeInfo(customTypeName2);
	TypeRegistry::get()->registerType(customTypeName2, customType);
	customType->addFloatMember("floaty", 0.001f);
	customType->addIntMember("anInt", 54321);
	customType->addStringMember("aString", "lolel");
	customType->addBoolMember("amItrue", true);
	customType->addObjectMember("anObject", ReflectionTestObject2::getTypeName(), 
								new ObjectMemberReference<ReflectionTestObject2>(localObject, nullptr, TypeRegistry::get()->getTypeInfo(ReflectionTestObject2::getTypeName())));
	CustomTypeInstance* typeInstance = customType->createInstance();

	ExpressionErrorManager errorManager;
	CatRuntimeContext context(TypeRegistry::get()->getTypeInfo("Root"), nullptr, customType, globalsRoot, "myObject", true, &errorManager);
	context.setCustomThisReference(new ObjectMemberReference<CustomTypeInstance>(typeInstance, nullptr, customType));
	context.setCustomGlobalsReference(new ObjectMemberReference<CustomTypeInstance>(globalsInstance, nullptr, globalsRoot));

	ReflectionTestRoot root;
	ObjectMemberReference<ReflectionTestRoot>* rootReference = new ObjectMemberReference<ReflectionTestRoot>(&root, nullptr, TypeRegistry::get()->getTypeInfo("Root"));
	context.setGlobalReference(rootReference);

	SLRParser* parser = grammar.createSLRParser();
	
	delete result;
	delete document;
	Tools::deleteElements(tokens);
	
	int functionNr = 0;
	testExpression<float>(context, "pi");
	testExpression<float>(context, "floaty");
	testExpression<float>(context, "pi * 2.0f");
	testExpression<float>(context, "42.0f");
	testExpression<float>(context, "test.aFloat + test2.list[0].aLot");
	testExpression<std::string>(context, "\"test\"");
	testExpression<std::string>(context, "test.list[0].what + \" \" + test.text");
	testExpression<std::string>(context, "\"hello \" + hello");
	testExpression<std::string>(context, "test3.getTest2().getWhat()");
	testExpression<std::string>(context, "test.map[test2.test2.getWhat()].what");
	testExpression<bool>(context, "no || (yes && amITrue)");
	testExpression<int>(context, "two + anInt - test.theInt");
	testExpression<float>(context, "test.map[\"second\"].aLot");
	testExpression<float>(context, "test.map[0].aLot");
	
	
	
	while (true)
	{
		std::string inputText;
		std::cout << "Type an expression: ";
		std::getline(std::cin, inputText);
		std::cout << "\n";
		if (inputText == "exit" || inputText == "quit")
		{
			break;
		}
		else if (inputText.find("auto:") == 0)
		{
			auto completion = AutoCompletion::autoComplete(inputText.substr(5), inputText.size() - 5, &context);
			for (auto& iter : completion)
			{
				std::cout << iter.autoCompletionValue << ": " << iter.newExpression << "\n";
			}
		}
		else
		{
			document = new Document(inputText.c_str(), inputText.size());
			tokens.clear();
			eofToken = new OneCharToken(new Lexeme(document, document->getDocumentSize(), 0), OneChar::Eof);
			tokenizer.tokenize(document, tokens, eofToken);
			result = parser->parse(tokens, WhitespaceToken::getID(), CommentToken::getID(), &context);
			if (result->success)
			{
				std::cout << "\nParse Success!\n\n";
				CatTypedExpression* expression = static_cast<CatTypedExpression*>(result->astRootNode);
				expression->print();
				std::cout << "\n\n";
				//std::cout << "Expression type check: " << expression->typeCheck() << "\n";
				std::cout << "Expression const: " << expression->isConst() << "\n";
				std::cout << "Const collapsed:\n";

				CatTypedExpression* newExpression = expression->constCollapse(&context);
				if (newExpression != expression)
				{
					delete expression;
					expression = newExpression;
					result->astRootNode = newExpression;
				}

				CatGenericType valueType = expression->typeCheck();

				std::cout << "\tType:" << valueType.toString() << "\n\tValue: ";
				expression->print();
				std::cout << "\n";
				if (valueType.isValidType())
				{
					LLVMCodeGenerator generator;
					LLVMCompileTimeContext llvmContext(&context);
					std::string functionName = Tools::append("test", functionNr);
					generator.generateAndDump(expression, &llvmContext, functionName);
					generator.compileAndTest(&context, functionName);
					functionNr++;

					std::cout << "\nExecute:\n";
					CatValue result = expression->execute(&context);
					std::cout << "\tType:" << toString(result.getValueType()) << "\n\tValue: ";
					result.printValue();
				}
				std::cout << "\n\n";
			}
			else
			{
				std::cout << "\nFailed!\n\n";
				std::cout << result->errorMessage << "\n";
				std::cout << inputText << "\n";
				std::cout << std::string(result->errorPosition, ' ') << "^\n\n";
			}
			delete result;
			delete document;
			Tools::deleteElements(tokens);
		}
	}
	delete parser;
	return 0;
}

