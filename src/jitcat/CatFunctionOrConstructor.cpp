/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/
#include "jitcat/CatFunctionOrConstructor.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatBuiltInFunctionCall.h"
#include "jitcat/CatOperatorNew.h"
#include "jitcat/CatMemberFunctionCall.h"
#include "jitcat/CatStaticFunctionCall.h"
#include "jitcat/CatTypeOrIdentifier.h"

using namespace jitcat;
using namespace jitcat::AST;


CatFunctionOrConstructor::CatFunctionOrConstructor(CatTypeOrIdentifier* typeOrIdentifier, CatArgumentList* argumentList, const Tokenizer::Lexeme& lexeme):
	CatASTNode(lexeme),
	typeOrIdentifier(typeOrIdentifier),
	argumentList(argumentList)

{
}


CatFunctionOrConstructor::CatFunctionOrConstructor(const CatFunctionOrConstructor& other):
	CatASTNode(other),
	typeOrIdentifier(other.typeOrIdentifier == nullptr ? nullptr : new CatTypeOrIdentifier(*other.typeOrIdentifier.get())),
	argumentList(other.argumentList == nullptr ? nullptr : new CatArgumentList(*other.argumentList.get()))
{
}


CatFunctionOrConstructor::~CatFunctionOrConstructor()
{
}


CatASTNode* CatFunctionOrConstructor::copy() const
{
	return new CatFunctionOrConstructor(*this);
}


void CatFunctionOrConstructor::print() const
{
	typeOrIdentifier->print();
	argumentList->print();
}


CatASTNodeType CatFunctionOrConstructor::getNodeType() const
{
	return CatASTNodeType::FunctionOrConstructorCall;
}


CatASTNode* CatFunctionOrConstructor::toConstructorCall()
{
	return typeOrIdentifier->toConstructorCall(argumentList.release());
}


CatASTNode* CatFunctionOrConstructor::toFunctionCall()
{
	return typeOrIdentifier->toFunctionCall(argumentList.release());
}
