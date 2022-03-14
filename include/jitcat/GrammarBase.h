/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


#include <map>
#include <memory>
#include <vector>

namespace jitcat
{
	namespace AST
	{
		class ASTNode;
	}
	namespace Parser
	{
		class ASTNodeParser;
		class SLRParser;
	}
	namespace Tokenizer
	{
		class TokenizerBase;
	}
	namespace Grammar
	{
		class Production;
		class ProductionTerminalToken;
		class ProductionToken;


		class GrammarBase
		{
			friend class Parser::SLRParser;
		public:
			GrammarBase(Tokenizer::TokenizerBase* tokenizer);
			virtual ~GrammarBase();
			virtual const char* getProductionName(int production) const = 0;
			ProductionToken* epsilon();
			std::unique_ptr<Parser::SLRParser> createSLRParser() const;

			typedef AST::ASTNode* (*SemanticAction)(const Parser::ASTNodeParser&);

			const Tokenizer::TokenizerBase* getTokenizer() const;

		protected:
			void rule(unsigned short productionId, std::initializer_list<ProductionToken*> tokens, SemanticAction action);

			template<typename EnumT>
			void rule(EnumT productionId, std::initializer_list<ProductionToken*> tokens, SemanticAction action);

			ProductionToken* term(unsigned short tokenId, unsigned short tokenSubType);

			template<typename TokenEnumT, typename TokenSubTypeEnumT>
			ProductionToken* term(TokenEnumT tokenId, TokenSubTypeEnumT tokenSubType);

			template<typename EnumT>
			ProductionToken* term(unsigned short tokenId, EnumT tokenSubType);

			ProductionToken* prod(unsigned short productionId);

			template<typename EnumT>
			ProductionToken* prod(EnumT productionId);

			void setRootProduction(unsigned short productionId, ProductionToken* eofToken);

			template<typename EnumT>
			void setRootProduction(EnumT productionId, ProductionToken* eofToken);

			void build();
		private:
			Production* findOrCreateProduction(unsigned short productionId);
			void buildEpsilonContainment();
			void buildFirstSets();
			void buildFollowSets();
		private:
			Tokenizer::TokenizerBase* tokenizer;
			Production* rootProduction;
			ProductionToken* epsilonInstance;
			std::vector<ProductionTerminalToken*> terminals;
			std::map<int, Production*> productions;
		};


		template<typename EnumT>
		inline void GrammarBase::rule(EnumT productionId, std::initializer_list<ProductionToken*> tokens, SemanticAction action)
		{
			static_assert(std::is_enum<EnumT>::value, "Expected an enum.");
			rule(static_cast<typename std::underlying_type<EnumT>::type>(productionId), tokens, action);
		}


		template<typename TokenEnumT, typename TokenSubTypeEnumT>
		inline ProductionToken* GrammarBase::term(TokenEnumT tokenId, TokenSubTypeEnumT tokenSubType)
		{
			static_assert(std::is_enum<TokenEnumT>::value, "Expected an enum.");
			static_assert(std::is_enum<TokenSubTypeEnumT>::value, "Expected an enum.");
			return term(static_cast<typename std::underlying_type<TokenEnumT>::type>(tokenId), static_cast<typename std::underlying_type<TokenSubTypeEnumT>::type>(tokenSubType));
		}


		template<typename EnumT>
		inline ProductionToken* GrammarBase::term(unsigned short tokenId, EnumT tokenSubType)
		{
			static_assert(std::is_enum<EnumT>::value, "Expected an enum:");
			return term(tokenId, static_cast<typename std::underlying_type_t<EnumT>>(tokenSubType));
		}


		template<typename EnumT>
		inline ProductionToken* GrammarBase::prod(EnumT productionId)
		{
			static_assert(std::is_enum<EnumT>::value, "Expected an enum:");
			return prod(static_cast<typename std::underlying_type<EnumT>::type>(productionId));
		}


		template<typename EnumT>
		inline void GrammarBase::setRootProduction(EnumT productionId, ProductionToken* eofToken)
		{
			static_assert(std::is_enum<EnumT>::value, "Expected an enum:");
			return setRootProduction(static_cast<typename std::underlying_type<EnumT>::type>(productionId), eofToken);
		}
	}//End namespace Grammar
}//End namespace jitcat