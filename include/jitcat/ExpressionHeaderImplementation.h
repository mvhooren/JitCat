/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "Expression.h"
#include "CatASTNodes.h"
#include "CatGenericType.h"
#include "CatType.h"
#include "Configuration.h"
#include "Document.h"
#include "ExpressionErrorManager.h"
#include "JitCat.h"

#include "SLRParseResult.h"
#include "Tools.h"
#include "TypeTraits.h"

#include <cassert>


template<typename T>
Expression<T>::Expression():
	getValueFunc(&getDefaultValue)
{
}


template<typename T>
inline Expression<T>::Expression(const char* expression):
	ExpressionBase(expression),
	getValueFunc(&getDefaultValue)
{
}


template<typename T>
Expression<T>::Expression(const std::string& expression):
	ExpressionBase(expression),
	getValueFunc(&getDefaultValue)
{
}

	
template<typename T>
Expression<T>::Expression(CatRuntimeContext* compileContext, const std::string& expression):
	ExpressionBase(compileContext, expression),
	getValueFunc(&getDefaultValue)
{
	compile(compileContext);
}


template<typename T>
Expression<T>::~Expression()
{

}

template<typename T>
void Expression<T>::compile(CatRuntimeContext* context)
{
	if (!parse(context, TypeTraits<T>::toGenericType()))
	{
		getValueFunc = &getDefaultValue;
	}
	else
	{
		if (isConstant)
		{
			if constexpr (!std::is_same<void, T>::value)
			{
				cachedValue = getActualValue(expressionAST->execute(context));
			}
		}
	}
}


template<typename T>
inline void Expression<T>::handleCompiledFunction(uintptr_t functionAddress)
{
	getValueFunc = reinterpret_cast<const T(*)(CatRuntimeContext*)>(functionAddress);
}


template<typename T>
inline const T Expression<T>::getValue(CatRuntimeContext* runtimeContext)
{
	if (isConstant)
	{
		if constexpr (!std::is_same<void, T>::value)
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
		if constexpr (Configuration::enableLLVM)
		{
			return getValueFunc(runtimeContext);
		}
		else
		{
			if (expressionAST != nullptr)
			{
				if constexpr (!std::is_same<void, T>::value)
				{
					std::any value = expressionAST->execute(runtimeContext);
					return getActualValue(value);
				}
				else
				{
					expressionAST->execute(runtimeContext);
					return;
				}
			}
			else
			{
				return T();
			}
		}
	}
}


template<typename T>
inline const T Expression<T>::getInterpretedValue(CatRuntimeContext* runtimeContext)
{
	if (isConstant)
	{
		return cachedValue;
	}
	else if (expressionAST != nullptr)
	{
		std::any value = expressionAST->execute(runtimeContext);
		return getActualValue(value);
	}
	else
	{
		return T();
	}
}


template<typename T>
CatType Expression<T>::getExpectedCatType() const
{
	return TypeTraits<T>::getCatType();
}


template<typename T>
inline T Expression<T>::getActualValue(const std::any& catValue)
{
	if constexpr (std::is_pointer<T>::value)
	{
		return static_cast<T>(std::any_cast<Reflectable*>(catValue));
	}
	else
	{
		return std::any_cast<T>(catValue);
	}
}


template<typename T>
inline const T Expression<T>::getDefaultValue(CatRuntimeContext*)
{
	return T();
}
