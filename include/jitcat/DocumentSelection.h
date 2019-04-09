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