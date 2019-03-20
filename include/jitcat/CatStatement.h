#pragma once

#include "CatASTNode.h"

namespace jitcat::AST
{

	class CatStatement: public CatASTNode
	{
	public:
		CatStatement() {}
		virtual ~CatStatement() {};
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) = 0;
	};

};
