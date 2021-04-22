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
#include "jitcat/LLVMTypes.h"
#ifdef ENABLE_LLVM
	#include <llvm/IR/IRBuilder.h>
	#include <llvm/IR/MDBuilder.h>
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
		}
}


std::any ArrayMemberFunctionInfo::call(CatRuntimeContext* runtimeContext, std::any& base, const std::vector<std::any>& parameters) const
{ 
	if (base.has_value())
	{
		ArrayTypeInfo::Array* baseArray = std::any_cast<ArrayTypeInfo::Array*>(base);
		switch (operation)
		{
			default:				assert(false); return std::any();
			case Operation::Index:
			{
				assert(parameters.size() == 1);
				return arrayTypeInfo->index(baseArray, std::any_cast<int>(parameters[0]));
			}
			case Operation::Size:
			{
				assert(parameters.size() == 0);
				if (baseArray != nullptr)
				{
					return baseArray->size;
				}
				else
				{
					return 0;
				}
			}
			case Operation::Init:
			{
				assert(parameters.size() == 1);
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
					if (arrayTypeInfo->getArrayItemType().isBasicType() 
						|| arrayTypeInfo->getArrayItemType().isPointerType()
						|| arrayTypeInfo->getArrayItemType().isEnumType())
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
				return std::any();
			}
			case Operation::Destroy:	
			{
				assert(parameters.size() == 0);
				//Basic types do not need to be destroyed
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
				return std::any();
			}
		}			
		
	}
	switch (operation)
	{
		default: assert(false);		return std::any();
		case Operation::Index:		return arrayTypeInfo->getArrayItemType().createDefault();
		case Operation::Size:		return std::any((int)0);
		case Operation::Init:		return std::any();
		case Operation::Destroy:	return std::any();
	}
}


MemberFunctionCallData ArrayMemberFunctionInfo::getFunctionAddress() const
{
	return MemberFunctionCallData(0, 0, inlineFunctionGenerator.get(), MemberFunctionCallType::InlineFunctionGenerator, false);
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
						//Get offset to array size member
						llvm::Value* offset = context->helper->createConstant((int)sizeof(ArrayTypeInfo::Array::arrayData));
						//Get a pointer to the size of the array
						llvm::Value* arraySizePtr = context->helper->createAdd(parameters[0], offset, "arraySizeAddr");
						llvm::Value* arraySizeIntPtr = context->helper->convertToIntPtr(arraySizePtr, "arraySizeIntAddr");
						//Get size of the array
						llvm::Value* size = context->helper->loadBasicType(LLVMTypes::intType, arraySizeIntPtr, "getSize");
						//Check that index is less than size
						llvm::Value* lessThanSize = builder->CreateICmpSLT(parameters[1],size, "LessThanSize");
						//&& the two checks
						llvm::Value* rangeCheck = builder->CreateAnd({greaterOrEqualToZero, lessThanSize});

						llvm::Function* currentFunction = builder->GetInsertBlock()->getParent();
						llvm::LLVMContext& llvmContext = context->helper->getContext();
						//Create basic blocks for the two branches for in-range and not-in-range and the continuation
						llvm::BasicBlock* thenBlock = llvm::BasicBlock::Create(llvmContext, "thenIsInRange", currentFunction);
						llvm::Value* thenResult = nullptr;
						llvm::BasicBlock* elseBlock = llvm::BasicBlock::Create(llvmContext, "elseIsNotInRange");
						llvm::Value* elseResult = nullptr;
						llvm::BasicBlock* continuationBlock = llvm::BasicBlock::Create(llvmContext, "afterRangeCheck");
						//Create branch prediction values that bias towards the index being in-range
						llvm::MDBuilder metadataBuilder(llvmContext);
						llvm::MDNode* branchPredictNode = metadataBuilder.createBranchWeights(2000, 1);
						builder->CreateCondBr(rangeCheck, thenBlock, elseBlock, branchPredictNode);
						builder->SetInsertPoint(thenBlock);
						{
							//Generate in-range code
							//Generate the code for indexing the array
							llvm::Value* arrayStartAddress = context->helper->loadPointerAtAddress(parameters[0], "loadArrayAddress");
							llvm::Value* itemSize = context->helper->createConstant((int)arrayTypeInfo->getArrayItemType().getTypeSize());
							llvm::Value* itemOffset = builder->CreateMul(itemSize, parameters[1], "arrayItemOffset");
							llvm::Value* itemAddress = context->helper->createAdd(arrayStartAddress, itemOffset, "arrayItemAddress");
							//Cast to correct type pointer
							llvm::PointerType* returnType = context->helper->toLLVMPtrType(arrayTypeInfo->getArrayItemType());
							thenResult = builder->CreatePointerCast(itemAddress, returnType, "arrayItemAddressCast");
							builder->CreateBr(continuationBlock);
						}
						currentFunction->getBasicBlockList().push_back(elseBlock);
						builder->SetInsertPoint(elseBlock);
						{
							//Generate out-of-range code
							elseResult = context->helper->createZeroInitialisedConstant(getIndexReturnType(context->helper));
							builder->CreateBr(continuationBlock);
						}
						// codegen of 'Else' can change the current block, update ElseBB for the PHI.
						elseBlock = builder->GetInsertBlock();
						//Generate a phi node in the continuation block
						currentFunction->getBasicBlockList().push_back(continuationBlock);
						builder->SetInsertPoint(continuationBlock);
						llvm::PHINode* phiNode = builder->CreatePHI(thenResult->getType(), 2, "indexResult");
						phiNode->addIncoming(thenResult, thenBlock);
						phiNode->addIncoming(elseResult, elseBlock);
						return static_cast<llvm::Value*>(phiNode);
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
						//Get offset to array size member
						llvm::Value* offset = context->helper->createConstant((int)sizeof(ArrayTypeInfo::Array::arrayData));
						//Get a pointer to the size of the array
						llvm::Value* arraySizePtr = context->helper->createAdd(parameters[0], offset, "arraySizeAddr");
						//Get size of the array
						return context->helper->loadBasicType(LLVMTypes::intType, arraySizePtr, "getSize");
					}, LLVMTypes::intType, context);
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

						llvm::Function* currentFunction = builder->GetInsertBlock()->getParent();
						llvm::LLVMContext& llvmContext = context->helper->getContext();

						//Get offset to array size member
						llvm::Value* offset = context->helper->createConstant((int)sizeof(ArrayTypeInfo::Array::arrayData));
						//Get a pointer to the size of the array
						llvm::Value* arraySizePtr = context->helper->createAdd(parameters[0], offset, "arraySizeAddr");

						//Create basic blocks for the two branches for in-range and not-in-range and the continuation
						llvm::BasicBlock* thenBlock = llvm::BasicBlock::Create(llvmContext, "thenIsInRange", currentFunction);
						llvm::Value* thenResult = nullptr;
						llvm::BasicBlock* elseBlock = llvm::BasicBlock::Create(llvmContext, "elseIsNotInRange");
						llvm::Value* elseResult = nullptr;
						llvm::BasicBlock* continuationBlock = llvm::BasicBlock::Create(llvmContext, "afterRangeCheck");
						//Create branch prediction values that bias towards the size being greater than zero
						llvm::MDBuilder metadataBuilder(llvmContext);
						llvm::MDNode* branchPredictNode = metadataBuilder.createBranchWeights(2000, 1);
						builder->CreateCondBr(greaterThanZero, thenBlock, elseBlock, branchPredictNode);
						builder->SetInsertPoint(thenBlock);
						{
							//Generate non-zero size code
							//Allocate memory and call constructors
							builder->CreateStore(parameters[1], builder->CreatePointerCast(arraySizePtr, LLVMTypes::intType->getPointerTo()));
							llvm::Value* itemSize = context->helper->createConstant((int)arrayTypeInfo->getArrayItemType().getTypeSize());
							llvm::Value* arraySize = builder->CreateMul(itemSize, parameters[1], "arraySizeBytes");
							llvm::Value* arrayData = context->helper->createIntrinsicCall(context, CatLinkedIntrinsics::_jc_allocateMemory, {arraySize}, "_jc_allocateMemory", true);
							builder->CreateStore(arrayData, builder->CreatePointerCast(parameters[0], arrayData->getType()->getPointerTo()));
							if (arrayTypeInfo->getArrayItemType().isBasicType() 
								|| arrayTypeInfo->getArrayItemType().isPointerType()
								|| arrayTypeInfo->getArrayItemType().isEnumType())
							{
								//Array can be initialised with a memset
								builder->CreateMemSet(arrayData, context->helper->createConstant((char)0), arraySize, llvm::MaybeAlign());
							}
							else
							{
								assert(arrayTypeInfo->getArrayItemType().isReflectableObjectType());
								//Initialize all the array items by calling the placement constructor in a for loop
								context->helper->generateLoop(context, arrayData, itemSize, builder->CreateAdd(arrayData, arraySize, "arrayEnd"),
									[&](LLVMCompileTimeContext* context, llvm::Value* iterator)
									{
										//iterator contains pointer to item
										TypeInfo* objectTypeInfo = arrayTypeInfo->getArrayItemType().getObjectType();								
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
											LLVMPreGeneratedExpression preGeneratedBase(iterator, arrayTypeInfo->getArrayItemType().toPointer(TypeOwnershipSemantics::Value, true, false));
											context->helper->generateMemberFunctionCall(functionInfo, &preGeneratedBase, {}, context);
										}
										else
										{
											//Call the placement constructor intrinsic function
											llvm::Value* typeInfoConstantAsIntPtr = context->helper->createTypeInfoGlobalValue(context, objectTypeInfo); 
											context->helper->createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_placementConstructType, {iterator, typeInfoConstantAsIntPtr}, "_jc_placementConstructType", true);
										}

									});
							}
							builder->CreateBr(continuationBlock);
						}
						currentFunction->getBasicBlockList().push_back(elseBlock);
						builder->SetInsertPoint(elseBlock);
						{
							//Generate zero (or negative) size code
							//Set size and pointer to null
							builder->CreateStore(context->helper->createZeroInitialisedConstant(LLVMTypes::pointerType), builder->CreatePointerCast(parameters[0], LLVMTypes::pointerType->getPointerTo()));
							builder->CreateStore(context->helper->createZeroInitialisedConstant(LLVMTypes::intType), builder->CreatePointerCast(arraySizePtr, LLVMTypes::intType->getPointerTo()));
							builder->CreateBr(continuationBlock);
						}
						currentFunction->getBasicBlockList().push_back(continuationBlock);
						builder->SetInsertPoint(continuationBlock);
						return (llvm::Value*)(nullptr);
					},
					LLVMTypes::voidType, context);
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

						//Get offset to array size member
						llvm::Value* offset = context->helper->createConstant((int)sizeof(ArrayTypeInfo::Array::arrayData));
						//Get a pointer to the size of the array
						llvm::Value* arraySizePtr = context->helper->createAdd(parameters[0], offset, "arraySizeAddr");
						arraySizePtr = builder->CreatePointerCast(arraySizePtr, LLVMTypes::intType->getPointerTo());
						llvm::Value* arraySize = builder->CreateLoad(arraySizePtr, "arraySize");
						llvm::Value* notEmpty = builder->CreateICmpSGT(arraySize, context->helper->createConstant((int)0), "notEmpty");
						//The array is not empty
						context->helper->createOptionalNullCheckSelect(notEmpty, [&](LLVM::LLVMCompileTimeContext* context)
							{
								llvm::Value* arrayPtr = builder->CreateLoad(builder->CreatePointerCast(parameters[0], LLVMTypes::pointerType->getPointerTo()), "arrayDataPtr");
								if (!arrayTypeInfo->getArrayItemType().isBasicType()
									&& !arrayTypeInfo->getArrayItemType().isEnumType()
									&& !(arrayTypeInfo->getArrayItemType().isPointerType() && arrayTypeInfo->getArrayItemType().getOwnershipSemantics() == TypeOwnershipSemantics::Weak))
								{
									//Need to call destructors
									llvm::Value* itemSize = context->helper->createConstant((int)arrayTypeInfo->getArrayItemType().getTypeSize());
									llvm::Value* arraySizeBytes = builder->CreateMul(itemSize, arraySize, "arraySizeBytes");
									context->helper->generateLoop(context, arrayPtr, itemSize, builder->CreateAdd(arrayPtr, arraySizeBytes, "arrayEnd"),
										[&](LLVMCompileTimeContext* context, llvm::Value* iterator)
										{
											if (arrayTypeInfo->getArrayItemType().isReflectableObjectType())
											{
												//iterator contains pointer to item
												TypeInfo* objectTypeInfo = arrayTypeInfo->getArrayItemType().getObjectType();								
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
													LLVMPreGeneratedExpression preGeneratedBase(iterator, arrayTypeInfo->getArrayItemType().toPointer(TypeOwnershipSemantics::Value, true, false));
													context->helper->generateMemberFunctionCall(functionInfo, &preGeneratedBase, {}, context);
												}
												else
												{
													//Call the placement destructor intrinsic function
													llvm::Value* typeInfoConstantAsIntPtr = context->helper->createTypeInfoGlobalValue(context, objectTypeInfo);
													context->helper->createIntrinsicCall(context, &CatLinkedIntrinsics::_jc_placementDestructType, {iterator, typeInfoConstantAsIntPtr}, "_jc_placementDestructType", true);
												}
											}
											else
											{
												//Need to implement destructors for owned/shared pointers
												assert(false);
											}
										});
								}

								context->helper->createIntrinsicCall(context, CatLinkedIntrinsics::_jc_freeMemory, {arrayPtr}, "_jc_freeMemory", true);
								builder->CreateStore(context->helper->createZeroInitialisedConstant(LLVMTypes::pointerType), builder->CreatePointerCast(parameters[0], LLVMTypes::pointerType->getPointerTo()));
								builder->CreateStore(context->helper->createZeroInitialisedConstant(LLVMTypes::intType), builder->CreatePointerCast(arraySizePtr, LLVMTypes::intType->getPointerTo()));
								return (llvm::Value*)(nullptr);
							}, LLVMTypes::voidType, context);
						return (llvm::Value*)(nullptr);
					},
					LLVMTypes::voidType, context);
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
