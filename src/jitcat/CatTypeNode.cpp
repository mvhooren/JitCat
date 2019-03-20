#include "jitcat/CatTypeNode.h"
#include "jitcat/CatLog.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/TypeRegistry.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


CatTypeNode::CatTypeNode(const CatGenericType& type):
	type(type),
	knownType(true)
{
}


jitcat::AST::CatTypeNode::CatTypeNode(const std::string& name):
	name(name),
	knownType(false)
{
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


void CatTypeNode::print() const
{
	CatLog::log(getTypeName());
}


CatASTNodeType CatTypeNode::getNodeType()
{
	return CatASTNodeType::TypeName;
}


void jitcat::AST::CatTypeNode::setType(const CatGenericType& newType)
{
	type = newType;
}


bool jitcat::AST::CatTypeNode::typeCheck(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	//Check the return type
	if (!isKnownType())
	{
		TypeInfo* typeInfo = TypeRegistry::get()->getTypeInfo(getTypeName());
		if (typeInfo == nullptr)
		{
			errorManager->compiledWithError(Tools::append("Type not found: ", getTypeName()), this);
			return false;
		}
		else
		{
			setType(CatGenericType(typeInfo));
		}
	}
	return true;
}