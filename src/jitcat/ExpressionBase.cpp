/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ExpressionBase.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatAssignableExpression.h"
#include "jitcat/CatFunctionCall.h"
#include "jitcat/CatPrefixOperator.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypedExpression.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/Document.h"
#include "jitcat/JitCat.h"
#ifdef ENABLE_LLVM
#include "jitcat/LLVMCodeGenerator.h"
#include "jitcat/LLVMCompileTimeContext.h"
#endif
#include "jitcat/SLRParseResult.h"
#include "jitcat/Tools.h"

#include <cassert>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::LLVM;
using namespace jitcat::Parser;
using namespace jitcat::Tokenizer;


ExpressionBase::ExpressionBase(bool expectAssignable):
	expressionIsLiteral(false),
	isConstant(false),
	errorManagerHandle(nullptr),
	expectAssignable(expectAssignable)
{
}


ExpressionBase::ExpressionBase(const char* expression, bool expectAssignable):
	expression(expression),
	expressionIsLiteral(false),
	isConstant(false),
	errorManagerHandle(nullptr),
	expectAssignable(expectAssignable)
{
}


ExpressionBase::ExpressionBase(const std::string& expression, bool expectAssignable):
	expression(expression),
	expressionIsLiteral(false),
	isConstant(false),
	errorManagerHandle(nullptr),
	expectAssignable(expectAssignable)
{
}


ExpressionBase::ExpressionBase(CatRuntimeContext* compileContext, const std::string& expression, bool expectAssignable):
	expression(expression),
	expressionIsLiteral(false),
	isConstant(false),
	errorManagerHandle(nullptr),
	expectAssignable(expectAssignable)
{
}


ExpressionBase::~ExpressionBase()
{
	if (errorManagerHandle.getIsValid())
	{
		static_cast<ExpressionErrorManager*>(errorManagerHandle.get())->errorSourceDeleted(this);
	}
}


void ExpressionBase::setExpression(const std::string& expression_, CatRuntimeContext* compileContext)
{
	if (expression != expression_)
	{
		expression = expression_;
		if (compileContext != nullptr)
		{
			compile(compileContext);
		}
	}
}


const std::string& ExpressionBase::getExpression() const
{
	return expression;
}


bool ExpressionBase::isLiteral() const
{
	return expressionIsLiteral;
}


bool ExpressionBase::isConst() const
{
	return isConstant;
}


bool ExpressionBase::hasError() const
{
	return !(parseResult.get() != nullptr
		     && parseResult->success);
}


const CatGenericType& ExpressionBase::getType() const
{
	return valueType;
}


bool ExpressionBase::parse(CatRuntimeContext* context, ExpressionErrorManager* errorManager, void* errorContext, const CatGenericType& expectedType)
{
	if (context == nullptr)
	{
		context = &CatRuntimeContext::defaultContext;
		context->getErrorManager()->clear();
	}
	errorManagerHandle = errorManager;

	isConstant = false;
	expressionIsLiteral = false;

	Document document(expression.c_str(), expression.length());
	context->getErrorManager()->setCurrentDocument(&document);
	parseResult.reset(JitCat::get()->parseExpression(&document, context, errorManager, errorContext));

	if (parseResult->success)
	{
		typeCheck(expectedType, context, errorManager, errorContext);
		if (parseResult->success)
		{
			constCollapse(context);
		}
	}
	handleParseErrors(context);
	//typeCheck may have changed parseResult->success
	if (parseResult->success && !isConstant)
	{
		compileToNativeCode(context);
	}
	if (!parseResult->success)
	{
		parseResult->astRootNode.reset(nullptr);
	}
	context->getErrorManager()->setCurrentDocument(nullptr);
	return parseResult->success;
}


void ExpressionBase::constCollapse(CatRuntimeContext* context)
{
	CatTypedExpression* newExpression = parseResult->getNode<CatTypedExpression>()->constCollapse(context);
	if (newExpression != parseResult->astRootNode.get())
	{
		parseResult->astRootNode.reset(newExpression);
	}
}


void ExpressionBase::typeCheck(const CatGenericType& expectedType, CatRuntimeContext* context, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (!parseResult->getNode<CatTypedExpression>()->typeCheck(context, errorManager, errorContext))
	{
		parseResult->success = false;
	}
	else 
	{	
		calculateLiteralStatus();
		valueType = parseResult->getNode<CatTypedExpression>()->getType();
		Lexeme expressionLexeme = parseResult->getNode<CatTypedExpression>()->getLexeme();
		if (!expectedType.isUnknown())
		{
			if (expectAssignable && !valueType.isWritable())
			{
				parseResult->success = false;
 				errorManager->compiledWithError(std::string(Tools::append("Expression result is read only. Expected a writable ", expectedType.toString(), ".")), errorContext, context->getContextName(), expressionLexeme);
			}
			if (expectedType.isPointerToReflectableObjectType())
			{
				const std::string typeName = expectedType.getPointeeType()->getObjectTypeName();
				if (!valueType.isPointerToReflectableObjectType())
				{
					parseResult->success = false;
					errorManager->compiledWithError(Tools::append("Expected a ", typeName), errorContext, context->getContextName(), expressionLexeme);
				}
				else if (valueType.getPointeeType()->getObjectTypeName() != typeName)
				{
					parseResult->success = false;
					errorManager->compiledWithError(Tools::append("Expected a ", typeName, ", got a ", valueType.getPointeeType()->getObjectTypeName()), errorContext, context->getContextName(), expressionLexeme);
				}
			}
			else if (expectedType.isVoidType() && valueType.isVoidType())
			{
				parseResult->success = true;
			}
			else if (valueType != expectedType)
			{
				if (expectedType.isVoidType())
				{
					//Insert an automatic type conversion to void.
					CatArgumentList* arguments = new CatArgumentList(expressionLexeme, std::vector<CatTypedExpression*>({parseResult->releaseNode<CatTypedExpression>()}));

					parseResult->astRootNode.reset(new CatFunctionCall("toVoid", expressionLexeme, arguments, expressionLexeme));
					parseResult->getNode<CatTypedExpression>()->typeCheck(context, errorManager, errorContext);
					
					valueType = parseResult->getNode<CatTypedExpression>()->getType();
				}
				else if (expectedType.isScalarType() && valueType.isScalarType())
				{
					//Insert an automatic type conversion if the scalar types do not match.
					CatArgumentList* arguments = new CatArgumentList(expressionLexeme, std::vector<CatTypedExpression*>({ parseResult->releaseNode<CatTypedExpression>() }));

					if (expectedType.isFloatType())
					{
						parseResult->astRootNode.reset(new CatFunctionCall("toFloat", expressionLexeme, arguments, expressionLexeme));
						parseResult->getNode<CatTypedExpression>()->typeCheck(context, errorManager, errorContext);
					}
					else if (expectedType.isIntType())
					{
						parseResult->astRootNode.reset(new CatFunctionCall("toInt", expressionLexeme, arguments, expressionLexeme));
						parseResult->getNode<CatTypedExpression>()->typeCheck(context, errorManager, errorContext);
					}
					else
					{
						assert(false);	//Missing a conversion here?
					}
					
					valueType = parseResult->getNode<CatTypedExpression>()->getType();
				}
				else
				{
					parseResult->success = false;
					errorManager->compiledWithError(std::string(Tools::append("Expected a ", expectedType.toString())), errorContext, context->getContextName(), expressionLexeme);
				}
			}
		}
		else if (expectAssignable && !valueType.isWritable())
		{
			parseResult->success = false;
			errorManager->compiledWithError("Expression result is read only. Expected a writable value.", errorContext, context->getContextName(), expressionLexeme);
		}
		if (parseResult->success)
		{
			isConstant = parseResult->getNode<CatTypedExpression>()->isConst();
		}
	}
}


void ExpressionBase::handleParseErrors(CatRuntimeContext* context)
{
	if (!parseResult->success)
	{
		expressionIsLiteral = false;
		isConstant = false;
		valueType = CatGenericType::unknownType;
	}
	else
	{
		context->getErrorManager()->compiledWithoutErrors(this);
	}
}


void ExpressionBase::compileToNativeCode(CatRuntimeContext* context)
{
#ifdef ENABLE_LLVM
	if (!isConstant)
	{
		LLVMCompileTimeContext llvmCompileContext(context);
		llvmCompileContext.options.enableDereferenceNullChecks = true;
		intptr_t functionAddress = 0;
		codeGenerator = context->getCodeGenerator();
		if (!expectAssignable)
		{
			functionAddress = codeGenerator->generateAndGetFunctionAddress(parseResult->getNode<CatTypedExpression>(), &llvmCompileContext);
		}
		else if (parseResult->getNode<CatTypedExpression>()->isAssignable())
		{
			functionAddress = codeGenerator->generateAndGetAssignFunctionAddress(parseResult->getNode<CatAssignableExpression>(), &llvmCompileContext);
		}
		if (functionAddress != 0)
		{
			handleCompiledFunction(functionAddress);
		}
		else
		{
			assert(false);
		}
	}
#endif //ENABLE_LLVM
}


void jitcat::ExpressionBase::calculateLiteralStatus()
{
	expressionIsLiteral = false;
	if (parseResult->success)
	{
		if (parseResult->getNode<CatTypedExpression>()->getNodeType() == CatASTNodeType::Literal)
		{
			expressionIsLiteral = true;
		}
		else if (parseResult->getNode<CatTypedExpression>()->getNodeType() == CatASTNodeType::PrefixOperator)
		{
			//If the expression is a minus prefix operator combined with a literal, then we need to count the whole expression as a literal.
			CatPrefixOperator* prefixOp = parseResult->getNode<CatPrefixOperator>();
			if (prefixOp->getRHS() != nullptr
				&& prefixOp->getOperator() == CatPrefixOperator::Operator::Minus
				&& prefixOp->getRHS()->getNodeType() == CatASTNodeType::Literal)
			{
				expressionIsLiteral = true;
			}
		}
	}
}
