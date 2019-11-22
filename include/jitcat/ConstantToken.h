/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ConstantTokenSubTypes.h"
#include "jitcat/ParseToken.h"

#include <cstddef>

namespace jitcat::Tokenizer
{

	class ConstantToken: public ParseToken
	{
	public:
		ConstantToken();
		ConstantToken(Lexeme* lexeme, ConstantType subType);
		virtual int getTokenID() const;
		virtual const char* getTokenName() const;
		virtual ParseToken* createIfMatch(Document* document, const char* currentPosition) const;
		virtual const char* getSubTypeName(int subType) const;
		virtual const char* getSubTypeSymbol(int subType) const;
		virtual int getTokenSubType() const;
	
	private:
		ConstantType parseConstant(const char* text, std::size_t textLength, std::size_t & offset) const;
		ConstantType parseIntOrFloat(const char* text, std::size_t textLength, std::size_t & offset) const;
		ConstantType parseFloatOrHexOrOct(const char* text, std::size_t textLength, std::size_t & offset) const;
		ConstantType parseFloatOrOct(const char* text, std::size_t textLength, std::size_t & offset) const;
		ConstantType parseFloat(const char* text, std::size_t textLength, std::size_t & offset, bool pastDot, bool pastExponent) const;
		ConstantType parseFloatWithExponent(const char* text, std::size_t textLength, std::size_t & offset, bool pastDot) const;
		bool parseFloatExponent(const char* text, std::size_t textLength, std::size_t & offset) const;
		ConstantType parseHex(const char* text, std::size_t textLength, std::size_t & offset) const;
		ConstantType parseString(const char* text, std::size_t textLength, std::size_t & offset, bool escaped) const;
		ConstantType parseChar(const char* text, std::size_t textLength, std::size_t & offset) const;
		ConstantType parseBool(const char* text, std::size_t textLength, std::size_t & offset) const;
	public:
		static const int getID();

	private:
		ConstantType subType;
	};

} //End namespace jitcat::Tokenizer