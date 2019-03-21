#pragma once

#include "CatASTNode.h"
#include "CatGenericType.h"


namespace jitcat::AST
{

	class CatTypeNode: public CatASTNode
	{
	public:
		CatTypeNode(const CatGenericType& type, const Tokenizer::Lexeme& lexeme);
		CatTypeNode(const std::string& name, const Tokenizer::Lexeme& lexeme);
		virtual ~CatTypeNode();

		bool isKnownType() const;
		std::string getTypeName() const;
		const CatGenericType& getType() const;

		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() override final;

		void setType(const CatGenericType& newType);

		bool typeCheck(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext);

	private:
		CatGenericType type;
		std::string name;
		bool knownType;
	};

};
