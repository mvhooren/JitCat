/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "LLVMCodeGenerator.h"
#include "CatASTNodes.h"
#include "LLVMCodeGeneratorHelper.h"
#include "LLVMCompileTimeContext.h"
#include "LLVMJit.h"
#include "LLVMTypes.h"
#include "MemberFunctionInfo.h"
#include "MemberReference.h"
#include "MemberInfo.h"

//QQQ not portable, fix
#pragma warning(push, 0)        
//Disable warnings from llvm includes
#include <llvm\IR\Constant.h>
#include <llvm\IR\Function.h>
#include <llvm\IR\LegacyPassManager.h>
#include <llvm\IR\PassManager.h>
#include <llvm\IR\IRBuilder.h>
#include <llvm\IR\Module.h>
#include <llvm\IR\Value.h>
#include <llvm\IR\Verifier.h>
#include <llvm\Support\raw_ostream.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm\Transforms\Scalar.h>
#include <llvm\Transforms\Scalar\GVN.h>
#pragma warning(pop)

#include <iostream>


LLVMCodeGenerator::LLVMCodeGenerator():
	currentModule(new llvm::Module("JitCat", LLVMJit::get().getContext())),
	builder(new llvm::IRBuilder<>(LLVMJit::get().getContext())),
	llvmContext(LLVMJit::get().getContext()),
	helper(new LLVMCodeGeneratorHelper(builder.get(), currentModule.get()))
{
	std::string targetTriple = LLVMJit::get().getTargetMachine().getTargetTriple().str();
	std::cout << "Constructed code generator for target: " << targetTriple << "\n";
	currentModule->setTargetTriple(targetTriple);
	currentModule->setDataLayout(LLVMJit::get().getDataLayout());
	helper->setCurrentModule(currentModule.get());
	// Create a new pass manager attached to it.
	passManager = llvm::make_unique<llvm::legacy::FunctionPassManager>(currentModule.get());

	// Do simple "peephole" optimizations and bit-twiddling optzns.
	passManager->add(llvm::createInstructionCombiningPass());
	// Reassociate expressions.
	passManager->add(llvm::createReassociatePass());
	// Eliminate Common SubExpressions.
	passManager->add(llvm::createGVNPass());
	// Simplify the control flow graph (deleting unreachable blocks, etc).
	passManager->add(llvm::createCFGSimplificationPass());

	passManager->doInitialization();

}


LLVMCodeGenerator::~LLVMCodeGenerator()
{
}


llvm::Value* LLVMCodeGenerator::generate(CatTypedExpression* expression, LLVMCompileTimeContext* context)
{
	switch (expression->getNodeType())
	{
		case CatASTNodeType::Literal:				return generate(static_cast<CatLiteral*>(expression), context);			
		case CatASTNodeType::Identifier:			return generate(static_cast<CatIdentifier*>(expression), context);		
		case CatASTNodeType::InfixOperator:			return generate(static_cast<CatInfixOperator*>(expression), context);	
		case CatASTNodeType::PrefixOperator:		return generate(static_cast<CatPrefixOperator*>(expression), context);	
		case CatASTNodeType::FunctionCall:			return generate(static_cast<CatFunctionCall*>(expression), context);		
		case CatASTNodeType::MemberAccess:			return generate(static_cast<CatMemberAccess*>(expression), context);		
		case CatASTNodeType::ArrayIndex:			return generate(static_cast<CatArrayIndex*>(expression), context);		
		case CatASTNodeType::MemberFunctionCall:	return generate(static_cast<CatMemberFunctionCall*>(expression), context);
		case CatASTNodeType::ScopeRoot:				return generate(static_cast<CatScopeRoot*>(expression), context);		
	}
	assert(false);
	return nullptr;
}


llvm::Function* LLVMCodeGenerator::generateExpressionFunction(CatTypedExpression* expression, LLVMCompileTimeContext* context, const std::string& name)
{
	//This parameter represents the CatRuntimeContext*
	std::vector<llvm::Type*> parameters;
	llvm::FunctionType* functionType = nullptr;
	CatType expressionType = expression->getType().getCatType();
	if (expressionType == CatType::String)
	{
		parameters.push_back(LLVMTypes::stringPtrType);
		parameters.push_back(LLVMTypes::pointerType);
		functionType = llvm::FunctionType::get(LLVMTypes::voidType, parameters, false);		
	}
	else
	{
		parameters.push_back(LLVMTypes::pointerType);
		functionType = llvm::FunctionType::get(helper->toLLVMType(expressionType), parameters, false);
	}

	llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkageTypes::ExternalLinkage, name.c_str(), currentModule.get());
	llvm::Argument* firstArg = function->arg_begin();
	if (expressionType == CatType::String)
	{
		firstArg->setName("stringRet");
		firstArg++;
		firstArg->setName("RuntimeContext");
		function->addParamAttr(0, llvm::Attribute::AttrKind::StructRet);
		function->addParamAttr(0, llvm::Attribute::AttrKind::NoAlias);
	}
	else
	{
		firstArg->setName("RuntimeContext");
	}
	
	
	context->currentFunction = function;
	//Function body
	llvm::BasicBlock* block = llvm::BasicBlock::Create(llvmContext, "entry", function);
	builder->SetInsertPoint(&function->getEntryBlock());
	llvm::Value* expressionValue = generate(expression, context);
	if (expressionType == CatType::String)
	{
		helper->intrinsics->callStringCopy(function->arg_begin(), expressionValue);
		helper->generateBlockDestructors(context);
		builder->CreateRetVoid();
	}
	else
	{
		helper->generateBlockDestructors(context);
		builder->CreateRet(expressionValue);
	}
	context->currentFunction = nullptr;
#ifdef _DEBUG //QQQ
	function->dump();
#endif
	if (!llvm::verifyFunction(*function, &llvm::outs()))
	{
		passManager->run(*function);
		
		return function;
	}
	else
	{
		assert(false);
		LLVMJit::logError("Function contains errors.");
		return nullptr;
	}
}


intptr_t LLVMCodeGenerator::generateAndGetFunctionAddress(CatTypedExpression* expression, LLVMCompileTimeContext* context)
{
	currentModule.reset(new llvm::Module(context->catContext->getContextName(), llvmContext));
	currentModule->setTargetTriple(LLVMJit::get().getTargetMachine().getTargetTriple().str());
	currentModule->setDataLayout(LLVMJit::get().getDataLayout());
	helper->setCurrentModule(currentModule.get());
	std::string functionName = Tools::append("expression_", context->catContext->getContextName(), "_", context->catContext->getNextFunctionIndex());
	llvm::Function* function = generateExpressionFunction(expression, context, functionName);
	assert(function != nullptr);
	LLVMJit::get().addModule(currentModule);
	return LLVMJit::get().getSymbolAddress(functionName.c_str());
}


void LLVMCodeGenerator::generateAndDump(CatTypedExpression* expression, LLVMCompileTimeContext* context, const std::string& functionName)
{
	if (expression->getType().isValidType())
	{
		llvm::Function* generatedValue = generateExpressionFunction(expression, context, functionName.c_str());
		#ifdef _DEBUG //QQQ
		if (generatedValue != nullptr)
		{
			generatedValue->dump();
		}
		#endif
	}
}


void LLVMCodeGenerator::compileAndTest(CatRuntimeContext* context, const std::string& functionName)
{
	std::cout << "Adding module " << currentModule->getName().str() << " machine: " << currentModule->getTargetTriple() << " layout: " << currentModule->getDataLayoutStr() << "\n";
	const auto& functionList = currentModule->getFunctionList();
	std::cout << "Functions:\n";
	const llvm::Function* function = nullptr;
	for (const auto& iter : functionList)
	{
		if (iter.getName() == functionName)
		{
			function = &iter;
		}
		std::cout << "\t" << iter.getName().str() << "\n";
	}
	llvm::Type* returnType = nullptr;
	if (function != nullptr)
	{
		returnType = function->getReturnType();
	}
	//function is destructed after addModule
	LLVMJit::get().addModule(currentModule);
	
	llvm::JITTargetAddress address = LLVMJit::get().getSymbolAddress(functionName.c_str());
	if (address == 0)
	{
		return;
	}

    if (returnType == LLVMTypes::floatType)
	{
		float (*getResult)(void* context) = reinterpret_cast<float (*)(void*)>(address);
		std::cout << "Result call (float): " << getResult(context) << "\n";	
	}
	else if (returnType == LLVMTypes::intType)
	{
		int (*getResult)(void* context) = reinterpret_cast<int (*)(void*)>(address);
		std::cout << "Result call (int): " << getResult(context) << "\n";	
	}
	else if (returnType == LLVMTypes::boolType)
	{
		bool (*getResult)(void* context) = reinterpret_cast<bool (*)(void*)>(address);
		std::cout << "Result call (bool): " << getResult(context) << "\n";	
	}
	else if (returnType == LLVMTypes::voidType)//QQQ base this on the AST instead of the IR
	{
		std::string (*getResult)(void* context) = reinterpret_cast<std::string (*)(void*)>(address);
		std::string result = getResult(context);
		std::cout << "Result call (string): " << result << "\n";	
	}
	else if (returnType == LLVMTypes::pointerType)
	{
		Reflectable* (*getResult)(void* context) = reinterpret_cast<Reflectable* (*)(void*)>(address);
		std::cout << "Result call (object): " << std::hex << getResult(context) << "\n";	
	}
}


llvm::Value* LLVMCodeGenerator::generate(CatIdentifier* identifier, LLVMCompileTimeContext* context)
{
	RootTypeSource source = identifier->getSource();
	llvm::Value* parentObjectAddress = getBaseAddress(source, context);

	const TypeMemberInfo* memberInfo = identifier->getMemberInfo();

	llvm::Value* result = memberInfo->generateDereferenceCode(parentObjectAddress, helper.get());
	if (result != nullptr)
	{
		return result;
	}
	else
	{
		assert(false);
		return LLVMJit::logError("ERROR: Not yet supported.");
	}
}


llvm::Value* LLVMCodeGenerator::generate(CatFunctionCall* functionCall, LLVMCompileTimeContext* context)
{
	CatArgumentList* arguments = functionCall->getArgumentList();
	switch (functionCall->getFunctionType())
	{
		case CatBuiltInFunctionType::ToInt:		return helper->convertType(generate(arguments->arguments[0].get(), context), LLVMTypes::intType, context);
		case CatBuiltInFunctionType::ToFloat:	return helper->convertType(generate(arguments->arguments[0].get(), context), LLVMTypes::floatType, context);
		case CatBuiltInFunctionType::ToBool:	return helper->convertType(generate(arguments->arguments[0].get(), context), LLVMTypes::boolType, context);
		case CatBuiltInFunctionType::Select:
		{
			llvm::Value* conditionValue = helper->convertType(generate(arguments->arguments[0].get(), context), LLVMTypes::boolType, context);
			llvm::Value* trueValue = generate(arguments->arguments[1].get(), context);
			llvm::Value* falseValue = helper->convertType(generate(arguments->arguments[2].get(), context), trueValue->getType(), context);
			return builder->CreateSelect(conditionValue, trueValue, falseValue);
		}
		case CatBuiltInFunctionType::Abs:
		{
			if (arguments->arguments[0]->getType() == CatType::Float)
			{
				return helper->callIntrinsic(llvm::Intrinsic::fabs, CatType::Float, generate(arguments->arguments[0].get(), context), context);
			}
			else if (arguments->arguments[0]->getType() == CatType::Int)
			{
				llvm::Value* intValue = generate(arguments->arguments[0].get(), context);
				llvm::Value* isGreaterOrEqualToZero = builder->CreateICmpSGT(intValue, helper->createConstant(-1));
				llvm::Value* negated = builder->CreateSub(helper->createConstant(0), intValue);
				return builder->CreateSelect(isGreaterOrEqualToZero, intValue, negated);
			}
			return nullptr;
		}
		case CatBuiltInFunctionType::Log:	return helper->callIntrinsic(llvm::Intrinsic::log10, CatType::Float, generate(arguments->arguments[0].get(), context), context);
		case CatBuiltInFunctionType::Sqrt:	return helper->callIntrinsic(llvm::Intrinsic::sqrt, CatType::Float, generate(arguments->arguments[0].get(), context), context);
		case CatBuiltInFunctionType::Pow:
		{
			if (arguments->arguments[1]->getType() == CatType::Float)
			{
				return helper->convertType(helper->callIntrinsic(llvm::Intrinsic::pow, CatType::Float, CatType::Float, generate(arguments->arguments[0].get(), context), generate(arguments->arguments[1].get(), context), context), helper->toLLVMType(arguments->arguments[0]->getType().getCatType()), context);
			}
			else if (arguments->arguments[1]->getType() == CatType::Int)
			{
				return helper->convertType(helper->callIntrinsic(llvm::Intrinsic::powi, CatType::Float, CatType::Int, generate(arguments->arguments[0].get(), context), generate(arguments->arguments[1].get(), context), context), helper->toLLVMType(arguments->arguments[0]->getType().getCatType()), context);
			}
			else
			{
				assert(false);
				return nullptr;
			}
		}
		case CatBuiltInFunctionType::Sin:		return helper->callIntrinsic(llvm::Intrinsic::sin, CatType::Float, generate(arguments->arguments[0].get(), context), context);
		case CatBuiltInFunctionType::Cos:		return helper->callIntrinsic(llvm::Intrinsic::cos, CatType::Float, generate(arguments->arguments[0].get(), context), context);
		case CatBuiltInFunctionType::Ceil:		return helper->callIntrinsic(llvm::Intrinsic::ceil, CatType::Float, generate(arguments->arguments[0].get(), context), context);
		case CatBuiltInFunctionType::Floor:		return helper->callIntrinsic(llvm::Intrinsic::floor, CatType::Float, generate(arguments->arguments[0].get(), context), context);
		case CatBuiltInFunctionType::Tan:		return builder->CreateFDiv(helper->callIntrinsic(llvm::Intrinsic::sin, CatType::Float, generate(arguments->arguments[0].get(), context), context), helper->callIntrinsic(llvm::Intrinsic::cos, CatType::Float, generate(arguments->arguments[0].get(), context), context));
		case CatBuiltInFunctionType::Cap:
		{
			llvm::Value* value = generate(arguments->arguments[0].get(), context);
			llvm::Value* minValue = generate(arguments->arguments[1].get(), context);
			llvm::Value* maxValue = generate(arguments->arguments[2].get(), context);
			if (arguments->arguments[0]->getType() == CatType::Float)
			{
				llvm::Value* convertedMin = helper->convertType(minValue, LLVMTypes::floatType, context);
				llvm::Value* convertedMax = helper->convertType(maxValue, LLVMTypes::floatType, context);
				llvm::Value* smallerThanMin = builder->CreateFCmp(llvm::CmpInst::Predicate::FCMP_OLT, value, convertedMin);
				llvm::Value* greaterThanMax = builder->CreateFCmp(llvm::CmpInst::Predicate::FCMP_OGT, value, convertedMax);
				llvm::Value* cappedToMin = builder->CreateSelect(smallerThanMin, convertedMin, value);
				llvm::Value* cappedToMax = builder->CreateSelect(greaterThanMax, convertedMax, cappedToMin);
				return cappedToMax;
			}
			else if (arguments->arguments[0]->getType() == CatType::Int)
			{
				llvm::Value* convertedMin = helper->convertType(minValue, LLVMTypes::intType, context);
				llvm::Value* convertedMax = helper->convertType(maxValue, LLVMTypes::intType, context);
				llvm::Value* smallerThanMin = builder->CreateICmp(llvm::CmpInst::Predicate::ICMP_SLT, value, convertedMin);
				llvm::Value* greaterThanMax = builder->CreateFCmp(llvm::CmpInst::Predicate::ICMP_SGT, value, convertedMax);
				llvm::Value* cappedToMin = builder->CreateSelect(smallerThanMin, convertedMin, value);
				llvm::Value* cappedToMax = builder->CreateSelect(greaterThanMax, convertedMax, cappedToMin);
				return cappedToMax;
			}
			else
			{
				assert(false);
				return nullptr;
			}
		}
		case CatBuiltInFunctionType::Min:
		{
			llvm::Value* value1 = generate(arguments->arguments[0].get(), context);
			llvm::Value* value2 = generate(arguments->arguments[1].get(), context);
			if (arguments->arguments[0]->getType() == CatType::Float)
			{
				llvm::Value* convertedValue2 = helper->convertType(value2, LLVMTypes::floatType, context);
				llvm::Value* lessThan = builder->CreateFCmp(llvm::CmpInst::Predicate::FCMP_OLT, value1, convertedValue2);
				return builder->CreateSelect(lessThan, value1, convertedValue2);
			}
			else if (arguments->arguments[0]->getType() == CatType::Int)
			{
				llvm::Value* convertedValue2 = helper->convertType(value2, LLVMTypes::intType, context);
				llvm::Value* lessThan = builder->CreateFCmp(llvm::CmpInst::Predicate::ICMP_SLT, value1, convertedValue2);
				return builder->CreateSelect(lessThan, value1, convertedValue2);
			}
			else
			{
				assert(false);
				return nullptr;
			}
		}
		case CatBuiltInFunctionType::Max:
		{
			llvm::Value* value1 = generate(arguments->arguments[0].get(), context);
			llvm::Value* value2 = generate(arguments->arguments[1].get(), context);
			if (arguments->arguments[0]->getType() == CatType::Float)
			{
				llvm::Value* convertedValue2 = helper->convertType(value2, LLVMTypes::floatType, context);
				llvm::Value* greaterThan = builder->CreateFCmp(llvm::CmpInst::Predicate::FCMP_OGT, value1, convertedValue2);
				return builder->CreateSelect(greaterThan, value1, convertedValue2);
			}
			else if (arguments->arguments[0]->getType() == CatType::Int)
			{
				llvm::Value* convertedValue2 = helper->convertType(value2, LLVMTypes::intType, context);
				llvm::Value* greaterThan = builder->CreateFCmp(llvm::CmpInst::Predicate::ICMP_SGT, value1, convertedValue2);
				return builder->CreateSelect(greaterThan, value1, convertedValue2);
			}
			else
			{
				assert(false);
				return nullptr;
			}
		}
		case CatBuiltInFunctionType::FindInString:
		case CatBuiltInFunctionType::ReplaceInString:
		case CatBuiltInFunctionType::StringLength:
		case CatBuiltInFunctionType::SubString:
		case CatBuiltInFunctionType::ToString:
		case CatBuiltInFunctionType::ToPrettyString:
		case CatBuiltInFunctionType::ToFixedLengthString:
		case CatBuiltInFunctionType::Random:
		case CatBuiltInFunctionType::RandomRange:
		case CatBuiltInFunctionType::Round:
		case CatBuiltInFunctionType::StringRound:

		default:
		{
			// Look up the name in the global module table.
			llvm::Function* functionInModule = currentModule->getFunction(functionCall->getFunctionName());
			if (functionInModule == nullptr)
			{
				assert(false);
				return LLVMJit::logError("Unknown function referenced: ", functionCall->getFunctionName());
			}

			// If argument mismatch error.
			if (functionInModule->arg_size() != arguments->arguments.size())
			{
				assert(false);
				return LLVMJit::logError("Incorrect # arguments passed in function: ", functionCall->getFunctionName());
			}

			std::vector<llvm::Value*> argumentValues;
			for (std::size_t i = 0, e = arguments->arguments.size(); i != e; ++i)
			{
				llvm::Value* generatedValue = generate(arguments->arguments[i].get(), context);
				assert(generatedValue != nullptr);
				argumentValues.push_back(generatedValue);
			}

			return builder->CreateCall(functionInModule, argumentValues, "calltmp");
		}
	}
}


llvm::Value* LLVMCodeGenerator::generate(CatInfixOperator* infixOperator, LLVMCompileTimeContext* context)
{
	llvm::Value* left = generate(infixOperator->lhs.get(), context);
	llvm::Value* right = generate(infixOperator->rhs.get(), context);
	assert(left != nullptr && right != nullptr);
	/*if (left->getType()->isArrayTy()
		|| right->getType()->isArrayTy())
	{
		assert(false);
		return LLVMJit::logError("ERROR: String operations not yet supported.");
	}
	if (!(left->getType() == LLVMTypes::intType || left->getType() == LLVMTypes::boolType || left->getType() == LLVMTypes::floatType)
		|| !(right->getType() == LLVMTypes::intType || right->getType() == LLVMTypes::boolType || right->getType() == LLVMTypes::floatType))
	{
		assert(false);
		return LLVMJit::logError("ERROR: Type not yet supported for infix operators.");
	}*/



	if (infixOperator->oper == CatInfixOperatorType::LogicalOr)
	{
		return builder->CreateOr(helper->convertType(left, LLVMTypes::boolType, context), helper->convertType(right, LLVMTypes::boolType, context), "or");
	}
	else if (infixOperator->oper == CatInfixOperatorType::LogicalAnd)
	{
		return builder->CreateAnd(helper->convertType(left, LLVMTypes::boolType, context), helper->convertType(right, LLVMTypes::boolType, context), "and");
	}
	else if (left->getType() != right->getType())
	{
		//Operators must always operate on identical types.
		//If types are different, we must first convert them.
		if (left->getType() == LLVMTypes::stringPtrType
			|| right->getType() == LLVMTypes::stringPtrType)
		{
			left = helper->convertType(left, LLVMTypes::stringPtrType, context);
			right = helper->convertType(right, LLVMTypes::stringPtrType, context);
		}
		else if (left->getType() == LLVMTypes::floatType
			|| right->getType() == LLVMTypes::floatType)
		{
			left = helper->convertType(left, LLVMTypes::floatType, context);
			right = helper->convertType(right, LLVMTypes::floatType, context);
		}
		else
		{
			//left and right are ints or booleans, booleans will be promoted to ints
			left = helper->convertType(left, LLVMTypes::intType, context);
			right = helper->convertType(right, LLVMTypes::intType, context);
		}
	}
	if (left->getType() == LLVMTypes::floatType)
	{
		switch (infixOperator->oper)
		{
			case CatInfixOperatorType::Plus:				return builder->CreateFAdd(left, right, "added");				
			case CatInfixOperatorType::Minus:				return builder->CreateFSub(left, right, "subtracted");		
			case CatInfixOperatorType::Multiply:			return builder->CreateFMul(left, right, "multiplied");		
			case CatInfixOperatorType::Divide:				return builder->CreateFDiv(left, right, "divided");			
			case CatInfixOperatorType::Modulo:				return builder->CreateFRem(left, right, "modulo");			
			case CatInfixOperatorType::Greater:				return builder->CreateFCmpUGT(left, right, "greater");		
			case CatInfixOperatorType::Smaller:				return builder->CreateFCmpULT(left, right, "smaller");		
			case CatInfixOperatorType::GreaterOrEqual:		return builder->CreateFCmpUGE(left, right, "greaterOrEqual");	
			case CatInfixOperatorType::SmallerOrEqual:		return builder->CreateFCmpULE(left, right, "lessOrEqual");	
			case CatInfixOperatorType::Equals:				return builder->CreateFCmpUEQ(left, right, "equal");			
			case CatInfixOperatorType::NotEquals:			return builder->CreateFCmpUNE(left, right, "notEqual");		
		}
	}
	else if (left->getType() == LLVMTypes::intType)
	{
		switch (infixOperator->oper)
		{
			case CatInfixOperatorType::Plus:				return builder->CreateAdd(left, right, "added");				
			case CatInfixOperatorType::Minus:				return builder->CreateSub(left, right, "subtracted");			
			case CatInfixOperatorType::Multiply:			return builder->CreateMul(left, right, "multiplied");			
			case CatInfixOperatorType::Divide:				return builder->CreateSDiv(left, right, "divided");			
			case CatInfixOperatorType::Modulo:				return builder->CreateSRem(left, right, "modulo");			
			case CatInfixOperatorType::Greater:				return builder->CreateICmpSGT(left, right, "greater");		
			case CatInfixOperatorType::Smaller:				return builder->CreateICmpSLT(left, right, "smaller");		
			case CatInfixOperatorType::GreaterOrEqual:		return builder->CreateICmpSGE(left, right, "greaterOrEqual");	
			case CatInfixOperatorType::SmallerOrEqual:		return builder->CreateICmpSLE(left, right, "smallerOrEqual");	
			case CatInfixOperatorType::Equals:				return builder->CreateICmpEQ(left, right, "equal");			
			case CatInfixOperatorType::NotEquals:			return builder->CreateICmpNE(left, right, "notEqual");		
		}
	}
	else if (left->getType() == LLVMTypes::stringPtrType)
	{
		switch (infixOperator->oper)
		{
			case CatInfixOperatorType::Plus:				return helper->intrinsics->callStringAppend(left, right, context);				
			case CatInfixOperatorType::Equals:				return helper->intrinsics->callStringEquals(left, right);			
			case CatInfixOperatorType::NotEquals:			return helper->intrinsics->callStringNotEquals(left, right);		
		}
	}
	assert(false);
	return LLVMJit::logError("ERROR: Invalid operation.");
}


llvm::Value* LLVMCodeGenerator::generate(CatLiteral* literal, LLVMCompileTimeContext* context)
{
	switch (literal->getValueType())
	{
		case CatType::Int:			return helper->createConstant(literal->getIntValue());
		case CatType::Float:		return helper->createConstant(literal->getFloatValue());
		case CatType::String:
		{
			llvm::Value* stringObjectAddress = helper->createIntPtrConstant(reinterpret_cast<std::uintptr_t>(&literal->getStringValue()), "stringLiteralAddress");
			return builder->CreateIntToPtr(stringObjectAddress, LLVMTypes::stringPtrType);													
		}
		case CatType::Bool:			return helper->createConstant(literal->getBoolValue());
		default:
		case CatType::Void:
		case CatType::Object:
		case CatType::Unknown:
		case CatType::Error:		assert(false); return LLVMJit::logError("ERROR: Not a basic type."); 
	}
}


llvm::Value* LLVMCodeGenerator::generate(CatMemberAccess* memberAccess, LLVMCompileTimeContext* context)
{
	llvm::Value* base = generate(memberAccess->getBase(), context);
	return memberAccess->getMemberInfo()->generateDereferenceCode(base, helper.get());
}


llvm::Value* LLVMCodeGenerator::generate(CatMemberFunctionCall* memberFunctionCall, LLVMCompileTimeContext* context)
{
	MemberFunctionInfo* functionInfo = memberFunctionCall->getMemberFunctionInfo();
	CatTypedExpression* base = memberFunctionCall->getBase();
	CatArgumentList* arguments = memberFunctionCall->getArguments();
	CatGenericType& returnType = functionInfo->returnType;

	llvm::Value* baseObject = generate(base, context);
	std::vector<llvm::Value*> argumentList;
	llvm::Value* functionThis = helper->convertToPointer(baseObject, memberFunctionCall->getMemberFunctionInfo()->memberFunctionName + "_This_Ptr");
	argumentList.push_back(functionThis);
	std::vector<llvm::Type*> argumentTypes;
	argumentTypes.push_back(LLVMTypes::pointerType);
	for (auto& iter : arguments->arguments)
	{
		argumentList.push_back(generate(iter.get(), context));
		argumentTypes.push_back(argumentList.back()->getType());
	}
	if (returnType.getCatType() != CatType::String && !returnType.isContainerType())
	{
		llvm::Type* returnLLVMType = helper->toLLVMType(returnType.getCatType());
		llvm::FunctionType* functionType = llvm::FunctionType::get(returnLLVMType, argumentTypes, false);
		return helper->callFunction(functionType, functionInfo->getFunctionAddress(), argumentList, base->getType().toString() + "." + memberFunctionCall->getMemberFunctionInfo()->memberFunctionName);
	}
	else if (returnType.getCatType() == CatType::String)
	{
		llvm::Value* stringObjectAllocation = helper->createStringAllocA(context, memberFunctionCall->getMemberFunctionInfo()->memberFunctionName + "_Result");
		argumentTypes.insert(++(argumentTypes.begin()), LLVMTypes::stringPtrType);
		llvm::FunctionType* functionType = llvm::FunctionType::get(LLVMTypes::voidType, argumentTypes, false);
		argumentList.insert(++(argumentList.begin()), stringObjectAllocation);
		llvm::CallInst* call = static_cast<llvm::CallInst*>(helper->callFunction(functionType, functionInfo->getFunctionAddress(), argumentList, base->getType().toString() + "." + memberFunctionCall->getMemberFunctionInfo()->memberFunctionName));
		call->addParamAttr(1, llvm::Attribute::AttrKind::StructRet);
		call->addDereferenceableAttr(2, sizeof(std::string));
		return stringObjectAllocation;
	}
	else
	{
		return LLVMJit::logError("ERROR: Not yet supported.");
	}
}


llvm::Value* LLVMCodeGenerator::generate(CatPrefixOperator* prefixOperator, LLVMCompileTimeContext* context)
{
	llvm::Value* right = generate(prefixOperator->rhs.get(), context);
	assert(right != nullptr);
	llvm::Type* rightType = right->getType();
	if (!(rightType == LLVMTypes::intType || rightType == LLVMTypes::boolType || rightType == LLVMTypes::floatType))
	{
		assert(false);
		return LLVMJit::logError("ERROR: Type not yet supported for prefix operators.");
	}
	if (prefixOperator->oper == CatPrefixOperator::Operator::Not)
	{
		return builder->CreateNot(helper->convertType(right, LLVMTypes::boolType, context), "not");
	}
	else if (prefixOperator->oper == CatPrefixOperator::Operator::Minus)
	{
		if (rightType == LLVMTypes::floatType)
		{
			return builder->CreateFNeg(right, "negative");
		}
		else
		{
			return builder->CreateNeg(helper->convertType(right, LLVMTypes::intType, context), "negative");
		}
	}
	else
	{
		assert(false);
		return LLVMJit::logError("ERROR: Operator not implemented.");
	}
}


llvm::Value* LLVMCodeGenerator::generate(CatArrayIndex* arrayIndex, LLVMCompileTimeContext* context)
{
	CatTypedExpression* base = arrayIndex->getBase();
	const TypeMemberInfo* memberInfo = nullptr;
	if (base->getNodeType() == CatASTNodeType::Identifier)
	{
		memberInfo = static_cast<CatIdentifier*>(base)->getMemberInfo();
	}
	else if (base->getNodeType() == CatASTNodeType::MemberAccess)
	{
		memberInfo = static_cast<CatMemberAccess*>(base)->getMemberInfo();
	}
	assert(memberInfo != nullptr);

	CatTypedExpression* index = arrayIndex->getIndex();
	llvm::Value* containerAddress = generate(base, context);
	llvm::Value* containerIndex = generate(index, context);
	return memberInfo->generateArrayIndexCode(containerAddress, containerIndex, helper.get());
}


llvm::Value* LLVMCodeGenerator::generate(CatScopeRoot* scopeRoot, LLVMCompileTimeContext* context)
{
	return getBaseAddress(scopeRoot->getSource(), context);
}


llvm::Value* LLVMCodeGenerator::getBaseAddress(RootTypeSource source, LLVMCompileTimeContext* context)
{
	llvm::Value* parentObjectAddress = nullptr;
	switch (source)
	{
		case RootTypeSource::Global:
		{
			MemberReferencePtr ref = context->catContext->getGlobalReference();
			Reflectable* object = ref->getParentObject();
			parentObjectAddress = llvm::ConstantInt::get(llvmContext, llvm::APInt(sizeof(std::uintptr_t) * 8, reinterpret_cast<std::size_t>(object), false));
		} break;
		case RootTypeSource::CustomGlobals:
		{
			MemberReferencePtr ref = context->catContext->getRootReference(RootTypeSource::CustomGlobals);
			Reflectable* object = ref->getParentObject();
			parentObjectAddress = llvm::ConstantInt::get(llvmContext, llvm::APInt(sizeof(std::uintptr_t) * 8, reinterpret_cast<std::size_t>(object), false));
		} break;
		case RootTypeSource::This:
		case RootTypeSource::CustomThis:
		{
			//Get the CatRuntimeContext argument from the current function
			assert(context->currentFunction != nullptr);
			assert(context->currentFunction->arg_size() > 0);
			llvm::Argument* argument = context->currentFunction->arg_begin();
			if (context->currentFunction->arg_size() == 2)
			{
				argument = context->currentFunction->arg_begin() + 1;
			}
			assert(argument->getName() == "RuntimeContext");
			assert(argument->getType() == LLVMTypes::pointerType);
			llvm::Value* address = nullptr;
			if (source == RootTypeSource::This)
			{
				address = helper->intrinsics->callGetThisPointer(argument); 
				
			}
			else if (source == RootTypeSource::CustomThis)
			{
				address = helper->intrinsics->callGetCustomThisPointer(argument);
			}
			assert(address != nullptr);
			parentObjectAddress = helper->convertToIntPtr(address, "CustomThis_IntPtr");
		} break;
	}
	return parentObjectAddress;
}

