#include "jitcat/CatInheritanceDefinition.h"
#include "jitcat/CatHostClass.h"
#include "jitcat/CatHostClasses.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatScope.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeInfo.h"

using namespace jitcat;
using namespace jitcat::AST;


jitcat::AST::CatInheritanceDefinition::CatInheritanceDefinition(CatTypeNode* typeNode, const Tokenizer::Lexeme& nameLexeme, const Tokenizer::Lexeme& lexeme):
	CatDefinition(lexeme),
	nameLexeme(nameLexeme),
	type(typeNode),
	hostClass(nullptr),
	inheritedMember(nullptr)
{
}


jitcat::AST::CatInheritanceDefinition::CatInheritanceDefinition(const CatInheritanceDefinition& other):
	CatDefinition(other),
	nameLexeme(other.nameLexeme),
	type(static_cast<CatTypeNode*>(other.type->copy())),
	hostClass(nullptr),
	inheritedMember(nullptr)
{
}


jitcat::AST::CatInheritanceDefinition::~CatInheritanceDefinition()
{
	if (errorManagerHandle.getIsValid())
	{
		static_cast<ExpressionErrorManager*>(errorManagerHandle.get())->errorSourceDeleted(this);
	}
}


CatASTNode* jitcat::AST::CatInheritanceDefinition::copy() const
{
	return new CatInheritanceDefinition(*this);
}


void CatInheritanceDefinition::print() const
{
	Tools::CatLog::log(Tools::append("inherits ", nameLexeme));
}


CatASTNodeType CatInheritanceDefinition::getNodeType() const
{
	return CatASTNodeType::InheritanceDefinition;
}


bool jitcat::AST::CatInheritanceDefinition::typeCheck(CatRuntimeContext* compiletimeContext)
{
	if (errorManagerHandle.getIsValid())
	{
		static_cast<ExpressionErrorManager*>(errorManagerHandle.get())->errorSourceDeleted(this);
		errorManagerHandle = nullptr;
	}
	ExpressionErrorManager* errorManager = compiletimeContext->getErrorManager();
	errorManagerHandle = errorManager;
	hostClass = nullptr;
	if (!type->typeCheck(compiletimeContext, errorManager, this))
	{
		return false;
	}
	else
	{
		CatGenericType inheritedType = type->getType();
		if (!inheritedType.isObjectType())
		{
			errorManager->compiledWithError(Tools::append("Inheritance only supports object types, ", inheritedType.toString(), " not supported."), this, compiletimeContext->getContextName(), getLexeme());
			return false;
		}
		else if (!inheritedType.getObjectType()->isCustomType())
		{
			hostClass = compiletimeContext->getHostClasses()->getHostClass(inheritedType.getObjectType()->getTypeName());
			if (hostClass == nullptr || !hostClass->isInheritable())
			{
				errorManager->compiledWithError(Tools::append("Host type cannot be inherited: ", inheritedType.toString(), ", enable inheritance through the CatHostClasses interface."), this, compiletimeContext->getContextName(), getLexeme());
				return false;
			}
			if (!hostClass->inheritTypeCheck(compiletimeContext, compiletimeContext->getCurrentClass(), errorManager, this))
			{
				return false;
			}
		}
		CatScope* currentScope = compiletimeContext->getCurrentScope();
		if (currentScope != nullptr)
		{
			inheritedMember = currentScope->getCustomType()->addMember(Tools::append("$", inheritedType.toString()), inheritedType.toWritable());
			inheritedMember->visibility = Reflection::MemberVisibility::Hidden;
		}
	}

	return true;
}


CatGenericType jitcat::AST::CatInheritanceDefinition::getType() const
{
	if (type != nullptr && type->isKnownType())
	{
		return type->getType();
	}
	return CatGenericType::errorType;
}


CatHostClass* jitcat::AST::CatInheritanceDefinition::getHostClass() const
{
	return hostClass;
}


Reflection::TypeMemberInfo* jitcat::AST::CatInheritanceDefinition::getInheritedMember() const
{
	return inheritedMember;
}
