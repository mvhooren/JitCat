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
#include "ObjectMemberReference.h"
#include "OneCharToken.h"
#include "SLRParser.h"
#include "ParseToken.h"
#include "ReflectionTestObject.h"
#include "ReflectionTestObject2.h"
#include "ReflectionTestRoot.h"
#include "Tools.h"
#include "TypeRegistry.h"
#include "WhitespaceToken.h"

#include <string>
#include <tchar.h>
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


int _tmain(int argc, _TCHAR* argv[])
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
	customTypeGlobals->addFloatMember("globalFloat", 66.0f);
	customTypeGlobals->addIntMember("globalInt", 2);
	customTypeGlobals->addStringMember("globalString", "ThisIsAString");
	CustomTypeInstance* typeGlobalsInstance = customTypeGlobals->createInstance();
	globalsRoot->addObjectMember(customTypeName, customTypeName,
								 new ObjectMemberReference<CustomTypeInstance>(typeGlobalsInstance, nullptr, customTypeGlobals));
	
	CustomTypeInfo* customType = new CustomTypeInfo(customTypeName);
	customType->addFloatMember("floaty", 0.001f);
	customType->addIntMember("anInt", 54321);
	customType->addStringMember("aString", "lolel");
	customType->addBoolMember("amItrue", true);
	customType->addObjectMember("anObject", ReflectionTestObject2::getTypeName(), 
								new ObjectMemberReference<ReflectionTestObject2>(localObject, nullptr, TypeRegistry::get()->getTypeInfo(ReflectionTestObject2::getTypeName())));
	CustomTypeInstance* typeInstance = customType->createInstance();



	CatRuntimeContext context(TypeRegistry::get()->getTypeInfo("Root"), nullptr, customType, globalsRoot, "myObject", true, nullptr);
	context.setCustomThisReference(new ObjectMemberReference<CustomTypeInstance>(typeInstance, nullptr, customType));
	context.setCustomGlobalsReference(new ObjectMemberReference<CustomTypeInstance>(globalsInstance, nullptr, globalsRoot));

	ReflectionTestRoot root;
	ObjectMemberReference<ReflectionTestRoot>* rootReference = new ObjectMemberReference<ReflectionTestRoot>(&root, nullptr, TypeRegistry::get()->getTypeInfo("Root"));
	context.setGlobalReference(rootReference);

	SLRParser* parser = grammar.createSLRParser();
	
	delete result;
	delete document;
	Tools::deleteElements(tokens);
	
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
				std::cout << "\nSuccess!\n\n";
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
				
				if (valueType.isValidType())
				{
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

