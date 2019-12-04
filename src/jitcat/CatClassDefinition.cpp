/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatClassDefinition.h"
#include "jitcat/CatAssignmentOperator.h"
#include "jitcat/CatFunctionDefinition.h"
#include "jitcat/CatFunctionParameterDefinitions.h"
#include "jitcat/CatIdentifier.h"
#include "jitcat/CatInheritanceDefinition.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatMemberFunctionCall.h"
#include "jitcat/CatOperatorNew.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatScopeBlock.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/CatVariableDefinition.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/Document.h"
#include "jitcat/ErrorContext.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/JitCat.h"
#include "jitcat/MemberInfo.h"
#include "jitcat/SLRParseResult.h"

#include <cassert>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


jitcat::AST::CatClassDefinition::CatClassDefinition(const std::string& name, std::vector<std::unique_ptr<CatDefinition>>&& definitions, const Tokenizer::Lexeme& lexeme, const Tokenizer::Lexeme& nameLexeme):
	CatDefinition(lexeme),
	name(name),
	nameLexeme(nameLexeme),
	definitions(std::move(definitions)),
	scopeId(InvalidScopeID),
	customType(makeTypeInfo<CustomTypeInfo>(this->name.c_str()))
{
	extractDefinitionLists();
}


jitcat::AST::CatClassDefinition::CatClassDefinition(const CatClassDefinition& other):
	CatDefinition(other),
	name(other.name),
	nameLexeme(other.nameLexeme),
	scopeId(InvalidScopeID),
	customType(makeTypeInfo<CustomTypeInfo>(this->name.c_str()))
{
	for (auto& iter : other.definitions)
	{
		definitions.emplace_back(static_cast<CatDefinition*>(iter->copy()));
	}
	extractDefinitionLists();
}


jitcat::AST::CatClassDefinition::~CatClassDefinition()
{
}


CatASTNode* jitcat::AST::CatClassDefinition::copy() const
{
	return new CatClassDefinition(*this);
}


void jitcat::AST::CatClassDefinition::print() const
{
	if (definitions.size() == 0)
	{
		CatLog::log("class ", name, "{}");
	}
	else
	{
		CatLog::log("class ", name, "\n{");
		for (auto& iter : definitions)
		{
			iter->print();
			CatLog::log("\n\n");
		}
		CatLog::log("}\n\n");
	}
}


CatASTNodeType jitcat::AST::CatClassDefinition::getNodeType() const
{
	return CatASTNodeType::ClassDefinition;
}


bool jitcat::AST::CatClassDefinition::typeCheck(CatRuntimeContext* compileTimeContext)
{
	CatClassDefinition* parentClass = compileTimeContext->getCurrentClass();
	compileTimeContext->setCurrentClass(this);
	bool noErrors = true;
	scopeId = compileTimeContext->addScope(customType.get(), nullptr, false);
	CatScope* previousScope = compileTimeContext->getCurrentScope();
	compileTimeContext->setCurrentScope(this);

	for (auto& iter : inheritanceDefinitions)
	{
		noErrors &= iter->typeCheck(compileTimeContext);
	}

	if (!previousScope->getCustomType()->addType(customType.get()))
	{
		compileTimeContext->getErrorManager()->compiledWithError(Tools::append("A type with name ", name, " already exists."), this, compileTimeContext->getContextName(), nameLexeme);
		noErrors = false;
	}

	for (auto& iter : classDefinitions)
	{
		noErrors &= iter->typeCheck(compileTimeContext);
	}

	for (auto& iter: variableDefinitions)
	{
		noErrors &= iter->typeCheck(compileTimeContext);;
	}

	if (noErrors)
	{
		noErrors &= generateConstructor(compileTimeContext);
		noErrors &= generateDestructor(compileTimeContext);
	}

	for (auto& iter: functionDefinitions)
	{
		noErrors &= iter->typeCheck(compileTimeContext);;
	}
	
	if (noErrors)
	{
		//Another pass to allow inheritance definitions to inspect the finalized, type-checked class.
		for (auto& iter : inheritanceDefinitions)
		{
			noErrors &= iter->postTypeCheck(compileTimeContext);
		}
	}

	compileTimeContext->removeScope(scopeId);
	compileTimeContext->setCurrentScope(previousScope);
	compileTimeContext->setCurrentClass(parentClass);

	if (noErrors)
	{
		compileTimeContext->getErrorManager()->compiledWithoutErrors(this);
	}
	
	return noErrors;
}


bool jitcat::AST::CatClassDefinition::isTriviallyCopyable() const
{
	for (const auto& iter : variableDefinitions)
	{
		if (!iter->getType().getType().isTriviallyCopyable())
		{
			return false;
		}
	}
	return true;
}


Reflection::CustomTypeInfo* jitcat::AST::CatClassDefinition::getCustomType()
{
	return customType.get();
}


CatScopeID jitcat::AST::CatClassDefinition::getScopeId() const
{
	return scopeId;
}


const std::string& jitcat::AST::CatClassDefinition::getClassName() const
{
	return name;
}


Tokenizer::Lexeme jitcat::AST::CatClassDefinition::getClassNameLexeme() const
{
	return nameLexeme;
}


CatVariableDefinition* jitcat::AST::CatClassDefinition::getVariableDefinitionByName(const std::string& name)
{
	for (auto& iter : variableDefinitions)
	{
		if (Tools::equalsWhileIgnoringCase(iter->getName(), name))
		{
			return iter;
		}
	}
	return nullptr;
}

CatFunctionDefinition* jitcat::AST::CatClassDefinition::getFunctionDefinitionByName(const std::string& name)
{
	for (auto& iter : functionDefinitions)
	{
		if (Tools::equalsWhileIgnoringCase(iter->getFunctionName(), name))
		{
			return iter;
		}
	}
	if (name == "__init" || name == "init")
	{
		return generatedConstructor.get();
	}
	return nullptr;
}


void jitcat::AST::CatClassDefinition::enumerateMemberVariables(std::function<void(const CatGenericType&, const std::string&)>& enumerator) const
{
	customType->enumerateMemberVariables(enumerator);
}


bool jitcat::AST::CatClassDefinition::injectCode(const std::string& functionName, const std::string& statement, CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	ErrorContext errorContextName(compileTimeContext, Tools::append("Injected code in ", functionName, ": ", statement));

	if (compileTimeContext->getCurrentScope() != this
		|| compileTimeContext->getCurrentClass() != this)
	{
		//code can only be injected while in the class' scope.
		assert(false);
	}

	Tokenizer::Document* previousDocument = errorManager->getCurrentDocument();

	CatFunctionDefinition* functionDefinition = getFunctionDefinitionByName(functionName);
	if (functionDefinition != nullptr)
	{
		Tokenizer::Document doc(statement);
		errorManager->setCurrentDocument(&doc);
		Parser::SLRParseResult* parseResult = JitCat::get()->parseStatement(&doc, compileTimeContext, errorManager, errorContext);
		if (parseResult->success)
		{
			CatStatement* statement = static_cast<CatStatement*>(parseResult->astRootNode.release());
			delete parseResult;

			//Build the context as it would have been in the function scope block
			CatScopeID functionParamsScopeId = InvalidScopeID;
			if (functionDefinition->getNumParameters() > 0)
			{
				functionParamsScopeId = compileTimeContext->addScope(functionDefinition->getParametersType(), nullptr, false);
			}
			compileTimeContext->setCurrentFunction(functionDefinition);
			
			if (!statement->typeCheck(compileTimeContext, errorManager, errorContext))
			{
				errorManager->setCurrentDocument(previousDocument);
				return false;
			}
			errorManager->setCurrentDocument(previousDocument);
			CatScopeBlock* functionEpilogBlock = functionDefinition->getOrCreateEpilogBlock(compileTimeContext, errorManager, errorContext);
			functionEpilogBlock->insertStatementFront(statement);

			compileTimeContext->removeScope(functionParamsScopeId);
			compileTimeContext->setCurrentFunction(nullptr);
			compileTimeContext->setCurrentScope(this);
		}
		else
		{
			errorManager->setCurrentDocument(previousDocument);
			return false;
		}
	}
	else
	{
		return false;
	}
	return true;
}


bool jitcat::AST::CatClassDefinition::generateConstructor(CatRuntimeContext* compileTimeContext)
{
	CatTypeNode* typeNode = new CatTypeNode(CatGenericType::voidType, nameLexeme);
	CatFunctionParameterDefinitions* parameters = new CatFunctionParameterDefinitions({}, nameLexeme);
	std::vector<CatStatement*> statements;
	for (auto& iter : inheritanceDefinitions)
	{
		TypeMemberInfo* inheritedMember = iter->getInheritedMember();
		TypeInfo* inheritedType = inheritedMember->catType.getPointeeType()->getObjectType();
		if (inheritedType->isCustomType())
		{
			//Call constructor for inherited type
			CatIdentifier* id = new CatIdentifier(inheritedMember->memberName, iter->getLexeme());
			std::string constructorName = "__init";
			if (inheritedType->getFirstMemberFunctionInfo("init") != nullptr)
			{
				constructorName = "init";
			}
			CatMemberFunctionCall* functionCall = new CatMemberFunctionCall(constructorName, iter->getLexeme(), id, new CatArgumentList(iter->getLexeme()), nameLexeme);
			statements.push_back(functionCall);
		}
	}
	for (auto& iter : variableDefinitions)
	{
		if (iter->getInitializationExpression() != nullptr)
		{
			CatTypedExpression* variableInitExpr = static_cast<CatTypedExpression*>(iter->getInitializationExpression()->copy());
			CatIdentifier* id = new CatIdentifier(iter->getName(), iter->getLexeme());
			CatAssignmentOperator* assignment = new CatAssignmentOperator(id, variableInitExpr, variableInitExpr->getLexeme(), variableInitExpr->getLexeme());
			statements.push_back(assignment);
		}
		else if (iter->getType().getType().isReflectableObjectType() 
			     && iter->getType().getType().getOwnershipSemantics() == TypeOwnershipSemantics::Value)
		{
			TypeInfo* dateMemberType = iter->getType().getType().getObjectType();
			if (dateMemberType->isCustomType())
			{
				//Call constructor for data member type
				CatIdentifier* id = new CatIdentifier(iter->getName(), iter->getLexeme());
				std::string constructorName = "__init";
				if (dateMemberType->getFirstMemberFunctionInfo("init") != nullptr)
				{
					constructorName = "init";
				}
				CatMemberFunctionCall* functionCall = new CatMemberFunctionCall(constructorName, iter->getLexeme(), id, new CatArgumentList(iter->getLexeme()), nameLexeme);
				statements.push_back(functionCall);
			}
		}
	}
	CatScopeBlock* scopeBlock = new CatScopeBlock(statements, nameLexeme);
	generatedConstructor = std::make_unique<CatFunctionDefinition>(typeNode, "__init", nameLexeme, parameters, scopeBlock, nameLexeme);
	generatedConstructor->setFunctionVisibility(MemberVisibility::Constructor);
	return generatedConstructor->typeCheck(compileTimeContext);

}


bool jitcat::AST::CatClassDefinition::generateDestructor(CatRuntimeContext* compileTimeContext)
{
	return true;
}


void jitcat::AST::CatClassDefinition::extractDefinitionLists()
{
	for (auto& iter : this->definitions)
	{
		switch (iter->getNodeType())
		{
		case CatASTNodeType::ClassDefinition:		classDefinitions.push_back(static_cast<CatClassDefinition*>(iter.get())); break;
		case CatASTNodeType::FunctionDefinition:	functionDefinitions.push_back(static_cast<CatFunctionDefinition*>(iter.get())); break;
		case CatASTNodeType::VariableDefinition:	variableDefinitions.push_back(static_cast<CatVariableDefinition*>(iter.get())); break;
		case CatASTNodeType::InheritanceDefinition: inheritanceDefinitions.push_back(static_cast<CatInheritanceDefinition*>(iter.get())); break;
		default:
			assert(false);
		}
	}
}
