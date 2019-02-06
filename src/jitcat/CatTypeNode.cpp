#include "jitcat/CatTypeNode.h"
#include "jitcat/CatLog.h"

using namespace jitcat;
using namespace jitcat::AST;
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
