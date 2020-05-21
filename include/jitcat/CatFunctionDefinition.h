/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


#include "jitcat/CatDefinition.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/CatScope.h"
#include "jitcat/CatScopeID.h"
#include "jitcat/FunctionSignature.h"
#include "jitcat/MemberVisibility.h"
#include "jitcat/ReflectableHandle.h"
#include "jitcat/ObjectInstance.h"
#include "jitcat/TypeTraits.h"

#include <any>
#include <cassert>
#include <memory>
#include <vector>


namespace jitcat::Reflection
{
	class CustomTypeInfo;
	struct CustomTypeMemberFunctionInfo;
	class Reflectable;
}

namespace jitcat::AST
{
	class CatAssignableExpression;
	class CatTypeNode;
	class CatFunctionParameterDefinitions;
	class CatScopeBlock;

	class CatFunctionDefinition: public CatDefinition, public CatScope, public Reflection::FunctionSignature
	{
	public:
		CatFunctionDefinition(CatTypeNode* type, const std::string& name, const Tokenizer::Lexeme& nameLexeme, CatFunctionParameterDefinitions* parameters, CatScopeBlock* scopeBlock, const Tokenizer::Lexeme& lexeme);
		CatFunctionDefinition(const CatFunctionDefinition& other);

		virtual ~CatFunctionDefinition();

		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;
		virtual bool typeCheck(CatRuntimeContext* compileTimeContext) override final;
		
		std::any executeFunctionWithPack(CatRuntimeContext* runtimeContext, CatScopeID packScopeId);

		std::any executeFunctionWithArguments(CatRuntimeContext* runtimeContext, const std::vector<std::any>& arguments);

		template<typename... ArgumentsT>
		std::any executeFunction(CatRuntimeContext* runtimeContext, ArgumentsT... arguments);

		jitcat::Reflection::CustomTypeInfo* getParametersType() const;
		CatTypeNode* getReturnTypeNode() const;
		virtual int getNumParameters() const override final;
		const std::string& getParameterName(int index) const;
		//const CatTypeNode* getParameterType(int index) const;
		virtual const CatGenericType& getParameterType(int index) const override final;

		Reflection::MemberVisibility getFunctionVisibility() const;
		void setFunctionVisibility(Reflection::MemberVisibility functionVisibility);

		virtual const std::string& getLowerCaseFunctionName() const;
		const std::string& getFunctionName() const;

		CatScopeBlock* getScopeBlock() const;
		CatScopeBlock* getEpilogBlock() const;
		CatScopeBlock* getOrCreateEpilogBlock(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext);

		// Inherited via CatScope
		virtual CatScopeID getScopeId() const override final;
		virtual Reflection::CustomTypeInfo* getCustomType() override final;

	private:
		CatScopeID pushScope(CatRuntimeContext* runtimeContext, unsigned char* instance);

	private:
		std::string name;
		std::string lowerCaseName;
		Tokenizer::Lexeme nameLexeme;
		std::unique_ptr<CatTypeNode> type;
		std::unique_ptr<CatFunctionParameterDefinitions> parameters;
		Reflection::MemberVisibility visibility;

		std::vector<std::unique_ptr<CatAssignableExpression>> parameterAssignables;
		CatScopeID parametersScopeId;

		std::unique_ptr<CatScopeBlock> scopeBlock;
		std::unique_ptr<CatScopeBlock> epilogBlock;

		Reflection::ReflectableHandle errorManagerHandle;
		
		//not owned
		Reflection::CustomTypeMemberFunctionInfo* memberFunctionInfo;
	};




	template<typename ...ArgumentsT>
	inline std::any CatFunctionDefinition::executeFunction(CatRuntimeContext* runtimeContext, ArgumentsT... arguments)
	{
		if constexpr (sizeof...(ArgumentsT) == 0)
		{
			return executeFunctionWithPack(runtimeContext, InvalidScopeID);
		}
		else
		{
			std::size_t numParameters = (std::size_t)getNumParameters();
			std::vector<std::any> anyArguments(numParameters);
			assert(sizeof...(ArgumentsT) == numParameters);
			int i = 0;
			int dummy[] = { 0, ( anyArguments[i++] = jitcat::TypeTraits<ArgumentsT>::getCatValue(std::forward<ArgumentsT>(arguments)), 0) ... };
			return executeFunctionWithArguments(runtimeContext, anyArguments);
		}
	}

}
