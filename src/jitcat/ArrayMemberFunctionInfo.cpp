#include "jitcat/ArrayMemberFunctionInfo.h"
#include "jitcat/ArrayTypeInfo.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/Configuration.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/LLVMCatIntrinsics.h"
#include "jitcat/LLVMCodeGenerator.h"
#include "jitcat/LLVMCodeGeneratorHelper.h"
#include "jitcat/LLVMCompileTimeContext.h"
#include "jitcat/LLVMPreGeneratedExpression.h"
#ifdef ENABLE_LLVM
	#include <llvm/IR/IRBuilder.h>
	#include <llvm/IR/MDBuilder.h>
	#include "jitcat/LLVMTargetConfig.h"
	#include "jitcat/LLVMTypes.h"
#endif
#include <cassert>

using namespace jitcat;
using namespace jitcat::Reflection;
using namespace LLVM;


const char* ArrayMemberFunctionInfo::toString(Operation operation)
{
	switch (operation)
	{
		default: assert(false);		return "none";
		case Operation::Index:		return "[]";
		case Operation::Size:		return "size";
		case Operation::Init:		return "__init";
		case Operation::Destroy:	return "__destroy";
		case Operation::Resize:		return "resize";
	}
}


ArrayMemberFunctionInfo::ArrayMemberFunctionInfo(Operation operation, ArrayTypeInfo* arrayTypeInfo):
	MemberFunctionInfo(toString(operation), CatGenericType::voidType),
	operation(operation),
	arrayTypeInfo(arrayTypeInfo)
{
	
	switch (operation)
	{
		case Operation::Index:
		{
			returnType = arrayTypeInfo->getArrayItemType().toPointer(TypeOwnershipSemantics::Weak, true, false);
			argumentTypes.push_back(CatGenericType::intType);
		} break;
		case Operation::Size:
		{
			returnType = CatGenericType::intType;
		} break;
		case Operation::Resize:
		case Operation::Init:
		{
			returnType = CatGenericType::voidType;
			argumentTypes.push_back(CatGenericType::intType);
		} break;
		case Operation::Destroy:
		{
			returnType = CatGenericType::voidType;
		} break;
	}

	switch (operation)
	{
		default: assert(false);	break;
		case Operation::Index:		createIndexGeneratorFunction();		break;
		case Operation::Size:		createSizeGeneratorFunction();		break;
		case Operation::Init:		createInitGeneratorFunction();		break;
		case Operation::Destroy:	createDestroyGeneratorFunction();	break;
		case Operation::Resize:		createResizeGeneratorFunction();	break;
	}
}


std::any ArrayMemberFunctionInfo::call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) const
{ 
	if (base.has_value())
	{
		switch (operation)
		{
			default:				assert(false); return std::any();
			case Operation::Index:
			{
				assert(parameters.size() == 1);
				ArrayTypeInfo::Array* baseArray = std::any_cast<ArrayTypeInfo::Array*>(base);
				return arrayTypeInfo->index(baseArray, std::any_cast<int>(parameters[0]));
			}
			case Operation::Size:
			{
				assert(parameters.size() == 0);
				ArrayTypeInfo::Array* baseArray = std::any_cast<ArrayTypeInfo::Array*>(base);
				if (baseArray != nullptr)
				{
					return baseArray->size;
				}
				else
				{
					return 0;
				}
			}
			case Operation::Init:		doInit(base, parameters);		return std::any();
			case Operation::Destroy:	doDestroy(base, parameters);	return std::any();
			case Operation::Resize:		doResize(base, parameters);		return std::any();
		}			
		
	}
	switch (operation)
	{
		default: assert(false);		return std::any();
		case Operation::Index:		return arrayTypeInfo->getArrayItemType().createDefault();
		case Operation::Size:		return std::any((int)0);
		case Operation::Resize:		return std::any();
		case Operation::Init:		return std::any();
		case Operation::Destroy:	return std::any();
	}
}


MemberFunctionCallData ArrayMemberFunctionInfo::getFunctionAddress(FunctionType functionType) const
{
	return MemberFunctionCallData(0, 0, inlineFunctionGenerator.get(), MemberFunctionCallType::InlineFunctionGenerator, false, !arrayTypeInfo->getArrayItemType().isPointerType() );
}


void Reflection::ArrayMemberFunctionInfo::doInit(std::any& base, const std::vector<std::any>& parameters) const
{
	assert(parameters.size() == 1);
	ArrayTypeInfo::Array* baseArray = std::any_cast<ArrayTypeInfo::Array*>(base);
	int size = std::any_cast<int>(parameters[0]);
	if (size == 0)
	{
		baseArray->arrayData = nullptr;
		baseArray->size = 0;
	}
	else
	{
		baseArray->arrayData = LLVM::CatLinkedIntrinsics::_jc_allocateMemory(arrayTypeInfo->getItemSize() * size);
		baseArray->size = size;
		if (arrayTypeInfo->getArrayItemType().isTriviallyConstructable())
		{
			memset(baseArray->arrayData, 0, arrayTypeInfo->getItemSize() * size);
		}
		else
		{
			for (int i = 0; i < size; i++)
			{
				arrayTypeInfo->getArrayItemType().placementConstruct(baseArray->arrayData + i * arrayTypeInfo->getItemSize(), arrayTypeInfo->getItemSize());
			}
		}
	}
	return;
}


void Reflection::ArrayMemberFunctionInfo::doDestroy(std::any& base, const std::vector<std::any>& parameters) const
{
	assert(parameters.size() == 0);
	ArrayTypeInfo::Array* baseArray = std::any_cast<ArrayTypeInfo::Array*>(base);
	// Basic types do not need to be destroyed
	if (!arrayTypeInfo->getArrayItemType().isBasicType())
	{
		for (int i = 0; i < baseArray->size; i++)
		{
			arrayTypeInfo->getArrayItemType().placementDestruct(baseArray->arrayData + i * arrayTypeInfo->getItemSize(), arrayTypeInfo->getItemSize());
		}
	}
	LLVM::CatLinkedIntrinsics::_jc_freeMemory(baseArray->arrayData);
	baseArray->arrayData = nullptr;
	baseArray->size = 0;
	return;
}


void Reflection::ArrayMemberFunctionInfo::doResize(std::any& base, const std::vector<std::any>& parameters) const
{
	assert(parameters.size() == 1);
	ArrayTypeInfo::Array* baseArray = std::any_cast<ArrayTypeInfo::Array*>(base);
	ArrayTypeInfo::Array oldArray = *baseArray;
	int newSize = std::any_cast<int>(parameters[0]);
	int minSize = std::min(newSize, oldArray.size);
	if (newSize == oldArray.size)
	{
		return;
	}
	else
	{
		if (newSize == 0)
		{
			baseArray->arrayData = nullptr;
			baseArray->size = 0;
		}
		else
		{
			baseArray->arrayData = LLVM::CatLinkedIntrinsics::_jc_allocateMemory(arrayTypeInfo->getItemSize() * newSize);
			baseArray->size = newSize;
		}
		if (newSize != 0)
		{
			// Copy data from the old array to the new array (either memcpy, move construct or copy construct, depending on type)
			if (arrayTypeInfo->getArrayItemType().isTriviallyCopyable())
			{
				memcpy(baseArray->arrayData, oldArray.arrayData, minSize * arrayTypeInfo->getItemSize());
			}
			else if (arrayTypeInfo->getArrayItemType().isMoveConstructible())
			{
				// Move construct existing items into the new array
				for (int i = 0; i < minSize; ++i)
				{
					arrayTypeInfo->getArrayItemType().moveConstruct(baseArray->arrayData + i * arrayTypeInfo->getItemSize(), arrayTypeInfo->getItemSize(), oldArray.arrayData + i * arrayTypeInfo->getItemSize(), arrayTypeInfo->getItemSize());
					arrayTypeInfo->placementDestruct(oldArray.arrayData + i * arrayTypeInfo->getItemSize(), arrayTypeInfo->getItemSize());
				}
			}
			else if (arrayTypeInfo->getArrayItemType().isCopyConstructible())
			{
				// Copy construct existing items into the new array
				for (int i = 0; i < minSize; ++i)
				{
					arrayTypeInfo->getArrayItemType().copyConstruct(baseArray->arrayData + i * arrayTypeInfo->getItemSize(), arrayTypeInfo->getItemSize(), oldArray.arrayData + i * arrayTypeInfo->getItemSize(), arrayTypeInfo->getItemSize());
					arrayTypeInfo->placementDestruct(oldArray.arrayData + i * arrayTypeInfo->getItemSize(), arrayTypeInfo->getItemSize());
				}
			}
		}
		// Initialize any items of the new array that were not copied over.
		if (newSize > oldArray.size)
		{
			if (arrayTypeInfo->getArrayItemType().isTriviallyConstructable())
			{
				memset(baseArray->arrayData + oldArray.size * arrayTypeInfo->getItemSize(), 0, arrayTypeInfo->getItemSize() * (newSize - oldArray.size));
			}
			else
			{
				for (int i = minSize; i < newSize; ++i)
				{
					arrayTypeInfo->getArrayItemType().placementConstruct(baseArray->arrayData + i * arrayTypeInfo->getItemSize(), arrayTypeInfo->getItemSize());
				}
			}
		}

		// Delete any remaining items of the old array
		if (newSize < oldArray.size)
		{
			// Delete all old objects that were not move/copy constructed
			for (int i = minSize; i < oldArray.size; ++i)
			{
				arrayTypeInfo->placementDestruct(oldArray.arrayData + i * arrayTypeInfo->getItemSize(), arrayTypeInfo->getItemSize());
			}
		}
	}
	LLVM::CatLinkedIntrinsics::_jc_freeMemory(oldArray.arrayData);
}


llvm::Value* ArrayMemberFunctionInfo::generateArraySizePtr(LLVM::LLVMCompileTimeContext* context, llvm::Value* arrayPointer)
{
#ifdef ENABLE_LLVM
	//Get offset to array size member
	llvm::Value* offset = context->helper->createConstant((int)sizeof(ArrayTypeInfo::Array::arrayData));
	//Get a pointer to the size of the array
	llvm::Value* arraySizePtr = context->helper->createAdd(arrayPointer, offset, "arraySizeAddr");
	return context->helper->getBuilder()->CreatePointerCast(arraySizePtr, context->targetConfig->getLLVMTypes().intType->getPointerTo());
#else
	return nullptr;
#endif
}


llvm::Value* ArrayMemberFunctionInfo::generateGetArraySize(LLVM::LLVMCompileTimeContext* context, llvm::Value* arrayPointer)
{
#ifdef ENABLE_LLVM
	llvm::Value* arraySizeIntPtr = generateArraySizePtr(context, arrayPointer);
	//Get size of the array
	return context->helper->loadBasicType(context->targetConfig->getLLVMTypes().intType, arraySizeIntPtr, "getSize");
#else
	return nullptr;
#endif
	
}


void ArrayMemberFunctionInfo::generateInitEmptyArray(LLVM::LLVMCompileTimeContext* context, llvm::Value* arrayPointer, llvm::Value* arraySizePointer)
{
#ifdef ENABLE_LLVM
	auto builder = context->helper->getBuilder();
	builder->CreateStore(context->helper->createZeroInitialisedConstant(context->targetConfig->getLLVMTypes().pointerType), builder->CreatePointerCast(arrayPointer, context->targetConfig->getLLVMTypes().pointerType->getPointerTo()));
	builder->CreateStore(context->helper->createZeroInitialisedConstant(context->targetConfig->getLLVMTypes().intType), builder->CreatePointerCast(arraySizePointer, context->targetConfig->getLLVMTypes().intType->getPointerTo()));
#endif
}


void ArrayMemberFunctionInfo::generateAllocateArray(LLVM::LLVMCompileTimeContext* context, ArrayTypeInfo* arrayType, llvm::Value* arrayPointer, llvm::Value* arraySizePointer, llvm::Value* arraySizeElements,
													llvm::Value*& arrayData, llvm::Value*& arraySizeBytes, llvm::Value*& arrayItemSizeBytes)
{
#ifdef ENABLE_LLVM
	auto builder = context->helper->getBuilder();
	builder->CreateStore(arraySizeElements, builder->CreatePointerCast(arraySizePointer, context->targetConfig->getLLVMTypes().intType->getPointerTo()));
	arrayItemSizeBytes = context->helper->createConstant((int)arrayType->getArrayItemType().getTypeSize());
	arraySizeBytes = builder->CreateMul(arrayItemSizeBytes, arraySizeElements, "arraySizeBytes");
	arrayData = context->helper->createIntrinsicCall(context, CatLinkedIntrinsics::_jc_allocateMemory, { arraySizeBytes }, "_jc_allocateMemory", true);
	builder->CreateStore(arrayData, builder->CreatePointerCast(arrayPointer, arrayData->getType()->getPointerTo()));
#endif
}


void ArrayMemberFunctionInfo::generateConstructArray(LLVM::LLVMCompileTimeContext* context, ArrayTypeInfo* arrayType, llvm::Value* arrayData, llvm::Value* arraySizeBytes, llvm::Value* arrayItemSizeBytes)
{
#ifdef ENABLE_LLVM
	auto builder = context->helper->getBuilder();
	if (arrayType->getArrayItemType().isTriviallyConstructable())
	{
		//Array can be initialised with a memset
		builder->CreateMemSet(arrayData, context->helper->createConstant((char)0), arraySizeBytes, llvm::MaybeAlign());
	}
	else
	{
		assert(arrayType->getArrayItemType().isReflectableObjectType());
		//Initialize all the array items by calling the placement constructor in a for loop
		context->helper->generateLoop(context, arrayData, arrayItemSizeBytes, builder->CreateAdd(arrayData, arraySizeBytes, "arrayEnd"),
			[&](LLVMCompileTimeContext* context, llvm::Value* iterator)
			{
				//iterator contains pointer to item
				TypeInfo* objectTypeInfo = arrayType->getArrayItemType().getObjectType();
				if (objectTypeInfo->getAllowConstruction()
					&& objectTypeInfo->isCustomType()
					&& static_cast<CustomTypeInfo*>(objectTypeInfo)->getClassDefinition() != nullptr)
				{
					//Call the init member function
					MemberFunctionInfo* functionInfo = objectTypeInfo->getFirstMemberFunctionInfo("init");
					if (functionInfo == nullptr)
					{
						functionInfo = objectTypeInfo->getFirstMemberFunctionInfo("__init");
					}
					assert(functionInfo != nullptr);
					LLVMPreGeneratedExpression preGeneratedBase(iterator, arrayType->getArrayItemType().toPointer(TypeOwnershipSemantics::Value, true, false));
					context->helper->generateMemberFunctionCall(functionInfo, &preGeneratedBase, {}, {}, context);
				}
				else
				{
					//Call the placement constructor intrinsic function
					llvm::Value* typeInfoConstantAsIntPtr = context->helper->createTypeInfoGlobalValue(context, objectTypeInfo);
					context->helper->createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_placementConstructType, { iterator, typeInfoConstantAsIntPtr }, "_jc_placementConstructType", true);
				}

			});
	}
	#endif
}


void ArrayMemberFunctionInfo::generateDestroyArray(LLVM::LLVMCompileTimeContext* context, ArrayTypeInfo* arrayType, llvm::Value* arrayData, llvm::Value* arraySizeElements)
{
#ifdef ENABLE_LLVM
	if (!arrayType->getArrayItemType().isBasicType()
		&& !arrayType->getArrayItemType().isEnumType()
		&& !(arrayType->getArrayItemType().isPointerType() && arrayType->getArrayItemType().getOwnershipSemantics() == TypeOwnershipSemantics::Weak))
	{
		auto builder = context->helper->getBuilder();
		//Need to call destructors
		llvm::Value* itemSize = context->helper->createConstant((int)arrayType->getArrayItemType().getTypeSize());
		llvm::Value* arraySizeBytes = builder->CreateMul(itemSize, arraySizeElements, "arraySizeBytes");
		context->helper->generateLoop(context, arrayData, itemSize, builder->CreateAdd(arrayData, arraySizeBytes, "arrayEnd"),
			[&](LLVMCompileTimeContext* context, llvm::Value* iterator)
			{
				if (arrayType->getArrayItemType().isReflectableObjectType())
				{
					//iterator contains pointer to item
					TypeInfo* objectTypeInfo = arrayType->getArrayItemType().getObjectType();
					if (objectTypeInfo->getAllowConstruction()
						&& objectTypeInfo->isCustomType()
						&& static_cast<CustomTypeInfo*>(objectTypeInfo)->getClassDefinition() != nullptr)
					{
						//Call the destroy member function
						MemberFunctionInfo* functionInfo = objectTypeInfo->getFirstMemberFunctionInfo("destroy");
						if (functionInfo == nullptr)
						{
							functionInfo = objectTypeInfo->getFirstMemberFunctionInfo("__destroy");
						}
						assert(functionInfo != nullptr);
						LLVMPreGeneratedExpression preGeneratedBase(iterator, arrayType->getArrayItemType().toPointer(TypeOwnershipSemantics::Value, true, false));
						context->helper->generateMemberFunctionCall(functionInfo, &preGeneratedBase, {}, {}, context);
					}
					else
					{
						//Call the placement destructor intrinsic function
						llvm::Value* typeInfoConstantAsIntPtr = context->helper->createTypeInfoGlobalValue(context, objectTypeInfo);
						context->helper->createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_placementDestructType, { iterator, typeInfoConstantAsIntPtr }, "_jc_placementDestructType", true);
					}
				}
				else
				{
					//Need to implement destructors for owned/shared pointers
					assert(false);
				}
			});
	}
#endif
}


void ArrayMemberFunctionInfo::generateFreeArray(LLVM::LLVMCompileTimeContext* context, llvm::Value* arrayPointer, llvm::Value* arrayData, llvm::Value* arraySizePointer)
{
#ifdef ENABLE_LLVM
	auto builder = context->helper->getBuilder();
	context->helper->createIntrinsicCall(context, CatLinkedIntrinsics::_jc_freeMemory, { arrayData }, "_jc_freeMemory", true);
	builder->CreateStore(context->helper->createZeroInitialisedConstant(context->targetConfig->getLLVMTypes().pointerType), builder->CreatePointerCast(arrayPointer, context->targetConfig->getLLVMTypes().pointerType->getPointerTo()));
	builder->CreateStore(context->helper->createZeroInitialisedConstant(context->targetConfig->getLLVMTypes().intType), builder->CreatePointerCast(arraySizePointer, context->targetConfig->getLLVMTypes().intType->getPointerTo()));
#endif
}


void ArrayMemberFunctionInfo::generateMoveArrayElements(LLVM::LLVMCompileTimeContext* context, ArrayTypeInfo* arrayType, llvm::Value* targetArrayData, llvm::Value* sourceArrayData, llvm::Value* numItemsToMove, llvm::Value* itemSize)
{
#ifdef ENABLE_LLVM
	auto builder = context->helper->getBuilder();
	llvm::Value* copySizeBytes = builder->CreateMul(itemSize, numItemsToMove, "copySizeBytes");
	if (arrayType->getArrayItemType().isTriviallyCopyable())
	{
		//Memcpy
		builder->CreateMemCpy(targetArrayData, llvm::MaybeAlign(0), sourceArrayData, llvm::MaybeAlign(0), copySizeBytes);
	}
	else
	{
		//Move or copy construct
		context->helper->generateLoop(context, context->helper->createZeroInitialisedConstant(itemSize->getType()), itemSize, copySizeBytes,
			[&](LLVMCompileTimeContext* context, llvm::Value* iterator)
			{
				assert(arrayType->getArrayItemType().isReflectableObjectType());
				llvm::Value* sourceItemPtr = builder->CreateAdd(sourceArrayData, iterator);
				llvm::Value* targetItemPtr = builder->CreateAdd(targetArrayData, iterator);
				llvm::Value* typeInfoConstantAsIntPtr = context->helper->createTypeInfoGlobalValue(context, arrayType->getArrayItemType().getObjectType());
				if (arrayType->getArrayItemType().isMoveConstructible())
				{
					context->helper->createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_placementMoveConstructType, { targetItemPtr, sourceItemPtr, typeInfoConstantAsIntPtr }, "_jc_placementMoveConstructType", true);
				}
				else if (arrayType->getArrayItemType().isCopyConstructible())
				{
					context->helper->createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_placementCopyConstructType, { targetItemPtr, sourceItemPtr, typeInfoConstantAsIntPtr }, "_jc_placementCopyConstructType", true);
				}
				else
				{
					assert(false);
				}
				context->helper->createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_placementDestructType, {sourceItemPtr, typeInfoConstantAsIntPtr }, "_jc_placementDestructType", true);
			});
	}
#endif
}


void ArrayMemberFunctionInfo::createIndexGeneratorFunction()
{
	#ifdef ENABLE_LLVM
	{
		inlineFunctionGenerator = std::make_unique<std::function<llvm::Value*(LLVM::LLVMCompileTimeContext* context, const std::vector<llvm::Value*>&)>>(
			[&](LLVM::LLVMCompileTimeContext* context, const std::vector<llvm::Value*>& parameters)
			{
				//First parameter is an Array*, second is index 
				assert(parameters.size() == 2);
				return context->helper->createOptionalNullCheckSelect(parameters[0],
					[&](LLVM::LLVMCompileTimeContext* context)
					{
						//Base is not null
						auto builder = context->helper->getBuilder();
						//Compare index is greater than or equal to zero
						llvm::Value* greaterOrEqualToZero = builder->CreateICmpSGE(parameters[1], context->helper->createConstant(0), "GreaterOrEqualToZero");

						//Get size of the array
						llvm::Value* size = generateGetArraySize(context, parameters[0]);
						//Check that index is less than size
						llvm::Value* lessThanSize = builder->CreateICmpSLT(parameters[1],size, "LessThanSize");
						//&& the two checks
						llvm::Value* rangeCheck = builder->CreateAnd({greaterOrEqualToZero, lessThanSize});

						return context->helper->createNullCheckSelect(rangeCheck,
							[&](LLVM::LLVMCompileTimeContext* context)
							{
								//Generate in-range code
								//Generate the code for indexing the array
								llvm::Value* arrayStartAddress = context->helper->loadPointerAtAddress(parameters[0], "loadArrayAddress");

								llvm::Value* itemSize = context->helper->createConstant((int)arrayTypeInfo->getArrayItemType().getTypeSize());
								llvm::Value* itemOffset = builder->CreateMul(itemSize, parameters[1], "arrayItemOffset");
								llvm::Value* itemAddress = context->helper->createAdd(arrayStartAddress, itemOffset, "arrayItemAddress");
								//Cast to correct type pointer
								llvm::PointerType* returnType = context->helper->toLLVMPtrType(arrayTypeInfo->getArrayItemType());
								return builder->CreatePointerCast(itemAddress, returnType, "arrayItemAddressCast");
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


void ArrayMemberFunctionInfo::createSizeGeneratorFunction()
{
	#ifdef ENABLE_LLVM
	{
		inlineFunctionGenerator = std::make_unique<std::function<llvm::Value*(LLVM::LLVMCompileTimeContext* context, const std::vector<llvm::Value*>&)>>(
			[&](LLVM::LLVMCompileTimeContext* context, const std::vector<llvm::Value*>& parameters)
			{
				//First parameter is an Array*
				assert(parameters.size() == 1);
				return context->helper->createOptionalNullCheckSelect(parameters[0],
					[&](LLVM::LLVMCompileTimeContext* context)
					{
						return generateGetArraySize(context, parameters[0]);
					}, context->targetConfig->getLLVMTypes().intType, context);
			});

	}
	#endif
}


void ArrayMemberFunctionInfo::createInitGeneratorFunction()
{
	#ifdef ENABLE_LLVM
	{
		inlineFunctionGenerator = std::make_unique<std::function<llvm::Value*(LLVM::LLVMCompileTimeContext* context, const std::vector<llvm::Value*>&)>>(
			[&](LLVM::LLVMCompileTimeContext* context, const std::vector<llvm::Value*>& parameters)
			{
				//First parameter is an Array*, second is size 
				assert(parameters.size() == 2);
				return context->helper->createOptionalNullCheckSelect(parameters[0],
					[&](LLVM::LLVMCompileTimeContext* context)
					{
						//Base is not null
						auto builder = context->helper->getBuilder();
						llvm::Value* greaterThanZero = builder->CreateICmpSGT(parameters[1], context->helper->createConstant(0), "GreaterThanZero");
						//Get a pointer to the size of the array
						llvm::Value* arraySizePtr = generateArraySizePtr(context, parameters[0]);

						context->helper->createNullCheckSelect(greaterThanZero, 
							[&](LLVM::LLVMCompileTimeContext*)
							{
								llvm::Value* arrayData = nullptr;
								llvm::Value* arraySize = nullptr;
								llvm::Value* itemSize = nullptr;

								generateAllocateArray(context, arrayTypeInfo, parameters[0], arraySizePtr, parameters[1], arrayData, arraySize, itemSize);

								generateConstructArray(context, arrayTypeInfo, arrayData, arraySize, itemSize);

								return (llvm::Value*)nullptr;
							},
							[&](LLVM::LLVMCompileTimeContext*)
							{
								//Generate zero (or negative) size code
								//Set size and pointer to null
								generateInitEmptyArray(context, parameters[0], arraySizePtr);
								return (llvm::Value*)nullptr;
							}, context);
						return (llvm::Value*)(nullptr);
					},
					context->targetConfig->getLLVMTypes().voidType, context);
			});
	}
	#endif
}


void ArrayMemberFunctionInfo::createDestroyGeneratorFunction()
{
	#ifdef ENABLE_LLVM
	{
		inlineFunctionGenerator = std::make_unique<std::function<llvm::Value*(LLVM::LLVMCompileTimeContext* context, const std::vector<llvm::Value*>&)>>(
			[&](LLVM::LLVMCompileTimeContext* context, const std::vector<llvm::Value*>& parameters)
			{
				//Parameter is an Array*
				assert(parameters.size() == 1);
				return context->helper->createOptionalNullCheckSelect(parameters[0],
					[&](LLVM::LLVMCompileTimeContext* context)
					{
						//Base is not null
						auto builder = context->helper->getBuilder();
						llvm::Function* currentFunction = builder->GetInsertBlock()->getParent();
						llvm::LLVMContext& llvmContext = context->helper->getContext();

						//Get a pointer to the size of the array
						llvm::Value* arraySizePtr = generateArraySizePtr(context, parameters[0]);

						arraySizePtr = builder->CreatePointerCast(arraySizePtr, context->targetConfig->getLLVMTypes().intType->getPointerTo());
						llvm::Value* arraySize = builder->CreateLoad(arraySizePtr, "arraySize");
						llvm::Value* notEmpty = builder->CreateICmpSGT(arraySize, context->helper->createConstant((int)0), "notEmpty");
						//The array is not empty
						context->helper->createOptionalNullCheckSelect(notEmpty, [&](LLVM::LLVMCompileTimeContext* context)
							{
								llvm::Value* arrayPtr = builder->CreateLoad(builder->CreatePointerCast(parameters[0], context->targetConfig->getLLVMTypes().pointerType->getPointerTo()), "arrayDataPtr");

								generateDestroyArray(context, arrayTypeInfo, arrayPtr, arraySize);

								generateFreeArray(context, parameters[0], arrayPtr, arraySizePtr);

								return (llvm::Value*)(nullptr);
							}, context->targetConfig->getLLVMTypes().voidType, context);
						return (llvm::Value*)(nullptr);
					},
					context->targetConfig->getLLVMTypes().voidType, context);
			});
	}
	#endif
}


void ArrayMemberFunctionInfo::createResizeGeneratorFunction()
{
#ifdef ENABLE_LLVM
	{

		inlineFunctionGenerator = std::make_unique<std::function<llvm::Value* (LLVM::LLVMCompileTimeContext* context, const std::vector<llvm::Value*>&)>>(
			[&](LLVM::LLVMCompileTimeContext* context, const std::vector<llvm::Value*>& parameters)
			{
				//Parameters are an Array* and a size
				assert(parameters.size() == 2);

				auto builder = context->helper->getBuilder();
				
				llvm::Value* arraySizePtr = generateArraySizePtr(context, parameters[0]);
				llvm::Value* currentArraySize = builder->CreateLoad(arraySizePtr, "arraySize");
				llvm::Value* currentArrayData = context->helper->loadPointerAtAddress(parameters[0], "loadArrayAddress");
				llvm::Value* isNotSameSize = builder->CreateICmpNE(currentArraySize, parameters[1]);

				context->helper->createNullCheckSelect(isNotSameSize,
					[&](LLVMCompileTimeContext* context)
					{
						llvm::Value* newSizeGreaterThanZero = builder->CreateICmpSGT(parameters[1], context->helper->createConstant(0), "NewSizeGreaterThanZero");
						context->helper->createNullCheckSelect(newSizeGreaterThanZero,
							[&](LLVMCompileTimeContext* context)
							{
								llvm::Value* arrayData = nullptr;
								llvm::Value* arraySizeBytes = nullptr;
								llvm::Value* arrayItemSizeBytes = nullptr;
								generateAllocateArray(context, arrayTypeInfo, parameters[0], arraySizePtr, parameters[1], arrayData, arraySizeBytes, arrayItemSizeBytes);
								llvm::Value* lessThan = builder->CreateICmp(llvm::CmpInst::Predicate::ICMP_SLT, currentArraySize, parameters[1]);
								llvm::Value* minSize = builder->CreateSelect(lessThan, currentArraySize, parameters[1]);

								//Move elements from the old array to the new array.
								generateMoveArrayElements(context, arrayTypeInfo, arrayData, currentArrayData, minSize, arrayItemSizeBytes);

								llvm::Value* newSizeGreaterThanOldSize = builder->CreateICmpSGT(parameters[1], currentArraySize, "NewSizeGreaterThanOldSize");
								//Default construct any remaining elements of the new array if the new array was larger than the old array
								context->helper->createNullCheckSelect(newSizeGreaterThanOldSize,
									[&](LLVMCompileTimeContext* context)
									{
										llvm::Value* sizeDiff = builder->CreateSub(parameters[1], currentArraySize);
										llvm::Value* sizeDiffBytes = builder->CreateMul(sizeDiff, arrayItemSizeBytes, "SizeDiffBytes");
										llvm::Value* minSizeBytes = builder->CreateMul(minSize, arrayItemSizeBytes, "SizeDiffBytes");
										generateConstructArray(context, arrayTypeInfo, context->helper->createAdd(arrayData, minSizeBytes, "ArrayRemainderPtr"), sizeDiffBytes, arrayItemSizeBytes);
										return (llvm::Value*)(nullptr);
									}, context->targetConfig->getLLVMTypes().voidType, context);								
								return (llvm::Value*)(nullptr);
							},
							[&](LLVMCompileTimeContext* context)
							{
								generateInitEmptyArray(context, parameters[0], arraySizePtr);
								return (llvm::Value*)(nullptr);
							}, context);
						
						return (llvm::Value*)(nullptr);
					}, context->targetConfig->getLLVMTypes().voidType, context);
				
				//Destruct remainder if the old array was larger than the new array
				llvm::Value* newSizeGreaterThanOldSize = builder->CreateICmpSGT(currentArraySize, parameters[1], "OldSizeGreaterThanNewSize");
				context->helper->createNullCheckSelect(newSizeGreaterThanOldSize,
					[&](LLVMCompileTimeContext* context)
					{
						llvm::Value* sizeDiff = builder->CreateSub(currentArraySize, parameters[1]);
						llvm::Value* arrayItemSizeBytes = context->helper->createConstant((int)arrayTypeInfo->getArrayItemType().getTypeSize());
						llvm::Value* sizeDiffBytes = builder->CreateMul(sizeDiff, arrayItemSizeBytes, "SizeDiffBytes");

						generateDestroyArray(context, arrayTypeInfo, context->helper->createAdd(currentArrayData, sizeDiffBytes, "OldRemainderOffset"), sizeDiff);
						return (llvm::Value*)(nullptr);
					}, context->targetConfig->getLLVMTypes().voidType, context);
				//Free the old array
				context->helper->createIntrinsicCall(context, CatLinkedIntrinsics::_jc_freeMemory, { currentArrayData }, "_jc_freeMemory", true);
				return (llvm::Value*)(nullptr);
			});
	}
#endif
}


llvm::Type* ArrayMemberFunctionInfo::getIndexReturnType(LLVM::LLVMCodeGeneratorHelper* helper) const
{
	#ifdef ENABLE_LLVM
		return helper->toLLVMPtrType(arrayTypeInfo->getArrayItemType());
	#else
		return nullptr;
	#endif
}
