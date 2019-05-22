/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatLinkNode.h"
#include "jitcat/CatLog.h"

using namespace jitcat::AST;
using namespace jitcat::Tools;


CatLinkNode::CatLinkNode(CatASTNode* me, CatASTNode* next, const jitcat::Tokenizer::Lexeme& lexeme):
	CatASTNode(lexeme),
	me(me),
	next(next)
{
}


jitcat::AST::CatLinkNode::CatLinkNode(const CatLinkNode& other):
	CatASTNode(other),
	me(other.me->copy()),
	next(other.next->copy())
{
}


CatASTNode* jitcat::AST::CatLinkNode::copy() const
{
	return new CatLinkNode(*this);
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


CatASTNodeType CatLinkNode::getNodeType() const
{
	return CatASTNodeType::LinkedList;
}


CatASTNode* jitcat::AST::CatLinkNode::releaseMe()
{
	return me.release();
}


CatASTNode* jitcat::AST::CatLinkNode::releaseNext()
{
	return next.release();
}
