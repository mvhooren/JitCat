/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatMemberAccess.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/CatLiteral.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/MemberInfo.h"
#include "jitcat/TypeInfo.h"

#include <cassert>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


CatMemberAccess::CatMemberAccess(CatTypedExpression* base, const std::string& memberName, const Tokenizer::Lexeme& lexeme):
	CatAssignableExpression(lexeme),
	base(base),
	memberInfo(nullptr),
	type(CatGenericType::unknownType),
	assignableType(CatGenericType::unknownType),
	memberName(memberName)
{
}


jitcat::AST::CatMemberAccess::CatMemberAccess(const CatMemberAccess& other):
	CatAssignableExpression(other),
	base(static_cast<CatTypedExpression*>(other.base->copy())),
	memberInfo(nullptr),
	type(CatGenericType::unknownType),
	assignableType(CatGenericType::unknownType),
	memberName(other.memberName)
{
}


CatASTNode* jitcat::AST::CatMemberAccess::copy() const
{
	return new CatMemberAccess(*this);
}


void CatMemberAccess::print() const
{
	base->print();
	CatLog::log(".");
	CatLog::log(memberName);
}


CatASTNodeType CatMemberAccess::getNodeType() const
{
	return CatASTNodeType::MemberAccess;
}


std::any CatMemberAccess::execute(CatRuntimeContext* runtimeContext)
{
	std::any baseValue = base->execute(runtimeContext);
	if (memberInfo != nullptr && runtimeContext != nullptr)
	{
		return memberInfo->getMemberReference(reinterpret_cast<unsigned char*>(base->getType().getRawPointer(baseValue)));
	}
	assert(false);
	return std::any();
}


std::any CatMemberAccess::executeAssignable(CatRuntimeContext* runtimeContext)
{
	std::any baseValue = base->execute(runtimeContext);
	if (memberInfo != nullptr && runtimeContext != nullptr)
	{
		std::any value = memberInfo->getAssignableMemberReference(reinterpret_cast<unsigned char*>(base->getType().getRawPointer(baseValue)));
		return value;
	}
	assert(false);
	return std::any();
}


bool CatMemberAccess::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	memberInfo = nullptr;
	type = CatGenericType::unknownType;
	assignableType = CatGenericType::unknownType;
	if (base->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		CatGenericType baseType = base->getType();
		CatGenericType expectedBaseType = baseType.removeIndirection().toPointer();
		IndirectionConversionMode conversionMode = IndirectionConversionMode::None;
		bool indirectionConversionSuccess = ASTHelper::doIndirectionConversion(base, expectedBaseType, true, conversionMode);
		//To silence unused variable warning in release builds.
		(void)indirectionConversionSuccess;
		assert(indirectionConversionSuccess);
		baseType = base->getType();
		if (!(baseType.isPointerToReflectableObjectType() || baseType.isReflectableHandleType()))
		{
			errorManager->compiledWithError(Tools::append("Expression to the left of '.' is not an object."), errorContext, compiletimeContext->getContextName(), getLexeme());
			return false;
		}
		memberInfo = baseType.getPointeeType()->getObjectType()->getMemberInfo(Tools::toLowerCase(memberName));
		if (memberInfo != nullptr)
		{
			type = memberInfo->catType;
			assignableType = type.toPointer();
			return true;
		}
		else
		{
			errorManager->compiledWithError(Tools::append("Member not found:", memberName), errorContext, compiletimeContext->getContextName(), getLexeme());
			return false;
		}
	}
	return false;
}


const CatGenericType& CatMemberAccess::getType() const
{
	return type;
}


const CatGenericType& jitcat::AST::CatMemberAccess::getAssignableType() const
{
	return assignableType;
}


bool CatMemberAccess::isConst() const
{
	if (memberInfo != nullptr)
	{
		return memberInfo->catType.isConst() && base->isConst();
	}
	return false;
}


CatTypedExpression* CatMemberAccess::constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	ASTHelper::updatePointerIfChanged(base, base->constCollapse(compileTimeContext, errorManager, errorContext));
	if (type.isValidType() && isConst())
	{
		return new CatLiteral(execute(compileTimeContext), getType(), getLexeme());
	}
	return this;
}


void jitcat::AST::CatMemberAccess::setTypeAndMemberInfo(Reflection::TypeMemberInfo* newMemberInfo, const CatGenericType& newMemberType)
{
	memberInfo = newMemberInfo;
	type = newMemberType;
}


CatTypedExpression* CatMemberAccess::getBase() const
{
	return base.get();
}


TypeMemberInfo* CatMemberAccess::getMemberInfo() const
{
	return memberInfo;
}
