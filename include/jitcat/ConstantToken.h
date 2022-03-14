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

	class ConstantToken: public TokenFactory
	{
	public:
		ConstantToken(unsigned short id): TokenFactory(id) {};

		virtual const char* getTokenName() const override final;
		virtual bool createIfMatch(Document& document, std::size_t& currentPosition) const override final;
		virtual const char* getSubTypeName(unsigned short subType) const override final;
		virtual const char* getSubTypeSymbol(unsigned short subType) const override final;
	
	private:
		ConstantType parseConstant(const char* text, std::size_t textLength, std::size_t& offset) const;
		ConstantType parseIntOrFloat(const char* text, std::size_t textLength, std::size_t& offset) const;
		ConstantType parseFloatOrHexOrOct(const char* text, std::size_t textLength, std::size_t& offset) const;
		ConstantType parseFloatOrOct(const char* text, std::size_t textLength, std::size_t& offset) const;
		ConstantType parseFloat(const char* text, std::size_t textLength, std::size_t& offset, bool pastDot, bool pastExponent) const;
		ConstantType parseFloatWithExponent(const char* text, std::size_t textLength, std::size_t& offset, bool pastDot) const;
		bool parseFloatExponent(const char* text, std::size_t textLength, std::size_t& offset) const;
		ConstantType parseHex(const char* text, std::size_t textLength, std::size_t& offset) const;
		ConstantType parseString(const char* text, std::size_t textLength, std::size_t& offset, bool escaped) const;
		ConstantType parseChar(const char* text, std::size_t textLength, std::size_t& offset) const;
		ConstantType parseBool(const char* text, std::size_t textLength, std::size_t& offset) const;
	};

} //End namespace jitcat::Tokenizer