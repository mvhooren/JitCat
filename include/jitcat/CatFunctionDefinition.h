/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


#include "jitcat/CatDefinition.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/CatScopeID.h"
#include "jitcat/CustomTypeInstance.h"
#include "jitcat/ReflectableHandle.h"

#include <any>
#include <cassert>
#include <memory>
#include <vector>


namespace jitcat::Reflection
{
	class CustomTypeInstance;
	class CustomTypeInfo;
}

namespace jitcat::AST
{
	class CatAssignableExpression;
	class CatTypeNode;
	class CatFunctionParameterDefinitions;
	class CatScopeBlock;

	class CatFunctionDefinition: public CatDefinition
	{
	public:
		CatFunctionDefinition(CatTypeNode* type, const std::string& name, CatFunctionParameterDefinitions* parameters, CatScopeBlock* scopeBlock, const Tokenizer::Lexeme& lexeme);
		virtual ~CatFunctionDefinition();

		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() override final;
		virtual bool typeCheck(CatRuntimeContext* compileTimeContext) override final;
		
		std::any executeFunctionWithPack(CatRuntimeContext* runtimeContext, CatScopeID packScopeId);

		std::any executeFunctionWithArguments(CatRuntimeContext* runtimeContext, const std::vector<std::any>& arguments);

		template<typename... ArgumentsT>
		std::any executeFunction(CatRuntimeContext* runtimeContext, ArgumentsT... arguments);

		jitcat::Reflection::CustomTypeInfo* getParametersType() const;
		CatTypeNode* getReturnTypeNode() const;
		int getNumParameters() const;
		const std::string& getParameterName(int index) const;
		const CatTypeNode* getParameterType(int index) const;

		const std::string& getFunctionName() const;

		template<typename... ArgumentsT>
		Reflection::CustomTypeInstance* createParameterPack(ArgumentsT... arguments);

	private:
		//Reflection::CustomTypeInstance* createAnyParametersPack(const std::vector<std::any>& arguments, CatScopeID& createdScopeId);
		Reflection::CustomTypeInstance* createCustomTypeInstance() const;
		CatScopeID pushScope(CatRuntimeContext* runtimeContext, Reflection::CustomTypeInstance* instance);

	private:
		std::string name;
		std::unique_ptr<CatTypeNode> type;
		std::unique_ptr<CatFunctionParameterDefinitions> parameters;
		std::vector<std::unique_ptr<CatAssignableExpression>> parameterAssignables;
		CatScopeID parametersScopeId;
		std::unique_ptr<CatScopeBlock> scopeBlock;
		Reflection::ReflectableHandle errorManagerHandle;
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
			std::unique_ptr<Reflection::CustomTypeInstance> instance(createParameterPack(std::forward<ArgumentsT>(arguments)...));
			CatScopeID scopeId = pushScope(runtimeContext, instance.get());
			return executeFunctionWithPack(runtimeContext, scopeId);
		}
	}


	template<typename... ArgumentsT>
	inline Reflection::CustomTypeInstance* CatFunctionDefinition::createParameterPack(ArgumentsT... arguments)
	{
		Reflection::CustomTypeInstance* instance = createCustomTypeInstance();
		assert(sizeof...(ArgumentsT) == (std::size_t)getNumParameters());
		int i = 0;
		(instance->setMemberValue(getParameterName(i++), (std::forward<ArgumentsT>(arguments))), ...);
		return instance;
	}

};
