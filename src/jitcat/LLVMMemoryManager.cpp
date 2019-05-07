#include "jitcat/LLVMMemoryManager.h"
#include "jitcat/Tools.h"

#include <llvm/Support/Memory.h>

using namespace jitcat;
using namespace jitcat::LLVM;


LLVMExpressionMemoryAllocator::LLVMExpressionMemoryAllocator(LLVMMemoryManager* memoryManager):
	memoryManager(memoryManager)
{
}


LLVMExpressionMemoryAllocator::~LLVMExpressionMemoryAllocator()
{
	freeMemory();
}


uint8_t* jitcat::LLVM::LLVMExpressionMemoryAllocator::allocateCodeSection(uintptr_t size, unsigned alignment, unsigned sectionID, llvm::StringRef sectionName)
{
	CodeSectionMemoryAllocation* allocation = memoryManager->allocateCodeSection(size, alignment);
	codeSectionMemoryAllocations.push_back(allocation);
	return allocation->data;
}


uint8_t* jitcat::LLVM::LLVMExpressionMemoryAllocator::allocateDataSection(uintptr_t size, unsigned alignment, unsigned sectionID, llvm::StringRef sectionName, bool isReadOnly)
{
	LLVMExpressionMemoryAllocator::DataSectionAllocation allocation;
	allocation.alignment = alignment;
	allocation.size = size;
	unsigned int minMallocAlignment = alignof(std::max_align_t);
	if (alignment <= minMallocAlignment)
	{
		allocation.data = (uint8_t*)malloc(size);
		allocation.alignedData = allocation.data;
	}
	else
	{
		allocation.size = (alignment - minMallocAlignment) + size;
		allocation.data = (uint8_t*)malloc(allocation.size);
		allocation.alignedData = (uint8_t*)(((uintptr_t)allocation.data + alignment - 1) & ~((uintptr_t)alignment - 1));
	}
	dataSectionMemoryAllocations.push_back(allocation);
	return allocation.alignedData;
}


bool jitcat::LLVM::LLVMExpressionMemoryAllocator::finalizeMemory(std::string* errMsg)
{
	for (auto& iter : codeSectionMemoryAllocations)
	{
		//memoryManager->markCodeSectionReadyForFinalization(iter);
		memoryManager->finalizeCodeSection(iter);
	}
	return false;
}


void jitcat::LLVM::LLVMExpressionMemoryAllocator::freeMemory()
{
	for (auto& iter : codeSectionMemoryAllocations)
	{
		memoryManager->freeCodeSection(iter);
	}
	codeSectionMemoryAllocations.clear();
	for (auto& iter : dataSectionMemoryAllocations)
	{
		free(iter.data);
	}
	dataSectionMemoryAllocations.clear();
}


/*void jitcat::LLVM::LLVMExpressionMemoryAllocator::finalizeExpressionMemory()
{
	for (auto& iter : codeSectionMemoryAllocations)
	{
		memoryManager->finalizeCodeSection(iter);
	}
}*/


LLVMMemoryManager::LLVMMemoryManager():
	maxReAllocationSizeDifference(16)
{
}


std::unique_ptr<LLVMExpressionMemoryAllocator> LLVMMemoryManager::createExpressionAllocator()
{
	return std::make_unique<LLVMExpressionMemoryAllocator>(this);
}


CodeSectionMemoryAllocation* LLVMMemoryManager::allocateCodeSection(uintptr_t size, unsigned int alignment)
{
	//First try to find a freed allocation that is the same, or close, size and alignment.
	auto freeIter = freedAllocations.lower_bound(size + alignment);
	if (freeIter != freedAllocations.end() && (freeIter->first - (size + alignment)) < maxReAllocationSizeDifference)
	{
		CodeSectionMemoryAllocation* allocation = freeIter->second;
		allocation->status = CodeSectionMemoryAllocationStatus::Pending;
		updateBlockPermissions(allocation->block);
		freedAllocations.erase(freeIter);
		return allocation;
	}
	//Then try to find an existing code block with space left to fit this allocation.
	for (auto& iter : blocks)
	{
		//Check if  there is enough space left in the block to store the aligned object.
		if (iter->blockSize - iter->usedSpace >= size + alignment)
		{
			CodeSectionMemoryAllocation* allocation = new CodeSectionMemoryAllocation();
			allocation->block = iter.get();
			uintptr_t alignedAllocationAddress = (uintptr_t(iter->block + iter->usedSpace) + alignment - 1) & ~((uintptr_t)alignment - 1);
			uintptr_t difference = alignedAllocationAddress - uintptr_t(iter->block + iter->usedSpace);
			//The previously allocated memory block can now grow to contain the difference caused by alignment.
			if (iter->allocations.size() > 0 && difference > 0)
			{
				iter->allocations.back()->size += difference;
			}
			iter->usedSpace = alignedAllocationAddress - (uintptr_t)iter->block + size;
			allocation->data = (uint8_t*)alignedAllocationAddress;
			allocation->size = size;
			allocation->status = CodeSectionMemoryAllocationStatus::Pending;
			iter->allocations.emplace_back(allocation);
			updateBlockPermissions(iter.get());
			return allocation;
		}
	}
	//Then finally create a new block if no space could be found.
	CodeSectionMemoryBlock* block = new CodeSectionMemoryBlock();
	std::error_code error;
	std::size_t blockSize = Tools::roundUp(size + alignment, 1 << 16);
	block->llvmMemoryBlock = llvm::sys::Memory::allocateMappedMemory(blockSize, nullptr, llvm::sys::Memory::MF_READ | llvm::sys::Memory::MF_WRITE, error);
	//Waiting for a llvm patch to be accepted that will add allocatedSize: block->blockSize = block->llvmMemoryBlock.allocatedSize();
	block->blockSize = block->llvmMemoryBlock.size();
	block->usedSpace = 0;
	block->block = (uint8_t*)block->llvmMemoryBlock.base();
	blocks.emplace_back(block);
	CodeSectionMemoryAllocation* allocation = new CodeSectionMemoryAllocation();
	allocation->block = block;
	uintptr_t alignedAllocationAddress = (uintptr_t(block->block + block->usedSpace) + alignment - 1) & ~((uintptr_t)alignment - 1);
	block->usedSpace = alignedAllocationAddress - (uintptr_t)block->block + size;
	allocation->data = (uint8_t*)alignedAllocationAddress;
	allocation->size = size;
	allocation->status = CodeSectionMemoryAllocationStatus::Pending;
	block->allocations.emplace_back(allocation);
	updateBlockPermissions(block);
	return allocation;
}


/*void jitcat::LLVM::LLVMMemoryManager::markCodeSectionReadyForFinalization(CodeSectionMemoryAllocation* allocation)
{
	assert(allocation->status == CodeSectionMemoryAllocationStatus::Pending
		   || allocation->status == CodeSectionMemoryAllocationStatus::ReadyForFinalization);
	allocation->status = CodeSectionMemoryAllocationStatus::ReadyForFinalization;
}*/


void jitcat::LLVM::LLVMMemoryManager::finalizeCodeSection(CodeSectionMemoryAllocation* allocation)
{
	allocation->status = CodeSectionMemoryAllocationStatus::Finalized;
	updateBlockPermissions(allocation->block);
}


void LLVMMemoryManager::freeCodeSection(CodeSectionMemoryAllocation* allocation)
{
	allocation->status = CodeSectionMemoryAllocationStatus::Freed;
	freedAllocations.insert({ allocation->size, allocation });
	updateBlockPermissions(allocation->block);
}


std::size_t jitcat::LLVM::LLVMMemoryManager::getAllocatedCodeMemory()
{
	std::size_t total = 0;
	for (auto& iter : blocks)
	{
		for (auto& iter2 : iter->allocations)
		{
			total += iter2->size;
		}
	}
	return total;
}


std::size_t jitcat::LLVM::LLVMMemoryManager::getReservedCodeMemory()
{
	std::size_t total = 0;
	for (auto& iter : blocks)
	{
		total += iter->blockSize;
	}
	return total;
}


void jitcat::LLVM::LLVMMemoryManager::updateBlockPermissions(CodeSectionMemoryBlock* block)
{
	bool allFinalizedOrFree = true;
	bool anyFinalized = false;
	for (auto& iter : block->allocations)
	{
		allFinalizedOrFree &= (iter->status == CodeSectionMemoryAllocationStatus::Finalized || iter->status == CodeSectionMemoryAllocationStatus::Freed);
		anyFinalized |= iter->status == CodeSectionMemoryAllocationStatus::Finalized;
	}
	unsigned int flags = llvm::sys::Memory::MF_READ;
	if (!allFinalizedOrFree)
	{
		flags |= llvm::sys::Memory::MF_WRITE;
	}
	if (anyFinalized)
	{
		flags |= llvm::sys::Memory::MF_EXEC;
	}
	llvm::sys::Memory::protectMappedMemory(block->llvmMemoryBlock, flags);
}
