/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/Lexeme.h"

namespace jitcat::AST
{

	class ASTNode
	{
	public:
		ASTNode(const Tokenizer::Lexeme& lexeme): lexeme(lexeme) {};
		virtual ~ASTNode() {};
		const Tokenizer::Lexeme& getLexeme() {return lexeme;};
		void setLexeme(const Tokenizer::Lexeme& lexeme_) {lexeme = lexeme_;}
		//virtual const Tokenizer::Lexeme& getTotalLexeme() = 0;

	protected:
		Tokenizer::Lexeme lexeme;
	};

} //End namespace jitcat::AST