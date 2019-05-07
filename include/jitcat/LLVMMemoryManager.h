/*
  This file is part of the JitCat library.

  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <llvm/ExecutionEngine/RTDyldMemoryManager.h>
#include <llvm/Support/Memory.h>
#include <memory>
#include <vector>


namespace jitcat::LLVM
{
	class LLVMMemoryManager;

	enum CodeSectionMemoryAllocationStatus
	{
		Pending,
		//ReadyForFinalization,
		Finalized,
		Freed
	};
	struct CodeSectionMemoryBlock;
	struct CodeSectionMemoryAllocation
	{
		CodeSectionMemoryBlock* block;
		CodeSectionMemoryAllocationStatus status;
		uint8_t* data;
		std::size_t size;
	};
	struct CodeSectionMemoryBlock
	{
		uint8_t* block;
		std::size_t blockSize;
		std::size_t usedSpace;
		llvm::sys::MemoryBlock llvmMemoryBlock;
		std::vector<std::unique_ptr<CodeSectionMemoryAllocation>> allocations;
	};

	//Manages all memory used for storing executable code and data sections by a single expression or source file.
	//This allocator attempts to reduce the size of memory allocations by compacting multiple expressions' code and data segment into a single page.
	class LLVMExpressionMemoryAllocator: public llvm::RTDyldMemoryManager
	{
		struct DataSectionAllocation
		{
			uint8_t* data;
			uint8_t* alignedData;
			std::size_t size;
			unsigned int alignment;
		};
	public:
		LLVMExpressionMemoryAllocator(LLVMMemoryManager* memoryManager);
		LLVMExpressionMemoryAllocator(const LLVMExpressionMemoryAllocator&) = delete;
		void operator=(const LLVMExpressionMemoryAllocator&) = delete;
		virtual ~LLVMExpressionMemoryAllocator() override;

		//Allocates a memory block of (at least) the given size suitable for
		//executable code.
		//
		//The value of \p Alignment must be a power of two.  If \p Alignment is zero
		//a default alignment of 16 will be used.
		virtual uint8_t* allocateCodeSection(uintptr_t size, unsigned alignment, unsigned sectionID, llvm::StringRef sectionName) override final;

		//Allocates a memory block of (at least) the given size suitable for
		//executable code. This allocator ignores the isReadOnly flag.
		//
		//The value of \p Alignment must be a power of two.  If \p Alignment is zero
		//a default alignment of 16 will be used.
		virtual uint8_t* allocateDataSection(uintptr_t size, unsigned alignment, unsigned sectionID, llvm::StringRef sectionName, bool isReadOnly) override final;

		//Update section-specific memory permissions and other attributes.
		//
		//This method is called when object loading is complete and section page
		//permissions can be applied.  It is up to the memory manager implementation
		//to decide whether or not to act on this method.  The memory manager will
		//typically allocate all sections as read-write and then apply specific
		//permissions when this method is called.  Code sections cannot be executed
		//until this function has been called.  In addition, any cache coherency
		//operations needed to reliably use the memory are also performed.
		//
		//LLVMExpressionMemoryAllocator does not actually set permissions through this call, but intead
		//marks the allocation as ready for finalization. Execution permissions are set by calling finalizeExpressionMemory() just 
		//before calling an expression for the first time.
		//
		//returns true if an error occurred, false otherwise.
		virtual bool finalizeMemory(std::string* errMsg = nullptr) override final;

		//Free all memory allocated by this allocator.
		//Multiple allocators may share pages of executable memory. In that case, the freed memory is not actually available
		//for re-use untill all allocators that share the page have freed their memory.
		void freeMemory();

		//This must be called by the expression before any code is executed.
		//It will change memory permissions to executable and read-only for all the pages that this allocator allocated memory in.
		//Any other LLVMExpressionMemoryAllocators that shared pages with this allocator will also have their code section finalized.
		//If another LLVMExpressionMemoryAllocator that shared a page with this allocator has not called finalizeMemory yet, an error will be generated.
		//void finalizeExpressionMemory();


	private:
		LLVMMemoryManager* memoryManager;
		//Data sections are just allocated directly and stored in this vector since we do not need to set page permissions (read only data is allocated as read/write).
		std::vector<DataSectionAllocation> dataSectionMemoryAllocations;
		std::vector<CodeSectionMemoryAllocation*> codeSectionMemoryAllocations;
	};


	class LLVMMemoryManager
	{
	public:
		LLVMMemoryManager();
		std::unique_ptr<LLVMExpressionMemoryAllocator> createExpressionAllocator();

		//Allocate a read/write code section with its status set to pending.
		//When the caller is done writing code into the code section, it should call markCodeSectionReadyForFinalization.
		//When the caller then wants to execute the code, it should call finalizeCodeSection.
		CodeSectionMemoryAllocation* allocateCodeSection(uintptr_t size, unsigned int alignment);

		//Marks the code section as ready for finalization.
		//void markCodeSectionReadyForFinalization(CodeSectionMemoryAllocation* allocation);

		//Will set execution permission for the code section's memory and the memory for any other code sections that share pages with this allocation.
		//If another code section shares a page with this allocation and that other code section is not yet marked ready for finalization, an error is generated.
		void finalizeCodeSection(CodeSectionMemoryAllocation* allocation);
		void freeCodeSection(CodeSectionMemoryAllocation* allocation);

		//Get the total size of all code sections
		std::size_t getAllocatedCodeMemory();
		//Get the total size of all reserved memory blocks. This will always be greater or equal to the allocated code memory.
		std::size_t getReservedCodeMemory();

	private:
		void updateBlockPermissions(CodeSectionMemoryBlock* block);

	private:
		std::vector<std::unique_ptr<CodeSectionMemoryBlock>> blocks;
		//A map of freed allocations that allows for re-use.
		std::multimap<std::size_t, CodeSectionMemoryAllocation*> freedAllocations;
		//The maximum difference in size between an allocation request and a re-used CodeSectionMemoryAllocation.
		const int maxReAllocationSizeDifference;
	};

}