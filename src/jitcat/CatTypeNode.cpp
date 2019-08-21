#include "jitcat/CatTypeNode.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatStaticScope.h"
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


jitcat::AST::CatTypeNode::CatTypeNode(CatStaticScope* parentScope, const std::string& name, const Tokenizer::Lexeme& lexeme):
	CatASTNode(lexeme),
	ownershipSemantics(TypeOwnershipSemantics::Value),
	name(name),
	parentScope(parentScope),
	knownType(false),
	isArrayType(false)
{
}


jitcat::AST::CatTypeNode::CatTypeNode(CatTypeNode* arrayItemType, Reflection::TypeOwnershipSemantics arrayOwnership, const Tokenizer::Lexeme& lexeme):
	CatASTNode(lexeme),
	ownershipSemantics(arrayOwnership),
	knownType(arrayItemType->isKnownType()),
	isArrayType(true),
	arrayItemType(arrayItemType)
{
	if (knownType)
	{
		setType(CatGenericType(&ArrayManipulator::createArrayManipulatorOf(arrayItemType->getType()), true, false).toPointer(ownershipSemantics, false, false));
	}
}


jitcat::AST::CatTypeNode::CatTypeNode(const CatTypeNode& other):
	CatASTNode(other),
	name(other.name),
	ownershipSemantics(other.ownershipSemantics),
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
					setType(CatGenericType(typeInfo, true, false));
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
			if (itemGenericType.isPointerToReflectableObjectType() && itemGenericType.getOwnershipSemantics() != TypeOwnershipSemantics::Value)
			{
				itemGenericType = itemGenericType.toWritable();
			}
			if (!itemGenericType.isConstructible())
			{
				errorManager->compiledWithError(Tools::append("Invalid array because ", itemGenericType.toString(), " is not constructible."), this, compileTimeContext->getContextName(), getLexeme());
				return false;
			}
			if (itemGenericType.isPointerToReflectableObjectType() 
				&& itemGenericType.getOwnershipSemantics() != TypeOwnershipSemantics::Owned 
				&& itemGenericType.getOwnershipSemantics() != TypeOwnershipSemantics::Value)
			{
				itemGenericType = itemGenericType.convertPointerToHandle();
			}

			setType(CatGenericType(&ArrayManipulator::createArrayManipulatorOf(itemGenericType), true, false).toPointer(ownershipSemantics, true, false));
		}
	}

	return true;
}


void CatTypeNode::setOwnershipSemantics(Reflection::TypeOwnershipSemantics ownership)
{
	ownershipSemantics = ownership;
}
