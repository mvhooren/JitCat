/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <llvm/ExecutionEngine/Orc/Core.h>

namespace jitcat::LLVM
{

	///A reexports generator. It will re-intern the symbol names 
	///with the execution sessions of the source and target libs.
	///
	///Interning means the symbol names are converted to a pointer within a string pool.
	///Symbols are then looked up by their pointer and not by their actual name.
	///Each execution session has its own string pool and therefore symbols 
	///cannot be resolved accross JITDyLibs created by different execution session.
	///
	///LLVMReinternReexportsGenerator will re-intern the symbol names that are being searched 
	///by a source JITDylib to the destination dylib.
	class LLVMReinterningReexportsGenerator : public llvm::orc::JITDylib::DefinitionGenerator 
	{
	public:

		LLVMReinterningReexportsGenerator(llvm::orc::JITDylib& sourceLib);

		llvm::Error tryToGenerate(llvm::orc::LookupKind kind, llvm::orc::JITDylib &targetLib,
								  llvm::orc::JITDylibLookupFlags libLookupFlags,
								  const llvm::orc::SymbolLookupSet& lookupSet) override final;
	private:
		llvm::orc::JITDylib &sourceLib;
	};

}