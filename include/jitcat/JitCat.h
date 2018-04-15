/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class CatGrammar;
class CatRuntimeContext;
class CatTokenizer;
class Document;
class SLRParser;
struct SLRParseResult;

class JitCat
{
	JitCat();
	~JitCat();
public:
	static JitCat* get();
	SLRParseResult* parse(Document* expression, CatRuntimeContext* context) const;
	
private:
	static JitCat* instance;

	CatGrammar* grammar;
	CatTokenizer* tokenizer;
	SLRParser* parser;
};