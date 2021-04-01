/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ASTHelper.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatAssignableExpression.h"
#include "jitcat/CatBuiltInFunctionCall.h"
#include "jitcat/CatIdentifier.h"
#include "jitcat/CatIndirectionConversion.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatScopeBlock.h"
#include "jitcat/CatTypedExpression.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/FunctionSignature.h"
#include "jitcat/ReflectableHandle.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeInfo.h"

#include <cassert>
#include <sstream>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;

void ASTHelper::updatePointerIfChanged(std::unique_ptr<CatScopeBlock>& uPtr, CatStatement* statement)
{
	if (uPtr.get() != static_cast<CatScopeBlock*>(statement))
	{
		uPtr.reset(static_cast<CatScopeBlock*>(statement));
	}
}

void ASTHelper::updatePointerIfChanged(std::unique_ptr<CatStatement>& uPtr, CatStatement* statement)
{
	if (uPtr.get() != statement)
	{
		uPtr.reset(statement);
	}
}


void ASTHelper::updatePointerIfChanged(std::unique_ptr<CatTypedExpression>& uPtr, CatStatement* expression)
{
	if (uPtr.get() != static_cast<CatTypedExpression*>(expression))
	{
		uPtr.reset(static_cast<CatTypedExpression*>(expression));
	}
}


void ASTHelper::updatePointerIfChanged(std::unique_ptr<CatAssignableExpression>& uPtr, CatStatement* expression)
{
	if (uPtr.get() != static_cast<CatAssignableExpression*>(expression))
	{
		uPtr.reset(static_cast<CatAssignableExpression*>(expression));
	}
}


void ASTHelper::updatePointerIfChanged(std::unique_ptr<CatIdentifier>& uPtr, CatStatement* expression)
{
	if (uPtr.get() != static_cast<CatIdentifier*>(expression))
	{
		uPtr.reset(static_cast<CatIdentifier*>(expression));
	}
}

bool ASTHelper::doTypeConversion(std::unique_ptr<CatTypedExpression>& uPtr, const CatGenericType& targetType)
{
	CatGenericType sourceType = uPtr->getType();
	//Currently, only basic type conversions are supported.
	if ((sourceType.isBasicType() || sourceType.isStringType()) 
		&& (targetType.isBasicType() || targetType.isStringType())
		&& sourceType != targetType)
	{
		CatTypedExpression* sourceExpression = uPtr.release();
		CatArgumentList* arguments = new CatArgumentList(sourceExpression->getLexeme(), std::vector<CatTypedExpression*>({sourceExpression}));

		const char* functionName = nullptr;

		if		(targetType.isIntType())	functionName = "toInt";
		else if (targetType.isDoubleType())	functionName = "toDouble";
		else if (targetType.isFloatType())	functionName = "toFloat";
		else if (targetType.isBoolType())	functionName = "toBool";
		else if (targetType.isStringType()) functionName = "toString";

		assert(functionName != nullptr);
		CatBuiltInFunctionCall* functionCall = new CatBuiltInFunctionCall(functionName, sourceExpression->getLexeme(), arguments, sourceExpression->getLexeme());
		uPtr.reset(functionCall);
		return true;
	}
	return false;
}


bool ASTHelper::doIndirectionConversion(std::unique_ptr<CatTypedExpression>& uPtr, const CatGenericType& expectedType, bool allowAddressOf, IndirectionConversionMode& conversionMode)
{
	conversionMode = expectedType.getIndirectionConversion(uPtr->getType());
	if (conversionMode != IndirectionConversionMode::None && isValidConversionMode(conversionMode))
	{
		if (!isDereferenceConversionMode(conversionMode) && !allowAddressOf)
		{
			return false;
		}
		std::unique_ptr<CatTypedExpression> expression(uPtr.release());
		uPtr = std::make_unique<CatIndirectionConversion>(expression->getLexeme(), expectedType, conversionMode, std::move(expression));
	}
	return isValidConversionMode(conversionMode);
}


std::any ASTHelper::doAssignment(CatAssignableExpression* target, CatTypedExpression* source, CatRuntimeContext* context)
{
	CatGenericType targetType = target->getAssignableType();
	CatGenericType sourceType = source->getType();

	std::any targetValue = target->executeAssignable(context);
	std::any sourceValue;

	if ((targetType.isPointerToHandleType() || targetType.isPointerToPointerType())
		 &&	(   targetType.getPointeeType()->getOwnershipSemantics() == TypeOwnershipSemantics::Owned
		    || targetType.getPointeeType()->getOwnershipSemantics() == TypeOwnershipSemantics::Shared)
		&& (sourceType.getOwnershipSemantics() == TypeOwnershipSemantics::Owned))
	{
		sourceValue = static_cast<CatAssignableExpression*>(source)->executeAssignable(context);
		sourceType = static_cast<CatAssignableExpression*>(source)->getAssignableType();
	}
	else
	{
		sourceValue = source->execute(context);
	}

	return doAssignment(targetValue, sourceValue, targetType, sourceType);
}


std::any jitcat::AST::ASTHelper::doGetArgument(CatTypedExpression* argument, const CatGenericType& parameterType, CatRuntimeContext* context)
{
	if (!parameterType.isPointerToReflectableObjectType()
		|| (parameterType.getOwnershipSemantics() != TypeOwnershipSemantics::Owned
			&& !(parameterType.getOwnershipSemantics() == TypeOwnershipSemantics::Shared && argument->getType().getOwnershipSemantics() == TypeOwnershipSemantics::Owned))
		|| argument->getType().getOwnershipSemantics() == TypeOwnershipSemantics::Value)
	{
		return argument->execute(context);
	}
	else
	{
		assert(argument->isAssignable());
		assert(argument->getType().getOwnershipSemantics() == TypeOwnershipSemantics::Owned);
		std::any sourceValue = static_cast<CatAssignableExpression*>(argument)->executeAssignable(context);
		const CatGenericType& sourceType = static_cast<CatAssignableExpression*>(argument)->getAssignableType();
		if (sourceType.isPointerToPointerType() && sourceType.getPointeeType()->getPointeeType()->isReflectableObjectType())
		{
			unsigned char** reflectableSource = reinterpret_cast<unsigned char**>(sourceType.getRawPointer(sourceValue));
			if (reflectableSource != nullptr)
			{
				unsigned char* value = *reflectableSource;
				*reflectableSource = nullptr;
				return sourceType.getPointeeType()->createFromRawPointer(reinterpret_cast<uintptr_t>(value));
			}
			return sourceType.getPointeeType()->createNullPtr();
		}
		else if (sourceType.isPointerToHandleType())
		{
			ReflectableHandle* handleSource = std::any_cast<ReflectableHandle*>(sourceValue);
			if (handleSource != nullptr)
			{
				unsigned char* value = reinterpret_cast<unsigned char*>(handleSource->get());
				*handleSource = nullptr;
				return sourceType.getPointeeType()->createFromRawPointer(reinterpret_cast<uintptr_t>(value));
			}
			return sourceType.getPointeeType()->createNullPtr();
		}
		else
		{
			assert(false);
			return sourceType.getPointeeType()->createNullPtr();
		}
	}
}


std::any ASTHelper::doAssignment(std::any& target, const std::any& source, const CatGenericType& targetType, const CatGenericType& sourceType)
{
	if (targetType.isPointerType() && targetType.getPointeeType()->isBasicType())
	{
		if (targetType.getPointeeType()->isIntType())
		{
			int* intTarget = std::any_cast<int*>(target);
			if (intTarget != nullptr)
			{
				*intTarget = std::any_cast<int>(source);
			}
		}
		else if (targetType.getPointeeType()->isFloatType())
		{
			float* floatTarget = std::any_cast<float*>(target);
			if (floatTarget != nullptr)
			{
				*floatTarget = std::any_cast<float>(source);
			}
		}
		else if (targetType.getPointeeType()->isDoubleType())
		{
			double* doubleTarget = std::any_cast<double*>(target);
			if (doubleTarget != nullptr)
			{
				*doubleTarget = std::any_cast<double>(source);
			}
		}
		else if (targetType.getPointeeType()->isBoolType())
		{
			bool* boolTarget = std::any_cast<bool*>(target);
			if (boolTarget != nullptr)
			{
				*boolTarget = std::any_cast<bool>(source);
			}
		}
		else if (targetType.isPointerToReflectableObjectType())
		{
			//Not supported for now. This would need to call operator= on the target object, not all objects will have implemented this.
		}
		return target;
	}
	else if (targetType.isPointerToPointerType() && targetType.getPointeeType()->isPointerToReflectableObjectType())
	{
		unsigned char** reflectableTarget = reinterpret_cast<unsigned char**>(targetType.getRawPointer(target));
		
		if (reflectableTarget != nullptr)
		{
			TypeOwnershipSemantics targetOwnership = targetType.getPointeeType()->getOwnershipSemantics();
			if (targetOwnership == TypeOwnershipSemantics::Owned && *reflectableTarget != nullptr)
			{
				targetType.getPointeeType()->getPointeeType()->getObjectType()->destruct(*reflectableTarget);
			}
			if (sourceType.isPointerToReflectableObjectType() || sourceType.isReflectableHandleType())
			{
				*reflectableTarget = reinterpret_cast<unsigned char*>(sourceType.getRawPointer(source));
			}
			else if (sourceType.isPointerToHandleType())
			{
				TypeOwnershipSemantics sourceOwnership = sourceType.getPointeeType()->getOwnershipSemantics();
				ReflectableHandle* sourceHandle = std::any_cast<ReflectableHandle*>(source);
				if (sourceHandle != nullptr)
				{
					*reflectableTarget = reinterpret_cast<unsigned char*>(sourceHandle->get());
					if ((targetOwnership == TypeOwnershipSemantics::Owned
						|| targetOwnership == TypeOwnershipSemantics::Shared)
						&& sourceOwnership == TypeOwnershipSemantics::Owned)
					{
						*sourceHandle = nullptr;
					}
				}
				else
				{
					*reflectableTarget = nullptr;
				}
			}
			else if (sourceType.isPointerToPointerType() && sourceType.getPointeeType()->isPointerToReflectableObjectType())
			{
				TypeOwnershipSemantics sourceOwnership = sourceType.getPointeeType()->getOwnershipSemantics();
				unsigned char** sourcePointer = reinterpret_cast<unsigned char**>(sourceType.getRawPointer(source));
				if (sourcePointer != nullptr)
				{
					*reflectableTarget = *sourcePointer;
					if ((targetOwnership == TypeOwnershipSemantics::Owned
						|| targetOwnership == TypeOwnershipSemantics::Shared)
						&& sourceOwnership == TypeOwnershipSemantics::Owned)
					{
						*sourcePointer = nullptr;
					}
				}
				else
				{
					*reflectableTarget = nullptr;
				}
			}
		}
		return target;
	}
	else if (targetType.isPointerToHandleType())
	{
		ReflectableHandle* handleTarget = std::any_cast<ReflectableHandle*>(target);
		if (handleTarget != nullptr)
		{
			TypeOwnershipSemantics targetOwnership = targetType.getPointeeType()->getOwnershipSemantics();
			if (targetOwnership == TypeOwnershipSemantics::Owned && handleTarget->getIsValid())
			{
				targetType.getPointeeType()->getPointeeType()->getObjectType()->destruct(reinterpret_cast<unsigned char*>(handleTarget->get()));
			}
			if (sourceType.isPointerToReflectableObjectType()
				|| sourceType.isReflectableHandleType())
			{
				*handleTarget = reinterpret_cast<Reflectable*>(sourceType.getRawPointer(source));
			}
			else if (sourceType.isPointerToHandleType())
			{
				ReflectableHandle* sourceHandle = std::any_cast<ReflectableHandle*>(source);
				if (sourceHandle != nullptr)
				{
					*handleTarget = sourceHandle->get();
					if ((targetOwnership == TypeOwnershipSemantics::Owned
						|| targetOwnership == TypeOwnershipSemantics::Shared)
						&& sourceType.getPointeeType()->getOwnershipSemantics() == TypeOwnershipSemantics::Owned)
					{
						*sourceHandle = nullptr;
					}
				}
				else
				{
					*handleTarget = nullptr;
				}
			}
			else if (sourceType.isPointerToPointerType() && sourceType.getPointeeType()->isPointerToReflectableObjectType())
			{
				unsigned char** sourcePointer = reinterpret_cast<unsigned char**>(sourceType.getRawPointer(source));
				if (sourcePointer != nullptr)
				{
					*handleTarget = reinterpret_cast<Reflectable*>(*sourcePointer);
					if ((targetOwnership == TypeOwnershipSemantics::Owned
						|| targetOwnership == TypeOwnershipSemantics::Shared)
						&& sourceType.getPointeeType()->getOwnershipSemantics() == TypeOwnershipSemantics::Owned)
					{
						*sourcePointer = nullptr;
					}
				}
				else
				{
					*handleTarget = nullptr;
				}
			}
		}
		return target;
	}
	assert(false);
	return std::any();
}


MemberFunctionInfo* ASTHelper::memberFunctionSearch(const std::string& functionName, const std::vector<CatGenericType>& argumentTypes, TypeInfo* type, 
													ExpressionErrorManager* errorManager, CatRuntimeContext* context, void* errorSource, const Tokenizer::Lexeme& lexeme)
{
	SearchFunctionSignature signature(functionName, argumentTypes);
	MemberFunctionInfo* functionInfo = type->getMemberFunctionInfo(signature);
	if (functionInfo != nullptr)
	{
		return functionInfo;
	}
	else if (functionName == "init")
	{
		//If it is the constructor function, also search for the auto-generated constructor if the user-defined constructor was not found.
		signature.setFunctionName("__init");
		functionInfo = type->getMemberFunctionInfo(signature);
		if (functionInfo != nullptr)
		{
			return functionInfo;
		}
	}
	else if (functionName == "destroy")
	{
		//If it is the constructor function, also search for the auto-generated constructor if the user-defined constructor was not found.
		signature.setFunctionName("__destroy");
		functionInfo = type->getMemberFunctionInfo(signature);
		if (functionInfo != nullptr)
		{
			return functionInfo;
		}
	}
	//The function was not found. Generate a list of all the functions with that name (that do not match the argument types).
	std::vector<MemberFunctionInfo*> foundFunctions = type->getMemberFunctionsByName(functionName);
	if (foundFunctions.size() == 0 && functionName == "init")
	{
		foundFunctions = type->getMemberFunctionsByName("__init");
	}
	else if (foundFunctions.size() == 0 && functionName == "detroy")
	{
		foundFunctions = type->getMemberFunctionsByName("__destroy");
	}
	if (foundFunctions.size() == 0)
	{
		//There exist no functions of that name.
		errorManager->compiledWithError(Tools::append("Member function not found: ", functionName, "."), errorSource, context->getContextName(), lexeme);
		return nullptr;
	}
	else 
	{
		MemberFunctionInfo* onlyFunction = nullptr;
		if (foundFunctions.size() == 1)
		{
			onlyFunction = foundFunctions[0];
		}
		else
		{
			MemberFunctionInfo* potentialOnlyFunction;
			int functionsWithSameNumberOfArguments = 0;
			for (auto& iter : foundFunctions)
			{
				if (iter->getNumParameters() == (int)argumentTypes.size())
				{
					functionsWithSameNumberOfArguments++;
					potentialOnlyFunction = iter;
				}
			}
			if (functionsWithSameNumberOfArguments == 1)
			{
				onlyFunction = potentialOnlyFunction;
			}
			else if (functionsWithSameNumberOfArguments > 1)
			{
				//Print an error with each potential match.
				std::ostringstream errorStream;
				errorStream << "Invalid argument(s) for function " << functionName << ". There are " << functionsWithSameNumberOfArguments << " potential candidates: \n";
				for (auto& iter : foundFunctions)
				{
					if (iter->getNumParameters() == (int)argumentTypes.size())
					{
						errorStream << "\t" << iter->getReturnType().toString() << " " << functionName << "(" << ASTHelper::getTypeListString(iter->getArgumentTypes()) << ")\n";
					}
				}
				errorManager->compiledWithError(errorStream.str(), errorSource, context->getContextName(), lexeme);
				return nullptr;
			}
		}
		//There is only one function of that name, or only one function with that number of arguments.
		//Print errors based on number of arguments and argument types.
		if (onlyFunction != nullptr)
		{
			if (onlyFunction->getNumberOfArguments() != argumentTypes.size())
			{
				errorManager->compiledWithError(Tools::append("Invalid number of arguments for function: ", functionName, " expected ", 
															  onlyFunction->getNumberOfArguments(), " arguments."), 
												errorSource, context->getContextName(), lexeme);
				return nullptr;
			}
			else
			{
				for (unsigned int i = 0; i < argumentTypes.size(); i++)
				{
					if (!onlyFunction->getArgumentType(i).compare(argumentTypes[i], false, false))
					{
						errorManager->compiledWithError(Tools::append("Invalid argument for function: ", functionName, " argument nr: ", i, 
																	  " expected: ", onlyFunction->getArgumentType(i).toString()), 
														errorSource, context->getContextName(), lexeme);
						return nullptr;
					}
					else if (!ASTHelper::checkOwnershipSemantics(onlyFunction->getArgumentType(i), argumentTypes[i], errorManager, context, errorSource, lexeme, "pass"))
					{
						return nullptr;
					}
				}
			}
		}
		else
		{
			//There are multiple functions with that name, all with a different number of arguments that the number of arguments supplied.
			//Print an error with each potential match.
			std::ostringstream errorStream;
			errorStream << "Invalid number of arguments for function " << functionName << ". There are " << foundFunctions.size() << " potential candidates: \n";
			for (auto& iter : foundFunctions)
			{
				errorStream << "\t" << iter->getReturnType().toString() << " " << functionName << "(" << ASTHelper::getTypeListString(iter->getArgumentTypes()) << ")\n";
			}
			errorManager->compiledWithError(errorStream.str(), errorSource, context->getContextName(), lexeme);
			return nullptr;
		}
	}
	return nullptr;
}


std::string jitcat::AST::ASTHelper::getTypeListString(const std::vector<CatGenericType>& types)
{
	std::ostringstream typeListStream;
	for (std::size_t i = 0; i < types.size(); ++i)
	{
		if (i != 0) typeListStream << ", ";
		typeListStream << types[i].toString();
	}
	return typeListStream.str();
}


bool jitcat::AST::ASTHelper::checkAssignment(const CatTypedExpression* lhs, const CatTypedExpression* rhs, ExpressionErrorManager* errorManager, CatRuntimeContext* context, void* errorSource, const Tokenizer::Lexeme& lexeme)
{
	CatGenericType leftType = lhs->getType();
	CatGenericType rightType = rhs->getType();
	if ((!leftType.isWritable()) 
		|| leftType.isConst() 
		|| !lhs->isAssignable())
	{
		errorManager->compiledWithError("Assignment failed because target cannot be assigned.", errorSource, context->getContextName(), lexeme);
		return false;
	}
	else
	{
		if (leftType.compare(rightType, false, false))
		{
			if (!checkOwnershipSemantics(leftType, rightType, errorManager, context, errorSource, lexeme, "assign"))
			{
				return false;
			}
			return true;
		}
		else
		{
			errorManager->compiledWithError(Tools::append("Cannot assign ", rightType.toString(), " to ", leftType.toString(), "."), errorSource, context->getContextName(), lexeme);
			return false;
		}
	}
}


bool jitcat::AST::ASTHelper::checkOwnershipSemantics(const CatGenericType& targetType, const CatGenericType& sourceType, ExpressionErrorManager* errorManager, CatRuntimeContext* context, void* errorSource, const Tokenizer::Lexeme& lexeme, const std::string& operation)
{
	TypeOwnershipSemantics leftOwnership = targetType.getOwnershipSemantics();
	TypeOwnershipSemantics rightOwnership = sourceType.getOwnershipSemantics();
	if (leftOwnership == TypeOwnershipSemantics::Owned)
	{
		if (rightOwnership == TypeOwnershipSemantics::Shared)
		{
			errorManager->compiledWithError(Tools::append("Cannot ", operation, " shared ownership value to unique ownership value."), errorSource, context->getContextName(), lexeme);
			return false;
		}
		else if (rightOwnership == TypeOwnershipSemantics::Weak)
		{
			errorManager->compiledWithError(Tools::append("Cannot ", operation, " weakly-owned value to unique ownership value."), errorSource, context->getContextName(), lexeme);
			return false;
		}
	}
	else if (leftOwnership == TypeOwnershipSemantics::Shared)
	{
		if (rightOwnership == TypeOwnershipSemantics::Weak)
		{
			errorManager->compiledWithError(Tools::append("Cannot ", operation, " weakly-owned value to shared ownership value."), errorSource, context->getContextName(), lexeme);
			return false;
		}
	}
	/*else if (leftOwnership == TypeOwnershipSemantics::Weak)
	{
		if (rightOwnership == TypeOwnershipSemantics::Value && !sourceType.isNullptrType())
		{
			errorManager->compiledWithError(Tools::append("Cannot ", operation, " owned temporary value to weak ownership value."), errorSource, context->getContextName(), lexeme);
			return false;
		}
	}*/

	if (rightOwnership == TypeOwnershipSemantics::Owned
		&& (leftOwnership == TypeOwnershipSemantics::Owned
			|| leftOwnership == TypeOwnershipSemantics::Shared))
	{
		if (!sourceType.isWritable() || sourceType.isConst())
		{
			errorManager->compiledWithError("Cannot write from owned value because rhs cannot be assigned.", errorSource, context->getContextName(), lexeme);
			return false;
		}
	}
	return true;
}
