/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatStaticIdentifier.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/ReflectedTypeInfo.h"
#include "jitcat/TypeInfo.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;


CatStaticIdentifier::CatStaticIdentifier(CatTypeNode* baseType, const Tokenizer::Lexeme& nameLexeme, const Tokenizer::Lexeme& lexeme):
	CatAssignableExpression(lexeme),
	baseType(baseType),
	idBaseType(nullptr),
	name(nameLexeme),
	nameLexeme(nameLexeme),
	type(CatGenericType::unknownType),
	assignableType(CatGenericType::unknownType),
	memberInfo(nullptr),
	nestedType(nullptr)
{
}


CatStaticIdentifier::CatStaticIdentifier(CatStaticIdentifier* idBaseType, const Tokenizer::Lexeme& nameLexeme, const Tokenizer::Lexeme& lexeme):
	CatAssignableExpression(lexeme),
	baseType(nullptr),
	idBaseType(idBaseType),
	name(nameLexeme),
	nameLexeme(nameLexeme),
	type(CatGenericType::unknownType),
	assignableType(CatGenericType::unknownType),
	memberInfo(nullptr),
	nestedType(nullptr)
{
}


CatStaticIdentifier::CatStaticIdentifier(const CatStaticIdentifier& other):
	CatAssignableExpression(other),
	baseType(other.baseType != nullptr ? static_cast<CatTypeNode*>(other.baseType->copy()) : nullptr),
	idBaseType(other.idBaseType != nullptr ? static_cast<CatStaticIdentifier*>(other.idBaseType->copy()) : nullptr),
	name(other.name),
	nameLexeme(other.nameLexeme),
	type(other.type),
	assignableType(other.assignableType),
	memberInfo(nullptr),
	nestedType(nullptr)
{
}


CatASTNode* CatStaticIdentifier::copy() const
{
	return new CatStaticIdentifier(*this);
}


const CatGenericType& CatStaticIdentifier::getType() const
{
	return type;
}


const CatGenericType& CatStaticIdentifier::getAssignableType() const
{
	return assignableType;
}


bool jitcat::AST::CatStaticIdentifier::isAssignable() const
{
	return !isNestedType() && !type.isConst();
}


void CatStaticIdentifier::print() const
{
	baseType->print();
	Tools::CatLog::log("::", name);
}


bool CatStaticIdentifier::isConst() const
{
	return isNestedType() || type.isConst();
}


CatTypedExpression* CatStaticIdentifier::constCollapse(CatRuntimeContext* compileTimeContext)
{
	return this;
}


CatASTNodeType CatStaticIdentifier::getNodeType() const
{
	return CatASTNodeType::StaticIdentifier;
}


std::any CatStaticIdentifier::execute(CatRuntimeContext* runtimeContext)
{
	assert(isStaticMember());
	return memberInfo->getMemberReference();
}


std::any CatStaticIdentifier::executeAssignable(CatRuntimeContext* runtimeContext)
{
	assert(isStaticMember());
	return memberInfo->getAssignableMemberReference();
}


bool CatStaticIdentifier::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	std::string lowerName = Tools::toLowerCase(name);
	memberInfo = nullptr;
	nestedType = nullptr;
	type = CatGenericType::unknownType;
	assignableType = CatGenericType::unknownType;
	TypeInfo* typeInfo = nullptr;
	if (baseType != nullptr)
	{
		if (!baseType->typeCheck(compiletimeContext, errorManager, errorContext))
		{
			return false;
		}
		const CatGenericType& scopeType = baseType->getType();

		if (scopeType == nullptr || !scopeType.isReflectableObjectType())
		{
			errorManager->compiledWithError(std::string("Not a static scope: ") + baseType->getTypeName(), errorContext, compiletimeContext->getContextName(), baseType->getLexeme());
			return false;
		}
		typeInfo = scopeType.getObjectType();
	}
	else if (idBaseType != nullptr)
	{
		if (!idBaseType->typeCheck(compiletimeContext, errorManager, errorContext))
		{
			return false;
		}
		if (idBaseType->nestedType != nullptr)
		{
			typeInfo = idBaseType->nestedType;
		}
		else
		{
			errorManager->compiledWithError(std::string("Not a static scope: ") + idBaseType->name, errorContext, compiletimeContext->getContextName(), idBaseType->nameLexeme);
			return false;
		}
	}
	else
	{
		assert(false);
	}

	memberInfo = typeInfo->getStaticMemberInfo(lowerName);

	if (memberInfo != nullptr)
	{
		type = memberInfo->catType;
		if (type.isPointerType() && type.getOwnershipSemantics() == TypeOwnershipSemantics::Value)
		{
			type.setOwnershipSemantics(TypeOwnershipSemantics::Weak);
		}
		assignableType = type.toPointer(TypeOwnershipSemantics::Weak, type.isWritable(), false);
	}
	else
	{
		//Static member does not exist. 
		//Perhaps it is a nested type.
		nestedType = typeInfo->getTypeInfo(name);
	}
	if ((memberInfo == nullptr && nestedType == nullptr)
		|| (memberInfo != nullptr && (!type.isValidType() || !assignableType.isValidType())))
	{
		errorManager->compiledWithError(std::string("Nested type or static member not found: ") + name, errorContext, compiletimeContext->getContextName(), getLexeme());
		return false;
	}
	return true;
}


bool CatStaticIdentifier::isNestedType() const
{
	return nestedType != nullptr;
}


bool CatStaticIdentifier::isStaticMember() const
{
	return memberInfo != nullptr;
}


const Reflection::StaticMemberInfo* CatStaticIdentifier::getMemberInfo() const
{
	return memberInfo;
}


const Reflection::TypeInfo* jitcat::AST::CatStaticIdentifier::getTypeInfo() const
{
	return nestedType;
}


const std::string& CatStaticIdentifier::getName() const
{
	return name;
}
