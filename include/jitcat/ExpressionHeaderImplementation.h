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
#ifdef ENABLE_LLVM
#include "LLVMCodeGenerator.h"
#include "LLVMCompileTimeContext.h"
#endif
#include "MemberReference.h"
#include "MemberReferencePtr.h"
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
		if (context != nullptr)
		{
#ifdef ENABLE_LLVM
			if (!isConstant)
			{
				LLVMCompileTimeContext llvmCompileContext(context);
				llvmCompileContext.options.enableDereferenceNullChecks = true;
				intptr_t functionAddress = context->getCodeGenerator()->generateAndGetFunctionAddress(expressionAST, &llvmCompileContext);
				if (functionAddress != 0)
				{
					getValueFunc = reinterpret_cast<const T(*)(CatRuntimeContext*)>(functionAddress);
				}
				else
				{
					assert(false);
				}
			}
#endif //ENABLE_LLVM
		}
	}
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
				CatValue value = expressionAST->execute(runtimeContext);
				if (value.getValueType() != CatType::Error)
				{
					return getActualValue(value);
				}
				else
				{
					return T();
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
		CatValue value = expressionAST->execute(runtimeContext);
		if (value.getValueType() != CatType::Error)
		{
			return getActualValue(value);
		}
		else
		{
			return T();
		}
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
inline T Expression<T>::getActualValue(const CatValue& catValue)
{
	if constexpr (std::is_same<T, float>::value)
	{
		return catValue.getFloatValue();
	}
	else if constexpr (std::is_same<T, int>::value)
	{
		return catValue.getIntValue();
	}
	else if constexpr (std::is_same<T, bool>::value)
	{
		return catValue.getBoolValue();
	}
	else if constexpr (std::is_same<T, std::string>::value)
	{
		return catValue.getStringValue();
	}
	else
	{
		if (catValue.getValueType() == CatType::Object)
		{
			MemberReferencePtr objectPtr = catValue.getCustomTypeValue();
			MemberReference* object = objectPtr.getPointer();
			return TypeTraits<T>::getValueFromMemberReference(object);
		}
		return T();
	}
	
}


template<typename T>
inline const T Expression<T>::getDefaultValue(CatRuntimeContext*)
{
	return T();
}
