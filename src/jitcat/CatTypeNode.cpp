/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatTypeNode.h"
#include "jitcat/CatClassDefinition.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/CatInheritanceDefinition.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatStaticScope.h"
#include "jitcat/CatVariableDefinition.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeRegistry.h"

#include <sstream>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


CatTypeNode::CatTypeNode(const CatGenericType& type, const Tokenizer::Lexeme& lexeme) :
	CatASTNode(lexeme),
	ownershipSemantics(type.getOwnershipSemantics()),
	type(type),
	knownType(true),
	isArrayType(false)
{
}


CatTypeNode::CatTypeNode(const std::string& name, Reflection::TypeOwnershipSemantics ownershipSemantics, const Tokenizer::Lexeme& lexeme):
	CatASTNode(lexeme),
	ownershipSemantics(ownershipSemantics),
	name(name),
	knownType(false),
	isArrayType(false)
{
}


CatTypeNode::CatTypeNode(CatStaticScope* parentScope, const std::string& name, const Tokenizer::Lexeme& lexeme):
	CatASTNode(lexeme),
	ownershipSemantics(TypeOwnershipSemantics::Value),
	name(name),
	knownType(false),
	isArrayType(false),
	parentScope(parentScope)
{
}


CatTypeNode::CatTypeNode(const CatTypeNode& other):
	CatASTNode(other),
	ownershipSemantics(other.ownershipSemantics),
	name(other.name),
	knownType(false),
	isArrayType(other.isArrayType)
{
	if (other.arrayItemType != nullptr)
	{
		arrayItemType.reset(static_cast<CatTypeNode*>(other.arrayItemType->copy()));
	}
	if (!isArrayType)
	{
		knownType = name == "";
	}
	else
	{
		knownType = arrayItemType->isKnownType();
	}
	if (knownType)
	{
		type = other.type;
	}
}


CatTypeNode::~CatTypeNode()
{
}


bool CatTypeNode::isKnownType() const
{
	return knownType;
}


std::string CatTypeNode::getTypeName() const
{
	if (knownType)
	{
		return type.toString();
	}
	else
	{
		return name;
	}
}


const CatGenericType& CatTypeNode::getType() const
{
	return type;
}


CatASTNode* CatTypeNode::copy() const
{
	return new CatTypeNode(*this);
}


void CatTypeNode::print() const
{
	if (parentScope != nullptr)
	{
		parentScope->print();
		CatLog::log("::");
	}
	CatLog::log(getTypeName());
}


CatASTNodeType CatTypeNode::getNodeType() const
{
	return CatASTNodeType::TypeName;
}


void CatTypeNode::setType(const CatGenericType& newType)
{
	type = newType;
	knownType = true;
}


bool CatTypeNode::defineCheck(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, 
							  void* errorContext, std::vector<const CatASTNode*>& loopDetectionStack)
{
	bool typeCheckResult = typeCheck(compileTimeContext, errorManager, errorContext);
	if (typeCheckResult && type.isReflectableObjectType())
	{
		TypeInfo* foundTypeInfo = type.getObjectType();
		if (foundTypeInfo->isCustomType())
		{
			CustomTypeInfo* customType = static_cast<CustomTypeInfo*>(foundTypeInfo);
			CatClassDefinition* classDefinition = customType->getClassDefinition();
			if (classDefinition != nullptr)
			{
				for (std::size_t i = 0; i < loopDetectionStack.size(); ++i)
				{
					if (loopDetectionStack[i] == classDefinition)
					{
						std::ostringstream errorStream;
						errorStream << "Recursive dependency detected.\n";
						for (i = i; i < loopDetectionStack.size(); ++i)
						{
							switch (loopDetectionStack[i]->getNodeType())
							{
								default: assert(false); break;
								case CatASTNodeType::ClassDefinition:
									errorStream << "\tIn class definition: " << static_cast<const CatClassDefinition*>(loopDetectionStack[i])->getClassName() << ".\n";
									break;
								case CatASTNodeType::VariableDefinition:
								{
									const CatVariableDefinition* variableDefinition = static_cast<const CatVariableDefinition*>(loopDetectionStack[i]);
									errorStream << "\tIn variable definition: " << variableDefinition->getName() << " of type: " << variableDefinition->getType().getType().toString() << ".\n";
									break;
								}
								case CatASTNodeType::InheritanceDefinition:
									errorStream << "\tIn inheritance definition of type " <<  static_cast<const CatInheritanceDefinition*>(loopDetectionStack[i])->getType().toString() << ".\n";
									break;
							}
						}
						errorManager->compiledWithError(errorStream.str(), errorContext, compileTimeContext->getContextName(), getLexeme());
						return false;
					}
				}
				return classDefinition->defineCheck(classDefinition->getCompiletimeContext(), loopDetectionStack);
			}

			return true;
		}
	}
	return typeCheckResult;
}


bool CatTypeNode::typeCheck(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	//Check the return type
	if (!isKnownType())
	{
		if (!isArrayType)
		{

			CatScopeID typeScope = InvalidScopeID;

			TypeInfo* typeInfo = nullptr;
			if (parentScope == nullptr)
			{
				typeInfo = compileTimeContext->findType(Tools::toLowerCase(getTypeName()), typeScope);
				if (typeInfo == nullptr)
				{
					typeInfo = TypeRegistry::get()->getTypeInfo(getTypeName());
				}
			}
			else
			{
				if (!parentScope->typeCheck(compileTimeContext, errorManager, errorContext))
				{
					return false;
				}
				else
				{
					typeInfo = parentScope->getScopeType();
				}
			}

			if (typeInfo == nullptr)
			{
				errorManager->compiledWithError(Tools::append("Type not found: ", getTypeName()), errorContext, compileTimeContext->getContextName(), getLexeme());
				return false;
			}
			else
			{
				if (ownershipSemantics != TypeOwnershipSemantics::Value)
				{
					setType(CatGenericType(CatGenericType(typeInfo), ownershipSemantics, false));
				}
				else
				{
					setType(CatGenericType(typeInfo, true, false));
				}
			}
		}
		else
		{
			errorManager->compiledWithError(Tools::append("Arrays are not supported for now."), errorContext, compileTimeContext->getContextName(), getLexeme());
			return false;
		}
	}

	return true;
}


void CatTypeNode::setOwnershipSemantics(Reflection::TypeOwnershipSemantics ownership)
{
	ownershipSemantics = ownership;
}


TypeInfo* jitcat::AST::CatTypeNode::findType(CatRuntimeContext* compileTimeContext, const std::string* typeName)
{
	return nullptr;
}
