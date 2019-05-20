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
	customType(new CustomTypeInfo("__ParameterPack"))
{
	for (auto& iter : parameterDeclarations)
	{
		parameters.emplace_back(iter);
	}
}


CatFunctionParameterDefinitions::~CatFunctionParameterDefinitions()
{
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


CatASTNodeType CatFunctionParameterDefinitions::getNodeType()
{
	return CatASTNodeType::FunctionParameterDefinitions;
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
		else
		{
			 const CatTypeNode& parameterTypeNode = iter->getType();
			 const std::string& parameterName = iter->getName();
			 const CatGenericType& parameterType = parameterTypeNode.getType();
			 customType->addMember(parameterName, parameterType);
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
