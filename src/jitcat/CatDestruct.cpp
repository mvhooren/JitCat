/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatDestruct.h"
#include "jitcat/CatAssignableExpression.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatAssignmentOperator.h"
#include "jitcat/CatIdentifier.h"
#include "jitcat/CatLiteral.h"
#include "jitcat/CatMemberFunctionCall.h"
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


CatDestruct::CatDestruct(const Tokenizer::Lexeme& lexeme, std::unique_ptr<CatIdentifier> identifier):
	CatStatement(lexeme),
	assignable(std::move(identifier))
{
}


CatDestruct::CatDestruct(const CatDestruct& other):
	CatStatement(other.lexeme),
	assignable(static_cast<CatAssignableExpression*>(other.assignable->copy()))
{
}


CatDestruct::~CatDestruct()
{
}


bool CatDestruct::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (destructorStatement != nullptr)
	{
		return destructorStatement->typeCheck(compiletimeContext, errorManager, errorContext);
	}
	if (!assignable->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		return false;
	}
	assignableType = assignable->getAssignableType();
	assert(assignableType.isPointerType());
	CatGenericType* pointeeType = assignableType.getPointeeType();
	if (!pointeeType->isConstructible())
	{
		errorManager->compiledWithError(Tools::append("Variable of type ", pointeeType->toString(), " cannot be destructed."), 
										errorContext, compiletimeContext->getContextName(), lexeme);
		return false;
	}
	if (pointeeType->isReflectableObjectType())
	{
		TypeInfo* objectTypeInfo = pointeeType->getObjectType();
		if (objectTypeInfo->getAllowConstruction() && objectTypeInfo->isCustomType() && static_cast<CustomTypeInfo*>(objectTypeInfo)->getClassDefinition() != nullptr)
		{
			destructorStatement = std::make_unique<CatMemberFunctionCall>("destroy", lexeme, assignable.release(), new CatArgumentList(lexeme, {}), lexeme);
			return destructorStatement->typeCheck(compiletimeContext, errorManager, errorContext);
		}
		else
		{
			if (!pointeeType->isConstructible())
			{
				errorManager->compiledWithError(Tools::append("Type ", assignableType.getPointeeType()->toString(), " is not destructible."), 
												errorContext, compiletimeContext->getContextName(), lexeme);
				return false;
			}
			return true;
		}
	}
	assert(false);
	errorManager->compiledWithError(Tools::append("Variable of type ", assignableType.getPointeeType()->toString(), " cannot be destructed."), 
									errorContext, compiletimeContext->getContextName(), lexeme);
	return false;
}


CatStatement* CatDestruct::constCollapse(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (destructorStatement != nullptr)
	{
		return destructorStatement.release();

	}
	if (assignable != nullptr)
	{
		ASTHelper::updatePointerIfChanged(assignable, assignable->constCollapse(compiletimeContext, errorManager, errorContext));
	}
	return this;
}


std::any CatDestruct::execute(jitcat::CatRuntimeContext* runtimeContext)
{
	std::any value = assignable->executeAssignable(runtimeContext);
	const unsigned char* buffer = nullptr;
	std::size_t bufferSize = 0;
	assignableType.getPointeeType()->toBuffer(value, buffer, bufferSize);
	assignableType.placementDestruct(const_cast<unsigned char*>(buffer), bufferSize);
	return std::any();
}


CatASTNode* CatDestruct::copy() const
{
	return new CatDestruct(*this);
}


void CatDestruct::print() const
{
	assignable->print();
}


CatASTNodeType CatDestruct::getNodeType() const
{
	return CatASTNodeType::Destruct;
}


const CatGenericType& jitcat::AST::CatDestruct::getType() const
{
	return assignableType;
}


CatAssignableExpression* jitcat::AST::CatDestruct::getAssignable() const
{
	return assignable.get();
}