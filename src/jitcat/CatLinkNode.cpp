/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CatLinkNode.h"
#include "CatLog.h"


CatLinkNode::CatLinkNode(CatASTNode* me, CatASTNode* next):
	me(me),
	next(next)
{
}


void CatLinkNode::print() const
{
	me->print();
	if (next != nullptr)
	{
		CatLog::log(", ");
		next->print();
	}
}


CatASTNodeType CatLinkNode::getNodeType()
{
	return CatASTNodeType::LinkedList;
}