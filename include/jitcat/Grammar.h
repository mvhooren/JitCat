/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class ASTNode;
class ASTNodeParser;
class Production;
class ProductionTerminalToken;
class ProductionToken;
class SLRParser;
class Tokenizer;

#include <map>
#include <vector>


class Grammar
{
	friend class SLRParser;
public:
	Grammar(Tokenizer* tokenizer);
	virtual ~Grammar();
	virtual const char* getProductionName(int production) const = 0;
	ProductionToken* epsilon();
	SLRParser* createSLRParser() const;

	typedef ASTNode* (*SemanticAction)(const ASTNodeParser&);

protected:
	void rule(int productionId, std::initializer_list<ProductionToken*> tokens, SemanticAction action);

	template<typename EnumT>
	void rule(EnumT productionId, std::initializer_list<ProductionToken*> tokens, SemanticAction action);

	ProductionToken* term(int tokenId, int tokenSubType);

	template<typename TokenEnumT, typename TokenSubTypeEnumT>
	ProductionToken* term(TokenEnumT tokenId, TokenSubTypeEnumT tokenSubType);

	template<typename EnumT>
	ProductionToken* term(int tokenId, EnumT tokenSubType);

	ProductionToken* prod(int productionId);

	template<typename EnumT>
	ProductionToken* prod(EnumT productionId);

	void setRootProduction(int productionId, ProductionToken* eofToken);

	template<typename EnumT>
	void setRootProduction(EnumT productionId, ProductionToken* eofToken);

	void build();
private:
	Production* findOrCreateProduction(int productionId);
	void buildEpsilonContainment();
	void buildFirstSets();
	void buildFollowSets();
private:
	Tokenizer* tokenizer;
	Production* rootProduction;
	ProductionToken* epsilonInstance;
	std::vector<ProductionTerminalToken*> terminals;
	std::map<int, Production*> productions;
};


template<typename EnumT>
inline void Grammar::rule(EnumT productionId, std::initializer_list<ProductionToken*> tokens, SemanticAction action)
{
	static_assert(std::is_enum<EnumT>::value, "Expected an enum.");
	rule(static_cast<typename std::underlying_type<EnumT>::type>(productionId), tokens, action);
}


template<typename TokenEnumT, typename TokenSubTypeEnumT>
inline ProductionToken* Grammar::term(TokenEnumT tokenId, TokenSubTypeEnumT tokenSubType)
{
	static_assert(std::is_enum<TokenEnumT>::value, "Expected an enum.");
	static_assert(std::is_enum<TokenSubTypeEnumT>::value, "Expected an enum.");
	return term(static_cast<typename std::underlying_type<TokenEnumT>::type>(tokenId), static_cast<typename std::underlying_type<TokenSubTypeEnumT>::type>(tokenSubType));
}


template<typename EnumT>
inline ProductionToken* Grammar::term(int tokenId, EnumT tokenSubType)
{
	static_assert(std::is_enum<EnumT>::value, "Expected an enum:");
	return term(tokenId, static_cast<typename std::underlying_type<EnumT>::type>(tokenSubType));
}


template<typename EnumT>
inline ProductionToken * Grammar::prod(EnumT productionId)
{
	static_assert(std::is_enum<EnumT>::value, "Expected an enum:");
	return prod(static_cast<typename std::underlying_type<EnumT>::type>(productionId));
}


template<typename EnumT>
inline void Grammar::setRootProduction(EnumT productionId, ProductionToken* eofToken)
{
	static_assert(std::is_enum<EnumT>::value, "Expected an enum:");
	return setRootProduction(static_cast<typename std::underlying_type<EnumT>::type>(productionId), eofToken);
}
