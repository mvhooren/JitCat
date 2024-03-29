/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatConstruct.h"
#include "jitcat/CatAssignableExpression.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatAssignmentOperator.h"
#include "jitcat/CatIdentifier.h"
#include "jitcat/CatLiteral.h"
#include "jitcat/CatMemberFunctionCall.h"
#include "jitcat/CatOperatorNew.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/FunctionSignature.h"
#include "jitcat/MemberFunctionInfo.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeInfo.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;


CatConstruct::CatConstruct(const Tokenizer::Lexeme& lexeme, std::unique_ptr<CatIdentifier> identifier, std::unique_ptr<CatArgumentList> arguments, bool autoDestruct):
	CatStatement(lexeme),
	assignable(std::move(identifier)),
	arguments(std::move(arguments)),
	isCopyConstructor(false),
	autoDestruct(autoDestruct)
{
	if (this->arguments == nullptr)
	{
		this->arguments = std::make_unique<CatArgumentList>(lexeme, std::vector<CatTypedExpression*>());
	}
}


CatConstruct::CatConstruct(const CatConstruct& other):
	CatStatement(other.lexeme),
	assignable(static_cast<CatAssignableExpression*>(other.assignable->copy())),
	arguments(static_cast<CatArgumentList*>(other.arguments->copy())),
	isCopyConstructor(false),
	autoDestruct(other.autoDestruct)
{
}


CatConstruct::~CatConstruct()
{
}


bool CatConstruct::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (constructorStatement != nullptr)
	{
		return constructorStatement->typeCheck(compiletimeContext, errorManager, errorContext);
	}
	if (!assignable->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		return false;
	}
	if (!assignable->isAssignable())
	{
		errorManager->compiledWithError("Construction failed because target cannot be assigned.", errorContext, compiletimeContext->getContextName(), lexeme);
		return false;
	}
	assignableType = assignable->getAssignableType();

	assert(assignableType.isPointerType());
	CatGenericType* pointeeType = assignableType.getPointeeType();
	if (!pointeeType->isConstructible())
	{
		errorManager->compiledWithError(Tools::append("Variable of type ", pointeeType->toString(), " cannot be constructed."), 
										errorContext, compiletimeContext->getContextName(), lexeme);
		return false;
	}
	if (pointeeType->isBasicType() || pointeeType->isPointerType())
	{
		CatTypedExpression* initExpression = nullptr;
		if (arguments->getNumArguments() == 1)
		{
			initExpression = arguments->releaseArgument(0);
		}
		else if (arguments->getNumArguments() == 0)
		{
			initExpression = new CatLiteral(pointeeType->createDefault(), *pointeeType, assignable->getLexeme());
		}
		else
		{
			assert(false);
			errorManager->compiledWithError(Tools::append("Invalid number of arguments for variable of type ", assignableType.getPointeeType()->toString(), "."), 
											errorContext, compiletimeContext->getContextName(), lexeme);
			return false;
		}
		//QQQ check initExpression for correct type before CatAssignmentOperator is created
		constructorStatement = std::make_unique<CatAssignmentOperator>(assignable.release(), initExpression, lexeme, lexeme);
		return constructorStatement->typeCheck(compiletimeContext, errorManager, errorContext);
	}
	else if (pointeeType->isReflectableObjectType())
	{
		TypeInfo* objectTypeInfo = pointeeType->getObjectType();
		if (objectTypeInfo->getAllowConstruction() && objectTypeInfo->isCustomType() && static_cast<CustomTypeInfo*>(objectTypeInfo)->getClassDefinition() != nullptr)
		{
			constructorStatement = std::make_unique<CatMemberFunctionCall>("init", lexeme, assignable.release(), arguments.release(), lexeme);
			return constructorStatement->typeCheck(compiletimeContext, errorManager, errorContext);
		}
		else if (objectTypeInfo->isArrayType())
		{
			// If there is no initialization, intialise with 0 size
			if (arguments->getNumArguments() == 0)
			{
				std::vector<CatTypedExpression*> argumentList = { new CatLiteral(0, lexeme) };
				arguments = std::make_unique<CatArgumentList>(lexeme, argumentList);
				constructorStatement = std::make_unique<CatMemberFunctionCall>("init", lexeme, assignable.release(), arguments.release(), lexeme);
				return constructorStatement->typeCheck(compiletimeContext, errorManager, errorContext);
			}
			if (!arguments->typeCheck(compiletimeContext, errorManager, errorContext))
			{
				return false;
			}
			CatTypedExpression* argument = arguments->releaseArgument(0);
			assert(argument->getNodeType() == CatASTNodeType::OperatorNew);
			CatOperatorNew* operatorNew = static_cast<CatOperatorNew*>(argument);
			CatGenericType newType = operatorNew->getType();
			bool succeeded= false;
			if (assignableType.compare(newType, false, false))
			{
				arguments.reset(operatorNew->releaseArguments());
				constructorStatement = std::make_unique<CatMemberFunctionCall>("init", lexeme, assignable.release(), arguments.release(), lexeme);
				return constructorStatement->typeCheck(compiletimeContext, errorManager, errorContext);
			}
			else
			{
				errorManager->compiledWithError("Type mismatch.", errorContext, compiletimeContext->getContextName(), lexeme);
				return false;
			}
			delete argument;
			return succeeded;
		}
		else
		{
			if (!arguments->typeCheck(compiletimeContext, errorManager, errorContext))
			{
				return false;
			}
			if (arguments->getNumArguments() == 0 && objectTypeInfo->getAllowConstruction())
			{
				//Use the default placement constructor.
				return true;
			}
			else if (arguments->getNumArguments() == 1 && arguments->getArgumentType(0).compare(*pointeeType, false, false))
			{
				//This is actually a copy constructor.
				isCopyConstructor = true;
				if (!pointeeType->isCopyConstructible())
				{
					errorManager->compiledWithError(Tools::append("Type ", assignableType.getPointeeType()->toString(), " is not copy-constructible."), 
													errorContext, compiletimeContext->getContextName(), lexeme);
					return false;
				}

				if (!arguments->applyIndirectionConversions({*pointeeType}, "init", compiletimeContext, errorManager, errorContext))
				{
					return false;
				}

				return true;
			}
			else
			{
				//Constructors of non-custom types are not allowed to have arguments yet.
				assert(false);
			}
		}
	}
	assert(false);
	errorManager->compiledWithError(Tools::append("Variable of type ", assignableType.getPointeeType()->toString(), " cannot be constructed."), 
									errorContext, compiletimeContext->getContextName(), lexeme);
	return false;
}


CatStatement* CatConstruct::constCollapse(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (constructorStatement != nullptr)
	{
		return constructorStatement.release();

	}
	if (assignable != nullptr)
	{
		ASTHelper::updatePointerIfChanged(assignable, assignable->constCollapse(compiletimeContext, errorManager, errorContext));
	}
	if (arguments != nullptr)
	{
		arguments->constCollapse(compiletimeContext, errorManager, errorContext);
	}
	return this;
}


std::any CatConstruct::execute(jitcat::CatRuntimeContext* runtimeContext)
{
	std::any value = assignable->executeAssignable(runtimeContext);
	const unsigned char* buffer = nullptr;
	std::size_t bufferSize = 0;
	assignableType.getPointeeType()->toBuffer(value, buffer, bufferSize);
	if (!isCopyConstructor)
	{
		assignableType.placementConstruct(const_cast<unsigned char*>(buffer), bufferSize);
	}
	else
	{
		std::any argumentValue = arguments->executeArgument(0, runtimeContext);
		const unsigned char* sourceBuffer = nullptr;
		std::size_t sourceBufferSize = 0;
		std::any tempAny;
		if (arguments->getArgumentType(0).isReflectableObjectType())
		{
			tempAny = arguments->getArgumentType(0).getAddressOf(argumentValue);
			arguments->getArgumentType(0).toBuffer(tempAny, sourceBuffer, sourceBufferSize);
		}
		else if (arguments->getArgumentType(0).isPointerType())
		{
			arguments->getArgumentType(0).getPointeeType()->toBuffer(argumentValue, sourceBuffer, sourceBufferSize);
		}
		assignableType.getPointeeType()->copyConstruct(const_cast<unsigned char*>(buffer), bufferSize, sourceBuffer, sourceBufferSize);
	}
	return std::any();
}


CatASTNode* CatConstruct::copy() const
{
	return new CatConstruct(*this);
}


void CatConstruct::print() const
{
	assignable->print();
}


CatASTNodeType CatConstruct::getNodeType() const
{
	return CatASTNodeType::Contruct;
}


const CatGenericType& jitcat::AST::CatConstruct::getType() const
{
	return assignableType;
}


CatAssignableExpression* jitcat::AST::CatConstruct::getAssignable() const
{
	return assignable.get();
}


CatArgumentList* jitcat::AST::CatConstruct::getArgumentList() const
{
	return arguments.get();
}


bool CatConstruct::getIsCopyConstructor() const
{
	return isCopyConstructor;
}


bool jitcat::AST::CatConstruct::getAutoDestruct() const
{
	return autoDestruct;
}
