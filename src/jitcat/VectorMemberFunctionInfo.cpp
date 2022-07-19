#include "jitcat/VectorMemberFunctionInfo.h"
#include "jitcat/VectorTypeInfo.h"
#ifdef ENABLE_LLVM
	#include <llvm/IR/IRBuilder.h>
	#include <llvm/IR/MDBuilder.h>
	#include "jitcat/LLVMTargetConfig.h"
	#include "jitcat/LLVMTypes.h"
#endif

#include <cassert>

using namespace jitcat;
using namespace jitcat::Reflection;


const char* VectorMemberFunctionInfo::toString(Operation operation)
{
	switch (operation)
	{
		default: assert(false);		return "none";
		case Operation::Index:		return "[]";
		case Operation::Init:		return "__init";
		case Operation::Destroy:	return "__destroy";
	}
}


VectorMemberFunctionInfo::VectorMemberFunctionInfo(Operation operation, VectorTypeInfo* vectorTypeInfo):
	MemberFunctionInfo(toString(operation), CatGenericType::voidType),
	operation(operation),
	vectorTypeInfo(vectorTypeInfo)
{
	switch (operation)
	{
		case Operation::Index:
		{
			returnType = vectorTypeInfo->getScalarType().toPointer(TypeOwnershipSemantics::Weak, true, false);
			argumentTypes.push_back(CatGenericType::intType);
		} break;
		case Operation::Init:
		case Operation::Destroy:
		{
			returnType = CatGenericType::voidType;
		} break;
	}

	switch (operation)
	{
		default: assert(false);	break;
		case Operation::Index:		createIndexGeneratorFunction();		break;
		case Operation::Init:		createInitGeneratorFunction();		break;
		case Operation::Destroy:	createDestroyGeneratorFunction();	break;
	}
}


std::any VectorMemberFunctionInfo::call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) const
{
	if (base.has_value())
	{
		switch (operation)
		{
			default:					assert(false); return std::any();
			case Operation::Index:		return doIndex(base, parameters);
			case Operation::Init:		doInit(base, parameters);	return std::any();
			case Operation::Destroy:	return std::any();
		}			
		
	}
	switch (operation)
	{
		default: assert(false);		return std::any();
		case Operation::Index:		return vectorTypeInfo->getScalarType().createDefault();
		case Operation::Init:		return std::any();
		case Operation::Destroy:	return std::any();
	}
}


MemberFunctionCallData VectorMemberFunctionInfo::getFunctionAddress(FunctionType functionType) const
{
	return MemberFunctionCallData();
}


std::any jitcat::Reflection::VectorMemberFunctionInfo::doIndex(std::any& base, const std::vector<std::any>& parameters) const
{
	assert(parameters.size() == 1);
	const unsigned char* constBuffer = nullptr;
	std::size_t bufferSize = 0;
	vectorTypeInfo->getTypeCaster()->toBuffer(base, constBuffer, bufferSize);
	unsigned char* buffer = const_cast<unsigned char*>(constBuffer);
	int index = std::any_cast<int>(parameters[0]);
	if (index >= 0 && (std::size_t)index < vectorTypeInfo->getLength())
	{
		unsigned char* elementPointer = buffer + index * vectorTypeInfo->getScalarSize();
		return vectorTypeInfo->getScalarType().getTypeCaster()->castFromRawPointer(reinterpret_cast<uintptr_t>(elementPointer));
	}
	return vectorTypeInfo->getScalarType().createDefault();
}


void VectorMemberFunctionInfo::doInit(std::any& base, const std::vector<std::any>& parameters) const
{
	assert(parameters.size() == 0);
	const unsigned char* constBuffer = nullptr;
	std::size_t bufferSize = 0;
	vectorTypeInfo->getTypeCaster()->toBuffer(base, constBuffer, bufferSize);
	unsigned char* buffer = const_cast<unsigned char*>(constBuffer);
	memset(buffer, 0, vectorTypeInfo->getSize());
}


void VectorMemberFunctionInfo::generateConstructVector(LLVM::LLVMCompileTimeContext* context, VectorTypeInfo* vectorType, llvm::Value* vectorData, llvm::Value* initialData)
{
#ifdef ENABLE_LLVM
	auto builder = context->helper->getBuilder();
	builder->CreateMemSet(vectorData, context->helper->createConstant((char)0), vectorType->getSize(), llvm::MaybeAlign());
#endif
}


void VectorMemberFunctionInfo::generateMoveArrayElements(LLVM::LLVMCompileTimeContext* context, VectorTypeInfo* vectorType, llvm::Value* targetVectorData, llvm::Value* sourceVectorData)
{
#ifdef ENABLE_LLVM
	auto builder = context->helper->getBuilder();
	builder->CreateMemCpy(targetVectorData, llvm::MaybeAlign(), sourceVectorData, llvm::MaybeAlign(), context->helper->createConstant(vectorType->getSize()));
#endif
}


void VectorMemberFunctionInfo::createIndexGeneratorFunction()
{
	#ifdef ENABLE_LLVM
	{
		inlineFunctionGenerator = std::make_unique<std::function<llvm::Value*(LLVM::LLVMCompileTimeContext* context, const std::vector<llvm::Value*>&)>>(
			[&](LLVM::LLVMCompileTimeContext* context, const std::vector<llvm::Value*>& parameters)
			{
				//First parameter is a pointer to the first scalar in the vector, second is index 
				assert(parameters.size() == 2);
				return context->helper->createOptionalNullCheckSelect(parameters[0],
					[&](LLVM::LLVMCompileTimeContext* context)
					{
						//Base is not null
						auto builder = context->helper->getBuilder();
						//Compare index is greater than or equal to zero
						llvm::Value* greaterOrEqualToZero = builder->CreateICmpSGE(parameters[1], context->helper->createConstant(0), "GreaterOrEqualToZero");

						//Get length of the vector
						llvm::Constant* length = context->helper->createConstant(vectorTypeInfo->getLength());
						//Check that index is less than size
						llvm::Value* lessThanSize = builder->CreateICmpSLT(parameters[1], length, "LessThanLength");
						//&& the two checks
						llvm::Value* rangeCheck = builder->CreateAnd({greaterOrEqualToZero, lessThanSize});

						return context->helper->createNullCheckSelect(rangeCheck,
							[&](LLVM::LLVMCompileTimeContext* context)
							{
								// Generate in-range code
								llvm::Value* vectorStartAddress = parameters[0];
								// Convert to parameter pointer to a llvm array type pointer.
								llvm::ArrayType* arrayType = getLLVMVectorType(context->helper);
								llvm::Value* vectorPointer = context->helper->convertToPointer(vectorStartAddress, "VectorCast", arrayType->getPointerTo());
								// Get element pointer to get the element in the vector.
								return builder->CreateGEP(vectorPointer, parameters[1], "GetVectorElement");
							},
							[&](LLVM::LLVMCompileTimeContext* context)
							{
								return context->helper->createZeroInitialisedConstant(getIndexReturnType(context->helper));
							}, context);
					},
					getIndexReturnType(context->helper), context);
			}
		);
	}
	#endif
}


void VectorMemberFunctionInfo::createInitGeneratorFunction()
{
	#ifdef ENABLE_LLVM
	{
		inlineFunctionGenerator = std::make_unique<std::function<llvm::Value*(LLVM::LLVMCompileTimeContext* context, const std::vector<llvm::Value*>&)>>(
			[&](LLVM::LLVMCompileTimeContext* context, const std::vector<llvm::Value*>& parameters)
			{
				//First parameter is a pointer to the first scalar in the vector
				assert(parameters.size() == 1);
				return context->helper->createOptionalNullCheckSelect(parameters[0],
					[&](LLVM::LLVMCompileTimeContext* context)
					{
						//Base is not null
						auto builder = context->helper->getBuilder();
						
						builder->CreateMemSet(parameters[0], context->helper->createConstant((char)0), context->helper->createConstant(vectorTypeInfo->getSize()), llvm::MaybeAlign());

						return (llvm::Value*)(nullptr);
					}, context->targetConfig->getLLVMTypes().voidType, context);
			}
		);
	}
	#endif
}


void VectorMemberFunctionInfo::createDestroyGeneratorFunction()
{
	#ifdef ENABLE_LLVM
	{
		inlineFunctionGenerator = std::make_unique<std::function<llvm::Value*(LLVM::LLVMCompileTimeContext* context, const std::vector<llvm::Value*>&)>>(
			[&](LLVM::LLVMCompileTimeContext* context, const std::vector<llvm::Value*>& parameters)
			{
				return (llvm::Value*)(nullptr);
			}
		);
	}
	#endif
}


llvm::Type* VectorMemberFunctionInfo::getIndexReturnType(LLVM::LLVMCodeGeneratorHelper* codeGeneratorHelper) const
{
#ifdef ENABLE_LLVM
	return codeGeneratorHelper->toLLVMType(vectorTypeInfo->getScalarType())->getPointerTo();
#else
	return nullptr;
#endif
}


llvm::ArrayType* jitcat::Reflection::VectorMemberFunctionInfo::getLLVMVectorType(LLVM::LLVMCodeGeneratorHelper* codeGeneratorHelper) const
{
#ifdef ENABLE_LLVM
	return llvm::ArrayType::get(codeGeneratorHelper->toLLVMType(vectorTypeInfo->getScalarType()), vectorTypeInfo->getLength());
#else
	return nullptr;
#endif
}
