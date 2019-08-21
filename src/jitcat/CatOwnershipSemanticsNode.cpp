/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatOwnershipSemanticsNode.h"
#include "jitcat/CatLog.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


CatOwnershipSemanticsNode::CatOwnershipSemanticsNode(Reflection::TypeOwnershipSemantics ownershipSemantics, const Tokenizer::Lexeme& lexeme):
	CatASTNode(lexeme),
	ownershipSemantics(ownershipSemantics)
{
}

CatOwnershipSemanticsNode::CatOwnershipSemanticsNode(const CatOwnershipSemanticsNode& other):
	CatASTNode(other),
	ownershipSemantics(other.ownershipSemantics)
{
}


CatOwnershipSemanticsNode::~CatOwnershipSemanticsNode()
{
}


CatASTNode* CatOwnershipSemanticsNode::copy() const 
{
	return new CatOwnershipSemanticsNode(*this);
}


void CatOwnershipSemanticsNode::print() const
{
	switch (ownershipSemantics)
	{
		case TypeOwnershipSemantics::Owned:		break;
		case TypeOwnershipSemantics::Shared:	break;
		case TypeOwnershipSemantics::Weak:		CatLog::log("&"); break;
		case TypeOwnershipSemantics::Value:		CatLog::log("@"); break;
	}
}


CatASTNodeType CatOwnershipSemanticsNode::getNodeType() const
{
	return CatASTNodeType::OwnershipSemantics;
}


Reflection::TypeOwnershipSemantics jitcat::AST::CatOwnershipSemanticsNode::getOwnershipSemantics() const
{
	return ownershipSemantics;
}

