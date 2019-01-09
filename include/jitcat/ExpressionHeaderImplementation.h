/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "Expression.h"
#include "CatASTNodes.h"
#include "CatGenericType.h"
#include "CatType.h"
#include "Document.h"
#include "ExpressionErrorManager.h"
#include "JitCat.h"
#include "LLVMCodeGenerator.h"
#include "LLVMCompileTimeContext.h"
#include "MemberReference.h"
#include "MemberReferencePtr.h"
#include "SLRParseResult.h"
#include "Tools.h"
#include "TypeTraits.h"

#include <cassert>


template<typename T>
Expression<T>::Expression():
	expressionIsLiteral(false),
	expressionAST(nullptr),
	isConstant(false),
	errorManagerHandle(nullptr),
	getValueFunc(&getDefaultValue)
{
}


template<typename T>
inline Expression<T>::Expression(const char* expression):
	expression(expression),
	expressionIsLiteral(false),
	expressionAST(nullptr),
	isConstant(false),
	errorManagerHandle(nullptr),
	getValueFunc(&getDefaultValue)
{
}


template<typename T>
Expression<T>::Expression(const std::string& expression):
	expression(expression),
	expressionIsLiteral(false),
	expressionAST(nullptr),
	isConstant(false),
	errorManagerHandle(nullptr),
	getValueFunc(&getDefaultValue)
{
}

	
template<typename T>
Expression<T>::Expression(CatRuntimeContext* compileContext, const std::string& expression):
	expression(expression),
	expressionIsLiteral(false),
	expressionAST(nullptr),
	isConstant(false),
	errorManagerHandle(nullptr),
	getValueFunc(&getDefaultValue)
{
	compile(compileContext);
}


template<typename T>
Expression<T>::~Expression()
{
	if (errorManagerHandle.getIsValid())
	{
		static_cast<ExpressionErrorManager*>(errorManagerHandle.get())->expressionDeleted(this);
	}
}


template<typename T>
void Expression<T>::setExpression(const std::string& expression_, CatRuntimeContext* context)
{
	if (expression != expression_)
	{
		expression = expression_;
		if (context != nullptr)
		{
			compile(context);
		}
	}
}


template<typename T>
const std::string& Expression<T>::getExpression() const
{
	return expression;
}


template<typename T>
bool Expression<T>::isLiteral() const
{
	return expressionIsLiteral;
}


template<typename T>
bool Expression<T>::isConst() const
{
	return isConstant;
}


template<typename T>
bool Expression<T>::hasError() const
{
	return !(parseResult.get() != nullptr
		     && parseResult->success);
}


template<typename T>
void Expression<T>::compile(CatRuntimeContext* context)
{
	parse(context);
	if (!parseResult->success)
	{
		std::string contextName = "";
		if (context != nullptr)
		{
			contextName = context->getContextName().c_str();
		}
		std::string errorMessage;
		if (contextName != "")
		{
			errorMessage = Tools::append("ERROR in ", contextName, ": \n", expression ,"\n", parseResult->errorMessage);
		}
		else
		{
			errorMessage = Tools::append("ERROR: \n", expression ,"\n", parseResult->errorMessage);
		}
		if (parseResult->errorPosition >= 0)
		{
			errorMessage = Tools::append(errorMessage, " Offset: ", parseResult->errorPosition);
		}
		if (context != nullptr)
		{
			context->getErrorManager()->compiledWithError(errorMessage, this);
		}
		expressionIsLiteral = false;
		expressionAST = nullptr;
	}
	else if (context != nullptr)
	{
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
		context->getErrorManager()->compiledWithoutErrors(this);
	}
}


template<typename T>
SLRParseResult* Expression<T>::parse(CatRuntimeContext* context)
{
	if (context != nullptr)
	{
		errorManagerHandle = context->getErrorManager();
	}
	isConstant = false;
	Document document(expression.c_str(), expression.length());
	parseResult.reset(JitCat::get()->parse(&document, context));
	if (parseResult->success)
	{
		CatTypedExpression* typedExpression = static_cast<CatTypedExpression*>(parseResult->astRootNode);
		bool isMinusPrefixWithLiteral = false;
		//If the expression is a minus prefix operator combined with a literal, then we need to count the whole expression as a literal.
		if (typedExpression->getNodeType() == CatASTNodeType::PrefixOperator)
		{
			CatPrefixOperator* prefixOp = static_cast<CatPrefixOperator*>(typedExpression);
			if (prefixOp->rhs != nullptr
				&& prefixOp->oper == CatPrefixOperator::Operator::Minus
				&& prefixOp->rhs->getNodeType() == CatASTNodeType::Literal)
			{
				isMinusPrefixWithLiteral = true;
			}
		}
		CatTypedExpression* newExpression = typedExpression->constCollapse(context);
		if (newExpression != typedExpression)
		{
			delete typedExpression;
			typedExpression = newExpression;
			parseResult->astRootNode = newExpression;
			expressionIsLiteral = isMinusPrefixWithLiteral;
		}
		else
		{
			expressionIsLiteral = typedExpression->getNodeType() == CatASTNodeType::Literal;
		}
		expressionAST = typedExpression;
		//Type check by just executing the expression and checking the result.
		//The compile-time context should give correct default values for type checking
		CatGenericType resultType = expressionAST->typeCheck();
		if (!resultType.isValidType())
		{
			parseResult->success = false;
			parseResult->errorMessage = resultType.getErrorMessage();
			getValueFunc = &getDefaultValue;
		}
		else 
		{	
			if (getCatType() == CatType::Object)
			{
				const std::string typeName = TypeTraits<T>::getTypeName();
				if (!resultType.isObjectType())
				{
					parseResult->success = false;
					parseResult->errorMessage = Tools::append("Expected a ", typeName);
				}
				else if (resultType.getObjectTypeName() != typeName)
				{
					parseResult->success = false;
					parseResult->errorMessage = Tools::append("Expected a ", typeName, ", got a ", resultType.getObjectTypeName());
				}
			}
			else if (getCatType() == CatType::Void && resultType.isVoidType())
			{
				parseResult->success = true;
				parseResult->astRootNode = expressionAST;
			}
			else if (!resultType.isEqualToBasicCatType(getCatType()))
			{
				if (isScalar(getCatType()) && resultType.isScalarType())
				{
					//Insert an automatic type conversion if the scalar types do not match.
					CatArgumentList* arguments = new CatArgumentList();
					arguments->arguments.emplace_back(static_cast<CatTypedExpression*>(parseResult->astRootNode));
					switch (getCatType())
					{
						case CatType::Float:	expressionAST = new CatFunctionCall("toFloat", arguments);		break;
						case CatType::Int:		expressionAST = new CatFunctionCall("toInt", arguments);		break;
						default:				assert(false);	//Missing a conversion here?
					}
					parseResult->astRootNode = expressionAST;
				}
				else
				{
					parseResult->success = false;
					parseResult->errorMessage = std::string(Tools::append("Expected a ", toString(getCatType())));
				}
			}
			if (parseResult->success)
			{
				if (expressionAST->isConst())
				{
					isConstant = true;
					if constexpr (!std::is_same<void, T>::value)
					{
						cachedValue = getActualValue(expressionAST->execute(context));
					}
				}
			}
		}
	}
	return parseResult.get();
}


template<typename T>
CatType Expression<T>::getType() const
{
	return TypeTraits<T>::getCatType();
}


template<typename T>
std::string&  Expression<T>::getExpressionForSerialisation()
{
	return expression;
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
		return getValueFunc(runtimeContext);
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
CatType Expression<T>::getCatType() const
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
