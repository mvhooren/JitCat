/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatInheritanceDefinition.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatScope.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/MemberInfo.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeInfo.h"

using namespace jitcat;
using namespace jitcat::AST;


CatInheritanceDefinition::CatInheritanceDefinition(CatTypeNode* typeNode, const Tokenizer::Lexeme& nameLexeme, const Tokenizer::Lexeme& lexeme):
	CatDefinition(lexeme),
	nameLexeme(nameLexeme),
	type(typeNode),
	inheritedMember(nullptr)
{
}


CatInheritanceDefinition::CatInheritanceDefinition(const CatInheritanceDefinition& other):
	CatDefinition(other),
	nameLexeme(other.nameLexeme),
	type(static_cast<CatTypeNode*>(other.type->copy())),
	inheritedMember(nullptr)
{
}


CatInheritanceDefinition::~CatInheritanceDefinition()
{
	if (errorManagerHandle.getIsValid())
	{
		static_cast<ExpressionErrorManager*>(errorManagerHandle.get())->errorSourceDeleted(this);
	}
}


CatASTNode* CatInheritanceDefinition::copy() const
{
	return new CatInheritanceDefinition(*this);
}


void CatInheritanceDefinition::print() const
{
	Tools::CatLog::log(Tools::append("inherits ", nameLexeme));
}


CatASTNodeType CatInheritanceDefinition::getNodeType() const
{
	return CatASTNodeType::InheritanceDefinition;
}


bool CatInheritanceDefinition::typeGatheringCheck(CatRuntimeContext* compileTimeContext)
{
	return true;
}


bool CatInheritanceDefinition::defineCheck(CatRuntimeContext* compiletimeContext, std::vector<const CatASTNode*>& loopDetectionStack)
{
	if (errorManagerHandle.getIsValid())
	{
		static_cast<ExpressionErrorManager*>(errorManagerHandle.get())->errorSourceDeleted(this);
		errorManagerHandle = nullptr;
	}
	ExpressionErrorManager* errorManager = compiletimeContext->getErrorManager();
	errorManagerHandle = errorManager;
	loopDetectionStack.push_back(this);
	bool result = type->defineCheck(compiletimeContext, errorManager, this, loopDetectionStack);
	loopDetectionStack.pop_back();
	if (!result)
	{
		return false;
	}
	const CatGenericType& inheritedType = type->getType();
	if (!inheritedType.isReflectableObjectType())
	{
		errorManager->compiledWithError(Tools::append("Inheritance only supports object types, ", inheritedType.toString(), " not supported."), this, compiletimeContext->getContextName(), getLexeme());
		return false;
	}
	else if (!inheritedType.getObjectType()->getAllowInheritance())
	{
		errorManager->compiledWithError(Tools::append("Inheritance from, ", inheritedType.toString(), " is not allowed."), this, compiletimeContext->getContextName(), getLexeme());
		return false;
	}

	CatScope* currentScope = compiletimeContext->getCurrentScope();
	if (currentScope != nullptr)
	{
		inheritedMember = currentScope->getCustomType()->addMember(Tools::append("$", inheritedType.toString()), type->getType());
		inheritedMember->setVisibility(Reflection::MemberVisibility::Hidden);
	}
	return true;
}


bool CatInheritanceDefinition::typeCheck(CatRuntimeContext* compiletimeContext)
{
	return true;
}


bool CatInheritanceDefinition::postTypeCheck(CatRuntimeContext* compileTimeContext)
{
	if (errorManagerHandle.getIsValid())
	{
		static_cast<ExpressionErrorManager*>(errorManagerHandle.get())->errorSourceDeleted(this);
		errorManagerHandle = nullptr;
	}
	ExpressionErrorManager* errorManager = compileTimeContext->getErrorManager();
	errorManagerHandle = errorManager;
	const CatGenericType& inheritedType = type->getType();
	if (!inheritedType.getObjectType()->inheritTypeCheck(compileTimeContext, compileTimeContext->getCurrentClass(), errorManager, this))
	{
		return false;
	}
	return true;
}


CatGenericType CatInheritanceDefinition::getType() const
{
	if (type != nullptr && type->isKnownType())
	{
		return type->getType();
	}
	return CatGenericType::unknownType;
}


Reflection::TypeMemberInfo* CatInheritanceDefinition::getInheritedMember() const
{
	return inheritedMember;
}
