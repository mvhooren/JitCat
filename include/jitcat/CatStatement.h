#pragma once

#include "CatASTNode.h"

#include <any>

namespace jitcat::AST
{

	class CatStatement: public CatASTNode
	{
	public:
		CatStatement(const Tokenizer::Lexeme& lexeme): CatASTNode(lexeme) {}
		virtual ~CatStatement() {};
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) = 0;
		virtual std::any execute(jitcat::CatRuntimeContext* runtimeContext) = 0;
	};

};
