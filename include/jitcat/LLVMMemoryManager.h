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

	enum class SectionPurpose
	{
		Code,
		Data
	};
	enum class SectionMemoryAllocationStatus
	{
		Pending,
		Finalized,
		Freed
	};
	struct SectionMemoryBlock;
	struct SectionMemoryAllocation
	{
		SectionMemoryBlock* block;
		SectionMemoryAllocationStatus status;
		uint8_t* data;
		std::size_t size;
		std::size_t alignment;
	};
	struct SectionMemoryBlock
	{
		SectionPurpose purpose;
		uint8_t* block;
		std::size_t blockSize;
		std::size_t usedSpace;
		llvm::sys::MemoryBlock llvmMemoryBlock;
		std::vector<std::unique_ptr<SectionMemoryAllocation>> allocations;
	};

	//Manages all memory used for storing executable code and data sections by a single expression or source file.
	//This allocator attempts to reduce the size of memory allocations by compacting multiple expressions' code and data segment into a single page.
	class LLVMExpressionMemoryAllocator: public llvm::RTDyldMemoryManager
	{
	public:
		LLVMExpressionMemoryAllocator(LLVMMemoryManager* memoryManager);
		LLVMExpressionMemoryAllocator(const LLVMExpressionMemoryAllocator&) = delete;
		void operator=(const LLVMExpressionMemoryAllocator&) = delete;
		virtual ~LLVMExpressionMemoryAllocator() override;

		virtual void registerEHFrames(uint8_t *Addr, uint64_t LoadAddr, size_t Size) override final;
		virtual void deregisterEHFrames() override final;

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

		//Get the llvm memory block that was last allocated for code.
		llvm::sys::MemoryBlock* getLastCodeBlock() const;

	private:
		LLVMMemoryManager* memoryManager;
		//Data sections are just allocated directly and stored in this vector since we do not need to set page permissions (read only data is allocated as read/write).
		std::vector<SectionMemoryAllocation*> dataSectionMemoryAllocations;
		std::vector<SectionMemoryAllocation*> codeSectionMemoryAllocations;

		std::vector<unsigned char*> registeredEHTables;

		uintptr_t imageBase;
	};


	class LLVMMemoryManager
	{
	public:
		LLVMMemoryManager();
		std::unique_ptr<LLVMExpressionMemoryAllocator> createExpressionAllocator();

		//Allocate a read/write code section with its status set to pending.
		//When the caller is done writing code into the code section, it should call markCodeSectionReadyForFinalization.
		//When the caller then wants to execute the code, it should call finalizeCodeSection.
		SectionMemoryAllocation* allocateCodeSection(uintptr_t size, unsigned int alignment, LLVMExpressionMemoryAllocator* allocator);
		SectionMemoryAllocation* allocateDataSection(uintptr_t size, unsigned int alignment, LLVMExpressionMemoryAllocator* allocator);

		//If a code section is passed, will set execution permission for the code section's memory and the memory for any other code sections that share pages with this allocation.
		//If another code section shares a page with this allocation and that other code section is not yet marked ready for finalization, an error is generated.
		//If a data section is passed, the data page will be set to read only once there are no more pending data sections.
		void finalizeSection(SectionMemoryAllocation* allocation);

		void freeCodeSection(SectionMemoryAllocation* allocation);
		void freeDataSection(SectionMemoryAllocation* allocation);

		//Get the total size of all code sections
		std::size_t getAllocatedCodeMemory();
		//Get the total size of all reserved memory blocks. This will always be greater or equal to the allocated code memory.
		std::size_t getReservedCodeMemory();

	private:
		static void updateBlockPermissions(SectionMemoryBlock* block);

		static SectionMemoryAllocation* alloccateFromSection(uintptr_t size, unsigned int alignment, LLVMExpressionMemoryAllocator* allocator, SectionPurpose sectionPurpose,
															 std::vector<std::unique_ptr<SectionMemoryBlock>>& blocks, 
															 std::map<std::size_t, std::multimap<std::size_t, SectionMemoryAllocation*>>& freedAllocations);

	private:
		std::vector<std::unique_ptr<SectionMemoryBlock>> codeSectionBlocks;
		std::vector<std::unique_ptr<SectionMemoryBlock>> dataSectionBlocks;

		//A map of freed code section allocations that allows for re-use.
		std::map<std::size_t, std::multimap<std::size_t, SectionMemoryAllocation*>> freedCodeSectionAllocations;
		//A map of freed data section allocations that allows for re-use.
		std::map<std::size_t, std::multimap<std::size_t, SectionMemoryAllocation*>> freedDataSectionAllocations;

		//The maximum difference in size between an allocation request and a re-used SectionMemoryAllocation.
		static const int maxReAllocationSizeDifference;
	};

}