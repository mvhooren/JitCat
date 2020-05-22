/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatTypeOrIdentifier.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatErrorExpression.h"
#include "jitcat/CatBuiltInFunctionCall.h"
#include "jitcat/CatFunctionOrConstructor.h"
#include "jitcat/CatIdentifier.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatMemberFunctionCall.h"
#include "jitcat/CatOperatorNew.h"
#include "jitcat/CatScopeFunctionCall.h"
#include "jitcat/CatStaticFunctionCall.h"
#include "jitcat/CatStaticIdentifier.h"
#include "jitcat/CatStaticScope.h"
#include "jitcat/CatTypeNode.h"

#include <cassert>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


CatTypeOrIdentifier::CatTypeOrIdentifier(CatTypeNode* type, Reflection::TypeOwnershipSemantics ownershipSemantics, const Tokenizer::Lexeme& lexeme):
	CatASTNode(lexeme),
	typeOrIdentifier(TypeOrIdentifier::Type),
	parentScope(nullptr),
	typeNode(type),
	identifier(""),
	identifierLexeme(identifier),
	ownershipSemantics(ownershipSemantics)
{
	typeNode->setOwnershipSemantics(ownershipSemantics);
}


CatTypeOrIdentifier::CatTypeOrIdentifier(const std::string& identifier, const Tokenizer::Lexeme& identifierLexeme, Reflection::TypeOwnershipSemantics ownershipSemantics, 
										 const Tokenizer::Lexeme& lexeme):
	CatASTNode(lexeme),
	typeOrIdentifier(ownershipSemantics == TypeOwnershipSemantics::None ? TypeOrIdentifier::Ambiguous : TypeOrIdentifier::Type),
	parentScope(nullptr),
	typeNode(nullptr),
	identifier(identifier),
	identifierLexeme(identifierLexeme),
	ownershipSemantics(ownershipSemantics)
{
}


CatTypeOrIdentifier::CatTypeOrIdentifier(CatStaticScope* parentScope, const std::string& identifier, const Tokenizer::Lexeme& identifierLexeme, Reflection::TypeOwnershipSemantics ownershipSemantics, const Tokenizer::Lexeme& lexeme):
	CatASTNode(lexeme),
	typeOrIdentifier(ownershipSemantics == TypeOwnershipSemantics::None ? TypeOrIdentifier::Ambiguous : TypeOrIdentifier::Type),
	parentScope(parentScope),
	typeNode(nullptr),
	identifier(identifier),
	identifierLexeme(identifierLexeme),
	ownershipSemantics(ownershipSemantics)
{
}


CatTypeOrIdentifier::CatTypeOrIdentifier(const CatTypeOrIdentifier& other):
	CatASTNode(other),
	typeOrIdentifier(other.typeOrIdentifier),
	parentScope(other.parentScope == nullptr ? nullptr : new CatStaticScope(*other.parentScope.get())),
	typeNode(other.typeNode == nullptr ? nullptr : new CatTypeNode(*other.typeNode.get())),
	identifier(other.identifier),
	identifierLexeme(other.identifierLexeme),
	ownershipSemantics(other.ownershipSemantics)
{
}


CatTypeOrIdentifier::~CatTypeOrIdentifier()
{
}


CatASTNode* CatTypeOrIdentifier::copy() const
{
	return new CatTypeOrIdentifier(*this);
}


void CatTypeOrIdentifier::print() const
{
	if (typeNode != nullptr)
	{
		typeNode->print();
		return;
	}

	switch (ownershipSemantics)
	{
		case TypeOwnershipSemantics::Weak:		CatLog::log("&");	break;
		case TypeOwnershipSemantics::Owned:		CatLog::log("@");	break;
		case TypeOwnershipSemantics::Value:		break;
		default:								assert(false);		break;
	}
	
	if (parentScope != nullptr)
	{
		parentScope->print();
		CatLog::log("::");
	}
	CatLog::log(identifier);
}


CatASTNodeType CatTypeOrIdentifier::getNodeType() const
{
	return CatASTNodeType::TypeOrIdentifier;
}


bool CatTypeOrIdentifier::isType() const
{
	return typeOrIdentifier == TypeOrIdentifier::Type;
}


bool CatTypeOrIdentifier::isAmbiguous() const
{
	return typeOrIdentifier == TypeOrIdentifier::Ambiguous;
}


bool jitcat::AST::CatTypeOrIdentifier::hasParentScope() const
{
	return parentScope != nullptr;
}


Reflection::TypeOwnershipSemantics jitcat::AST::CatTypeOrIdentifier::getOwnershipSemantics() const
{
	return ownershipSemantics;
}


CatIdentifier* CatTypeOrIdentifier::toIdentifier()
{
	return new CatIdentifier(identifier, lexeme);
}


CatStaticIdentifier* jitcat::AST::CatTypeOrIdentifier::toStaticIdentifier()
{
	return new CatStaticIdentifier(parentScope.release(), identifierLexeme, lexeme);
}


CatTypeNode* CatTypeOrIdentifier::toType()
{
	if (typeNode != nullptr)
	{
		return typeNode.release();
	}
	CatTypeNode* typeNode = new CatTypeNode(parentScope.release(), identifier, lexeme);
	if (ownershipSemantics == TypeOwnershipSemantics::None)
	{
		ownershipSemantics = TypeOwnershipSemantics::Value;
	}
	typeNode->setOwnershipSemantics(ownershipSemantics);
	return typeNode;
}


CatStaticScope* jitcat::AST::CatTypeOrIdentifier::toStaticScope()
{
	return new CatStaticScope(ownershipSemantics == TypeOwnershipSemantics::None && typeNode == nullptr, parentScope.release(), identifier, identifierLexeme, lexeme);
}


CatErrorExpression* jitcat::AST::CatTypeOrIdentifier::toErrorExpression(const std::string& errorMessage)
{
	std::string contents(lexeme);
	return new CatErrorExpression(contents, errorMessage, lexeme);
}


CatFunctionOrConstructor* jitcat::AST::CatTypeOrIdentifier::toFunctionOrConstructorCall(CatArgumentList* argumentList)
{
	return new CatFunctionOrConstructor(this, argumentList, lexeme);
}


CatASTNode* jitcat::AST::CatTypeOrIdentifier::toFunctionCall(CatArgumentList* argumentList)
{
	if (hasParentScope() && !isType())
	{
		return new CatStaticFunctionCall(parentScope.release(), identifier, argumentList, lexeme, identifierLexeme);
	}
	else if (!hasParentScope() && !isType())
	{
		if (CatBuiltInFunctionCall::isBuiltInFunction(identifier.c_str(), (int)argumentList->getNumArguments()))
		{
			return new CatBuiltInFunctionCall(identifier, identifierLexeme, argumentList, lexeme);
		}
		else
		{
			return new CatScopeFunctionCall(identifier,argumentList, lexeme, identifierLexeme);
		}	
	}
	else
	{
		return toErrorExpression("Invalid function call.");
	}
}


CatOperatorNew* jitcat::AST::CatTypeOrIdentifier::toConstructorCall(CatArgumentList* argumentList)
{
	return new CatOperatorNew(toType(), argumentList, lexeme);
}
