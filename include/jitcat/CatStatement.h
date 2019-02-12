#pragma once

#include "CatASTNode.h"

namespace jitcat::AST
{

	class CatStatement: public CatASTNode
	{
	public:
		CatStatement() {}
		virtual ~CatStatement() {};
	};

};
