/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatClassDefinition.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatAssignmentOperator.h"
#include "jitcat/CatConstruct.h"
#include "jitcat/CatDestruct.h"
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


CatClassDefinition::CatClassDefinition(const std::string& name, std::vector<std::unique_ptr<CatDefinition>>&& definitions, const Tokenizer::Lexeme& lexeme, const Tokenizer::Lexeme& nameLexeme):
	CatDefinition(lexeme),
	name(name),
	qualifiedName(name),
	nameLexeme(nameLexeme),
	checkStatus(CheckStatus::Unchecked),
	parentClass(nullptr),
	definitions(std::move(definitions)),
	scopeId(InvalidScopeID)
{
	customType = makeTypeInfo<CustomTypeInfo>(this, HandleTrackingMethod::ExternallyTracked);
	extractDefinitionLists();
}


CatClassDefinition::CatClassDefinition(const CatClassDefinition& other):
	CatDefinition(other),
	name(other.name),
	nameLexeme(other.nameLexeme),
	checkStatus(CheckStatus::Unchecked),
	parentClass(other.parentClass),
	scopeId(InvalidScopeID)
{
	customType = makeTypeInfo<CustomTypeInfo>(this, HandleTrackingMethod::ExternallyTracked);
	for (auto& iter : other.definitions)
	{
		definitions.emplace_back(static_cast<CatDefinition*>(iter->copy()));
	}
	extractDefinitionLists();
}


CatClassDefinition::~CatClassDefinition()
{
}


CatASTNode* CatClassDefinition::copy() const
{
	return new CatClassDefinition(*this);
}


void CatClassDefinition::print() const
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


CatASTNodeType CatClassDefinition::getNodeType() const
{
	return CatASTNodeType::ClassDefinition;
}


bool CatClassDefinition::typeGatheringCheck(CatRuntimeContext* compileTimeContext_)
{
	assert(checkStatus == CheckStatus::Unchecked);
	CatClassDefinition* parentClass = compileTimeContext_->getCurrentClass();

	compileTimeContext = compileTimeContext_->clone();
	compileTimeContext->setCurrentClass(this);
	scopeId = compileTimeContext->addDynamicScope(customType.get(), nullptr);
	CatScope* previousScope = compileTimeContext->getCurrentScope();

	compileTimeContext->setCurrentClass(parentClass);
	bool noErrors = true;

	if (!previousScope->getCustomType()->addType(customType.get()))
	{
		compileTimeContext->getErrorManager()->compiledWithError(Tools::append("A type with name ", name, " already exists."), this, compileTimeContext->getContextName(), nameLexeme);
		noErrors = false;
	}

	compileTimeContext->removeScope(scopeId);

	for (auto& iter : classDefinitions)
	{
		iter->setParentClass(this);
		noErrors &= iter->typeGatheringCheck(compileTimeContext_);
	}

	if (noErrors)
	{
		compileTimeContext->getErrorManager()->compiledWithoutErrors(this);
	}
	checkStatus = noErrors ? CheckStatus::Unsized : CheckStatus::Failed;
	return noErrors;
}


bool CatClassDefinition::defineCheck(CatRuntimeContext* compileTimeContext_, std::vector<const CatASTNode*>& loopDetectionStack)
{
	assert(compileTimeContext_ == compileTimeContext.get());
	if (checkStatus == CheckStatus::Failed)
	{
		return false;
	}
	if (checkStatus == CheckStatus::Sized)
	{
		return true;
	}
	assert(checkStatus == CheckStatus::Unsized);

	CatClassDefinition* parentClass = compileTimeContext->getCurrentClass();
	compileTimeContext->setCurrentClass(this);
	scopeId = compileTimeContext->addDynamicScope(customType.get(), nullptr);
	CatScope* previousScope = compileTimeContext->getCurrentScope();
	compileTimeContext->setCurrentScope(this);

	bool noErrors = true;

	loopDetectionStack.push_back(this);

	for (auto& iter : inheritanceDefinitions)
	{
		noErrors &= iter->defineCheck(compileTimeContext.get(), loopDetectionStack);
	}

	for (auto& iter: variableDefinitions)
	{
		noErrors &= iter->defineCheck(compileTimeContext.get(), loopDetectionStack);
	}
	
	if (noErrors)
	{
		noErrors &= defineConstructor(compileTimeContext.get());
		noErrors &= defineCopyConstructor(compileTimeContext.get());
		noErrors &= defineOperatorAssign(compileTimeContext.get());
		noErrors &= defineDestructor(compileTimeContext.get());
	}

	loopDetectionStack.pop_back();
	checkStatus = noErrors ? CheckStatus::Sized : CheckStatus::Failed;

	for (auto& iter: functionDefinitions)
	{
		iter->setParentClass(this);
		noErrors &= iter->defineCheck(compileTimeContext.get(), loopDetectionStack);
	}

	compileTimeContext->removeScope(scopeId);

	for (auto& iter : classDefinitions)
	{
		iter->setParentClass(this);
		noErrors &= iter->defineCheck(iter->getCompiletimeContext(), loopDetectionStack);
	}
	

	compileTimeContext->setCurrentScope(previousScope);
	compileTimeContext->setCurrentClass(parentClass);

	if (noErrors)
	{
		compileTimeContext->getErrorManager()->compiledWithoutErrors(this);
	}
	return noErrors;
}


bool CatClassDefinition::typeCheck(CatRuntimeContext* compileTimeContext_)
{
	assert(compileTimeContext_ == compileTimeContext.get());
	if (checkStatus == CheckStatus::Failed)
	{
		return false;
	}
	assert(checkStatus == CheckStatus::Sized);
	CatClassDefinition* parentClass = compileTimeContext->getCurrentClass();
	compileTimeContext->setCurrentClass(this);
	CatScopeID addedScopeId = compileTimeContext->addDynamicScope(customType.get(), nullptr);
	assert(scopeId == addedScopeId);
	(void)addedScopeId;
	CatScope* previousScope = compileTimeContext->getCurrentScope();
	compileTimeContext->setCurrentScope(this);

	bool noErrors = true;

	for (auto& iter : inheritanceDefinitions)
	{
		noErrors &= iter->typeCheck(compileTimeContext.get());
	}

	for (auto& iter: variableDefinitions)
	{
		noErrors &= iter->typeCheck(compileTimeContext.get());
	}

	if (noErrors)
	{
		noErrors &= generateConstructor(compileTimeContext.get());
		noErrors &= generateCopyConstructor(compileTimeContext.get());
		noErrors &= generateOperatorAssign(compileTimeContext.get());
		noErrors &= generateDestructor(compileTimeContext.get());
	}

	for (auto& iter: functionDefinitions)
	{
		iter->setParentClass(this);
		noErrors &= iter->typeCheck(compileTimeContext.get());
	}
	
	if (noErrors)
	{
		//Another pass to allow inheritance definitions to inspect the finalized, type-checked class.
		for (auto& iter : inheritanceDefinitions)
		{
			noErrors &= iter->postTypeCheck(compileTimeContext.get());
		}
	}

	compileTimeContext->removeScope(scopeId);

	for (auto& iter : classDefinitions)
	{
		iter->setParentClass(this);
		noErrors &= iter->typeCheck(iter->getCompiletimeContext());
	}
	compileTimeContext->setCurrentScope(previousScope);
	compileTimeContext->setCurrentClass(parentClass);

	if (noErrors)
	{
		if (!customType->setDefaultConstructorFunction("init"))
		{
			if (!customType->setDefaultConstructorFunction("__init"))
			{
				assert(false);
			}
		}
		if (!customType->setDestructorFunction("destroy"))
		{
			if (!customType->setDestructorFunction("__destroy"))
			{
				assert(false);
			}
		}
		compileTimeContext->getErrorManager()->compiledWithoutErrors(this);
	}
	checkStatus = noErrors ? CheckStatus::Succeeded : CheckStatus::Failed;
	return noErrors;
}


CatRuntimeContext* CatClassDefinition::getCompiletimeContext() const
{
	return compileTimeContext.get();
}


bool CatClassDefinition::isTriviallyCopyable() const
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


Reflection::CustomTypeInfo* CatClassDefinition::getCustomType() const
{
	return customType.get();
}


CatScopeID CatClassDefinition::getScopeId() const
{
	return scopeId;
}


const std::string& CatClassDefinition::getClassName() const
{
	return name;
}


const std::string& CatClassDefinition::getQualifiedName() const
{
	return qualifiedName;
}


Tokenizer::Lexeme CatClassDefinition::getClassNameLexeme() const
{
	return nameLexeme;
}


CatVariableDefinition* CatClassDefinition::getVariableDefinitionByName(const std::string& name) const
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

CatFunctionDefinition* CatClassDefinition::getFunctionDefinitionByName(const std::string& name) const
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
	else if (name == "__destroy" || name == "destroy")
	{
		return generatedDestructor.get();
	}
	return nullptr;
}


void CatClassDefinition::enumerateMemberVariables(std::function<void(const CatGenericType&, const std::string&)>& enumerator) const
{
	customType->enumerateMemberVariables(enumerator);
}


bool CatClassDefinition::injectCode(const std::string& functionName, const std::string& statement, CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
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
		std::unique_ptr<Parser::SLRParseResult> parseResult = JitCat::get()->parseStatement(doc, compileTimeContext, errorManager, errorContext);
		if (parseResult->success)
		{
			CatStatement* statement = static_cast<CatStatement*>(parseResult->astRootNode.release());

			//Build the context as it would have been in the function scope block
			CatScopeID functionParamsScopeId = InvalidScopeID;
			if (functionDefinition->getNumParameters() > 0)
			{
				functionParamsScopeId = compileTimeContext->addDynamicScope(functionDefinition->getParametersType(), nullptr);
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


void CatClassDefinition::setDylib(llvm::orc::JITDylib* generatedDylib)
{
	customType->setDylib(generatedDylib);
	for (auto& iter : classDefinitions)
	{
		iter->setDylib(generatedDylib);
	}
}


const std::vector<CatClassDefinition*>& CatClassDefinition::getClassDefinitions() const
{
	return classDefinitions;
}


const std::vector<CatFunctionDefinition*>& CatClassDefinition::getFunctionDefinitions() const
{
	return functionDefinitions;
}


const std::vector<CatVariableDefinition*>& CatClassDefinition::getVariableDefinitions() const
{
	return variableDefinitions;
}


const std::vector<CatInheritanceDefinition*>& CatClassDefinition::getInheritanceDefinitions() const
{
	return inheritanceDefinitions;
}


void CatClassDefinition::setParentClass(const CatClassDefinition* classDefinition)
{
	parentClass = classDefinition;
	qualifiedName = Tools::append(parentClass->getQualifiedName(), "::", name);
}


bool CatClassDefinition::defineConstructor(CatRuntimeContext* compileTimeContext)
{
	CatTypeNode* typeNode = new CatTypeNode(CatGenericType::voidType, nameLexeme);
	CatFunctionParameterDefinitions* parameters = new CatFunctionParameterDefinitions({}, nameLexeme);
	CatScopeBlock* scopeBlock = new CatScopeBlock({}, nameLexeme);
	generatedConstructor = std::make_unique<CatFunctionDefinition>(typeNode, "__init", nameLexeme, parameters, scopeBlock, nameLexeme);
	generatedConstructor->setParentClass(this);
	generatedConstructor->setFunctionVisibility(MemberVisibility::Constructor);
	std::vector<const CatASTNode*> loopDetection;
	return generatedConstructor->defineCheck(compileTimeContext, loopDetection);
}


bool CatClassDefinition::defineCopyConstructor(CatRuntimeContext* compileTimeContext)
{
	return true;
}


bool CatClassDefinition::defineDestructor(CatRuntimeContext* compileTimeContext)
{
	CatTypeNode* typeNode = new CatTypeNode(CatGenericType::voidType, nameLexeme);
	CatFunctionParameterDefinitions* parameters = new CatFunctionParameterDefinitions({}, nameLexeme);
	CatScopeBlock* scopeBlock = new CatScopeBlock({}, nameLexeme);
	generatedDestructor = std::make_unique<CatFunctionDefinition>(typeNode, "__destroy", nameLexeme, parameters, scopeBlock, nameLexeme);
	generatedDestructor->setParentClass(this);
	generatedDestructor->setFunctionVisibility(MemberVisibility::Constructor);
	std::vector<const CatASTNode*> loopDetection;
	return generatedDestructor->defineCheck(compileTimeContext, loopDetection);
}


bool CatClassDefinition::defineOperatorAssign(CatRuntimeContext* compileTimeContext)
{
	return true;
}


bool CatClassDefinition::generateConstructor(CatRuntimeContext* compileTimeContext)
{
	CatScopeBlock* scopeBlock = generatedConstructor->getScopeBlock();
	for (auto& iter : inheritanceDefinitions)
	{
		TypeMemberInfo* inheritedMember = iter->getInheritedMember();
		scopeBlock->addStatement(new CatConstruct(iter->getLexeme(), std::make_unique<CatIdentifier>(inheritedMember->getMemberName(), iter->getLexeme()), nullptr, false));
	}
	for (auto& iter : variableDefinitions)
	{
		std::unique_ptr<CatArgumentList> arguments;
		if (iter->getInitializationExpression() != nullptr)
		{
			CatTypedExpression* variableInitExpr = static_cast<CatTypedExpression*>(iter->getInitializationExpression()->copy());
			arguments = std::make_unique<CatArgumentList>(variableInitExpr->getLexeme(), std::vector<CatTypedExpression*>({variableInitExpr}));
		}
		scopeBlock->addStatement(new CatConstruct(iter->getLexeme(), std::make_unique<CatIdentifier>(iter->getName(), iter->getLexeme()), std::move(arguments), false));
	}
	return generatedConstructor->typeCheck(compileTimeContext);
}


bool CatClassDefinition::generateCopyConstructor(CatRuntimeContext* compileTimeContext)
{
	//First check that no copy constructor has been defined by the user.

	//Then check if all members are copy-constructible

	//Check if all members are trivially copyable and if so, generate a memcpy.
	//Otherwise, generate the copy constructor for each member.
	//assert(false);
	return true;
}


bool CatClassDefinition::generateDestructor(CatRuntimeContext* compileTimeContext)
{
	CatScopeBlock* scopeBlock = generatedDestructor->getScopeBlock();
	for (auto& iter : inheritanceDefinitions)
	{
		TypeMemberInfo* inheritedMember = iter->getInheritedMember();
		scopeBlock->addStatement(new CatDestruct(iter->getLexeme(), std::make_unique<CatIdentifier>(inheritedMember->getMemberName(), iter->getLexeme())));
	}
	for (auto& iter : variableDefinitions)
	{
		if (iter->getType().getType().isReflectableObjectType())
		{
			scopeBlock->addStatement(new CatDestruct(iter->getLexeme(), std::make_unique<CatIdentifier>(iter->getName(), iter->getLexeme())));
		}
	}
	return generatedDestructor->typeCheck(compileTimeContext);
}


bool CatClassDefinition::generateOperatorAssign(CatRuntimeContext* compileTimeContext)
{
	//First check that no operator= has been defined by the user.

	//Then check if all members have an operator= or copy contructor defined.

	//Check if all members are trivially copyable and if so, generate a memcpy.

	//Otherwise, generate the operator= or copy constructor for each member.
	//assert(false);
	return true;
}


void CatClassDefinition::extractDefinitionLists()
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
