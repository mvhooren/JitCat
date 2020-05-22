/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


namespace jitcat::Tokenizer
{
	struct DocumentSelection
	{
		DocumentSelection(int startLine, int startCharacter, int endLine, int endCharacter): 
			startLine(startLine), 
			startCharacter(startCharacter), 
			endLine(endLine), 
			endCharacter(endCharacter) 
		{}

		int startLine;
		int startCharacter;
		int endLine;
		int endCharacter;
	};
}