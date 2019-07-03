#include "jitcat/CatTypeNode.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/ContainerManipulator.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/TypeRegistry.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


CatTypeNode::CatTypeNode(const CatGenericType& type, const Tokenizer::Lexeme& lexeme) :
	CatASTNode(lexeme),
	type(type),
	ownershipSemantics(type.getOwnershipSemantics()),
	knownType(true),
	isArrayType(false)
{
}


jitcat::AST::CatTypeNode::CatTypeNode(const std::string& name, Reflection::TypeOwnershipSemantics ownershipSemantics, const Tokenizer::Lexeme& lexeme):
	CatASTNode(lexeme),
	ownershipSemantics(ownershipSemantics),
	name(name),
	knownType(false),
	isArrayType(false)
{
}


jitcat::AST::CatTypeNode::CatTypeNode(CatTypeNode* arrayItemType, Reflection::TypeOwnershipSemantics arrayOwnership, const Tokenizer::Lexeme& lexeme):
	CatASTNode(lexeme),
	ownershipSemantics(arrayOwnership),
	knownType(false),
	isArrayType(true),
	arrayItemType(arrayItemType)
{
}


jitcat::AST::CatTypeNode::CatTypeNode(const CatTypeNode& other):
	CatASTNode(other),
	name(other.name),
	ownershipSemantics(other.ownershipSemantics)
{
	knownType = name != "";
	if (knownType)
	{
		type = other.type;
	}
}


CatTypeNode::~CatTypeNode()
{
}


bool jitcat::AST::CatTypeNode::isKnownType() const
{
	return knownType;
}


std::string jitcat::AST::CatTypeNode::getTypeName() const
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


CatASTNode* jitcat::AST::CatTypeNode::copy() const
{
	return new CatTypeNode(*this);
}


void CatTypeNode::print() const
{
	CatLog::log(getTypeName());
}


CatASTNodeType CatTypeNode::getNodeType() const
{
	return CatASTNodeType::TypeName;
}


void jitcat::AST::CatTypeNode::setType(const CatGenericType& newType)
{
	type = newType;
	knownType = true;
}


bool jitcat::AST::CatTypeNode::typeCheck(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	//Check the return type
	if (!isKnownType())
	{
		if (!isArrayType)
		{
			CatScopeID typeScope = InvalidScopeID;
			TypeInfo* typeInfo = compileTimeContext->findType(Tools::toLowerCase(getTypeName()), typeScope);
			if (typeInfo == nullptr)
			{
				typeInfo = TypeRegistry::get()->getTypeInfo(getTypeName());
			}
			if (typeInfo == nullptr)
			{
				errorManager->compiledWithError(Tools::append("Type not found: ", getTypeName()), this, compileTimeContext->getContextName(), getLexeme());
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
					setType(CatGenericType(typeInfo, false, false));
				}
			}
		}
		else
		{
			if (!arrayItemType->typeCheck(compileTimeContext, errorManager, errorContext))
			{
				return false;
			}
			CatGenericType itemGenericType = arrayItemType->getType();
			if (itemGenericType.isPointerToReflectableObjectType() 
				&& itemGenericType.getOwnershipSemantics() != TypeOwnershipSemantics::Owned 
				&& itemGenericType.getOwnershipSemantics() != TypeOwnershipSemantics::Value)
			{
				itemGenericType = itemGenericType.convertPointerToHandle();
			}

			setType(CatGenericType(ContainerType::Array, &ArrayManipulator::createArrayManipulatorOf(itemGenericType), false, false));
		}
	}
	return true;
}