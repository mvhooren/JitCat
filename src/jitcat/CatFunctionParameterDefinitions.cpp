/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatFunctionParameterDefinitions.h"
#include "jitcat/CatVariableDeclaration.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/CustomTypeInfo.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;


CatFunctionParameterDefinitions::CatFunctionParameterDefinitions(const std::vector<CatVariableDeclaration*>& parameterDeclarations, const Tokenizer::Lexeme& lexeme):
	CatASTNode(lexeme),
	customType(makeTypeInfo<CustomTypeInfo>("__ParameterPack", HandleTrackingMethod::None))
{
	for (auto& iter : parameterDeclarations)
	{
		parameters.emplace_back(iter);
	}
}


jitcat::AST::CatFunctionParameterDefinitions::CatFunctionParameterDefinitions(const CatFunctionParameterDefinitions& other):
	CatASTNode(other),
	customType(makeTypeInfo<CustomTypeInfo>("__ParameterPack", HandleTrackingMethod::None))
{
	for (auto& iter : other.parameters)
	{
		parameters.emplace_back(static_cast<CatVariableDeclaration*>(iter->copy()));
	}
}


CatFunctionParameterDefinitions::~CatFunctionParameterDefinitions()
{
}


CatASTNode* jitcat::AST::CatFunctionParameterDefinitions::copy() const
{
	return new CatFunctionParameterDefinitions(*this);
}


void CatFunctionParameterDefinitions::print() const
{
	bool addComma = false;
	for (auto& iter : parameters)
	{
		if (addComma)
		{
			Tools::CatLog::log(", ");
		}
		addComma = true;
		iter->print();
	}
}


CatASTNodeType CatFunctionParameterDefinitions::getNodeType() const
{
	return CatASTNodeType::FunctionParameterDefinitions;
}


bool jitcat::AST::CatFunctionParameterDefinitions::defineCheck(CatRuntimeContext* runtimeContext, ExpressionErrorManager* errorManager, void* errorContext, std::vector<const CatASTNode*>& loopDetectionStack)
{
	bool success = true;
	for (auto& iter : parameters)
	{
		if (!iter->defineCheck(runtimeContext, errorManager, errorContext))
		{
			success = false;
		}
	}
	return success;
}


bool jitcat::AST::CatFunctionParameterDefinitions::typeCheck(CatRuntimeContext* runtimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	bool success = true;
	for (auto& iter : parameters)
	{
		if (!iter->typeCheck(runtimeContext, errorManager, errorContext))
		{
			success = false;
		}
	}
	return success;
}


Reflection::CustomTypeInfo* jitcat::AST::CatFunctionParameterDefinitions::getCustomType() const
{
	return customType.get();
}


int jitcat::AST::CatFunctionParameterDefinitions::getNumParameters() const
{
	return (int)parameters.size();
}


const std::string& jitcat::AST::CatFunctionParameterDefinitions::getParameterName(int index) const
{
	return parameters[index]->getName();
}


const CatTypeNode* jitcat::AST::CatFunctionParameterDefinitions::getParameterType(int index) const
{
	return &(parameters[index]->getType());
}


const Tokenizer::Lexeme& jitcat::AST::CatFunctionParameterDefinitions::getParameterLexeme(int index) const
{
	return parameters[index]->getLexeme();
}
