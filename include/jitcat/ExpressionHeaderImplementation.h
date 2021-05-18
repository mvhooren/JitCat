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
		getValuePtr(&Expression<ExpressionResultT>::getDefaultValue)
	{
	}


	template<typename ExpressionResultT>
	inline Expression<ExpressionResultT>::Expression(const char* expression):
		ExpressionBase(expression),
		getValuePtr(&Expression<ExpressionResultT>::getDefaultValue)
	{
	}


	template<typename ExpressionResultT>
	Expression<ExpressionResultT>::Expression(const std::string& expression):
		ExpressionBase(expression),
		getValuePtr(&Expression<ExpressionResultT>::getDefaultValue)
	{
	}

	
	template<typename ExpressionResultT>
	Expression<ExpressionResultT>::Expression(CatRuntimeContext* compileContext, const std::string& expression):
		ExpressionBase(compileContext, expression),
		getValuePtr(&Expression<ExpressionResultT>::getDefaultValue)
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
			context = &CatRuntimeContext::getDefaultContext();
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
				getValuePtr = &Expression<ExpressionResultT>::getCachedValue;
			}
			else if (!Configuration::enableLLVM && !Configuration::usePreCompiledExpressions)
			{
				getValuePtr = &Expression<ExpressionResultT>::getExecuteInterpretedValue;
			}
		}
	}


	template<typename ExpressionResultT>
	inline void Expression<ExpressionResultT>::handleCompiledFunction(uintptr_t functionAddress)
	{
		static_assert(sizeof(getValuePtr) == sizeof(uintptr_t) || sizeof(getValuePtr) == 2 * sizeof(uintptr_t));
		if (functionAddress != 0)
		{
			memcpy(reinterpret_cast<unsigned char*>(&getValuePtr), reinterpret_cast<unsigned char*>(&functionAddress), sizeof(uintptr_t));
		}
		else
		{
			getValuePtr = &Expression<ExpressionResultT>::getExecuteInterpretedValue;
		}
	}


	template<typename ExpressionResultT>
	inline void Expression<ExpressionResultT>::resetCompiledFunctionToDefault()
	{
		getValuePtr = &Expression<ExpressionResultT>::getDefaultValue;
	}


	template<typename ExpressionResultT>
	inline const ExpressionResultT Expression<ExpressionResultT>::getValue(CatRuntimeContext* runtimeContext)
	{
		return (this->*getValuePtr)(runtimeContext);
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
			return getExecuteInterpretedValue(runtimeContext);
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
	inline const ExpressionResultT Expression<ExpressionResultT>::getExecuteInterpretedValue(CatRuntimeContext * runtimeContext)
	{
		if (runtimeContext == nullptr)
		{
			runtimeContext = &CatRuntimeContext::getDefaultContext();
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


	template<typename ExpressionResultT>
	inline const ExpressionResultT Expression<ExpressionResultT>::getCachedValue(CatRuntimeContext* runtimeContext)
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
	
	
	template<typename ExpressionResultT>
	inline const ExpressionResultT Expression<ExpressionResultT>::getDefaultValue(CatRuntimeContext* runtimeContext)
	{
		return ExpressionResultT();
	}
 

}//End namespace jitcat