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
#include "MemberReference.h"
#include "MemberReferencePtr.h"
#include "SLRParseResult.h"
#include "Tools.h"
#include "TypeTraits.h"


template<typename T>
Expression<T>::Expression():
	expressionIsLiteral(false),
	expressionAST(nullptr),
	isConstant(false),
	errorManagerHandle(nullptr)
{
}


template<typename T>
inline Expression<T>::Expression(const char* expression):
	expression(expression),
	expressionIsLiteral(false),
	expressionAST(nullptr),
	isConstant(false),
	errorManagerHandle(nullptr)
{
}


template<typename T>
Expression<T>::Expression(const std::string& expression):
	expression(expression),
	expressionIsLiteral(false),
	expressionAST(nullptr),
	isConstant(false),
	errorManagerHandle(nullptr)
{
}

	
template<typename T>
Expression<T>::Expression(CatRuntimeContext* compileContext, const std::string& expression):
	expression(expression),
	expressionIsLiteral(false),
	expressionAST(nullptr),
	isConstant(false),
	errorManagerHandle(nullptr)
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
	//Success var can change based on typechecking
	if (!parseResult->success)
	{
		std::string contextName = context->getContextName().c_str();
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
		context->getErrorManager()->compiledWithError(errorMessage, this);
		expressionIsLiteral = false;
		expressionAST = nullptr;
	}
	else
	{
		context->getErrorManager()->compiledWithoutErrors(this);
	}
}


template<typename T>
SLRParseResult* Expression<T>::parse(CatRuntimeContext* context)
{
	errorManagerHandle = context->getErrorManager();
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
		}
		else if (getCatType() == CatType::Object)
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
			else if (expressionAST->isConst())
			{
				isConstant = true;
				cachedValue = getActualValue(expressionAST->execute(context));
			}
		}
		else if (!resultType.isEqualToBasicCatType(getCatType())
			&& !(isScalar(getCatType()) && resultType.isScalarType()))
		{
			parseResult->success = false;
			parseResult->errorMessage = std::string(Tools::append("Expected a ", toString(getCatType())));
		}
		else if (expressionAST->isConst())
		{
			isConstant = true;
			cachedValue = getActualValue(expressionAST->execute(context));
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
const T Expression<T>::getValue(CatRuntimeContext* runtimeContext)
{
	if (isConstant)
	{
		return cachedValue;
	}
	else if (expressionAST != nullptr)
	{
		CatValue value = expressionAST->execute(runtimeContext);
		return getActualValue(value);
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


template<>
inline float Expression<float>::getActualValue(const CatValue& catValue)
{
	if (catValue.getValueType() == CatType::Int)
	{
		return (float)catValue.getIntValue();
	}
	else 
	{
		return catValue.getFloatValue();
	}
}


template<>
inline int Expression<int>::getActualValue(const CatValue& catValue)
{
	if (catValue.getValueType() == CatType::Float)
	{
		return (int)catValue.getFloatValue();
	}
	else 
	{
		return catValue.getIntValue();
	}
}


template<>
inline bool Expression<bool>::getActualValue(const CatValue& catValue)
{
	return catValue.getBoolValue();
}


template<>
inline std::string Expression<std::string>::getActualValue(const CatValue& catValue)
{
	return catValue.getStringValue();
}


template<typename T>
inline T Expression<T>::getActualValue(const CatValue& catValue)
{
	if (catValue.getValueType() == CatType::Object)
	{
		MemberReferencePtr objectPtr = catValue.getCustomTypeValue();
		MemberReference* object = objectPtr.getPointer();
		return TypeTraits<T>::getValueFromMemberReference(object);
	}
	return T();
}
