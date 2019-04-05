#pragma once

#include "CatASTNode.h"

#include <any>
#include <optional>

namespace jitcat::AST
{

	class CatStatement: public CatASTNode
	{
	public:
		CatStatement(const Tokenizer::Lexeme& lexeme): CatASTNode(lexeme) {}
		virtual ~CatStatement() {};
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) = 0;
		virtual std::any execute(jitcat::CatRuntimeContext* runtimeContext) = 0;
		virtual std::optional<bool> checkControlFlow(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext, bool& unreachableCodeDetected) const {return std::nullopt;}
	};

};
