/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

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
	SectionMemoryAllocation* allocation = memoryManager->allocateCodeSection(size, alignment, this);
	codeSectionMemoryAllocations.push_back(allocation);
	return allocation->data;
}


uint8_t* jitcat::LLVM::LLVMExpressionMemoryAllocator::allocateDataSection(uintptr_t size, unsigned alignment, unsigned sectionID, llvm::StringRef sectionName, bool isReadOnly)
{
	SectionMemoryAllocation* allocation = memoryManager->allocateDataSection(size, alignment, this);
	dataSectionMemoryAllocations.push_back(allocation);
	return allocation->data;
}


bool jitcat::LLVM::LLVMExpressionMemoryAllocator::finalizeMemory(std::string* errMsg)
{
	for (auto& iter : codeSectionMemoryAllocations)
	{
		memoryManager->finalizeSection(iter);
	}
	for (auto& iter : dataSectionMemoryAllocations)
	{
		memoryManager->finalizeSection(iter);
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
		memoryManager->freeDataSection(iter);
	}
	dataSectionMemoryAllocations.clear();
}


llvm::sys::MemoryBlock* jitcat::LLVM::LLVMExpressionMemoryAllocator::getLastCodeBlock() const
{
	if (codeSectionMemoryAllocations.size() > 0)
	{
		return &codeSectionMemoryAllocations.back()->block->llvmMemoryBlock;
	}
	return nullptr;
}


LLVMMemoryManager::LLVMMemoryManager()
{
}


std::unique_ptr<LLVMExpressionMemoryAllocator> LLVMMemoryManager::createExpressionAllocator()
{
	return std::make_unique<LLVMExpressionMemoryAllocator>(this);
}


SectionMemoryAllocation* LLVMMemoryManager::allocateCodeSection(uintptr_t size, unsigned int alignment, LLVMExpressionMemoryAllocator* allocator)
{
	return alloccateFromSection(size, alignment, allocator, SectionPurpose::Code, codeSectionBlocks, freedCodeSectionAllocations);
}


SectionMemoryAllocation* jitcat::LLVM::LLVMMemoryManager::allocateDataSection(uintptr_t size, unsigned int alignment, LLVMExpressionMemoryAllocator* allocator)
{
	return alloccateFromSection(size, alignment, allocator, SectionPurpose::Data, dataSectionBlocks, freedDataSectionAllocations);
}


void jitcat::LLVM::LLVMMemoryManager::finalizeSection(SectionMemoryAllocation* allocation)
{
	allocation->status = SectionMemoryAllocationStatus::Finalized;
	updateBlockPermissions(allocation->block);
}


void LLVMMemoryManager::freeCodeSection(SectionMemoryAllocation* allocation)
{
	allocation->status = SectionMemoryAllocationStatus::Freed;
	auto iter = freedCodeSectionAllocations.find(allocation->alignment);
	if (iter != freedCodeSectionAllocations.end())
	{
		iter->second.insert({ allocation->size, allocation });
	}
	else
	{
		freedCodeSectionAllocations[allocation->alignment].insert({ allocation->size, allocation });
	}
	updateBlockPermissions(allocation->block);
}


void jitcat::LLVM::LLVMMemoryManager::freeDataSection(SectionMemoryAllocation* allocation)
{
	allocation->status = SectionMemoryAllocationStatus::Freed;
	auto iter = freedDataSectionAllocations.find(allocation->alignment);
	if (iter != freedDataSectionAllocations.end())
	{
		iter->second.insert({ allocation->size, allocation });
	}
	else
	{
		freedDataSectionAllocations[allocation->alignment].insert({ allocation->size, allocation });
	}
	updateBlockPermissions(allocation->block);
}


std::size_t jitcat::LLVM::LLVMMemoryManager::getAllocatedCodeMemory()
{
	std::size_t total = 0;
	for (auto& iter : codeSectionBlocks)
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
	for (auto& iter : codeSectionBlocks)
	{
		total += iter->blockSize;
	}
	return total;
}


void jitcat::LLVM::LLVMMemoryManager::updateBlockPermissions(SectionMemoryBlock* block)
{
	bool allFinalizedOrFree = true;
	bool anyFinalized = false;
	for (auto& iter : block->allocations)
	{
		allFinalizedOrFree &= (iter->status == SectionMemoryAllocationStatus::Finalized || iter->status == SectionMemoryAllocationStatus::Freed);
		anyFinalized |= iter->status == SectionMemoryAllocationStatus::Finalized;
	}
	unsigned int flags = llvm::sys::Memory::MF_READ;
	if (!allFinalizedOrFree)
	{
		flags |= llvm::sys::Memory::MF_WRITE;
	}
	if (anyFinalized && block->purpose == SectionPurpose::Code)
	{
		flags |= llvm::sys::Memory::MF_EXEC;
	}
	llvm::sys::Memory::protectMappedMemory(block->llvmMemoryBlock, flags);
}


SectionMemoryAllocation* jitcat::LLVM::LLVMMemoryManager::alloccateFromSection(uintptr_t size, unsigned int alignment, LLVMExpressionMemoryAllocator* allocator, SectionPurpose sectionPurpose,
																			   std::vector<std::unique_ptr<SectionMemoryBlock>>& blocks, 
																			   std::map<std::size_t, std::multimap<std::size_t, SectionMemoryAllocation*>>& freedAllocations)
{
	//First try to find a freed allocation that is the same, or close, size and alignment.
	for (auto& iter : freedAllocations)
	{
		if (iter.first >= alignment)
		{
			auto freeIter = iter.second.lower_bound(size);
			if (freeIter != iter.second.end() && (freeIter->first - size) < maxReAllocationSizeDifference)
			{
				SectionMemoryAllocation* allocation = freeIter->second;
				allocation->status = SectionMemoryAllocationStatus::Pending;
				updateBlockPermissions(allocation->block);
				memset(allocation->data, 0xCCCCCCCC, allocation->size);
				iter.second.erase(freeIter);
				assert(allocation->block->purpose == sectionPurpose);
				return allocation;
			}
		}
	}

	//Then try to find an existing code block with space left to fit this allocation.
	for (auto& iter : blocks)
	{
		//Check if  there is enough space left in the block to store the aligned object.
		if (iter->blockSize - iter->usedSpace >= size + alignment)
		{
			SectionMemoryAllocation* allocation = new SectionMemoryAllocation();
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
			allocation->alignment = alignment;
			allocation->status = SectionMemoryAllocationStatus::Pending;
			iter->allocations.emplace_back(allocation);
			updateBlockPermissions(iter.get());
			assert(allocation->block->purpose == sectionPurpose);
			return allocation;
		}
	}
	//Then finally create a new block if no space could be found.
	SectionMemoryBlock* block = new SectionMemoryBlock();
	std::error_code error;
	std::size_t blockSize = Tools::roundUp(size + alignment, 1 << 16);
	block->llvmMemoryBlock = llvm::sys::Memory::allocateMappedMemory(blockSize, allocator->getLastCodeBlock(), llvm::sys::Memory::MF_READ | llvm::sys::Memory::MF_WRITE, error);
	memset(block->llvmMemoryBlock.base(), 0xCCCCCCCC, block->llvmMemoryBlock.allocatedSize());
	//Waiting for a llvm patch to be accepted that will add allocatedSize: block->blockSize = block->llvmMemoryBlock.allocatedSize();
	block->blockSize = block->llvmMemoryBlock.allocatedSize();
	block->purpose = sectionPurpose;
	block->usedSpace = 0;
	block->block = (uint8_t*)block->llvmMemoryBlock.base();
	blocks.emplace_back(block);
	SectionMemoryAllocation* allocation = new SectionMemoryAllocation();
	allocation->block = block;
	uintptr_t alignedAllocationAddress = (uintptr_t(block->block + block->usedSpace) + alignment - 1) & ~((uintptr_t)alignment - 1);
	block->usedSpace = alignedAllocationAddress - (uintptr_t)block->block + size;
	allocation->data = (uint8_t*)alignedAllocationAddress;
	allocation->size = size;
	allocation->alignment = alignment;
	allocation->status = SectionMemoryAllocationStatus::Pending;
	block->allocations.emplace_back(allocation);
	updateBlockPermissions(block);
	return allocation;
}


const int jitcat::LLVM::LLVMMemoryManager::maxReAllocationSizeDifference = 16;