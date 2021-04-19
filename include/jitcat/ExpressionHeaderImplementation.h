/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/Expression.h"
#include "jitcat/CatASTNodes.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/Configuration.h"
#include "jitcat/Document.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/JitCat.h"

#include "jitcat/SLRParseResult.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeTraits.h"

#include <cassert>

namespace jitcat
{

	template<typename ExpressionResultT>
	Expression<ExpressionResultT>::Expression():
		getValueFunc(&getDefaultValue)
	{
	}


	template<typename ExpressionResultT>
	inline Expression<ExpressionResultT>::Expression(const char* expression):
		ExpressionBase(expression),
		getValueFunc(&getDefaultValue)
	{
	}


	template<typename ExpressionResultT>
	Expression<ExpressionResultT>::Expression(const std::string& expression):
		ExpressionBase(expression),
		getValueFunc(&getDefaultValue)
	{
	}

	
	template<typename ExpressionResultT>
	Expression<ExpressionResultT>::Expression(CatRuntimeContext* compileContext, const std::string& expression):
		ExpressionBase(compileContext, expression),
		getValueFunc(&getDefaultValue)
	{
		compile(compileContext);
	}


	template<typename ExpressionResultT>
	Expression<ExpressionResultT>::~Expression()
	{

	}


	template<typename ExpressionResultT>
	void Expression<ExpressionResultT>::compile(CatRuntimeContext* context)
	{
		if (context == nullptr)
		{
			context = &CatRuntimeContext::defaultContext;
			context->getErrorManager()->clear();
		}
		if (!parse(context, context->getErrorManager(), this, TypeTraits<ExpressionResultT>::toGenericType()))
		{
			resetCompiledFunctionToDefault();
		}
		else
		{
			if (isConstant)
			{
				if constexpr (!std::is_same<void, ExpressionResultT>::value)
				{
					cachedValue = getActualValue(parseResult.getNode<AST::CatTypedExpression>()->execute(context));
				}
			}
		}
	}


	template<typename ExpressionResultT>
	inline void Expression<ExpressionResultT>::handleCompiledFunction(uintptr_t functionAddress)
	{
		getValueFunc = reinterpret_cast<const ExpressionResultT(*)(CatRuntimeContext*)>(functionAddress);
	}


	template<typename ExpressionResultT>
	inline void Expression<ExpressionResultT>::resetCompiledFunctionToDefault()
	{
		getValueFunc = &getDefaultValue;
	}


	template<typename ExpressionResultT>
	inline const ExpressionResultT Expression<ExpressionResultT>::getValue(CatRuntimeContext* runtimeContext)
	{
		if (isConstant)
		{
			if constexpr (!std::is_same<void, ExpressionResultT>::value)
			{
				return cachedValue;
			}
			else
			{
				return;
			}
		}
		else
		{
			if (runtimeContext == nullptr)
			{
				runtimeContext = &CatRuntimeContext::defaultContext;
			}
			else if constexpr (Configuration::enableLLVM || Configuration::usePreCompiledExpressions)
			{
				if (Configuration::enableLLVM || getValueFunc != &getDefaultValue)
				{
					if constexpr (!std::is_same<void, ExpressionResultT>::value)
					{
						return getValueFunc(runtimeContext);
					}
					else
					{
						getValueFunc(runtimeContext);
						return;
					}
				}
			}
			if constexpr (!Configuration::enableLLVM)
			{
				if (parseResult.success)
				{
					if constexpr (!std::is_same<void, ExpressionResultT>::value)
					{
						std::any value = parseResult.getNode<AST::CatTypedExpression>()->execute(runtimeContext);
						runtimeContext->clearTemporaries();
						return getActualValue(value);
					}
					else
					{
						parseResult.getNode<AST::CatTypedExpression>()->execute(runtimeContext);
						runtimeContext->clearTemporaries();
						return;
					}
				}
				else
				{
					return ExpressionResultT();
				}
			}
			return ExpressionResultT();
		}
	}


	template<typename ExpressionResultT>
	inline const ExpressionResultT Expression<ExpressionResultT>::getInterpretedValue(CatRuntimeContext* runtimeContext)
	{
		if (isConstant)
		{
			if constexpr (!std::is_same<void, ExpressionResultT>::value)
			{
				return cachedValue; 
			}
			else
			{
				return;
			}
		}
		else if (parseResult.success)
		{
			if (runtimeContext == nullptr)
			{
				runtimeContext = &CatRuntimeContext::defaultContext;
			}
			if constexpr (!std::is_same<void, ExpressionResultT>::value)
			{
				std::any value = parseResult.getNode<AST::CatTypedExpression>()->execute(runtimeContext);
				runtimeContext->clearTemporaries();
				return getActualValue(value);
			}
			else
			{
				parseResult.getNode<AST::CatTypedExpression>()->execute(runtimeContext);
				runtimeContext->clearTemporaries();
				return;
			}
		}
		else
		{
			return ExpressionResultT();
		}
	}


	template<typename ExpressionResultT>
	CatGenericType Expression<ExpressionResultT>::getExpectedCatType() const
	{
		return TypeTraits<ExpressionResultT>::toGenericType();
	}


	template<typename ExpressionResultT>
	inline ExpressionResultT Expression<ExpressionResultT>::getActualValue(const std::any& catValue)
	{
		return TypeTraits<ExpressionResultT>::getValue(catValue);
	}


	template<typename ExpressionResultT>
	inline const ExpressionResultT Expression<ExpressionResultT>::getDefaultValue(CatRuntimeContext*)
	{
		return ExpressionResultT();
	}

}//End namespace jitcat